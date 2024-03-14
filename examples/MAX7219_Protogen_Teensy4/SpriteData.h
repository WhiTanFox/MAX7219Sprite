#ifndef SPRITEDATA_H
#define SPRITEDATA_H

const uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

const uint8_t startup[8] = {
  0xAA,
  0x55,
  0xAA,
  0x55,
  0xAA,
  0x55,
  0xAA,
  0x55,
};

const uint8_t boringEye[16] = {
  0b00000001,
  0b00000011,
  0b00000111,
  0b00000111,
  0b00000111,
  0b00000011,
  0b00000001,
  0b00000000,

  0b11000000,
  0b11100000,
  0b11110000,
  0b11110000,
  0b11110000,
  0b11100000,
  0b11000000,
  0b00000000,
};

const uint8_t boopEyeL[16] = {
  0b00000000,
  0b00000011,
  0b00001111,
  0b00111111,
  0b01111000,
  0b00011111,
  0b00000011,
  0b00000000,

  0b11110000,
  0b11100000,
  0b11000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b11000000,
  0b00000000,
};

const uint8_t boopEyeR[16] = {
  reverse(boopEyeL[8]),
  reverse(boopEyeL[9]),
  reverse(boopEyeL[10]),
  reverse(boopEyeL[11]),
  reverse(boopEyeL[12]),
  reverse(boopEyeL[13]),
  reverse(boopEyeL[14]),
  reverse(boopEyeL[15]),
  
  reverse(boopEyeL[0]),
  reverse(boopEyeL[1]),
  reverse(boopEyeL[2]),
  reverse(boopEyeL[3]),
  reverse(boopEyeL[4]),
  reverse(boopEyeL[5]),
  reverse(boopEyeL[6]),
  reverse(boopEyeL[7]),
};

const uint8_t eyeL[16] = {
  0b00000000,
  0b00000111,
  0b00011111,
  0b01111111,
  0b11100000,
  0b10000000,
  0b00000000,
  0b00000000,

  0b11110000,
  0b11111100,
  0b11111110,
  0b11111111,
  0b00001111,
  0b00000110,
  0b00000000,
  0b00000000,
};

const uint8_t eyeR[16] = {
  reverse(eyeL[8]),
  reverse(eyeL[9]),
  reverse(eyeL[10]),
  reverse(eyeL[11]),
  reverse(eyeL[12]),
  reverse(eyeL[13]),
  reverse(eyeL[14]),
  reverse(eyeL[15]),
  
  reverse(eyeL[0]),
  reverse(eyeL[1]),
  reverse(eyeL[2]),
  reverse(eyeL[3]),
  reverse(eyeL[4]),
  reverse(eyeL[5]),
  reverse(eyeL[6]),
  reverse(eyeL[7]),
};

const uint8_t blinkR[16] = {
  0b00000111,
  0b00011111,
  0b00111111,
  0b00111111,
  0b00111111,
  0b00011111,
  0b00011111,
  0b00001111,

  0b11111100,
  0b11111010,
  0b11111010,
  0b11111010,
  0b11110100,
  0b11110100,
  0b11111000,
  0b10000000,
};

const uint8_t blinkL[16] = {
  reverse(blinkR[8]),
  reverse(blinkR[9]),
  reverse(blinkR[10]),
  reverse(blinkR[11]),
  reverse(blinkR[12]),
  reverse(blinkR[13]),
  reverse(blinkR[14]),
  reverse(blinkR[15]),
  
  reverse(blinkR[0]),
  reverse(blinkR[1]),
  reverse(blinkR[2]),
  reverse(blinkR[3]),
  reverse(blinkR[4]),
  reverse(blinkR[5]),
  reverse(blinkR[6]),
  reverse(blinkR[7]),
};



const uint16_t pupilR[2] = {
  0b0000000110000000,
  0b0000000110000000,
};

const uint16_t pupilL[2] = {
  0b0000000110000000,
  0b0000000110000000,
};


const uint8_t eyeSquintR[16] = {
  
  0b00000000,
  0b00000000,
  0b00000001,
  0b00000111,
  0b00011111,
  0b00111110,
  0b00111111,
  0b00001111,

  0b00011000,
  0b01111000,
  0b11110000,
  0b11000000,
  0b00000000,
  0b00000000,
  0b10000000,
  0b11100000,
};

const uint8_t eyeSquintL[16] = {
  reverse(eyeSquintR[8]),
  reverse(eyeSquintR[9]),
  reverse(eyeSquintR[10]),
  reverse(eyeSquintR[11]),
  reverse(eyeSquintR[12]),
  reverse(eyeSquintR[13]),
  reverse(eyeSquintR[14]),
  reverse(eyeSquintR[15]),
  
  reverse(eyeSquintR[0]),
  reverse(eyeSquintR[1]),
  reverse(eyeSquintR[2]),
  reverse(eyeSquintR[3]),
  reverse(eyeSquintR[4]),
  reverse(eyeSquintR[5]),
  reverse(eyeSquintR[6]),
  reverse(eyeSquintR[7]),
};

