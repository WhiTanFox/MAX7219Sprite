MAX7219Sprite.h
---------------

Sprite wrangling for MAX7219-based displays.

Version 1.0

By Charles H (@CharlesCAN#4335 on Discord - reach out if you need any help!)


Hardware:
---------

The MAX7219 displays are SPI displays, hook them up accordingly:
- DIN to MOSI
- CLK to CLK
- CS to a GPIO pin of your choosing, to be specified in the DisplayTarget constructor

For an Arduino Nano, this will be:
- DIN to pin 11
- CLK to pin 13
- CS to pin 8 (for the example code, at least; you can use whatever you'd like)
- Vcc to 5V
- GND to GND


How to use it:
--------------

- Construct a splerp::DisplayTarget
For this document, let's call it matrix.
The constructor for matrix takes two parameters:
- N_matrices, the number of MAX7219s connected to one another
- CSPin, the pin number you have connected to the MAX7219 CS line. The example code uses pin 8.

- Call matrix.setup() in your setup() function
You'll need to pass in an intensity value for the matrices. This goes from 0x00 to 0x0F, where 0x00 is very dim and 0x0F is very bright.
This enables the SPI peripheral with appropriate configuration, then sends a few bytes of setup and reset data to each
MAX7219 to ensure they're going to actually work as we would hope.

In your program's main loop:
- Call whatever render functions you want
- Once everything of interest has been rendered, call
matrix.display();
to send the frame buffer to the displays
Note that this will overwrite all pixels (since this has a lower computational cost than tracking your changes).
However, the frame buffer is not cleared after calling display(), so unless you clear it, the replacement data should
not show any unexpected changes.


Hey, the outputs keep going glitchy?
------------------------------------

Sometimes, your MAX7219s will do some weird stuff.

Reflow your solder joints, use nicer wire, and inject power from both ends of the array.

However, if you've done what you readily can and are tearing your hair out, there's one dirty solution:
Call matrix.reboot() every once in a while - say, twice per second.
You'll need to feed it your intensity value, but that's probably fine.


Functions:
----------

display()	writes out the framebuffer to the display.
setup(		configures the PSI peripheral and all of the MAX7219s
	uint8_t intensity, 		Display brightness
)		
reboot(		Smacks all the settings back into the displays
	uint8_t intensity, 		Display brightness
)

drawSprite(			Copy the source sprite to the target location.
	int index,		render target start position
	uint8_t * src,	input sprite
	int count,		number of 8x8 panels included in the sprite
)

void drawLerp16(		Interpolates between two 16x8 sprites
	int index,		render target start position
	uint8_t * spr1,		output for progress = 0
	uint8_t * spr2,		output for progress = 8
	int progress,		progress from 0 to 8, inclusive
	uint8_t LTR,		which rows to blur from left-to-right. Bitmask.
	uint8_t RTL		which rows to blur from right-to-left. Bitmask.
)
void drawLerp(		Interpolates between two 8x8 sprites
	int index,		render target start position
	uint8_t * spr1,		output for progress = 0
	uint8_t * spr2,		output for progress = 8
	int progress		progress from 0 to 8, inclusive
)

void drawEye(		Renders an eye with an eyelid and pupil. (Don't use this)
	int index,		render target start position
	uint8_t * dat,		the eye outline sprite. 16x8
	uint16_t * pupil,	the special pupil matrix (two 16-bit integers).
	int x,			current pupil x offset
	int y,			current pupil y offset
	uint8_t * blink,	the closed-eye sprite. 16x8
	int blinkDist		extent, from 0 to 8, of eyelid closure.
)

drawBlank(			Renders blank (black) pixels.
	int index,		render target start position in 8x8 blocks
	int index,		render target start position in 8x8 blocks
	int count		number of 8x8's of blank to output
)

void xorMatrix(		Renders the bitwise XOR of two sprites
void orMatrix(		Renders the bitwise OR of two sprites
void andMatrix(		Renders the bitwise AND of two sprites
	int index,		Render target start position
	uint8_t * src1,	first sprite
	uint8_t * src2,	second sprite
	int count		size of the input sprite in 8x8 blocks
)
void drawInvert(	Get the bitwise NOT of a sprite
	int index,		Where in this target to place the sprite
	uint8_t * src1,	input sprite
	int count		size of the input sprite in 8x8 blocks
)

offsetMatrix(		Shift along X and Y the given sprite
offsetMatrixWrap(	Shift along X and Y the given sprite, but wrap around the edge on X
	int index,		render target start position
	uint8_t * src,	input sprite
	int count,		number of 8x8 panels included in the sprite
	int hor,		horizontal offset (towards the 0th matrix)
	int ver			vertical offset (towards the 0th row)
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

The data storage is the exact same for framebuffers, sprites, and render targets. To the program, they are interchangable (unless
one is a const; be careful!).

What this means for you is that you can allocate a block of memory of appropriate size and pass it as the target to any rendering
function, such as an interpolation or animation.

The contents of a RenderTarget or DisplayTarget can be passed around via its public .buffer member.