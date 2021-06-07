MAX7219Sprite.h
---------------
Sprite wrangling for MAX7219-based displays.

Version 0.1

By charles H (@CharlesCAN#4335 on Discord)

Common terms:
-------------

In all of the functions, several arguments have consistent meanings:
int index		The offset in 8x8 matrices from the head of the frame buffer at which to write the result.
int count		The number of 8x8 matrices to send to output, and also take from the input


How to use it:
--------------

In the library:
- Set
const int CSpin
to reflect your design's wiring
- Set
const int nMatrices
to reflect your design's count of MAX7219 matrices

In your setup function:
- Call
void splerp::setup()
this enables the SPI peripheral with appropriate configuration, then sends a few bytes of setup and reset data to each
MAX7219 to ensure they're going to actually work as we would hope.

In your program's main loop:
- Call whatever render functions you want
- Once everything of interest has been sent to the primary frame buffer (target = NULL), call
void splerp::apply()
to send the frame buffer to the displays
Note that this will overwrite all pixels (since this has a lower computational cost than tracking your changes).
However, the frame buffer is not cleared after calling apply(), so unless you clear it, the replacement data should
not show any unexpected changes.


Functions:
----------
apply()		writes out the framebuffer to the display.
setup()		configures the PSI peripheral and all of the MAX7219s
void sendMatrixLerp16(		Interpolates between two 16x8 sprites
	int index,		render target start position
	uint8_t * spr1,		output for progress = 0
	uint8_t * spr2,		output for progress = 8
	int progress,		progress from 0 to 8, inclusive
	uint8_t LTR,		which rows to blur from left-to-right. Bitmask.
	uint8_t RTL		which rows to blur from right-to-left. Bitmask.
)
void sendMatrixLerp(		Interpolates between two 8x8 sprites
	int index,		render target start position
	uint8_t * spr1,		output for progress = 0
	uint8_t * spr2,		output for progress = 8
	int progress		progress from 0 to 8, inclusive
)
void sendMatrixEye(		Renders an eye with an eyelid and pupil
	int index,		render target start position
	uint8_t * dat,		the eye outline sprite. 16x8
	uint16_t * pupil,	the special pupil matrix (two 16-bit integers)
	int x,			current pupil x offset
	int y,			current pupil y offset
	uint8_t * blink,	the closed-eye sprite. 16x8
	int blinkDist		extent, from 0 to 8, of eyelid closure.
)
blankMatrix(			Renders blank (black) pixels.
	int index,		render target start position
	int count		number of 8x8's of blank to output
)
void xorMatrix(			Renders the bitwise XOR of two sprites
void orMatrix(			Renders the bitwise OR of two sprites
void andMatrix(			Renders the bitwise AND of two sprites
	int index,		render target start position
	uint8_t * src1,		first sprite
	uint8_t * src2,		second sprite
	int count		number of 8x8 panels included in the sprites
)
void invertMatrix(		Get the bitwise inverse of this sprite
	int index,		render target start position
	uint8_t * src1,		input sprite
	int count		number of 8x8 panels included in the sprite
)
offsetMatrix(			Shift along X and Y the given sprite
offsetMatrixWrap(		Shift along X and Y the given sprite, but wrap around the edge on X
	int index,		render target start position
	uint8_t * src,		input sprite
	int count,		number of 8x8 panels included in the sprite
	int hor,		horizontal offset (towards the 0th matrix)
	int ver			vertical offset (towards the 0th row)
)
sendMatrix(			Copy the source sprite to the target location.
	int index,		render target start position
	uint8_t * src,		input sprite
	int count,		number of 8x8 panels included in the sprite
)


Defining sprites:
-----------------

Sprites, the frame buffer, and output buffers are all interchangable, and all defined in the same way.
All of them should be handled as "uint8_t[]", or "uint8_t *".



Consider a four-matrix strip (off of Amazon or Ebay):
+--------+--------+--------+--------+
|        |        |        |        |
|        |        |        |        | CLK
|        |        |        |        | CS
| MTRX0  | MTRX1  | MTRX2  | MTRX3  | DIN
|        |        |        |        | GND
|        |        |        |        | VCC
|        |        |        |        |
|        |        |        |        |
+--------+--------+--------+--------+

The 0th matrix is furthest from the data-in side.

In the model used by this package, each driver of LED's is
a consecutive 8-byte block of memory.

+--------+--------+--------+--------+
| spr[0] | spr[8] |        |        |
| spr[1] | spr[9] |        |        | CLK
| spr[2] | spr[10]|        |        | CS
| MTRX0  | MTRX1  | MTRX2  | MTRX3  | DIN
| spr[4] |    .   |        |   .    | GND
| spr[5] |    .   |        |   .    | VCC
| spr[6] |    .   |        |   .    |
| spr[7] |        |        | spr[31]|
+--------+--------+--------+--------+

For instance:
  MTRX0   MTRX1
+--------+--------+
|    XXXX|XXXX    |
|   XXXX | XXXX   |
|  XXXX  |  XXXX  |
|  XXXX  |  XXXX  |
|  XXXX  |  XXXX  |
|  XXXX  |  XXXX  |
|   XXXX | XXXX   |
|XXXXXXX | XXXXXXX|
+--------+--------+

yields 

uint8_t Omega[16] = {
	// MTRX0
	0b00001111,
	0b00011110,
	0b00111100,
	0b00111100,
	0b00111100,
	0b00111100,
	0b00011110,
	0b11111110,
	// MTRX1
	0b11110000,
	0b01111000,
	0b00111100,
	0b00111100,
	0b00111100,
	0b00111100,
	0b01111000,
	0b01111111,
};


Framebuffers, sprites, and render targets:
------------------------------------------

The data storage is the exact same for framebuffers, sprites, and render targets. To the program, the are interchangable (unless
one is a const; be careful!).

What this means for you is that you can allocate a block of memory of appropriate size and pass it as the target to any rendering
function, such as an interpolation or animation.

For instance:

// Allocate sixteen bytes (Probably don't do this on the stack, but at the same time...)
uint8_t eyebuffer[16] = {0};
// Later, run the eye renderer with eyebuffer as the render target:
splerp::sendMatrixEye(eyebuffer, 0, eye, pupil, x+1, y, blink, bd);
// With our eye image now sent to the eyebuffer, we can use eyebuffer as an input to an interpolation function,
// which draws directly to the screen:
splerp::sendMatrixLerp16(NULL, 2, eyebuffer, eyeSquint, stepLerp, 0b11111111, 0b00000000);
// Always send apply() once you've set up your output buffer!
splerp::apply();
