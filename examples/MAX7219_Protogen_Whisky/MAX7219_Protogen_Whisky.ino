// Protogen head controller
// By Charles H

// This is designed to work an Arduino Nano


#include <SPI.h>
#include "MAX7219Sprite.h"
#include "SpriteData.h"

#include <FastLED.h>
#include "dinoGame.h"

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;



// Wiring:
// -------
// IMU attached to I2C pins (SDA -> A4, SDL -> A5

// Boop sensor getting power and sending signal to one pin
const int pin_boopSensor = A0;

// Matrices wired as follows:
// DIN -> MOSI (Pin 11)
// CLK -> SCLK (Pin 13)
// CS pin for displays
const int pin_matrixCS = 8;

// Cheek fin and ear WS2812s
#define LED_PIN_FINS 2
#define LED_PIN_EARS 3

// LED configuration for those LED strips
// Number of LEDs in the ears
#define NUM_LEDS_EARS 39
// Number of LEDs in the ears that are obscured by thick fur
#define NUM_LEDS_EARS_MISSING 15
// Number of LEDs in each cheek fin
#define NUM_LEDS_FINS 38
#define CHIPSET     WS2812
#define COLOR_ORDER GRB
CRGB leds_Ears[NUM_LEDS_EARS];
CRGB leds_Fins[NUM_LEDS_FINS];

// Display settings
// ----------------

// How long to spend interpolating between images:
const int T_interpolate = 80;

// How bright do we want the displays?
const int displayBrightness = 0x0E;


// Initialize the main display matrix
// arg0 is the number of 8x8 segments described in the flame buffer.
// arg1 is the CS pin in use for our matrix.
splerp::DisplayTarget matrix(14, pin_matrixCS);

// Create some frame buffers to use as intermediate rendering targets.
// Since this is going to be used for the eye animation, we only need
//  to include two MAX7219's worth of memory.
splerp::RenderTarget eyeBufferL(2);
splerp::RenderTarget eyeBufferR(2);

splerp::RenderTarget mouthBufferL(4);
splerp::RenderTarget mouthBufferR(4);

splerp::RenderTarget snootBuffer(2);


// Operating state variables
bool IMUavailable = true;// Is an IMU connected to the I2C pins?
long int lastReboot = 0; // Time in milliseconds at which we last refreshed the display settings

void setup() {
  Serial.begin(115200);
  //setupFrequencySelector(); // This was for a very fun experiment elsewhere.

  // We only need to setup the display target once, and only for the output matrix itself
  matrix.setupDisplayTarget(displayBrightness);

  // To confirm that things are working, throw a checkerboard onto every matrix.
  for (int i = 0; i < 2; ++i) {
    eyeBufferL.drawSprite(i, startup, 1);
    eyeBufferR.drawSprite(i, startup, 1);
  }
  for (int i = 0; i < 4; ++i) {
    mouthBufferL.drawSprite(i, startup, 1);
    mouthBufferR.drawSprite(i, startup, 1);
  }
  snootBuffer.drawSprite(0, startup, 1);
  snootBuffer.drawSprite(1, startup, 1);

  matrix.drawSprite(0, mouthBufferR.buffer, 4);
  matrix.drawSprite(4, eyeBufferR.buffer, 2);
  matrix.drawSprite(6, snootBuffer.buffer, 2);
  matrix.drawSprite(8, eyeBufferL.buffer, 2);
  matrix.drawSprite(10, mouthBufferL.buffer, 4);
  matrix.display();
  
  // Add our cheek fin and ear LEDs to the FastLED controller
  FastLED.addLeds<CHIPSET, LED_PIN_EARS, COLOR_ORDER>(leds_Ears, NUM_LEDS_EARS);
  FastLED.addLeds<CHIPSET, LED_PIN_FINS, COLOR_ORDER>(leds_Fins, NUM_LEDS_FINS);
  
  // Turn on the ears
  for (int i = 0; i < NUM_LEDS_EARS; i++) {
    // Don't light the LEDs that are hidden by the construction
    if (i > NUM_LEDS_EARS_MISSING) {
      leds_Ears[i] = CRGB(0,0,255);
    } else {
      leds_Ears[i] = CRGB(0,0,0);
    }
  }

  // Turns on the fins, for now
  for (int i = 0; i < NUM_LEDS_FINS; i++) {
    leds_Fins[i] = CRGB(0,0, 10);
  }
  FastLED.show();

  // Try to initialize MPU6050.
  // If that fails, mark down that the IMU isn't usable and move along.
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    IMUavailable = false;
  }

  if (IMUavailable) {
    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
  }

  pinMode(pin_boopSensor , INPUT);

  // Give the user a moment to discover what all is messed up
  delay(1000);
}

