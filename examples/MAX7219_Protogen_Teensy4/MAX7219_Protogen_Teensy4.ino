
#define DEMOENABLED (false)

#include <SPI.h>

#include <OctoWS2811.h>

#include "MAX7219Sprite.h"
#include "SpriteData.h"


#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Initialize the main display matrix
// arg0 is the number of 8x8 segments described in the flame buffer.
// arg1 is the CS pin in use for our matrix.
splerp::DisplayTarget matrix(14, 9);

// Create some frame buffers to use as intermediate rendering targets.
// Since this is going to be used for the eye animation, we only need
//  to include two MAX7219's worth of memory.
splerp::RenderTarget eyeBufferL(2);
splerp::RenderTarget eyeBufferR(2);

splerp::RenderTarget mouthBufferL(4);
splerp::RenderTarget mouthBufferR(4);

splerp::RenderTarget snootBuffer(2);

Adafruit_MPU6050 mpu;

const int numPins = 8;
byte pinList[numPins] = {2, 3, 4, 5, 17, 16, 21, 20,};

byte pinBoop = 22;
byte pinButton = 6;

#define CH_Ear1 0
#define CH_Ear2 1
#define CH_Ear3 2
#define CH_Ear4 3
#define CH_Cheek1 4
#define CH_Cheek2 5
#define CH_Aux1 6
#define CH_Aux2 7

const int ledsPerCheek = 38;
const int ledsPerAux = 50;
const int ledsPerEar = 0; // Big oof, lol

const int LEDconfig = WS2811_GRB | WS2811_800kHz;

const int ledsPerStrip = 51;//max(ledsPerCheek, max(ledsPerAux, ledsPerEar));

const int bytesPerLED = 3;
DMAMEM int displayMemory[ledsPerStrip * numPins * bytesPerLED / 4];
int drawingMemory[ledsPerStrip * numPins * bytesPerLED / 4];

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, LEDconfig, numPins, pinList);

uint32_t gRGB(uint8_t R, uint8_t G, uint8_t B) {
  uint32_t col = (((uint32_t)R) << 16) | (((uint32_t)G) << 8) | (((uint32_t)B) << 0);
  return col;
}
//
//#define NUM_LEDS_FINS 38
//#define CHIPSET     WS2812
//#define COLOR_ORDER GRB
//CRGB leds_Fins0[NUM_LEDS_FINS];
//CRGB leds_Fins1[NUM_LEDS_FINS];

const int T_interpolate = 80;

bool IMUavailable = true;

void debug(const char * str) {
  Serial.println(str);
}

void setLEDsGlowy() {
  for (int i = 0; i < leds.numPixels()/numPins; ++i) {
    uint16_t t = millis()/10 + i * 256/ledsPerStrip;
    leds.setPixel(i+ledsPerStrip*0, gRGB(t % 256 / 2, 0, 0));
    leds.setPixel(i+ledsPerStrip*1, gRGB(0, t % 256 / 2, 0));
    leds.setPixel(i+ledsPerStrip*2, gRGB(0, 0, t % 256 / 2));
    leds.setPixel(i+ledsPerStrip*3, gRGB(t % 256 / 2, t % 256 / 2, 0));
    leds.setPixel(i+ledsPerStrip*4, gRGB(0, t % 256, t % 256));
    leds.setPixel(i+ledsPerStrip*5, gRGB(t % 256, 0, t % 256));
    leds.setPixel(i+ledsPerStrip*6, gRGB(t % 256, t % 256, 0));
    leds.setPixel(i+ledsPerStrip*7, gRGB(t % 256, t % 256, 0));
  }
}

void setup() {
  delay(1000); // Sit with thumb up but for a while to give the power supply time to stabilize.
  
  Serial.begin(115200);
  debug("Serial on");
  delay(1000);
  //setupFrequencySelector();
  
  // We only need to setup the display target once, and only for the output matrix itself
  matrix.setupDisplayTarget(0x01);
  debug("Display target setup");
  
  // Fill all of our output buffers with a crosshatch pattern:
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

  // Copy all of our buffers to our output and throw it on the displays!
  matrix.drawSprite(0, mouthBufferR.buffer, 4);
  matrix.drawSprite(4, eyeBufferR.buffer, 2);
  matrix.drawSprite(6, snootBuffer.buffer, 2);
  matrix.drawSprite(8, eyeBufferL.buffer, 2);
  matrix.drawSprite(10, mouthBufferL.buffer, 4);
  matrix.display();
  debug("Matrices drawn");

  leds.begin();
  debug("WS2812 LED output initialized");

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
  
  debug("Setting LEDs");
  setLEDsGlowy();
  debug("LEDs set, moving onto loop");
  
  pinMode(pinButton, INPUT_PULLUP);
  pinMode(pinBoop, INPUT);
  
  delay(50);
}


#define SMILING 0 
#define HEART_EYES 1
#define SHOCK 2
#define ERROR404 3
#define BOOPED 4
#define N_Options 5

// State tracking for things
long int stateChangeTime = 0;
long int stateUpdateTime = 0;
int currentState = SMILING;

long int lastManual = 0;
long int lastReboot = 0;

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
  bool in = !digitalRead(pinBoop);
  if (in && (!lastBoopState || (millis() - startofboop) < 5000)) {
    if (startofboop < 0) startofboop = millis();
    setNewState(BOOPED);
  }
  if (!in) {
    startofboop = -5000;
  }
  lastBoopState = in;
}

