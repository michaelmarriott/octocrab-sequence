#include "FastLED.h"                                          // FastLED library.

#define NUM_LEDS_PER_STRIP 150
#define NUM_LEDS_PER TAIL 90
#define NUM_STRIPS 8
struct CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];
int rdms[NUM_STRIPS][3]; //OSCILATECOMPLEX

CRGB colorOC[NUM_STRIPS];  //OSCILATECOMPLEX



int loopCounter = 0; // ALWAYS RESET TO 0 WHEN PATTERN CHANGES
int pattern = 1; // What pattern playing boy?
int brightness = 100;
bool isStarted = true; //SET TO FALSE WHNE WORKING WITH PI!
int mainDelay = 100;
int beginDelay = 100;



void setup() {
  delay(100);//Safe Gaurd
  // put your setup code here, to run once:
  LEDS.addLeds<WS2811_PORTD, NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP);
  LEDS.setBrightness(brightness);
  colorOC[1] = CRGB(255, 255, 255); //WHITE
  colorOC[1] = CRGB(255, 255, 0);
  colorOC[2] = CRGB(255, 0, 0); //RED
  colorOC[3] = CRGB(0, 255, 255);
  colorOC[4] = CRGB(0, 255, 0); //GREEN
  colorOC[5] = CRGB(0, 0, 255); // BLUE
  colorOC[6] = CRGB(255, 0, 255); //PINK
  colorOC[7] = CRGB(125, 125, 125); //light white
}

void loop() {
  // put your main code here, to run repeatedly:
  if (isStarted) {
    EVERY_N_MILLIS_I(thisTimer, beginDelay) {
      PatternSchedule(loopCounter);
      loopCounter = loopCounter + 1;
      thisTimer.setPeriod(mainDelay);
    }
  }
}

/////PATTERN SCHEDULE
void PatternSchedule(int loopCounter) {
  switch (pattern) {
    case 1:
      Calibrate();
      break;

    default:
      pattern = 1;
      loopCounter = 0;
      break;
  }
}

//OSCILATE COMPLEX
int sizeOfStrip = 0;
void Calibrate() {
  Serial.println("OscialateComplexSequence");
  int colorSpeed = 1;
  int middle = NUM_LEDS_PER_STRIP / 2;
  //  CRGB wcolor = CRGB::WhiteSmoke;
  for (int strip = 0; strip < NUM_STRIPS; strip++) {
    for (int n = 0; n < NUM_LEDS_PER_STRIP; n++) {
      leds[(strip * NUM_LEDS_PER_STRIP) + n ] = colorOC[strip];
    }
  }

  SetSpeed(50);
  FastLED.show();
}


//FUNCTIONS
void SetBightness(int newBrightness) {
  if (brightness > 10) {
    LEDS.setBrightness(newBrightness);
  }
}

void SetPattern(int newPattern) {
  pattern = newPattern;
}
void SetColorPallet(int newColourPallet) {
}

void SetSpeed(int newSpeed) {
  mainDelay = newSpeed;
}

int Increment(int value, int minValue, int maxValue, int increaseBy) {
  int result = value + increaseBy;
  if (result > maxValue) {
    return minValue;
  }
  if (result < minValue) {
    return minValue;
  }
  return result;
}