const uint8_t mouthR[32] = {
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000111,
  0b00011111,
  0b01111000,
  0b11100000,
  0b10000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b11000000,
  0b11110000,
  0b00111100,
  0b00001111,
  0b00000011,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000111,
  0b00011111,
  0b01111000,
  0b11100000,

  0b00000100,
  0b00011110,
  0b01111011,
  0b11100011,
  0b11111111,
  0b00000000,
  0b00000000,
  0b00000000,
};

const uint8_t mouthL[32] = {
  reverse(mouthR[24]),
  reverse(mouthR[25]),
  reverse(mouthR[26]),
  reverse(mouthR[27]),
  reverse(mouthR[28]),
  reverse(mouthR[29]),
  reverse(mouthR[30]),
  reverse(mouthR[31]),

  reverse(mouthR[16]),
  reverse(mouthR[17]),
  reverse(mouthR[18]),
  reverse(mouthR[19]),
  reverse(mouthR[20]),
  reverse(mouthR[21]),
  reverse(mouthR[22]),
  reverse(mouthR[23]),
  
  reverse(mouthR[8]),
  reverse(mouthR[9]),
  reverse(mouthR[10]),
  reverse(mouthR[11]),
  reverse(mouthR[12]),
  reverse(mouthR[13]),
  reverse(mouthR[14]),
  reverse(mouthR[15]),
  
  reverse(mouthR[0]),
  reverse(mouthR[1]),
  reverse(mouthR[2]),
  reverse(mouthR[3]),
  reverse(mouthR[4]),
  reverse(mouthR[5]),
  reverse(mouthR[6]),
  reverse(mouthR[7]),
};

const uint8_t snoot[16] = {
  0b00000000,
  0b11111110,
  0b01111111,
  0b00000011,
  0b00000011,
  0b00000001,
  0b00000000,
  0b00000000,
  
  0b00000000,
  0b01111111,
  0b11111110,
  0b11000000,
  0b11000000,
  0b10000000,
  0b00000000,
  0b00000000,
  
};

const uint8_t errorCode[16] = {
  0b00000000,
  0b00100000,
  0b01100011,
  0b10100100,
  0b11110100,
  0b00100100,
  0b00100011,
  0b00000000,

  0b00000000,
  0b00001001,
  0b00011001,
  0b10101001,
  0b10111101,
  0b10001000,
  0b00001001,
  0b00000000,
};

const uint8_t heartEye[16] = {
  0b00000000,
  0b00100110,
  0b01001111,
  0b01001111,
  0b01001111,
  0b00100111,
  0b00010011,
  0b00000001,

  0b00000000,
  0b11001000,
  0b11100100,
  0b11100100,
  0b11100100,
  0b11001000,
  0b10010000,
  0b00000000,
};

const uint8_t hypnoEyesAnimation_Frames = 6;
const uint8_t hypnoEyesAnimation [hypnoEyesAnimation_Frames] [16] = {
{
0b00000010,
0b00001111,
0b00011000,
0b00010001,
0b00010010,
0b00011011,
0b00001101,
0b00000000,

0b00000000,
0b10110000,
0b11011000,
0b01001000,
0b10001000,
0b00011000,
0b11110000,
0b01000000,
},
{
0b00000011,
0b00001100,
0b00010000,
0b00010011,
0b00010110,
0b00010110,
0b00001011,
0b00000000,

0b00000000,
0b11010000,
0b01101000,
0b01101000,
0b11001000,
0b00001000,
0b00110000,
0b11000000,
},
{
0b00000011,
0b00001100,
0b00011011,
0b00010111,
0b00010100,
0b00010100,
0b00001010,
0b00000011,

0b11000000,
0b01010000,
0b00101000,
0b00101000,
0b11101000,
0b11011000,
0b00110000,
0b11000000,
},
{
0b00000011,
0b00001100,
0b00010001,
0b00010110,
0b00001001,
0b00001001,
0b00001100,
0b00000011,

0b11000000,
0b00110000,
0b10010000,
0b10010000,
0b01101000,
0b10001000,
0b00110000,
0b11000000,
},
{
0b00000000,
0b00001111,
0b00001101,
0b00001000,
0b00011001,
0b00011001,
0b00001100,
0b00000011,

0b11000000,
0b00110000,
0b10011000,
0b10011000,
0b00010000,
0b10110000,
0b11000000,
0b00000000,
},
{
0b00000010,
0b00001111,
0b00011000,
0b00010000,
0b00010011,
0b00010011,
0b00001101,
0b00000010,

0b01000000,
0b10110000,
0b11001000,
0b11001000,
0b00001000,
0b00011000,
0b11110000,
0b01000000,
},
};

//const uint8_t heartEyeL[16] = {
//  reverse(heartEyeR[8]),
//  reverse(heartEyeR[9]),
//  reverse(heartEyeR[10]),
//  reverse(heartEyeR[11]),
//  reverse(heartEyeR[12]),
//  reverse(heartEyeR[13]),
//  reverse(heartEyeR[14]),
//  reverse(heartEyeR[15]),
//  
//  reverse(heartEyeR[0]),
//  reverse(heartEyeR[1]),
//  reverse(heartEyeR[2]),
//  reverse(heartEyeR[3]),
//  reverse(heartEyeR[4]),
//  reverse(heartEyeR[5]),
//  reverse(heartEyeR[6]),
//  reverse(heartEyeR[7]),
//};

#endif
