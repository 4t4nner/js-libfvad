type detectorOptions = {
        frameMs?: number = 30,
        sampleRate?: number = 8000,
        mode?: number = 3,
        silenceMaxMs?: number = 400,
}

/**
 * 
 * @param {string} fileName 
 * @throws {string} error
 */
export function detectSilence(fileName: string, opts?: detectorOptions): {
    silence: boolean,
    outFile?: string,
    timeSilence?: number,
}