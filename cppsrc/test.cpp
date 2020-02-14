#include "test.h"

#include <sndfile.h>
#include <fvad.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <boost/format.hpp>
#include <napi.h>
#include <iostream>

#define DEBUG=1;

using namespace std;
using namespace test;

struct BufFrame
{
    size_t len;
    int frame_ms;
    int cnt;
    int silence_max_ms;
};

void coutError(string e){
    cout << "error: " << e << endl;
}

std::tuple<SNDFILE *,string> openOutTmpFile(int samplerate = 8000){

    std::string tmpFileName = std::tmpnam(nullptr);
    SF_INFO s = (SF_INFO){
                .samplerate = samplerate,
                .channels = 1,
                .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE
            };

    SNDFILE *out_sf = sf_open(const_cast<char*>(tmpFileName.c_str()), SFM_WRITE, &s);
    if(!out_sf){
        string s = (boost::format("Cannot open output file '%s': %s\n") % tmpFileName % std::string(sf_strerror(NULL))).str();
        // throw boost::format("Cannot open output file '%s': %s\n") % tmpFileName % std::string(sf_strerror(NULL));
        throw s;
    }

    return std::make_tuple(out_sf,tmpFileName);
}

static detectorRet process_sf(
    SNDFILE *infile,
    Fvad *vad,
    // SNDFILE *outfiles[2],
    struct BufFrame buf_info)
{
    size_t framelen = buf_info.len;
    bool success = false;
    double *buf0 = NULL;
    int16_t *buf1 = NULL;
    int vadres, prev = -1;
    long frames[2] = {0, 0};
    long segments[2] = {0, 0};
    int cntUnvoiced = 0;
    int silenceFrameMax = buf_info.silence_max_ms/buf_info.frame_ms;
    double *voiceBuf = (double *) malloc(buf_info.cnt * framelen * (sizeof buf0));
    int voiceBufLength = 0;
    detectorRet ret = {.error="",.outFile="",.silence=false};
    uint64_t time = 0;

    if (framelen > SIZE_MAX / sizeof(double) || !(buf0 = (double *)malloc(framelen * sizeof *buf0)) || !(buf1 = (int16_t *)malloc(framelen * sizeof *buf1)))
    {
        ret.error = "failed to allocate buffers\n";
        coutError(ret.error);
        goto end;
    }

    while (sf_read_double(infile, buf0, framelen) == (sf_count_t)framelen)
    {

        // Convert the read samples to int16
        for (size_t i = 0; i < framelen; i++)
            buf1[i] = buf0[i] * INT16_MAX;

        vadres = fvad_process(vad, buf1, framelen);
        if (vadres < 0) {
            coutError(ret.error);
            ret.error = "VAD processing failed\n";
            goto end;
        }

        // if (listfile) {
        //     fprintf(listfile, "%d\n", vadres);
        // }

        vadres = !!vadres; // make sure it is 0 or 1

      
        if(vadres){
            memcpy(&voiceBuf[voiceBufLength],buf0,framelen * sizeof buf0);
            voiceBufLength += framelen;
            
            cntUnvoiced = 0;
        } else {
            if(cntUnvoiced++ < 4){
                memcpy(&voiceBuf[voiceBufLength],buf0,framelen * sizeof buf0);
                voiceBufLength += framelen;
                // sf_write_double(outfiles[1], buf0, framelen);
            }
            if(cntUnvoiced >= silenceFrameMax){
                ret.time_silence = (double)time/1000;
                ret.silence = true;
            }
        }
        
        frames[vadres]++;
        if (prev != vadres) segments[vadres]++;
        prev = vadres;
        time += buf_info.frame_ms;
    }

    // printf("voice detected in %ld of %ld frames (%.2f%%)\n",
    //     frames[1], frames[0] + frames[1],
    //     frames[0] + frames[1] ?
    //         100.0 * ((double)frames[1] / (frames[0] + frames[1])) : 0.0);
    // printf("%ld voice segments, average length %.2f frames\n",
    //     segments[1], segments[1] ? (double)frames[1] / segments[1] : 0.0);
    // printf("%ld non-voice segments, average length %.2f frames\n",
    //     segments[0], segments[0] ? (double)frames[0] / segments[0] : 0.0);

