const detector_addon = require('./build/Debug/detector_addon.node');
// const detector_addon = require('./build/Release/detector_addon.node');

function detectSilence(fileName,options = {}){
    let newOptions = {
        frameMs: 30,
        sampleRate: 8000,
        mode: 3,
        ...options
    }
    const {silence,error,outFile} = detector_addon.detectSilence(
        fileName,
        newOptions
    )

    if(error){
        throw error
    }

    return {silence,outFile}
}

module.exports = {detectSilence};