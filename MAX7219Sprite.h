/* 
 * MAX7219 animation and interpolation tool. 
 * For all your 8xN monochrome matrix mangling needs!
 * Version 0.2
 * 
 */
//https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
namespace splerp {

class RenderTarget {
  public:
  // This is our frame buffer.
  // Each byte represents a row of pixels on a single 8x8 matrix.
  // Each matrix's data is stored back-to-back, so elements 0-7 are for matrix 0, 16-23 are for matrix 2, etc.
  uint8_t * buffer;
  
  // Number of matries worth of buffer to allocate.
  int nMatrices;
  
  RenderTarget(const int numberOfMatrices) {
    nMatrices = numberOfMatrices;
    buffer = (uint8_t*)malloc(8*nMatrices);
    drawBlank(0, nMatrices);
  }

  // Destructor is important, too!
  ~RenderTarget() {
    delete[] buffer;
  }
  
  /*
   * Send a 16x8 image to the matrix at and after 'index' (so the i'th and i+1'th matrix)
   * Will horizontally blur between sprite 1 (spr1) and sprite 2 (spr2).
   * - Sprites are uint8_t arrays.
   * - Element 0 will go in row 0 of matrix 0.
   * - Element 15 will go in row 7 of matrix 1
   * -- e.g: {0b00001111,0b11110000,...} will produce a contiguous eight lit pixels across the two panels.
   * 
   * The extent of the interpolation is given by 'progress'
   * - 0 yields spr1
   * - 8 yields spr2 
   * - 3 yields maximun distortion.
   * 
   * LTR and RTL are bitmasks that determine which rows are blurred in which direction.
   * - set RTL[i] when SPR1 is to the right of SPR2 in that row.
   * - set LTR[i] when SPR1 is to the left of SPR2 in that row.
   * - set both if you want to.
   * 
   */
  void drawLerp16(const int index, const uint8_t * spr1, const uint8_t * spr2, const int progress, const uint8_t LTR, const uint8_t RTL) {
	uint8_t * target = buffer;
	for (int i = 0; i < 8; ++i) { // Rows -> i = f(x)
	  // Compact our row into a single 16-bit field
	  uint16_t slice1 = spr1[i + 8] | (spr1[i]<<8);
	  uint16_t slice2 = spr2[i + 8] | (spr2[i]<<8);
  
	  // Make our blur masks by duplicating our start and end state.
	  uint16_t mask1 = slice1;
	  uint16_t mask2 = slice2;
	  // Either blur from left to right or right to left. Or both.
	  if (LTR & (1 << i)) {
		// If we're left-to-right, blur the sprite 1 mask to the right
		for (int i = 0; i < progress; ++i) {
		  mask1 = mask1 | mask1 >> 1;
		}
		// and blur the sprite 2 mask to the left
		for (int i = progress; i < 8; ++i) {
		  mask2 = mask2 | mask2 << 1;
		}
	  }
	  if (RTL & (1 << i)) {
		// If we're left-to-right, blur the sprite 1 mask to the left
		for (int i = 0; i < progress; ++i) {
		  mask1 = mask1 | mask1 << 1;
		}
		// and blur the sprite 2 mask to the right
		for (int i = progress; i < 8; ++i) {
		  mask2 = mask2 | mask2 >> 1;
		}
	  }
  
	  // To get our final matrix state for this row...
	  uint16_t blend = 0;
	  // If we're less than 4 of 8, then use sprite 1 or-ed with the bitwise-and of the mask,
	  // Otherwise use sprite 2 .
	  if (progress < 4) {
		blend = slice1 | (mask1 & mask2);
	  } else {
		blend = slice2 | (mask1 & mask2);
	  }
  
	  // Enable this for debugging; it'll write the progress as a binary number to the 0th row.
	  /*if (i == 0) {
		blend = (progress) | (progress << 8);
	  }*/
  
	  // Now, actually throw our bits into our frame buffer:
	  target[(index  )*8 + i] = (blend>>8)&0xff;
	  target[(index+1)*8 + i] = blend&(0xff);
	}
  }
  
