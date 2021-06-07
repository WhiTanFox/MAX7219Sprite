#include <SPI.h>
#include "MAX7219Sprite.h"

/*
 * A sample of all of the features available in the MAX7219Sprite library.
 * 
 */

const int nMatrices = 4;

// Viewed with the downward-facing-port to the left, this representation works quite well!
const uint8_t plonk[8] = {
  0b11111111,
  0b11000000,
  0b00000000,
  0b10000000,
  0b00000000,
  0b10000000,
  0b01000000,
  0b11000111,
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

const uint8_t testLerpA[8] = {
  0b00111100,
  0b01000010,
  0b10110001,
  0b10110001,
  0b10000001,
  0b10000001,
  0b01000010,
  0b00111100,
};

const uint8_t testLerpB[8] = {
  0b00000000,
  0b01100110,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11011011,
  0b01100110,
  0b00000000,
};

const uint8_t testLerp16A[16] = {
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00110000,
  0b01001000,
  0b01001000,
  0b00110000,
  
  0b00011000,
  0b00100100,
  0b00100100,
  0b00011000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
};

const uint8_t testLerp16B[16] = {
  
  0b00000110,
  0b00001001,
  0b00001001,
  0b00000110,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00110000,
  0b01001000,
  0b01001000,
  0b00110000,
};


const uint8_t eyeLerp16A[16] = {
  0b00000011,
  0b00000100,
  0b00001011,
  0b00001011,
  0b00001000,
  0b00001000,
  0b00000100,
  0b00000011,
  
  0b11000000,
  0b00100000,
  0b00010000,
  0b00010000,
  0b00010000,
  0b00010000,
  0b00100000,
  0b11000000,
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
// 4 is the number of 8x8 segments described in the flame buffer.
// 3 is the CS pin in use for our matrix.
splerp::DisplayTarget matrix(nMatrices, 3);

// Create some frame buffers to use as intermediate rendering targets.
// Since this is going to be used for the eye animation, we only need
//  to include two MAX7219's worth of memory.
splerp::RenderTarget eyeBufferL(2);
splerp::RenderTarget eyeBufferR(2);

splerp::RenderTarget marquee(nMatrices);

void setup() {
  Serial.begin(115200);
  // We only need to setup the display target for the display target.
  matrix.setupDisplayTarget();

  marquee.drawSprite(0,eyeLerp16A,2);
}



void loop() {
  matrix.display();
  int state = (millis()/5000)%10;
  if (state == 1) {
    //matrix.offsetMatrixWrap(NULL, 2, eyeLerp16A, 2, 8-(millis()/100), 0);
    matrix.drawSprite(0, testLerpA, 1);
    matrix.drawOffsetWrap(2, eyeLerp16A, 2, -16+((millis()/100))%32, 0);
    matrix.drawOffsetWrap(1, testLerpB, 1, 0, -((millis()/400)%2));
  }

  if (state == 2) {
    //matrix.offsetMatrixWrap(NULL, 2, eyeLerp16A, 2, 8-(millis()/100), 0);
    matrix.drawSprite(0, testLerpA, 1);
    matrix.drawOffsetWrap(2, eyeLerp16A, 2, -4+(millis()/100), 1-((millis()/200)%3));
    matrix.drawOffsetWrap(1, testLerpB, 1, 0, -((millis()/400)%2));
  }

  if (state == 3) {
    matrix.drawSprite(0, testLerpA, 1);
    matrix.drawSprite(1, testLerpB, 1);
    matrix.drawBlank(2, 1);
    int test = (millis()/800)%4;
    if (test == 0) matrix.drawBlank(3, 1);
    if (test == 1) matrix.xorMatrix(3, testLerpA, testLerpB, 1);
    if (test == 2) matrix.orMatrix(3, testLerpA, testLerpB, 1);
    if (test == 3) matrix.andMatrix(3, testLerpA, testLerpB, 1);
  }

  if (state == 4) {
    int tlerp = (millis()/50)%32;
    int stepLerp = 0;
    if (tlerp < 8) stepLerp = tlerp;
    else if (tlerp < 16) stepLerp = 8;
    else if (tlerp < 24) stepLerp = 24-tlerp;
    else stepLerp = 0;
    
    matrix.drawSprite(0, (millis()/800)%2 ? testLerp16A : testLerp16B, 2);
    matrix.drawLerp16(2, testLerp16A, testLerp16B, stepLerp, 0b11110000, 0b00001111);
  }

  if (state == 5) {
    int tlerp = (millis()/50)%32;
    int stepLerp = 0;
    if (tlerp < 8) stepLerp = tlerp;
    else if (tlerp < 16) stepLerp = 8;
    else if (tlerp < 24) stepLerp = 24-tlerp;
    else stepLerp = 0;
    
    matrix.drawSprite(0, (millis()/800)%2 ? eyeSquintL : eyeLerp16A, 2);
    matrix.drawLerp16(2, eyeSquintL, eyeLerp16A, stepLerp, 0b11110000, 0b00001111);
  }

  if (state == 6) {
    int t = (millis()/100);
    int x = xs[t%20];
    int y = ys[t%20];
    int bd = max(0, y + 4 - 1);

    int tlerp = (millis()/10)%128;
    int stepLerp = 0;
    if (tlerp < 8) stepLerp = tlerp;
    else if (tlerp < 16) stepLerp = 8;
    else if (tlerp < 24) stepLerp = 24-tlerp;
    else stepLerp = 0;
    
    int ebts = (millis()/100) % 25;
    
    if (ebts == 0) {
      bd = (millis()%100)/25;
    }
    if (ebts == 1) {
      bd = 8;
    }
    if (ebts == 2) {
      bd = 8 - (millis()%100)/25;
    }
    if (ebts > 4) {
      bd = max(0, y +4 - 2);
    }
    matrix.drawEye(2, eyeL, pupilL, x+1, y, blinkL, bd);
    matrix.drawEye(0, eyeR, pupilR, x-1, y, blinkR, bd);
  }

  if (state == 7) {
    int t = (millis()/100);
    int x = xs[t%20];
    int y = ys[t%20];
    int bd = max(0, y + 4 - 1);

    int tlerp = (millis()/10)%128;
    int stepLerp = 0;
    if (tlerp < 8) stepLerp = tlerp;
    else if (tlerp < 16) stepLerp = 8;
    else if (tlerp < 24) stepLerp = 24-tlerp;
    else stepLerp = 0;
    
    int ebts = (millis()/100) % 25;
    
    if (ebts == 0) {
      bd = (millis()%100)/25;
    }
    if (ebts == 1) {
      bd = 8;
    }
    if (ebts == 2) {
      bd = 8 - (millis()%100)/25;
    }
    if (ebts > 4) {
      bd = max(0, y +4 - 2);
    }
    eyeBufferL.drawEye(0, eyeL, pupilL, x+1, y, blinkL, bd);
    eyeBufferR.drawEye(0, eyeR, pupilR, x-1, y, blinkR, bd);
    
    matrix.drawLerp16(2, eyeBufferL.buffer, eyeSquintL, stepLerp, 0b11111111, 0b00000000);
    matrix.drawLerp16(0, eyeBufferR.buffer, eyeSquintR, stepLerp, 0b00000000, 0b11111111);
  }

  if (state == 8) {
    matrix.drawBlank(0, 4);
  }

}
