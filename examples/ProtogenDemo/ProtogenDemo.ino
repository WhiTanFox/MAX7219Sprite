#include <SPI.h>
#include "MAX7219Sprite.h"

/* Draw a protogen face on the attached 14 panels of LED matrices.
 *
 *
 *
 */

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

const uint8_t eyeL[16] = {
  0b00000111,
  0b00011000,
  0b00100000,
  0b00100000,
  0b00100000,
  0b00010000,
  0b00010000,
  0b00001111,
  
  0b11111100,
  0b00000010,
  0b00000010,
  0b00000010,
  0b00000100,
  0b00000100,
  0b01111000,
  0b10000000,
};

const uint8_t blinkL[16] = {
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

const uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

const uint8_t blinkR[16] = {
  reverse(blinkL[8]),
  reverse(blinkL[9]),
  reverse(blinkL[10]),
  reverse(blinkL[11]),
  reverse(blinkL[12]),
  reverse(blinkL[13]),
  reverse(blinkL[14]),
  reverse(blinkL[15]),
  
  reverse(blinkL[0]),
  reverse(blinkL[1]),
  reverse(blinkL[2]),
  reverse(blinkL[3]),
  reverse(blinkL[4]),
  reverse(blinkL[5]),
  reverse(blinkL[6]),
  reverse(blinkL[7]),
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

const uint16_t pupilL[2] = {
  0b0000000110000000,
  0b0000000110000000,
};

const uint16_t pupilR[2] = {
  0b0000000110000000,
  0b0000000110000000,
};


const uint8_t eyeSquintL[16] = {
  
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

const uint8_t eyeSquintR[16] = {
  reverse(eyeSquintL[8]),
  reverse(eyeSquintL[9]),
  reverse(eyeSquintL[10]),
  reverse(eyeSquintL[11]),
  reverse(eyeSquintL[12]),
  reverse(eyeSquintL[13]),
  reverse(eyeSquintL[14]),
  reverse(eyeSquintL[15]),
  
  reverse(eyeSquintL[0]),
  reverse(eyeSquintL[1]),
  reverse(eyeSquintL[2]),
  reverse(eyeSquintL[3]),
  reverse(eyeSquintL[4]),
  reverse(eyeSquintL[5]),
  reverse(eyeSquintL[6]),
  reverse(eyeSquintL[7]),
};

const uint8_t mouthL[32] = {
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

const uint8_t mouthR[32] = {
  reverse(mouthL[24]),
  reverse(mouthL[25]),
  reverse(mouthL[26]),
  reverse(mouthL[27]),
  reverse(mouthL[28]),
  reverse(mouthL[29]),
  reverse(mouthL[30]),
  reverse(mouthL[31]),

  reverse(mouthL[16]),
  reverse(mouthL[17]),
  reverse(mouthL[18]),
  reverse(mouthL[19]),
  reverse(mouthL[20]),
  reverse(mouthL[21]),
  reverse(mouthL[22]),
  reverse(mouthL[23]),
  
  reverse(mouthL[8]),
  reverse(mouthL[9]),
  reverse(mouthL[10]),
  reverse(mouthL[11]),
  reverse(mouthL[12]),
  reverse(mouthL[13]),
  reverse(mouthL[14]),
  reverse(mouthL[15]),
  
  reverse(mouthL[0]),
  reverse(mouthL[1]),
  reverse(mouthL[2]),
  reverse(mouthL[3]),
  reverse(mouthL[4]),
  reverse(mouthL[5]),
  reverse(mouthL[6]),
  reverse(mouthL[7]),
};

const uint8_t snoot[16] = {
  0b00000011,
  0b00000110,
  0b00001100,
  0b00011000,
  0b00110001,
  0b01110011,
  0b00111110,
  0b00011000,
  
  0b11000000,
  0b01100000,
  0b00110000,
  0b00011000,
  0b10001100,
  0b11001110,
  0b01111100,
  0b00011000,
  
};


int8_t xs[20] = {
  -1, -1, -1, -1, -2,
  -3, -2, -1,  0, 1,
   1,  1,  0,  0, -1,
   -1, -1, -1, -1, -1,
};

int8_t ys[20] = {
   0, -1, -1, -1,  0,
   0,  0,  0,  0,  0,
   1,1, 1, 1, 1,
  1, 1, 1, -0, -0,
};


// Initialize the main display matrix
// arg0 is the number of 8x8 segments described in the frame buffer.
// arg1 is the CS pin in use for our matrix.
splerp::DisplayTarget matrix(14, 3);

// Create some frame buffers to use as intermediate rendering targets.
// Since this is going to be used for the eye animation, we only need
//  to include two MAX7219's worth of memory.
splerp::RenderTarget eyeBufferL(2);
splerp::RenderTarget eyeBufferR(2);

splerp::RenderTarget mouthBufferL(4);
splerp::RenderTarget mouthBufferR(4);

splerp::RenderTarget snootBuffer(2);

void setup() {
  Serial.begin(115200);
  // We only need to setup the display target once, and only for the output matrix itself
  matrix.setupDisplayTarget(0x07);

  for (int i = 0; i < 2; ++i) {
    eyeBufferL.drawSprite(i,startup,1);
    eyeBufferR.drawSprite(i,startup,1);
  }
  for (int i = 0; i < 4; ++i) {
    mouthBufferL.drawSprite(i,startup,1);
    mouthBufferR.drawSprite(i,startup,1);
  }

  matrix.drawSprite(0,mouthBufferR.buffer,4);
  matrix.drawSprite(4,eyeBufferR.buffer,2);
  matrix.drawSprite(6,snootBuffer.buffer,2);
  matrix.drawSprite(8,eyeBufferL.buffer,2);
  matrix.drawSprite(10,mouthBufferL.buffer,4);
  matrix.display();
  delay(500);
}



void loop() {
  eyeBufferL.drawEye(0, eyeL, pupilL, -1, 0, blinkL, 2);
  eyeBufferR.drawEye(0, eyeR, pupilR, 1, 0, blinkR, 2);
  
  mouthBufferL.drawSprite(0,mouthL,4);
  mouthBufferR.drawSprite(0,mouthR,4);

  snootBuffer.drawSprite(0,snoot,2);
  

  matrix.drawSprite(0,mouthBufferR.buffer,4);
  matrix.drawSprite(4,eyeBufferR.buffer,2);
  matrix.drawSprite(6,snootBuffer.buffer,2);
  matrix.drawSprite(8,eyeBufferL.buffer,2);
  matrix.drawSprite(10,mouthBufferL.buffer,4);
  
  long int startTime = micros();
  matrix.display();
  long int stopTime = micros();
  Serial.println(stopTime - startTime);
  delay(5);
}