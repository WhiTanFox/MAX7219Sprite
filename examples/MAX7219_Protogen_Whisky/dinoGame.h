namespace DinoGame {
const uint8_t ground[8] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0xff,
};

const uint8_t tree[8] = {
  0,
  0,
  0,
  0b00100000,
  0b01110000,
  0b01110000,
  0b00100000,
  0b00100000,
};

const uint8_t dinoBoi[8] = {
  0,
  0b00001110,
  0b00001010,
  0b00001111,
  0b00011100,
  0b00011100,
  0b00011110,
  0,
};

splerp::RenderTarget jumpyBoi(1);

long int buttonPressTime = -10000;
uint8_t lastButton = 0;

uint8_t stillAlive = 1;

long int lastFrame = 0;
long int gameStartTime = 0;
long int gameEndTime = 0;

const int jumpFrames = 9;

int score = 0;

const uint8_t jumpYs[jumpFrames] = {
  1,
  4,
  5,5,5,5,5,
  4,
  2,
};

bool scoredForThisTime = 0;

bool runGame(splerp::RenderTarget * outputTarget, CRGB * LED_Colour, uint8_t button) {
  long int frameTime = millis();

  // Catch overflow on frameTime... yeahhhh...
  if (frameTime < 0) {
    // Panic!!!
    return false;
  }
  
  if (frameTime - lastFrame > 1000) {
    // Reset game state... whatever that means?
    
    gameStartTime = frameTime;
    gameEndTime = 0;
    
    score = 0;
    buttonPressTime = -10000;
    lastButton = 1;
    
    stillAlive = 1;
    
    lastFrame = 0;
    
    score = 0;
    scoredForThisTime = 0;
  }
  
  long int gameTime = frameTime - gameStartTime;
  Serial.println(gameTime);
  

  // Manage our input. Singular.
  if (button && !lastButton) {
    buttonPressTime = gameTime;
  }
  
  // Clear output frames:
  outputTarget -> drawBlank(0,4);

  // Override our animation timing if we're dead.
  long int motionTime = gameTime;
  if (!stillAlive) {
    motionTime = gameEndTime;
  }
  int stepTime = constrain(200 - (motionTime / 500), 0, 200) + 20;
  
  // Put our dino in a frame.
  int y = (motionTime/(stepTime/2))%2;
  if (motionTime - buttonPressTime < stepTime*jumpFrames) {
    int jumpFrame = (motionTime - buttonPressTime)/stepTime;
    y = jumpYs[jumpFrame];
  }
  jumpyBoi.drawOffset(0,dinoBoi,1,0,y);
  
  // Draw our tree
  int x = 32-(motionTime/stepTime)%32;
  int ind = x/8;
  outputTarget -> drawOffset(ind, tree, 1, x%8,0);
  if (ind+1 <= 4) {
    outputTarget -> drawOffset(ind+1,tree,1,-8+x%8,0);
  }

  // Detect collision and end the game if so.
  if (x == 5) {
    if (!(motionTime - buttonPressTime < stepTime*jumpFrames)) {
        if (stillAlive) {
          gameEndTime = gameTime;
        }
        stillAlive = false;
    } else {
      if (!scoredForThisTime) {
        score++;
        scoredForThisTime = 1;
      }
    }
  } else {
    scoredForThisTime = 0;
  }
  
  // Drop our jumpyBoi on the screen
  outputTarget -> orMatrix(0, outputTarget->buffer, jumpyBoi.buffer, 1);
  
  // Use bitwise operators to draw the ground under our feet.
  for (int i = 0; i < 4; ++i) {
    outputTarget -> orMatrix(i,outputTarget -> buffer+(i*8),ground,1);
  }

  if (stillAlive || (gameTime/250) % 2) {
    for (int i = 0; i < score%8;++i) {
      outputTarget -> buffer[16+0] |= (1<<i);
    }
    for (int i = 0; i < (score/8)%8;++i) {
      outputTarget -> buffer[16+1] |= (1<<i); 
    }
    for (int i = 0; i < (score/64)%8;++i) {
      outputTarget -> buffer[16+2] |= (1<<i);
    }
  }

  LED_Colour -> g = constrain(score*5, 0, 255);
  LED_Colour -> r = constrain(score/4, 0, 255);
  
  // Draw "game over", or something
  if (!stillAlive) {
    //outputTarget -> drawSprite(2, errorCode, 2);
    LED_Colour -> r = 100;
    LED_Colour -> g = 0;
    LED_Colour -> b = 0;
    if (gameTime - gameEndTime > 5000) {
      return false;
    }
  }

  // Clean up:
  lastButton = button;
  lastFrame = frameTime;
  
  return true;
}
}