// All of our potential emotes
// This is the first way I thought of to do this
#define SMILING 0 
#define HEART_EYES 1
#define SHOCK 2
#define ERROR404 3
#define BOOPED 4
#define DINOSAUR_GAME 5
#define N_Options 6

const bool canBeBooped[N_Options] = {
  1,
  1,
  1,
  1,
  1,
  0,
};

long int stateChangeTime = 0;
long int stateUpdateTime = 0;
int currentState = SMILING;

// A buffer to hold onto the last state of the eye from before a new state is called for.
splerp::RenderTarget eyeBufferL_hold(2);
splerp::RenderTarget eyeBufferR_hold(2);

// A buffer to hold the new eye image that is called for.
splerp::RenderTarget eyeFutureL(2);
splerp::RenderTarget eyeFutureR(2);

// Draw our simple smiling eyes
void drawSmilingEyes(splerp::RenderTarget * leftEye, splerp::RenderTarget * rightEye) {
  leftEye -> drawSprite(0, eyeL, 2);
  rightEye -> drawSprite(0, eyeR, 2);
  mouthBufferL.drawSprite(0, mouthL, 4);
  mouthBufferR.drawSprite(0, mouthR, 4);
}

// Call this function to enter a new emote state
void setNewState(int state) {
  stateUpdateTime = millis();
  if (currentState == state) return;
  stateChangeTime = millis();
  currentState = state;
  eyeBufferL_hold.drawSprite(0, eyeBufferL.buffer, 2);
  eyeBufferR_hold.drawSprite(0, eyeBufferR.buffer, 2);
}

// Our emotes only really count for 
void drawInterpolatedEyes(splerp::RenderTarget * target, uint8_t * eventual, uint8_t * previous) {
  long int t = (millis() - stateChangeTime)/(T_interpolate / 8);
  if (t < 8) {
    //eyeBufferL.drawLerp16(0, previous, eventual, t, 0b00111100, 0b11111100);
    target -> drawLerp16(0, previous, eventual, t, 0xFF, 0xFF);
  } else {
    target -> drawSprite(0, eventual, 2);
  }
}

// Boop sensor operating variables
int lastBoopState = 0;
long int startofboop = -5000;

long int startofbooprun = 0;
int numBoops = 0;

uint8_t getBoop() {
  return !digitalRead(A0);
}

// If we're being booped, show the correct animation.
// If it's just once, show the "BOOPED" state (>W<).
// If it's a sequence of several back-to-back boops (we've hit the boop sensor more than five times in...
//  I dunno, a second?), then trigger the dino game.

void checkBoopSensor() {
  bool in = getBoop();
  if (in && (!lastBoopState || (millis() - startofboop) < 5000)) {
    if (startofboop < 0) startofboop = millis();
    setNewState(BOOPED);
  }

  // Check for rising edge, ignoring sunlight workaround.
  if (in && !lastBoopState) {

    // If we're still in the same run of boops:
    if (millis() - startofbooprun < 1500) {
      // Uh... do nothing, actually?
      
    // Otherwise, we've overrun our timing window, so reset.
    } else {
      startofbooprun = millis();
      numBoops = 0;
    }
    numBoops ++;
    

    if (numBoops >= 5) {
      setNewState(DINOSAUR_GAME);
      numBoops = 0;
      startofbooprun = 0;
    }
  }
  
  if (!in) {
    startofboop = -5000;
  }
  
  lastBoopState = in;
}