bool lastButtonState = 0;
long int lastButtonTime = 0;
int buttonMood = SMILING;
const int MaxButtonFace = (ERROR404);

void getButton() {
  bool in = !digitalRead(pinButton);
  if (in) {
    if (millis() - lastButtonTime > 20 && !lastButtonState) {
      buttonMood = (buttonMood+1)%(MaxButtonFace + 1);
      setNewState(buttonMood);
    }
    lastButtonTime = millis();
  }
  lastButtonState = in;
}

void loop() {
  long int startTime = micros();
  bool demoMode = false;
  
  if (Serial.available()) {
    char a = Serial.read();
    int ind = a - '0';
    if (ind >= 0 && ind < N_Options) {
      setNewState(ind);
      Serial.print("Triggered ");
      Serial.print(ind);
      Serial.println();
      lastManual = millis();
    }
  }


  // Demonstrate things!
  if (millis() - lastManual > 10000 && DEMOENABLED) {
    demoMode = true;
    if (millis() - stateUpdateTime > 1000) {
      setNewState((millis() / 1000) % N_Options);
    }
  }

  getBoop();
  getButton();
  
  long int emoteTimer = millis() - stateUpdateTime;
  
  if (currentState == SMILING) {
    drawSmilingEyes(&eyeFutureL, &eyeFutureR);

    for (int i = 0; i < leds.numPixels()/numPins; ++i) {
      uint16_t t = millis()/10 + i * 256/ledsPerStrip;
      leds.setPixel(i+ledsPerStrip*CH_Cheek1, gRGB(0, 60, 60));
      leds.setPixel(i+ledsPerStrip*CH_Cheek1, gRGB(0, 60, 60));
    }
  } else if (currentState == HEART_EYES) {
    int t = millis();
    eyeFutureL.drawOffset(0, heartEye, 2, (t / 80) % 2, 0);
    eyeFutureR.drawOffset(0, heartEye, 2, 1-(t / 80) % 2, 0);

    for (int i = 0; i < leds.numPixels()/numPins; ++i) {
      uint16_t t = millis()/10 + i * 256/ledsPerStrip;
      leds.setPixel(i+ledsPerStrip*CH_Cheek1, gRGB(100, 20, 20));
      leds.setPixel(i+ledsPerStrip*CH_Cheek1, gRGB(100, 20, 20));
    }

    //if (emoteTimer > 1000 && !demoMode) setNewState(SMILING);
  } else if (currentState == SHOCK) {
    eyeFutureL.drawOffset(0, boringEye, 2, -1, 0);
    eyeFutureR.drawSprite(0, boringEye, 2);
    //Serial.println("Drawing shocked eyes to buffer...");

    for (int i = 0; i < leds.numPixels()/numPins; ++i) {
      uint16_t t = millis()/10 + i * 256/ledsPerStrip;
      leds.setPixel(i+ledsPerStrip*CH_Cheek1, gRGB(0, 0, 60));
      leds.setPixel(i+ledsPerStrip*CH_Cheek1, gRGB(0, 0, 60));
    }
    
    //if (emoteTimer > 100 && !demoMode) setNewState(SMILING);
  } else if (currentState == ERROR404) {
    eyeFutureL.drawSprite(0, errorCode, 2);
    eyeFutureR.drawSprite(0, errorCode, 2);
    //Serial.println("Drawing ERROR404 to buffer...");
    
    for (int i = 0; i < leds.numPixels()/numPins; ++i) {
      uint16_t t = millis()/10 + i * 256/ledsPerStrip;
      leds.setPixel(i+ledsPerStrip*CH_Cheek1, gRGB(t % 256 / 2, 0, 0));
      leds.setPixel(i+ledsPerStrip*CH_Cheek1, gRGB(t % 256 / 20, 0, 0));
    }
    
    //if (emoteTimer > 1000 && !demoMode) setNewState(SMILING);
  } else if (currentState == BOOPED) {
    eyeFutureL.drawSprite(0, boopEyeR, 2);
    eyeFutureR.drawSprite(0, boopEyeL, 2);
    
    setLEDsGlowy();

    int runningTime = millis() - stateChangeTime;
    
    if (emoteTimer > 100 && !demoMode) setNewState(buttonMood);
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
  
  //matrix.drawOffset(0, eyeBufferL.buffer, 2, (millis()/500)%2 - 1, 0);
  //matrix.drawOffset(12, eyeBufferR.buffer, 2, -(millis()/500)%2 + 1, 0);

  matrix.drawOffset(0, eyeBufferL.buffer, 2, hor, ver+delta);
  matrix.drawOffset(12, eyeBufferR.buffer, 2, hor, ver-delta);
  matrix.drawSprite(2, mouthBufferL.buffer, 4);
  matrix.drawSprite(6, snootBuffer.buffer, 2);
  matrix.drawSprite(8, mouthBufferR.buffer, 4);

  if (millis() - lastReboot > 700) {
    matrix.reboot();
    lastReboot = millis();
    debug("Rebooted matrix");
  }
  leds.show();
//  long int t = micros();
  delay(5);
  matrix.display();
//  long int t2 = micros(); 
  
  
  
//  Serial.print(t - startTime);
//  Serial.print(",");
//  Serial.print(t2 - t);
//  Serial.print(",");
//  Serial.print(t2 - startTime);
//  Serial.println();

  delay(5);
}
