// Load libraries
let bmp = require('bmp-js')
let fs = require('fs')

// Create a image buffer
let bmpBuffer = fs.readFileSync(`./fonts/${process.argv[2]}/font.bmp`)
let symbols = require(`./fonts/${process.argv[2]}/symbols.json`)
let sizes = require(`./fonts/${process.argv[2]}/sizes.json`)
let bmpData = bmp.decode(bmpBuffer)
let bitData = []
Object.keys(bmpData.data).forEach(el => {
    if (el % 4 == 1) {
        bitData.push(bmpData.data[el] ? true : false)
    }
})

// Create a output array
let bytesPerX = Math.ceil(sizes[0] / 8)
let outputdata = [sizes[0], sizes[1], bytesPerX, symbols.length]
for (let i = 0; i < symbols.length; i++) outputdata.push(symbols[i])

// Array for calculate pixels
let charFullSize = [
    sizes[0] + sizes[3] + sizes[5],
    sizes[1] + sizes[2] + sizes[4],
]
charFullSize.push(charFullSize[0] * charFullSize[1])

Object.keys(symbols).forEach(symbol => {
    symbol = parseInt(symbol)
    let charData = bitData.slice(symbol * charFullSize[2], (symbol + 1) * charFullSize[2])

    let byteData = []
    Object.keys(charData).forEach(byteNum => {
        let x = byteNum % charFullSize[0]
        let y = ((byteNum - x) / charFullSize[0]) % charFullSize[1]
        let newX = x - sizes[5]
        let newY = y - sizes[2]
        if (newX >= 0 && newX < sizes[0] && newY >= 0 && newY < sizes[1]) {
            let byteID = newY * bytesPerX + Math.floor(newX / 8)
            if (!byteData[byteID]) byteData[byteID] = 0
            byteData[byteID] += Math.pow(2, 7 - newX % 8) * charData[byteNum]
        }
    })
    for (let i = 0; i < byteData.length; i++) outputdata.push(byteData[i])
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
for (let byte of outputdata) outputHEX += `${getHexString(byte)}, `
// Delete ", " from end 
outputHEX = outputHEX.substr(0, outputHEX.length - 2)

console.log(`const uint8_t PROGMEM font_${process.argv[2]}[] = { ${outputHEX} };`);