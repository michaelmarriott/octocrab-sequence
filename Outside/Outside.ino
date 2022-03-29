
#include "FastLED.h"                                          // FastLED library.

//SPEFIC VARIBALES
#define NUM_LEDS_PER_STRIP 750
#define NUM_LEDS_OF_ROWS 5
#define NUM_STRIPS 4

int divider = 5;
int minOscillator = 5;
int delayOnColorWipeRain = 1;
bool revesableOnColorWipeRain = true;
int twinkleAmounts = 5;

// CHANGED ACCROSS SEQ

CRGB colorOC[NUM_STRIPS * NUM_LEDS_OF_ROWS]; //OSCILATECOMPLEX
int rdms[NUM_STRIPS * NUM_LEDS_OF_ROWS][3]; //OSCILATECOMPLEX

//

struct CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

CRGB listOfColors[14]; //List of predefined colors

int pattern = 7; // What pattern to start playing?

int loopCounter = 0; // ALWAYS RESET TO 0 WHEN PATTERN CHANGES

int brightness = 210; //255 is max
bool isStarted = true; //SET TO FALSE WHEN WORKING WITH PI AS CONTROLLER!
int mainDelay = 0;

int beginDelay = 200;
int patternSpeed = 200 / divider;
int shootingSpeed = 2000 / divider;
int colourDelayLoop = beginDelay;
int twinkleSpeed = 20;

int ChangeColorNumber = 0;

void setup() {
  delay(100);//Safe Gaurd

  // put your setup code here, to run once:
  LEDS.addLeds<NEOPIXEL, NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP);
  LEDS.setBrightness(brightness);
  SetListOfColors(listOfColors);

  Serial.setTimeout(50);
  Serial.flush();
  while ( Serial.available() ) Serial.read();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (isStarted) {
    EVERY_N_MILLIS_I(thisTimer, beginDelay) {
      thisTimer.setPeriod(mainDelay);
      PatternSchedule(loopCounter);
      loopCounter = loopCounter + 1;
    }
  }
  int startChar = Serial.read();
  SerialRead(startChar);
}

// Listen to incoming commands to sync
void SerialRead(int startChar) {
  if (startChar == '{') {
    Serial.println("SerialRead");
    //000 000 000
    //Content-Type: text/html
    String result = Serial.readStringUntil('}');
    int nextPattern = (result.substring(0, 3).toInt());
    int nextBrightness = (result.substring(4, 7).toInt());
    int nextColour = (result.substring(8, 11).toInt());

    if (nextBrightness != 0) {
      brightness = nextBrightness;
    }
    if (nextColour != 0) {
      ChangeColorNumber = nextColour;
    }
    if (nextPattern != pattern) {
      loopCounter = 0;
      pattern = nextPattern;
    }

  } else if (startChar == '%') {
    pattern += 1;
    String result = Serial.readStringUntil('\n');
  } else if (startChar == '?') {
    //Check to see if TEENSY
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.write(',');
    Serial.print(0);
    Serial.println();
    isStarted = true;
  } else if (startChar >= 0) {
    // discard unknown characters
  }
}


// Pattern schedule
void PatternSchedule(int loopCounter) {
  switch (pattern) {
    case 1:
      mainDelay = patternSpeed;
      OscialateComplexSequenceWrapper(minOscillator);  // fix tail to different length
      break;
    case 2:
      mainDelay = patternSpeed;
      TheaterChaseRainbowSequenceWrapper(1);
      break;
    case 3:
      mainDelay = patternSpeed;
      FireSequenceWrapper();
      break;
    case 4:
      mainDelay = shootingSpeed;
      //chnage to have 5-8 run down, 1-4 run up.
      ColorWipeRainSequenceWrapper(revesableOnColorWipeRain, delayOnColorWipeRain); //100 at 150 pixels, maybe run all the to tail![chnage color every 2nd time.
      break;
    case 5:
      mainDelay = colourDelayLoop;
      //chnage to have 5-8 run down, 1-4 run up.
      ChangeColourSequenceWrapper(20); //100 at 150 pixels, maybe run all the to tail![chnage color every 2nd time.
      break;
    case 6:
      mainDelay = twinkleSpeed;
      TwinkleWrapper(twinkleAmounts);
      break;
    case 7:
      mainDelay = twinkleSpeed;
      TwinkleMapPixelsWrapper();
      break;
    case 8:
      mainDelay = twinkleSpeed;
      Strobe(0xff, 0x77, 0x22, 10, 50, 1000);
      break;
    default:
      pattern = 1;
      loopCounter = 0;
      break;
  }
}