void loop() {
  long int startTime = micros();
  
  if (Serial.available()) {
    char a = Serial.read();
    int ind = a - '0';
    if (ind >= 0 && ind < N_Options) {
      setNewState(ind);
      Serial.print("Triggered ");
      Serial.print(ind);
      Serial.println();
    }
  }
  
  if (canBeBooped[currentState]) {
    checkBoopSensor();
  }
  
  long int emoteTimer = millis() - stateUpdateTime;
  
  if (currentState == SMILING) {
    drawSmilingEyes(&eyeFutureL, &eyeFutureR);

    for (int i = 0; i < NUM_LEDS_FINS; i++) {
      leds_Fins[i] = CRGB(0,0,15);
    }
  } else if (currentState == HEART_EYES) {
    int t = millis();
    eyeFutureL.drawOffset(0, heartEye, 2, (t / 50) % 2, 0);
    eyeFutureR.drawOffset(0, heartEye, 2, 1-(t / 50) % 2, 0);
    for (int i = 0; i < NUM_LEDS_FINS; i++) {
      leds_Fins[i] = CRGB(20,3,3);
    }

    if (emoteTimer > 1000) setNewState(SMILING);
    
  } else if (currentState == SHOCK) {
    eyeFutureL.drawOffset(0, boringEye, 2, -1, 0);
    eyeFutureR.drawSprite(0, boringEye, 2);
    Serial.println("Drawing shocked eyes to buffer...");
    
    for (int i = 0; i < NUM_LEDS_FINS; i++) {
      leds_Fins[i] = CRGB(0,0,15);
    }
    
    if (emoteTimer > 100) setNewState(SMILING);
    
  } else if (currentState == ERROR404) {
    eyeFutureL.drawSprite(0, errorCode, 2);
    eyeFutureR.drawSprite(0, errorCode, 2);
    Serial.println("Drawing ERROR404 to buffer...");
    
    for (int i = 0; i < NUM_LEDS_FINS; i++) {
      leds_Fins[i] = CRGB(20,0,0);
    }
    
    if (emoteTimer > 1000) setNewState(SMILING);
  } else if (currentState == BOOPED) {
    eyeFutureL.drawSprite(0, boopEyeR, 2);
    eyeFutureR.drawSprite(0, boopEyeL, 2);

    int runningTime = millis() - stateChangeTime;

    int maxR = NUM_LEDS_FINS*3;
    int maxB = NUM_LEDS_FINS*3;
    if (runningTime < 100) {
      maxR = runningTime;
      maxB = 20+runningTime;
    }
    
    for (int i = 0; i < NUM_LEDS_FINS; ++i) {
      leds_Fins[i] = CRGB(
        (i*1 + runningTime/5) % maxR,
        0,
        (i*3 + runningTime/5) % maxB
        );
    }
    
    if (emoteTimer > 100) setNewState(SMILING);
  } else if (currentState == DINOSAUR_GAME) {
    drawSmilingEyes(&eyeFutureL, &eyeFutureR);
    
    CRGB output(0,15,0);
    
    bool gameOngoing = DinoGame::runGame(&mouthBufferL, &output, getBoop());
    mouthBufferR.drawSprite(0, mouthBufferL.buffer, 4);
    
    for (int i = 0; i < NUM_LEDS_FINS; ++i) {
      leds_Fins[i] = output;
    }

    if (!gameOngoing) {
      setNewState(SMILING);
    }
    
  }
  
  drawInterpolatedEyes(&eyeBufferL, eyeFutureL.buffer, eyeBufferL_hold.buffer);
  drawInterpolatedEyes(&eyeBufferR, eyeFutureR.buffer, eyeBufferR_hold.buffer);

  snootBuffer.drawSprite(0, snoot, 2);


  int hor = 0;
  int ver = 0;
  int delta = 0;
  if (IMUavailable) {
    // Need to wait for new data somehow
    sensors_event_t a, g, temp;
    // THIS ISN'T THE PROBLEM
    mpu.getEvent(&a, &g, &temp);
  
    // Z is yaw
    // Y is pitch
    // X is roll
  
    hor = g.gyro.z*1.25;
    ver = -g.gyro.y*1.25;
    delta = g.gyro.x*1.25;
  }
  
  matrix.drawOffset(0, eyeBufferL.buffer, 2, hor, ver+delta);
  matrix.drawOffset(12, eyeBufferR.buffer, 2, hor, ver-delta);
  matrix.drawSprite(2, mouthBufferL.buffer, 4);
  matrix.drawSprite(6, snootBuffer.buffer, 2);
  matrix.drawSprite(8, mouthBufferR.buffer, 4);
  matrix.display();

  if (millis() - lastReboot > 700) {
    matrix.reboot(displayBrightness);
    lastReboot = millis();
  }

  FastLED.show();
}
