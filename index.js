const detector_addon = require('bindings')('detector_addon');

function detectSilence(fileName,options = {}){
    let newOptions = {
        frameMs: 30,
        sampleRate: 8000,
        mode: 3,
        silenceMaxMs: 400,
        ...options
    }
    const {silence,error,outFile,timeSilence} = detector_addon.detectSilence(
        fileName,
        newOptions
    )

    if(error){
        throw error
    }

    return {silence,outFile,timeSilence}
}

module.exports = {detectSilence};