//PATTERNS
//-------------------------------------------
//STROBE
void Strobe(byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int EndPause) {
  for (int j = 0; j < StrobeCount; j++) {
    setAll(red, green, blue);
    FastLED.show();
    delay(FlashDelay);
    setAll(0, 0, 0);
    FastLED.show();
    delay(FlashDelay);
  }

  delay(EndPause);
}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS_PER_STRIP * NUM_STRIPS; i++ ) {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}

// Set a LED color (not yet visible)
void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}


//TWINKLE
enum { SteadyDim, GettingBrighter, GettingDimmerAgain };
uint8_t PixelState[NUM_LEDS_PER_STRIP * NUM_STRIPS];

// Base background color
#define BASE_COLOR       CRGB(0,0,0)

// Peak color to twinkle up to
#define PEAK_COLOR       CRGB(255,125,40)

// Currently set to brighten up a bit faster than it dims down,
// but this can be adjusted.
// Amount to increment the color by each loop as it gets brighter:
#define DELTA_COLOR_UP   CRGB(8,4,1)
// Amount to decrement the color by each loop as it gets dimmer:
#define DELTA_COLOR_DOWN CRGB(1,1,1)
#define CHANCE_OF_TWINKLE 25

void TwinkleMapPixelsWrapper() {

  // Serial.println("TwinkleMapPixelsWrapper");
  if (loopCounter == 0) {
    memset( PixelState, sizeof(PixelState), SteadyDim); // initialize all the pixels to SteadyDim.

    fill_solid( leds, NUM_LEDS_PER_STRIP * NUM_STRIPS, BASE_COLOR);
    FastLED.show();
    //  Serial.println("BASE");
  }
  TwinkleMapPixels();
  FastLED.show();
  FastLED.delay(20);
}

void TwinkleMapPixels()
{
  for ( uint16_t i = 0; i < NUM_LEDS_PER_STRIP * NUM_STRIPS; i++) {
    if ( PixelState[i] == SteadyDim) {
      // this pixels is currently: SteadyDim
      // so we randomly consider making it start getting brighter
      if ( random8() < CHANCE_OF_TWINKLE) {
        PixelState[i] = GettingBrighter;
      }
    } else if ( PixelState[i] == GettingBrighter ) {
      // this pixels is currently: GettingBrighter
      // so if it's at peak color, switch it to getting dimmer again
      if ( leds[i] >= PEAK_COLOR ) {
        PixelState[i] = GettingDimmerAgain;
      } else {
        // otherwise, just keep brightening it:
        leds[i] += DELTA_COLOR_UP;
      }
    } else { // getting dimmer again
      // this pixels is currently: GettingDimmerAgain
      // so if it's back to base color, switch it to steady dim
      if ( leds[i] <= BASE_COLOR ) {
        leds[i] = BASE_COLOR; // reset to exact base color, in case we overshot
        PixelState[i] = SteadyDim;
      } else {
        // otherwise, just keep dimming it down:
        leds[i] -= DELTA_COLOR_DOWN;
      }
    }
  }
}

void TwinkleWrapper(int twinkleAmounts) {
  //changing the third variable changes how quickly the lights fade

  // fadeToBlackBy( leds, NUM_STRIPS*NUM_LEDS_PER_STRIP, 10);
  for (int16_t i = 0; i < NUM_STRIPS * NUM_LEDS_PER_STRIP; i++) {
    leds[i] = makeDarker(leds[i], 5);
  }

  for (int16_t i = 0; i < twinkleAmounts; i++) {
    addTwinkle(240);
    random16_add_entropy( random());
  }

  FastLED.show();
}

