declare module "@4t4nner/silence-detector" {
    export function detectSilence(fileName: string): {
        silence: boolean,
        error: string,
        outFile: string,
    }
}