    if(ret.silence){
        SNDFILE *out_sf;
        std::tie(out_sf,ret.outFile) = openOutTmpFile();
        sf_write_double(out_sf, voiceBuf, voiceBufLength);
    }

end:
    if (buf0) free(buf0);
    if (buf1) free(buf1);
    if (voiceBuf) free(voiceBuf);
    return ret;
}

static bool parse_int(int *dest, const char *s, int min, int max)
{
    char *endp;
    long val;

    errno = 0;
    val = strtol(s, &endp, 10);
    if (!errno && !*endp && val >= min && val <= max) {
        *dest = val;
        return true;
    } else {
        return false;
    }
}


detectorRet detector(const char *fileName,DetectorParams dp){
    const char *in_fname, *out_fname[2] = {NULL, NULL};
    SNDFILE *in_sf = NULL, *out_sf[2] = {NULL, NULL};
    SF_INFO in_info = {0}, out_info[2];
    FILE *list_file = NULL;
    int frame_ms = dp.frame_ms;
    Fvad *vad = NULL;
    std::string error;
    detectorRet ret;
    size_t framelen = 0;
    BufFrame buf_info;

    /*
     * create fvad instance
     */
    vad = fvad_new();
    if (!vad) {
        ret.error = "!vad - out of memory\n";
        coutError(ret.error);
        goto fail;
    }

    fvad_set_mode(vad, dp.mode);
    out_fname[1];

    in_info.format   = SF_FORMAT_RAW | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
    in_info.channels = 1;
    in_info.samplerate = dp.sampleRate;


    in_sf = sf_open(fileName, SFM_READ, &in_info);
    if (!in_sf) {
        const char * t = sf_strerror(NULL);
        string t1 = string(t);
        ret.error = (boost::format("Cannot open input file '%s': %s\n") % (in_fname, sf_strerror(NULL))).str();

        coutError(ret.error);
        coutError(t);
        goto fail;
    }

    if (in_info.channels != 1) {
        ret.error = (boost::format("only single-channel wav files supported; input file has %d channels\n") % (in_info.channels)).str();

        goto fail;
    }
    if (fvad_set_sample_rate(vad, in_info.samplerate) < 0) {
        ret.error = (boost::format("invalid sample rate: %d Hz\n") % (in_info.samplerate)).str();

        coutError(ret.error);
        goto fail;
    }


    framelen = (size_t)in_info.samplerate / 1000 * frame_ms;
    buf_info = {
        .len = framelen,
        .frame_ms = frame_ms,
        .cnt = in_info.frames,
        .silence_max_ms = dp.silence_max_ms,
    };
    /*
     * run main loop
     */
    ret = process_sf(
        in_sf,
        vad,
        // out_sf,
        buf_info);

    if (ret.error != "")
        goto fail;

    /*
     * cleanup
     */  
success:
    goto end;

fail:
    goto end;

end:
    if (in_sf) sf_close(in_sf);
    for (int i = 0; i < 2; i++)
        if (out_sf[i]) sf_close(out_sf[i]);
    if (list_file) fclose(list_file);
    if (vad) fvad_free(vad);

    return ret;
}

Napi::Object test::detectorWrapper(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Object ret = Napi::Object::New(env);
    
    if(info.Length() != 2){
        Napi::TypeError::New(env, "should be 2 args - string fileName and options Object").ThrowAsJavaScriptException();
        return ret;
    }
    int mode,mode1,frameMs,sampleRate,sampleRate1;
    bool t;
    try{
    
        string fileName = info[0].As<Napi::String>().Utf8Value();
        Napi::Object options = info[1].ToObject();
        
        DetectorParams dp = {
            .mode = options.Get("mode").ToNumber().Int32Value(),
            .sampleRate = options.Get("sampleRate").ToNumber().Int32Value(),
            .frame_ms = options.Get("frameMs").ToNumber().Int32Value(),
            .silence_max_ms = options.Get("silenceMaxMs").ToNumber().Int32Value(),
        };

        detectorRet dr = detector(fileName.c_str(),dp);

        ret.Set("error",dr.error);
        ret.Set("outFile",dr.outFile);
        ret.Set("timeSilence",dr.time_silence);
        ret.Set("silence",dr.silence);
    } catch (exception e){
        Napi::TypeError::New(env, "should be 2 args - string fileName and options Object").ThrowAsJavaScriptException();
    }

    return ret;
    
}

Napi::Object test::Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        "detectSilence", Napi::Function::New(env, test::detectorWrapper)
    );

    return exports;
}