CRGB makeDarker( const CRGB& color, fract8 howMuchDarker)
{
  CRGB newcolor = color;
  if (color.r > 10) {
    newcolor.r = newcolor.r - howMuchDarker;
  } else {
    newcolor.r = 5;
  }
  if (color.g > 10) {
    newcolor.g = newcolor.g - howMuchDarker;
  } else {
    newcolor.g = 5;
  }
  if (color.b > 10) {
    newcolor.b = newcolor.b - howMuchDarker;
  } else {
    newcolor.b = 5;
  }
  // Serial.print("Red");
  //    Serial.print(newcolor.r);
  //     Serial.print("Green");
  //      Serial.print(newcolor.g);
  //       Serial.print("Blue");
  //        Serial.println(newcolor.b);



  // newcolor.nscale8( 255 - howMuchDarker);
  return newcolor;
}

void addTwinkle( fract8 chanceOfGlitter) {
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_STRIPS * NUM_LEDS_PER_STRIP) ] += RandomColor(220, 254, 0, 125, 0, 50);
  }
}

//Change Colour Sequence
int hue = 2;
int sat = 200;
int bright = 100;
int bounceDirection = 0;

void ChangeColourSequenceWrapper(int minBright) {
  //  Serial.println("ChangeColourSequenceWrapper");
  random16_add_entropy( random());
  ChangeColourSequence(minBright);
  FastLED.show(); // display this frame
}

void ChangeColourSequence(int minBright) {
  //Serial.println("ChangeColourSequence");

  if (bounceDirection == 0) {
    bright++;
    bright++;
    if (bright >= 250) {
      bounceDirection = 1;
    }
  }

  if (bounceDirection == 1) {
    bright--;
    bright--;
    if (bright <= 30) {
      hue = Increment(hue, 0, 42, 4);
      sat = Increment(sat, 150, 254, 22);
      bounceDirection = 0;
    }
  }

  if (bright <= minBright) {
    colourDelayLoop = 10;
  } else if (bright <= 80 || bright >= 235) {
    colourDelayLoop = 80;
  } else {
    colourDelayLoop = 30;
  }

  for (int x = 0; x < NUM_STRIPS; x++) {
    for (int y = 0; y < NUM_LEDS_PER_STRIP; y++) {
      leds[(x * NUM_LEDS_PER_STRIP) + (y)] = CHSV(hue, sat, bright);
    }
  }

}

///Fire Sequence
#define COOLING  NUM_LEDS_PER_STRIP/4
#define SPARKING NUM_LEDS_PER_STRIP/1
bool gReverseDirection = false;

void FireSequenceWrapper() {
  random16_add_entropy( random());
  Fire2014();
  FastLED.show(); // display this frame
}

void Fire2014()
{
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS_PER_STRIP];

  for ( int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS_PER_STRIP) + 2));
  }

  for ( int k = NUM_LEDS_PER_STRIP - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(100, 255) );
  }

  for ( int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette( CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Orange,  CRGB(255, 250, 125)), colorindex);
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS_PER_STRIP - 1) - j;
    } else {
      pixelnumber = j;
    }
    for (uint16_t i = 0; i < NUM_STRIPS; i++) {
      leds[pixelnumber + (i * NUM_LEDS_PER_STRIP)] = color;
    }
  }
}

//ColorWipe Rain Sequence
void ColorWipeRainSequenceWrapper(bool allowReverse, int wait) {
  // Serial.println("ColorWipeRainWrapper");
  ColorWipeRainSequence(ChangeColor(), allowReverse, wait);
}

int amountOfAparts = 0;
int apart = 0;
int trailMax = 0;
int r  = 0;

