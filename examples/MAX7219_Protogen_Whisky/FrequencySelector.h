
// For now, just, like, don't. :(
#if FALSE

#ifndef FREQUENCYSELECTOR_H
#define FREQUENCYSELECTOR_H

void setupFrequencySelector() {
  // Put the ADC in party mode (free running, triggering interrupts!)
  cli();
  ADMUX |= (1 << REFS0); // Set ADC reference to AVCC
  
  ADCSRA |= (1 << ADEN);  // Enable ADC
  ADCSRA |= (1 << ADATE); // Enable auto-triggering
  ADCSRA |= (1 << ADIE);  // Enable ADC Interrupt
  
  //ADCSRA &= ~(1<<ADPS2);
  ADCSRA |=  (1<<ADPS2);
  //ADCSRA &= ~(1<<ADPS1);
  ADCSRA |=  (1<<ADPS1);
  //ADCSRA &= ~(1<<ADPS0);
  ADCSRA |=  (1<<ADPS0);
  
  sei();                 // Enable Global Interrupts
  
  ADCSRA |= (1 << ADSC);  // Start A2D Conversions
}

struct Sample {
  uint16_t frequency;
  uint16_t amplitude;
};


const int testTimeInterval = 20000;


// Variables in our interrupt
int32_t DCbias = 338;
int32_t nextTestTime = testTimeInterval;
int8_t lastSign = 0;
int32_t lastRisingEdge = 0;
const int queueSize = 32;
int32_t lastPeriod[queueSize] = {0};
uint8_t queueHead = 0;

// Variables used beyond the interrupt
const int sampleCount = 64;
Sample samples[sampleCount];
int sampleHead = 0;

int32_t envelope = 0;
int32_t noisefloor = 0;

static inline int8_t sign(int val) {
  return (val>0)-(val<0);
}

ISR(ADC_vect) {
  uint16_t high,low,rawin;
  low = ADCL;   //Make certain to read ADCL first, it locks the values
  high = ADCH;  //and ADCH releases them.
  rawin = (high << 8) | low;

  DCbias = (DCbias * 1014 + rawin * 10) / 1024;
  int input = rawin - DCbias;
  int8_t thisSign = sign(input);
  
  if (thisSign > lastSign) {
    queueHead = (queueHead + 1) % queueSize;
    lastPeriod[queueHead] = micros() - lastRisingEdge;
    lastRisingEdge = micros();
  }
  
  /*int absin = abs(input);
  if (absin > envelope) {
    envelope = absin;
  } else {
    envelope = (envelope * 63) / 64;
  }

  noisefloor = (noisefloor * 4086 + envelope * 10) / 4096;
  */
  lastSign = thisSign;
}


void updateFrequencySelector() {
  if (micros() > nextTestTime) {
    nextTestTime += testTimeInterval;
    int32_t avgPeriod = 0;
    
    for (int i = 0; i < queueSize; ++i) {
      avgPeriod += lastPeriod[i];
    }
    
    avgPeriod /= queueSize;
    int32_t frequency = 1000000/avgPeriod;
    Serial.print(avgPeriod/1000);
    Serial.print('\t');
    Serial.print(DCbias);
    Serial.print('\t');
    Serial.print(frequency);
    Serial.print('\t');
    Serial.print(envelope);
    Serial.print('\t');
    Serial.print(noisefloor);
    //Serial.print('\t');
    //Serial.print(envelope > noisefloor ? "1000" : "0");
    Serial.println();
  }
}

#endif
#endif