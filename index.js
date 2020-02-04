const testAddon = require('./build/Debug/testaddon.node');
const path = require('path');

try {
    const testFilePath = path.join(__dirname,"test.raw");
    console.log(testAddon.detectSilence(testFilePath));
} catch (e) {
    console.error("error:\n",e)
}

module.exports = testAddon;