type detectorOptions = {
        frameMs: number | 30,
        sampleRate: number | 8000,
        mode: number | 3,
}

/**
 * 
 * @param {string} fileName 
 * @throws {string} error
 */
export function detectSilence(fileName: string): {
    silence: boolean,
    outFile?: string,
}