#include <SPI.h>
#include "MAX7219Sprite.h"
#include "SpriteData.h"

#include <FastLED.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>


Adafruit_MPU6050 mpu;


#define LED_PIN_FINS 2
#define LED_PIN_EARS 3
#define NUM_LEDS_EARS 39
#define NUM_LEDS_EARS_MISSING 15
#define NUM_LEDS_FINS 38
#define CHIPSET     WS2812
#define COLOR_ORDER GRB
CRGB leds_Ears[NUM_LEDS_EARS];
CRGB leds_Fins[NUM_LEDS_FINS];

// Initialize the main display matrix
// arg0 is the number of 8x8 segments described in the flame buffer.
// arg1 is the CS pin in use for our matrix.
splerp::DisplayTarget matrix(14, 8);

// Create some frame buffers to use as intermediate rendering targets.
// Since this is going to be used for the eye animation, we only need
//  to include two MAX7219's worth of memory.
splerp::RenderTarget eyeBufferL(2);
splerp::RenderTarget eyeBufferR(2);

splerp::RenderTarget mouthBufferL(4);
splerp::RenderTarget mouthBufferR(4);

splerp::RenderTarget snootBuffer(2);

const int T_interpolate = 80;

bool IMUavailable = true;

long int lastReboot = 0;

const int displayBrightness = 0x0E;

void setup() {
  Serial.begin(115200);
  //setupFrequencySelector();

  // We only need to setup the display target once, and only for the output matrix itself
  matrix.setupDisplayTarget(displayBrightness);

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

  FastLED.addLeds<CHIPSET, LED_PIN_EARS, COLOR_ORDER>(leds_Ears, NUM_LEDS_EARS);
  FastLED.addLeds<CHIPSET, LED_PIN_FINS, COLOR_ORDER>(leds_Fins, NUM_LEDS_FINS);
  
  // Turn on the ears
  for (int i = 0; i < NUM_LEDS_EARS; i++) {
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

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    IMUavailable = false;
    delay(100);
//    while (1) {
//      FastLED.showColor(CRGB(0, 0, 10));
//      delay(500);
//      FastLED.showColor(CRGB(10, 0, 0));
//      delay(500);
//    }
  }

  pinMode(A0, INPUT);

  if (IMUavailable) {
    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
  }
  
  delay(10);
  delay(1000);
}

#define SMILING 0 
#define HEART_EYES 1
#define SHOCK 2
#define ERROR404 3
#define BOOPED 4
#define N_Options 5


long int stateChangeTime = 0;
long int stateUpdateTime = 0;
int currentState = SMILING;

splerp::RenderTarget eyeBufferL_hold(2);
splerp::RenderTarget eyeBufferR_hold(2);

splerp::RenderTarget eyeFutureL(2);
splerp::RenderTarget eyeFutureR(2);

void drawSmilingEyes(splerp::RenderTarget * leftEye, splerp::RenderTarget * rightEye) {
  leftEye -> drawSprite(0, eyeL, 2);
  rightEye -> drawSprite(0, eyeR, 2);
}

// Latch our current eye output in the hold buffer.
void setNewState(int state) {
  stateUpdateTime = millis();
  if (currentState == state) return;
  stateChangeTime = millis();
  currentState = state;
  eyeBufferL_hold.drawSprite(0, eyeBufferL.buffer, 2);
  eyeBufferR_hold.drawSprite(0, eyeBufferR.buffer, 2);
}

void drawInterpolatedEyes(splerp::RenderTarget * target, uint8_t * eventual, uint8_t * previous) {
  long int t = (millis() - stateChangeTime)/(T_interpolate / 8);
  if (t < 8) {
    //eyeBufferL.drawLerp16(0, previous, eventual, t, 0b00111100, 0b11111100);
    target -> drawLerp16(0, previous, eventual, t, 0xFF, 0xFF);
  } else {
    target -> drawSprite(0, eventual, 2);
  }
}

int lastBoopState = 0;
long int startofboop = 0;
void getBoop() {
  bool in = !digitalRead(A0);
  if (in && (!lastBoopState || (millis() - startofboop) < 5000)) {
    if (startofboop < 0) startofboop = millis();
    setNewState(BOOPED);
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
  getBoop();
  
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
  }
  
  drawInterpolatedEyes(&eyeBufferL, eyeFutureL.buffer, eyeBufferL_hold.buffer);
  drawInterpolatedEyes(&eyeBufferR, eyeFutureR.buffer, eyeBufferR_hold.buffer);

  mouthBufferL.drawSprite(0, mouthL, 4);
  mouthBufferR.drawSprite(0, mouthR, 4);

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
