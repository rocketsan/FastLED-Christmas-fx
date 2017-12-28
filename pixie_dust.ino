/* f/x Pixie Dust for FastLED
 *
 * By: rocketsan
 * 
 * Date: December, 2017
 *
 * replication from youtube video by Stephen Culley:
 * https://www.youtube.com/watch?v=3Io4OeBP2GQ
 * 
 */

#include <FastLED.h>

#define NUM_LEDS     98
#define PIN          6
#define BRIGHTNESS   128

#define time4RotateColor 10                     // time interval for color rotate in seconds (R->G->B)

CRGB leds[NUM_LEDS];

enum { GettingBrighter, GettingDimmerAgain };
uint8_t PixelState[NUM_LEDS];                   // for dimming effect
int16_t brightness_val[NUM_LEDS];               // for dimming effect
int16_t hue_val[NUM_LEDS];                      // for "rotate color" effect

CHSV mainHSVColor;
uint8_t deltaUp = 2;                            // for dimming effect
uint8_t deltaDown = 2;                          // for dimming effect
uint8_t dustSize = 5;                           // (+1 white)
uint8_t baseVal;
uint8_t peakVal;
unsigned long last_switch_time;
uint8_t dustPos = 255; // over limits

void setup() {
  delay(1000);                                  // Soft startup to ease the flow of electrons.
  Serial.begin(115200);

  FastLED.addLeds<WS2811, PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);  

  FastLED.clear();

  //mainHSVColor = rgb2hsv_approximate(CRGB(255,0,0));  //convert rgb to hsv
  mainHSVColor = CHSV(0,255,128); // RED
 
  fill_solid( leds, NUM_LEDS, mainHSVColor);
  FastLED.show();

  baseVal = mainHSVColor.val - 32;              // low brightness amplitude
  peakVal = mainHSVColor.val + 32;              // high brightness amplitude

  // init arrays
  for(uint16_t i=0; i<NUM_LEDS; i++) {
    PixelState[i] = (random8(2)==0) ? GettingBrighter : GettingDimmerAgain;
    brightness_val[i] = mainHSVColor.val;
    hue_val[i] = mainHSVColor.hue;
  }

  last_switch_time=millis();  
}

void loop() {
  pixie_dust();
    
  delay(30);
  FastLED.show();  //Update display
}

uint8_t next_color(uint8_t hue) {
  if (hue==0) return 85; // R->G
  if (hue==85) return 171; // G->B
  if (hue==171) return 0; // B->R
}

void pixie_dust() {
  uint8_t curSec = (millis()-last_switch_time)/1000;
  if (curSec>time4RotateColor && dustPos>=NUM_LEDS) {
     dustPos = 0; // start dust!
     last_switch_time=millis();
  }
  
  for(uint16_t i=0; i<NUM_LEDS; i++) {
    if( PixelState[i] == GettingBrighter ) {
      if( brightness_val[i] >= peakVal ) {
        PixelState[i] = GettingDimmerAgain;
      } else {
        brightness_val[i] = qadd8(brightness_val[i], deltaUp); // not to exceed the 255 value
      }    
    } else if( PixelState[i] == GettingDimmerAgain ) {
      if( brightness_val[i] <= baseVal ) {
        PixelState[i] = GettingBrighter;
        brightness_val[i] = baseVal;
      } else {
        brightness_val[i] -= deltaDown;
      }
    }
    leds[i] = CHSV(hue_val[i],255,brightness_val[i]);    
  }

  // move dust up over the led srtip
  if (dustPos<NUM_LEDS) {
    leds[dustPos] = rgb2hsv_approximate(CRGB(255,255,255)); // bright white
    hue_val[dustPos] = next_color(hue_val[dustPos]);
  }
  
  // display dust tail
  for (uint8_t k=1;k<dustSize;k++)
    if (dustPos-k>=0 && dustPos-k<NUM_LEDS)
      leds[dustPos-k] = CHSV(random8(), 255, 255); // random color

  if (dustPos<NUM_LEDS+dustSize-1) dustPos++; // move dust upper
}