void ColorWipeRainSequence(CRGB color, bool allowReverse, uint8_t wait) {
  // Serial.println("ColorWipeRainSequence");
  int reverse = 0;
  if (allowReverse) {
    r = Increment(r, 16, 30, 5);
    if (r % 2 == 0) {
      reverse = NUM_LEDS_PER_STRIP - 1;
    }
  }

  int colorWipeSpeed = wait;
  //random16_add_entropy(random());

  trailMax = Increment(trailMax, 4 * NUM_LEDS_OF_ROWS, 12 * NUM_LEDS_OF_ROWS, 3);
  apart = Increment(apart, 6 * NUM_LEDS_OF_ROWS, 20 * NUM_LEDS_OF_ROWS, 4);
  amountOfAparts = Increment(amountOfAparts, 5 * NUM_LEDS_OF_ROWS, 25 * NUM_LEDS_OF_ROWS, 6);
  for (int i = (0 - (trailMax)); i < NUM_LEDS_PER_STRIP + trailMax + (apart * amountOfAparts); i++) {

    for (int strip = 0; strip < NUM_STRIPS; strip++) {

      for (int t = 0; t < trailMax; t++) {

        if (i - t > 0 && i - t < NUM_LEDS_PER_STRIP) {
          leds[(strip * NUM_LEDS_PER_STRIP) + abs(reverse - (i - t))] = color;
          leds[(strip * NUM_LEDS_PER_STRIP) + abs(reverse - (i - t))].fadeToBlackBy(t * (int)(256 / trailMax));
        }
        for (int a = 1; a < amountOfAparts; a++) {
          int iapart = ((i - t) - (apart * a));
          if (iapart > 0 && (iapart) < NUM_LEDS_PER_STRIP) {
            leds[(strip * NUM_LEDS_PER_STRIP) + abs(reverse - iapart)] = color;
            leds[(strip * NUM_LEDS_PER_STRIP) + abs(reverse - iapart)].fadeToBlackBy(t * (int)(256 / trailMax));
          }
        }
      }
    }
    FastLED.show();
    delay(colorWipeSpeed);
    for (int strip = 0; strip < NUM_STRIPS; strip++) {
      for (int t = 0; t < trailMax; t++) {
        if (i - t > 0 && i - t < NUM_LEDS_PER_STRIP) {
          leds[(strip * NUM_LEDS_PER_STRIP) + abs(reverse - (i - t))].fadeToBlackBy(210);
        }
        for (int a = 1; a < amountOfAparts; a++) {
          int iapart = ((i - t) - (apart * a));
          if (iapart > 0 && (iapart) < NUM_LEDS_PER_STRIP) {
            leds[(strip * NUM_LEDS_PER_STRIP) + abs(reverse - 0)].fadeToBlackBy(250);
            leds[(strip * NUM_LEDS_PER_STRIP) + abs(reverse - iapart)].fadeToBlackBy(210);
          }
        }
      }

    }
  }
}

// THEATER Sequence

void TheaterChaseRainbowSequenceWrapper(int speed) {
  TheaterChaseSequenceRainbow(speed);
}
void TheaterChaseSequenceRainbow(int speed) {
  for (int j = 0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {

      for (int i = 0; i < NUM_LEDS_PER_STRIP; i = i + 3) {
        for (int strip = 0; strip < NUM_STRIPS; strip++) {
          leds[(strip * NUM_LEDS_PER_STRIP) + (i + q)] =  Wheel((i + j) % 255); //turn every third pixel on
        }
      }
      FastLED.show();
      delay(speed);

      for (int i = 0; i < NUM_LEDS_PER_STRIP; i = i + 3) {
        for (int strip = 0; strip < NUM_STRIPS; strip++) {
          leds[(strip * NUM_LEDS_PER_STRIP) + (i + q)] = CRGB::Black;
        }
      }
    }
  }
}

//OSCILATE COMPLEX
int sizeOfStrip = 0;