  /* 
   *  Interpolate an 8x8 matrix between two sprites.
   *  This is a bit of a mess. Doesn't work well. Decidedly less fun than the 16x8 method.
   */
  void drawLerp(const int index, const uint8_t * spr1, const uint8_t * spr2, const int progress) {
    uint8_t * target = buffer;
    for (int i = 0; i < 8; ++i) { // Rows -> i = f(x)
      uint8_t slice1 = spr1[i];
      uint8_t slice2 = spr2[i];
      
      uint8_t mask1 = slice1;
      uint8_t mask2 = slice2;
      for (int i = 0; i < progress; ++i) {
        mask1 = mask1 | mask1 >> 1 | mask1 << 1;
      }
      for (int i = progress; i < 8; ++i) {
        mask2 = mask2 | mask2 >> 1 | mask2 << 1;
      }
      
      uint8_t blend = 0;
      if (progress < 4) {
        blend = slice1 | (mask1 & mask2);
      } else {
        blend = slice2 | (mask1 & mask2);
      }
  
      // Put this data into the frame buffer
      target[index*8 + i] = blend;
    }
  }
  
  // This one is definitely a bit weird...
  // X and Y are the offsets applied to the pupil sprite
  // *dat is the main eye sprite
  // *pupil is the pupil sprite (which is most readily stored as a uint16_t... for... reasons.
  //   -- This matrix only has two rows right now. That was a rather dumb arbitrary decision.
  // *blink is the sprite that will be slid down from the top if blinkdist > 0
  // blinkdist is the distance that the upper eyelid has shifted downwards.
  void drawEye(const int index, const uint8_t * dat, const uint16_t * pupil, const int x, const int y, const uint8_t * blink, const int blinkDist) {
    uint8_t * target = buffer;
    for (int i = 0; i < 8; ++i) {
      // Unless otherwise true, assume a blank pupil line
      uint16_t pupilSlice = 0x0000;
      if (i == y+3 || i == y+4) {
        pupilSlice = pupil[i-3-y];
      }
      if (x > 0) pupilSlice >>=  x;
      if (x < 0) pupilSlice <<= -x;
  
      // Iterate over the two matrix elements needed to display an eye.
      for (int j = 0; j < 2; ++j) { 
        int joffset = j*8;
        uint16_t thisPupilSlice = 0; // This will get cut down soon enough.
        uint8_t disp = dat[i + joffset];
        
        if (i < blinkDist) {
          disp |= blink[i + joffset];
        }
        
        if (i == y+3 || i == y+4) {
          // Get the subbitstring of our pupilSlice; we're gonna need it.
          if (j == 0) thisPupilSlice = ((pupilSlice & 0xFF00)  >> 8);
          if (j == 1) thisPupilSlice = ((pupilSlice & 0x00FF));
        }
  
        target[joffset + index*8 + i] = disp | (uint8_t)thisPupilSlice;
        //buffer[i + (index+j)*8]
      }
    }
  }
  
