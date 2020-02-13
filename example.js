const path = require('path');
const {detectSilence} = require('./index')

console.log(detectSilence(path.join(__dirname,'test.raw')))