void OscialateComplexSequenceWrapper(int minimun) {
  if (loopCounter == 0 ) {
    ///  Serial.println("OscialateComplex");
    // CRGB wcolor = CRGB::WhiteSmoke;
    int middle = (NUM_LEDS_PER_STRIP / NUM_LEDS_OF_ROWS) / 2;
    for (int x = 0; x < NUM_STRIPS * NUM_LEDS_OF_ROWS; x++) {
      for (int y = 0; y < NUM_LEDS_PER_STRIP / NUM_LEDS_OF_ROWS; y++) {
        leds[(x * (NUM_LEDS_PER_STRIP / NUM_LEDS_OF_ROWS)) + (y)] = CRGB(25, 5, 2);
      }
    }
    FastLED.show(); // display this frame
    sizeOfStrip = Increment(sizeOfStrip, middle - 6, middle - 1, 2);
    for (int s = 0; s < (NUM_STRIPS * NUM_LEDS_OF_ROWS); s++) {
      rdms[s][0] = sizeOfStrip; //Size
      rdms[s][1] = 0;//COUNTER
      rdms[s][2] = 0;//0 = up, 1 =down;
    }

    //Got Out
    for (int s = 0; s < NUM_STRIPS * NUM_LEDS_OF_ROWS; s++) {
      colorOC[s] = CRGB(random8(), random8(), random8());
    }
    FastLED.show();
  }
  OscialateComplexSequence(minimun);
}

int middle = (NUM_LEDS_PER_STRIP / NUM_LEDS_OF_ROWS) / 2;
int colorSpeed = 1;

void OscialateComplexSequence(int minimun) {
  //  CRGB wcolor = CRGB::WhiteSmoke;
  for (int strip = 0; strip < (NUM_STRIPS * NUM_LEDS_OF_ROWS); strip++) {
    if (rdms[strip][1] <= 0) {// if you get to zero
      rdms[strip][2] = 0; //direction up
      rdms[strip][0] = random16(minimun, middle - 2);
      colorOC[strip] = CRGB(random8(), random8(), random8());
    }
    else if (rdms[strip][1] >= rdms[strip][0]) {// counter greater than size
      rdms[strip][2] = 1; //switch direction down
      colorOC[strip] = CRGB(random8(), random8(), random8());
    }

    if (rdms[strip][2] == 0) {// if direction up
      rdms[strip][1] = rdms[strip][1] + 1; //count up
    }
    else {
      rdms[strip][1] = rdms[strip][1] - 1; //count down
    }
  }

  // delay(colorSpeed);
  for (int strip = 0; strip < NUM_STRIPS * NUM_LEDS_OF_ROWS; strip++) {
    // Serial.println(rdms[strip][1]);
    leds[(strip * (NUM_LEDS_PER_STRIP / NUM_LEDS_OF_ROWS)) + (middle - rdms[strip][1])] = colorOC[strip];
    leds[(strip * (NUM_LEDS_PER_STRIP / NUM_LEDS_OF_ROWS)) + (middle + rdms[strip][1])] = colorOC[strip];
  }

  FastLED.show();
}

//---------------------------------------------------------------------------------

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

CRGB ChangeColor() {
  ChangeColorNumber = ChangeColorNumber + 1;
  if (ChangeColorNumber > 13) {
    ChangeColorNumber = 0;
  }
  return listOfColors[ChangeColorNumber];
}

CRGB RandomColor(uint8_t minR, uint8_t maxR, uint8_t minG, uint8_t maxG , uint8_t minB , uint8_t maxB) {
  CRGB randomColor = CRGB(random8(minR, maxR), random8(minG, maxG), random8(minB, maxB));
  return randomColor;
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

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
CRGB Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void SetListOfColors(CRGB listOfColors[14]) {
  listOfColors[0] = CRGB(162, 58, 136);
  listOfColors[1] = CRGB(87, 210, 95);
  listOfColors[2] = CRGB(219, 30, 90);
  listOfColors[3] = CRGB(204, 126, 189);
  listOfColors[4] = CRGB(56, 58, 136);
  listOfColors[5] = CRGB(212, 58, 136);
  listOfColors[6] = CRGB(162, 21, 136);
  listOfColors[7] = CRGB(162, 58, 181);
  listOfColors[8] = CRGB(162, 123, 136);
  listOfColors[9] = CRGB(215, 58, 136);
  listOfColors[10] = CRGB(162, 4, 136);
  listOfColors[11] = CRGB(162, 58, 109);
  listOfColors[12] = CRGB(162, 179, 136);
  listOfColors[13] = CRGB(28, 58, 136);
}
