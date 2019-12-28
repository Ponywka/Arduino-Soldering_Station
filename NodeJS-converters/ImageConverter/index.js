// Load libraries
let bmp = require('bmp-js')
let fs = require('fs')

// Create a image buffer
let bmpBuffer = fs.readFileSync(`./${process.argv[2]}.bmp`)
let bmpData = bmp.decode(bmpBuffer)
let bitData = []
Object.keys(bmpData.data).forEach(el => {
    if (el % 4 == 1) {
        bitData.push(bmpData.data[el] ? true : false)
        /*
            //Debug:
            process.stdout.write(bmpData.data[el]? "#" : " ")
            if(((el - 1) % (bmpData.width * 4)) == ((bmpData.width - 1) * 4)) process.stdout.write("| <- EndLine\n")
        */
    }
})

let bytesPerX = Math.ceil(bmpData.width / 8)
let byteData = []
Object.keys(bitData).forEach(byteNum => {
    //process.stdout.write(bitData[byteNum]? "#" : " ")
    //if(!(byteNum % bmpData.width)) process.stdout.write("\n") 
    let newX = byteNum % bmpData.width
    let newY = ((byteNum - newX) / bmpData.width) % bmpData.height
    let byteID = newY * bytesPerX + Math.floor(newX / 8)
    if (!byteData[byteID]) byteData[byteID] = 0
    byteData[byteID] += Math.pow(2, 7 - newX % 8) * bitData[byteNum]
})

// Convert array to HEX array C++
const getHexString = (int) => {
    let out = int.toString(16)
    if (out.length == 1) {
        out = `0${out}`
    }
    return `0x${out}`
}

let outputHEX = ""
for (let byte of byteData) outputHEX += `${getHexString(byte)}, `
// Delete ", " from end 
outputHEX = outputHEX.substr(0, outputHEX.length - 2)

console.log(`const uint8_t PROGMEM image_${process.argv[2]}[] = { ${outputHEX} };`);