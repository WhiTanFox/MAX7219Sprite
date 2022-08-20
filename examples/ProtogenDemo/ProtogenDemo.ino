#include <SPI.h>
#include "MAX7219Sprite.h"
#include "SpriteData.h"

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

void setup() {
  Serial.begin(115200);
  //setupFrequencySelector();

  // We only need to setup the display target once, and only for the output matrix itself
  matrix.setupDisplayTarget(0x02);
  
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

  delay(1000);
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
  if (millis() - lastManual > 10000) {
    demoMode = true;
    if (millis() - stateUpdateTime > 1000) {
      setNewState((millis() / 1000) % N_Options);
    }
  }

  getBoop();
  
  long int emoteTimer = millis() - stateUpdateTime;
  
  if (currentState == SMILING) {
    drawSmilingEyes(&eyeFutureL, &eyeFutureR);
  } else if (currentState == HEART_EYES) {
    int t = millis();
    eyeFutureL.drawOffset(0, heartEye, 2, (t / 50) % 2, 0);
    eyeFutureR.drawOffset(0, heartEye, 2, 1-(t / 50) % 2, 0);

    if (emoteTimer > 1000 && !demoMode) setNewState(SMILING);
    
  } else if (currentState == SHOCK) {
    eyeFutureL.drawOffset(0, boringEye, 2, -1, 0);
    eyeFutureR.drawSprite(0, boringEye, 2);
    //Serial.println("Drawing shocked eyes to buffer...");
    
    if (emoteTimer > 100 && !demoMode) setNewState(SMILING);
    
  } else if (currentState == ERROR404) {
    eyeFutureL.drawSprite(0, errorCode, 2);
    eyeFutureR.drawSprite(0, errorCode, 2);
    //Serial.println("Drawing ERROR404 to buffer...");
    
    if (emoteTimer > 1000 && !demoMode) setNewState(SMILING);
  } else if (currentState == BOOPED) {
    eyeFutureL.drawSprite(0, boopEyeR, 2);
    eyeFutureR.drawSprite(0, boopEyeL, 2);

    int runningTime = millis() - stateChangeTime;
    
    if (emoteTimer > 100 && !demoMode) setNewState(SMILING);
  }
  
  drawInterpolatedEyes(&eyeBufferL, eyeFutureL.buffer, eyeBufferL_hold.buffer);
  drawInterpolatedEyes(&eyeBufferR, eyeFutureR.buffer, eyeBufferR_hold.buffer);
  mouthBufferL.drawSprite(0, mouthL, 4);
  mouthBufferR.drawSprite(0, mouthR, 4);
  snootBuffer.drawSprite(0, snoot, 2);
  

  matrix.drawOffset(0, eyeBufferL.buffer, 2, (millis()/500)%2 - 1, 0);
  matrix.drawOffset(12, eyeBufferR.buffer, 2, -(millis()/500)%2 + 1, 0);

  matrix.drawSprite(2, mouthBufferL.buffer, 4);
  matrix.drawSprite(6, snootBuffer.buffer, 2);
  matrix.drawSprite(8, mouthBufferR.buffer, 4);
  
  long int t = micros();
  matrix.display();
  long int t2 = micros();
  Serial.print(t - startTime);
  Serial.print(",");
  Serial.print(t2 - t);
  Serial.print(",");
  Serial.print(t2 - startTime);
  Serial.println();
}