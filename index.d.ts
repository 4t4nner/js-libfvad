/**
 * 
 * @param {string} fileName 
 * @throws {string} error
 */
export function detectSilence(fileName: string): {
    silence: boolean,
    outFile: string,
}