  void drawBlank(const int index, const int count) {
    uint8_t * target = buffer;
    
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < count; ++j) {
        target[(index + j)*8 + i] = 0;
      }
    }
  }
  
  
  /*
   * Take the bitwise XOR of the two inputs, and write it to the target
   */
  void xorMatrix(const int index, const uint8_t * src1, const uint8_t * src2, const int count) {
    uint8_t * target = buffer;
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < count; ++j) {
        target[(index + j)*8 + i] = src1[j*8 + i] ^ src2[j*8 + i];
      }
    }
  }
  
  /*
   * Take the bitwise OR of the two inputs, and write it to the target
   */
  void orMatrix(const int index, const uint8_t * src1, const uint8_t * src2, const int count) {
    uint8_t * target = buffer;
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < count; ++j) {
        target[(index + j)*8 + i] = src1[j*8 + i] | src2[j*8 + i];
      }
    }
  }
  
  /*
   * Take the bitwise AND of the two inputs, and write it to the target
   */
  void andMatrix(const int index, const uint8_t * src1, const uint8_t * src2, const int count) {
    uint8_t * target = buffer;
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < count; ++j) {
        target[(index + j)*8 + i] = src1[j*8 + i] & src2[j*8 + i];
      }
    }
  }
  
  /*
   * Perform a bitwise inversion of the source matrix and place it at the target.
   */
  void drawInvert(const int index, const uint8_t * src1, const int count) {
    uint8_t * target = buffer;
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < count; ++j) {
        target[(index + j)*8 + i] = ~src1[j*8 + i];
      }
    }
  }
  
  // Copy count arrays of byte from SRC to target at index, offseting the source data by hor pixels along, and ver pixels vertically.
  // Positive hor will offset down the display array, as though index were fractionally increased.
  // Positive ver will offset towards i=7 on the matrix.
  void drawOffset(const int index, const uint8_t * src, const int count, const int horin, const int ver) {
    uint8_t * target = buffer;
    // This is how many matrices we need to offset everything by.
    int horizontalMatrixOffset = horin/8;
    // This is where the trouble starts...
    int hor = horin % 8;
    // Iterate over our target matrix rows, as we do.
    for (int i = 0; i < 8; ++i) {
      // Check if we're out-of-bounds on the vertical offset for this row, and just send blank if we are.
      if (i < -ver || i > 7-ver) {
        for (int j = 0; j < count; ++j) {
          target[(index + j)*8 + i] = 0x00;
        }
      // We're in-bounds with the vertical offset
      } else {
        // For each element
        for (int j = 0; j < count; ++j) {
          // Check that our offfset leaves us with usable matrix.
          if ((j < horizontalMatrixOffset)|| (j >= count+horizontalMatrixOffset)) {
            target[(index + j)*8 + i] = 0;
          } else {
            uint8_t sourcebyte = src[i + ver + j*8 - horizontalMatrixOffset*8];
            uint8_t extrabyte = 0;
            // Works.
            if (hor > 0) {
              if (j > horizontalMatrixOffset) {
                // Pull from the left
                extrabyte = src[i + ver + j*8 - 8 - horizontalMatrixOffset*8];
              }
              sourcebyte = sourcebyte >> hor | extrabyte << (8-hor);
            }
            // Presently fuckered; not even reaching it.
            if (hor < 0) {
              if (j+1 < count+horizontalMatrixOffset) {
               // Pull from the right
               extrabyte = src[i + ver + j*8 + 8 - horizontalMatrixOffset*8];
              }
              sourcebyte = sourcebyte << -hor | extrabyte >> (8+hor);
            }
            target[(index + j)*8 + i] = sourcebyte;
          }
        }
      }
    }
  }
  
  // Copy count arrays of byte from SRC to target at index, offseting the source data by hor pixels along, and ver pixels vertically.
  // This will wrap graphics along X in the specified matrix count.
  // Positive hor will offset down the display array, as though index were fractionally increased.
  // Positive ver will offset towards i=7 on the matrix.
  void drawOffsetWrap(const int index, const uint8_t * src, const int count, const int horin, const int ver) {
    uint8_t * target = buffer;
  
    int hor = horin % (count*8);
    
    // This is how many matrices we need to offset everything by.
    int horizontalMatrixOffset = hor/8;
    
    // This is where the trouble starts...
    hor = hor % 8;
    
    // Iterate over our target matrix rows, as we do.
    for (int i = 0; i < 8; ++i) {
      // Check if we're out-of-bounds on the vertical offset for this row, and just send blank if we are.
      if (i < -ver || i > 7-ver) {
        for (int j = 0; j < count; ++j) {
          target[(index + j)*8 + i] = 0x00;
        }
      // We're in-bounds with the vertical offset
      } else {
        // For each element
        for (int j = 0; j < count; ++j) {
          
          uint8_t sourcebyte = src[i + ver + ((128 + j - horizontalMatrixOffset)%count)*8];
          uint8_t extrabyte = 0;
          
          if (hor > 0) {
            // Pull from the left
            extrabyte = src[i + ver + ((128 + j - 1 - horizontalMatrixOffset)%count)*8];
            sourcebyte = sourcebyte >> hor | extrabyte << (8-hor);
          }
          
          if (hor < 0) {
            // Pull from the right
            extrabyte = src[i + ver + ((128 + j + 1 - horizontalMatrixOffset)%count)*8];
            sourcebyte = sourcebyte << -hor | extrabyte >> (8+hor);
          }
          
          target[(index + j)*8 + i] = sourcebyte;
        }
      }
    }
  }
  
  // Write a sprite at SRC to TARGET[index]. Copies 'count' 8x8 matrices.
  // Write up the list - index, index+1, index+2....
  void drawSprite(const int index, const uint8_t * src, const int count = 1) {
    uint8_t * target = buffer;
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < count; ++j) {
        target[(index + j)*8 + i] = src[(j)*8 + i];
      }
    }
  }

  // Experimental flip the sprite?
  void drawSpriteFlipped(const int index, const uint8_t * src, const int count = 1) {
	uint8_t * target = buffer;
    for (int i = 0; i < 8; ++i) { // For each row
      for (int j = 0; j < count; ++j) { // For each matrix...
        target[(index + j)*8 + i] = src[(j)*8 + 7 - i];
      }
    }
  }
  
};


