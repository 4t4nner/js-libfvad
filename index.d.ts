
export default interface Detector {
    detectSilence(fileName:string) : {
        silence: boolean,
        error: string,
        outFile: string,
    }
}