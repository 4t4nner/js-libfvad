const detector_addon = require('./build/Release/detector_addon.node');

function detectSilence(fileName){
    const {silence,error,outFile} = detector_addon.detectSilence(fileName)

    if(error){
        throw error
    }

    return {silence,outFile}
}

module.exports = detector_addon;