// A render target that actually maps to a group of MAX7219 LED arrays
class DisplayTarget : public RenderTarget {
  // Chip-select pin 
  int CSpin;
	
  // Send a byte into the shift register stack with the given address and data byte
  void TX (const uint8_t adr, const uint8_t dat) {
    SPI.transfer(adr);
    SPI.transfer(dat);
  }
  
  // Send a CS-pin pulse to make the 7219's read the data in the shift registers.
  void pulseCS() {
    digitalWrite(CSpin, HIGH);
	delayMicroseconds(1);
    digitalWrite(CSpin, LOW);
  }
  
  
public:
  DisplayTarget(const int numberOfMatrices, const int ChipSelectPin)
    : RenderTarget(numberOfMatrices) {
    
	CSpin = ChipSelectPin;
    nMatrices = numberOfMatrices;
      //setupDisplayTarget();
  }
  
  // Destructor is important, too!
  ~DisplayTarget() {
    
  }
  
  // Send our frame buffer to the matrices.
  void display() {
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
	for (int i = 0; i < 8; i++) {
	  for (int j = 0; j < nMatrices; ++j) {
		TX(1 + i, buffer[j*8 + i]);
	  }
	  pulseCS();
	}
	SPI.endTransaction();
  }
	
  void reboot(const int intensity = 0x04) {
	pinMode(CSpin, OUTPUT);
    digitalWrite(CSpin, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    
	// Disable shutdown ([0x0C] = 1)
    for (int i = 0; i < nMatrices; ++i){
      TX(0x0C, 0x00);
    }
    pulseCS();
    // Disable shutdown ([0x0C] = 1)
    for (int i = 0; i < nMatrices; ++i){
      TX(0x0C, 0x01);
    }
    pulseCS();
    
    // Set decode mode to 0x00, for bitwise driving
    for (int i = 0; i < nMatrices; ++i){
      TX(0x09, 0x00);
    }
    pulseCS();
	
    // Set intensity to something
    for (int i = 0; i < nMatrices; ++i){
      TX(0x0F, intensity);
    }
    pulseCS();
	
    // Maximum scan limit
    for (int i = 0; i < nMatrices; ++i){
      TX(0x0B, 0x07);
    }
    pulseCS();
	SPI.endTransaction();
  }
	
  // Set up our chip select lines and prepare our device
  void setupDisplayTarget(const int intensity = 0x04) {
    pinMode(CSpin, OUTPUT);
    digitalWrite(CSpin, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    
    // Disable shutdown ([0x0C] = 1)
    for (int i = 0; i < nMatrices; ++i){
      TX(0x0C, 0x01);
    }
    pulseCS();
    // Set test mode to 1
    for (int i = 0; i < nMatrices; ++i){
      TX(0x0F, 0x01);
    }
    pulseCS();
    delay(100);
    // Set test mode to 0
    for (int i = 0; i < nMatrices; ++i){
      TX(0x0F, 0x00);
    }
    pulseCS();
    // Entering test mode enables the shutdown register!!!
    
    // Disable shutdown ([0x0C] = 1)
    for (int i = 0; i < nMatrices; ++i){
      TX(0x0C, 0x01);
    }
    pulseCS();
    
    // Set decode mode to 0x00, for bitwise driving
    for (int i = 0; i < nMatrices; ++i){
      TX(0x09, 0x00);
    }
    pulseCS();
    
    // Set intensity to something
    for (int i = 0; i < nMatrices; ++i){
      TX(0x0A, intensity);
    }
    pulseCS();
  
    // Maximum scan limit
    for (int i = 0; i < nMatrices; ++i){
      TX(0x0B, 0x07);
    }
    pulseCS();
	SPI.endTransaction();
  }
  
};

}
