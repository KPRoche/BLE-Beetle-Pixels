/* Flashy Collar v2 -- with PowerDown
  Kevin Roche
  May 4, 2015

  starting from code in SparkFun WS2812 Breakout Board Example
  AND in the AdaFruit_NeoPixel strandtest example
*/
#include <LowPower.h>
#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"
#include <EEPROM.h>
#define FCVers "4c: matching belt"

#define PIN 2
#define LED_COUNT 38
#define BELT_COUNT 144
//the lapels are two strips this long
#define NO_OF_EYES 2
#define SCAN_WAIT 20
#define ALL_SCANS_COUNT 4

float belt2collar = BELT_COUNT / LED_COUNT;
int scaled_wait = SCAN_WAIT * belt2collar;

// Create an instance of the Adafruit_NeoPixel class called "collar".
Adafruit_NeoPixel collar = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);
byte ScannerTask = 0;
byte MaxTask = 24;

void clearLEDS();
void theaterChaseRainbow(uint8_t wait);
void colorFade(uint8_t wait, unsigned long color = WHITE,  byte stepsize = 25, byte maxBrightness = 64);
void theaterChaseColor(uint8_t wait, unsigned long color = WHITE );
void multi_cylon(unsigned long color, byte wait, int num_eyes = 3);
void multi_meteor(unsigned long color, byte wait, int num_eyes, boolean roundabout = false);
void rainbow(byte startPosition);
uint32_t rainbowOrder(byte position);
uint32_t Wheel(byte WheelPos);
void finland( byte wait, int num_eyes);
void AllScans();

void setup()
{
  pinMode(13, OUTPUT);


  Serial.begin(9600);
  // Read the stored task from last time and modulo it
  Serial.print(F("Flashy Collar with Sleep version "));
  Serial.println(FCVers);
  ScannerTask = EEPROM.read(0);
  Serial.print("Stored ScannerTask: ");
  Serial.println(ScannerTask);
  ScannerTask++;
  if (ScannerTask > MaxTask) {
    ScannerTask = 0;
  }
  // store it to eeprom
  EEPROM.write(0, ScannerTask);
  collar.begin();  // Call this to start up the LED strip.

  clearLEDS();   // This function, defined below, turns all collar off...
  collar.show();   // ...but the collar don't actually update until you call this.

  collar.setBrightness(64);


}

void loop()
{
  digitalWrite(13, HIGH);
  delay(5);
  digitalWrite(13, LOW);
  collar.setBrightness(64);

  switch (ScannerTask) {
    case 0:
      Serial.println(F("Starting AllScans"));
      AllScans();
      break;
    case 1:
      Serial.println(F("Starting rainbow"));
      for (int i = LED_COUNT * 10; i >= 0; i--)
      {
        rainbow(i);
        delay(20);  // Delay between rainbow slides
      }
      break;
    case 2:
      Serial.println(F("Starting Larson scanner"));
      // cylon function: first param is color, second is time (in ms) between cycles
      multi_cylon(INDIGO, scaled_wait, NO_OF_EYES); // Indigo cylon eye!
      multi_cylon(BLUE, scaled_wait, NO_OF_EYES); // Blue  multi_cylon eye!
      multi_cylon(GREEN, scaled_wait, NO_OF_EYES); // GREEN  multi_cylon eye!
      multi_cylon(YELLOW, scaled_wait, NO_OF_EYES); // YELLOW  multi_cylon eye!
      multi_cylon(ORANGERED, scaled_wait, NO_OF_EYES); // ORANGERED  multi_cylon eye!
      multi_cylon(DARKRED, scaled_wait, NO_OF_EYES); // DARKRED cylon eye!
      break;
    case 3:
      Serial.println(F("Starting Theater Chase"));
      theaterChaseRainbow(50);
      break;
    case 4:
      Serial.println(F("Starting meteor"));
      multi_meteor(INDIGO, scaled_wait / 2, NO_OF_EYES); // Indigo meteor eye!
      multi_meteor(BLUE, scaled_wait / 2, NO_OF_EYES); // BLUE  multi_meteor eye!
      multi_meteor(GREEN, scaled_wait / 2, NO_OF_EYES); // GREEN  multi_meteor eye!
      multi_meteor(YELLOW, scaled_wait / 2, NO_OF_EYES); // YELLOW  multi_meteor eye!
      multi_meteor(ORANGERED, scaled_wait / 2, NO_OF_EYES); // ORANGERED  multi_meteor eye!
      multi_meteor(DARKRED, scaled_wait / 2, NO_OF_EYES); // RED meteor eye!
      break;
    case 5:
      Serial.println(F("Starting White Cylon"));
      multi_cylon(WHITE, scaled_wait, NO_OF_EYES); // WHITE cylon eye!
      break;
    case 6:
      Serial.println(F("Starting White meteor"));
      multi_meteor(WHITE, scaled_wait / 2, NO_OF_EYES); // WHITE meteor eye!
      break;
    case 7: // Finnish cylon!
      Serial.println(F("Starting Helsinki Cylon"));
      multi_cylon(WHITE, scaled_wait, NO_OF_EYES); // WHITE cylon eye!
      multi_cylon(BLUE, scaled_wait, NO_OF_EYES); // BLUE meteor eye!
      break;
    case 8:
      Serial.println(F("Starting Helsinki meteor"));
      multi_meteor(WHITE, scaled_wait / 2, NO_OF_EYES); // WHITE cylon eye!
      multi_meteor(BLUE, scaled_wait / 2, NO_OF_EYES); // BLUE meteor eye!
      break;
    case 9:
      Serial.println(F("Starting Finland meteor"));
      finland( scaled_wait / 2, 2); // BLue and White meteor eye!
      break;
    case 10:
      Serial.println(F("Starting Theater Chase all White"));
      theaterChaseColor(50);
      break;
    case 11:
      Serial.println(F("Starting Theater Chase all Indigo"));
      theaterChaseColor(50,INDIGO);
      break;
    case 12:
      Serial.println(F("Starting Theater Chase all Blue"));
      theaterChaseColor(50,BLUE);
      break;
    case 13:
      Serial.println(F("Starting Theater Chase all GREEN"));
      theaterChaseColor(50,GREEN);
      break;
    case 14:
      Serial.println(F("Starting Theater Chase all YELLOW"));
      theaterChaseColor(50,YELLOW);
      break;
    case 15:
      Serial.println(F("Starting Theater Chase all OrangRed"));
      theaterChaseColor(50,ORANGERED);
      break;
    case 16:
      Serial.println(F("Starting Theater Chase all DarkRed"));
      theaterChaseColor(50,DARKRED);
      break;
    case 17:
      Serial.println(F("beginning White fade now!"));
      colorFade(25, WHITE, 2);
      break;
    case 18:
      Serial.println(F("Starting fade Indigo"));
      colorFade(25,INDIGO,2);
      break;
    case 19:
      Serial.println(F("Starting fade Blue"));
      colorFade(25,BLUE,2);
      break;
    case 20:
      Serial.println(F("Starting fade GREEN"));
      colorFade(25,GREEN,2);
      break;
    case 21:
      Serial.println(F("Starting fade YELLOW"));
      colorFade(25,YELLOW,2);
      break;
    case 22:
      Serial.println(F("Starting fade OrangRed"));
      colorFade(25,ORANGERED,2);
      break;
    case 23:
      Serial.println(F("Starting fade DarkRed"));
      colorFade(25,DARKRED,2);
      break;
    case 24:
      Serial.println(F("Going to Sleep Now!"));
      delay(300);
      clearLEDS();
      collar.show();
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
      break;
    default:
      AllScans();
  }
}

// Implements a little triple larson "cylon" sanner.
// This'll run one full cycle, down one way and back the other
void multi_cylon(unsigned long color, byte wait, int num_eyes)
{
  // weight determines how much lighter the outer "eye" colors are
  const byte weight = 4;
  // It'll be easier to decrement each of these colors individually
  // so we'll split them out of the 24-bit color value
  byte red = (color & 0xFF0000) >> 16;
  byte green = (color & 0x00FF00) >> 8;
  byte blue = (color & 0x0000FF);
  int gap = LED_COUNT / num_eyes;
  // Start at closest LED, and move to the outside
  for (int i = 0; i <= gap; i++)
  {
    clearLEDS();
    collar.setPixelColor(i, red, green, blue);  // Set the bright middle eye
    collar.setPixelColor(i + gap, red, green, blue); // Set the bright middle eye
    collar.setPixelColor(i + 2 * gap, red, green, blue); // Set the bright middle eye
    // Now set two eyes to each side to get progressively dimmer
    for (int j = 1; j < 3; j++)
    {
      if (i - j >= 0) {
        collar.setPixelColor(i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
        collar.setPixelColor(gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
        collar.setPixelColor(2 * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
      }
      if (i - j <= LED_COUNT) {
        collar.setPixelColor(i + j, red / (weight * j), green / (weight * j), blue / (weight * j));
        collar.setPixelColor(gap + i + j, red / (weight * j), green / (weight * j), blue / (weight * j));
        collar.setPixelColor(2 * gap + i + j, red / (weight * j), green / (weight * j), blue / (weight * j));
      }
    }
    collar.show();  // Turn the collar on
    delay(wait);  // Delay for visibility
  }
  // Now we go back to where we came. Do the same thing.
  for (int i = gap - 2; i >= 1; i--)
  {
    clearLEDS();
    collar.setPixelColor(i, red, green, blue);
    collar.setPixelColor(gap + i, red, green, blue);
    collar.setPixelColor(2 * gap + i, red, green, blue);
    for (int j = 1; j < 3; j++)
    {
      if (i - j >= 0) {
        collar.setPixelColor(i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
        collar.setPixelColor(gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
        collar.setPixelColor(2 * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
      }
      if (i - j <= LED_COUNT) {
        collar.setPixelColor(i + j, red / (weight * j), green / (weight * j), blue / (weight * j));
        collar.setPixelColor(gap + i + j, red / (weight * j), green / (weight * j), blue / (weight * j));
        collar.setPixelColor(2 * gap + i + j, red / (weight * j), green / (weight * j), blue / (weight * j));
      }
    }
    collar.show();
    delay(wait);
  }
}
// Implements a little cascade with a fade effect
void multi_meteor(unsigned long color, byte wait, int num_eyes, boolean roundabout)
{
  const byte weight = 4;
  // It'll be easier to decrement each of these colors individually
  // so we'll split them out of the 24-bit color value
  byte red = (color & 0xFF0000) >> 16;
  byte green = (color & 0x00FF00) >> 8;
  byte blue = (color & 0x0000FF);
  int gap = LED_COUNT / num_eyes;
  // Start at closest LED, and move to the outside
  for (int m = 0; m < num_eyes; m++) {
    for (int i = 0; i <= gap; i++)
    {
      clearLEDS();
      for (int g = 0; g < num_eyes; g++) {
        collar.setPixelColor(i + g * gap, red, green, blue); // Set the bright middle eye
      }
      // Now set four eyes after to get progressively dimmer
      for (int j = 1; j < 5; j++)
      {
        if (i - j >= 0)
          for (int k = 0; k < num_eyes; k++) {
            collar.setPixelColor(k * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
          }
      }
      collar.show();  // Turn the collar on
      delay(wait);  // Delay for visibility
    }
  }
}

// Sets all collar to off, but DOES NOT update the display;
// call collar.show() to actually turn them off after this.
void clearLEDS()
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    collar.setPixelColor(i, 0);
  }
}
// Prints a rainbow on the LED strip.
//  The rainbow begins at a specified position on the color wheel
void rainbow(byte startPosition)
{
  // Need to scale our rainbow. We want a variety of colors, even if there
  // are just 10 or so pixels.
  int rainbowScale = 192 / LED_COUNT;
  // Next we setup each pixel with the right color
  for (int i = 0; i < LED_COUNT; i++)
  {
    collar.setPixelColor(i, rainbowOrder((rainbowScale * (i + startPosition)) % 192));
  }
  collar.show();
}

// Input a value 0 to 191 to get a color value.
// The colors are a transition red->yellow->green->aqua->blue->fuchsia->red...
//  Adapted from Wheel function in the Adafruit_NeoPixel library example sketch
uint32_t rainbowOrder(byte position)
{
  // 6 total zones of color change:
  if (position < 31)  // Red -> Yellow (Red = FF, blue = 0, green goes 00-FF)
  {
    return collar.Color(0xFF, position * 8, 0);
  }
  else if (position < 63)  // Yellow -> Green (Green = FF, blue = 0, red goes FF->00)
  {
    position -= 31;
    return collar.Color(0xFF - position * 8, 0xFF, 0);
  }
  else if (position < 95)  // Green->Aqua (Green = FF, red = 0, blue goes 00->FF)
  {
    position -= 63;
    return collar.Color(0, 0xFF, position * 8);
  }
  else if (position < 127)  // Aqua->Blue (Blue = FF, red = 0, green goes FF->00)
  {
    position -= 95;
    return collar.Color(0, 0xFF - position * 8, 0xFF);
  }
  else if (position < 159)  // Blue->Fuchsia (Blue = FF, green = 0, red goes 00->FF)
  {
    position -= 127;
    return collar.Color(position * 8, 0, 0xFF);
  }
  else  //160 <position< 191   Fuchsia->Red (Red = FF, green = 0, blue goes FF->00)
  {
    position -= 159;
    return collar.Color(0xFF, 0x00, 0xFF - position * 8);
  }
}


//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j += 4) { // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < collar.numPixels(); i = i + 3) {
        collar.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      collar.show();
      delay(wait);
      for (int i = 0; i < collar.numPixels(); i = i + 3) {
        collar.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights in a single color
void theaterChaseColor(uint8_t wait, unsigned long color = WHITE ) {

  for (int q = 0; q < 3; q++) {
    for (int i = 0; i < collar.numPixels(); i = i + 3) {
      collar.setPixelColor(i + q, color); //turn every third pixel on
    }
    collar.show();
    delay(wait);
    for (int i = 0; i < collar.numPixels(); i = i + 3) {
      collar.setPixelColor(i + q, 0);      //turn every third pixel off

    }
  }

}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return collar.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return collar.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return collar.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void AllScans()
{
  for (int reps = 0; reps < ALL_SCANS_COUNT; reps++) {
    for (int i = LED_COUNT * 10; i >= 0; i--)
    {
      rainbow(i);
      delay(20);  // Delay between rainbow slides
    }
  }
  for (int reps = 0; reps < ALL_SCANS_COUNT; reps++) {
    multi_cylon(INDIGO, scaled_wait, NO_OF_EYES); // Indigo cylon eye!
    multi_cylon(BLUE, scaled_wait, NO_OF_EYES); // Blue  multi_cylon eye!
    multi_cylon(GREEN, scaled_wait, NO_OF_EYES); // Green  multi_cylon eye!
    multi_cylon(YELLOW, scaled_wait, NO_OF_EYES); // Yellow  multi_cylon eye!
    multi_cylon(ORANGERED, scaled_wait, NO_OF_EYES); // OrangeRed  multi_cylon eye!
    multi_cylon(DARKRED, scaled_wait, NO_OF_EYES); // DarkRed cylon eye!
  }
  for (int reps = 0; reps < ALL_SCANS_COUNT; reps++) {
    theaterChaseRainbow(60);
  }
  for (int reps = 0; reps < ALL_SCANS_COUNT; reps++) {
    multi_meteor(INDIGO, scaled_wait, NO_OF_EYES); // Indigo multi_meteor eye!
    multi_meteor(BLUE, scaled_wait, NO_OF_EYES); // blue  multi_meteor eye!
    multi_meteor(GREEN, scaled_wait, NO_OF_EYES); // green  multi_meteor eye!
    multi_meteor(YELLOW, scaled_wait, NO_OF_EYES); // yellow  multi_meteor eye!
    multi_meteor(ORANGERED, scaled_wait, NO_OF_EYES); // OrangeRed  multi_meteor eye!
    multi_meteor(DARKRED, scaled_wait, NO_OF_EYES); // DarkRed cylon eye!
  }
}
//*********************************************************************************************
void finland( byte wait, int num_eyes)
{
  const byte weight = 4;
  // It'll be easier to decrement each of these colors individually
  // so we'll split them out of the 24-bit color value
  byte red = (WHITE & 0xFF0000) >> 16;
  byte green = (WHITE & 0x00FF00) >> 8;
  byte blue = (WHITE & 0x0000FF);
  int gap = LED_COUNT / num_eyes;
  // Start at closest LED, and move to the outside
  for (int m = 0; m < num_eyes; m++) {
    for (int i = 0; i <= gap; i = i + 2)
    {
      clearLEDS();
      red = (WHITE & 0xFF0000) >> 16;
      green = (WHITE & 0x00FF00) >> 8;
      blue = (WHITE & 0x0000FF);
      for (int g = 0; g < num_eyes; g = g + 2) {
        collar.setPixelColor(i + g * gap, red, green, blue); // Set the bright middle eye

      }
      // Now set four eyes after to get progressively dimmer
      for (int j = 1; j < 5; j++)
      {
        if (i - j >= 0)
          for (int k = 0; k < num_eyes; k++) {
            collar.setPixelColor(k * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));

          }
        red = (BLUE & 0xFF0000) >> 16;
        green = (BLUE & 0x00FF00) >> 8;
        blue = (BLUE & 0x0000FF);
        for (int g = 0; g < num_eyes; g = g + 2) {
          collar.setPixelColor(i + (g + 1) * gap, red, green, blue); // Set the bright middle eye

        }

        // Now set four eyes after to get progressively dimmer
        for (int j = 1; j < 5; j++)
        {
          if (i - j >= 0)
            for (int k = 0; k < num_eyes; k++) {
              collar.setPixelColor((k + 1) * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));

            }
        }
        collar.show();  // Turn the collar on

        delay(wait);  // Delay for visibility
      }
    }
  }
}

void colorFade(uint8_t wait, unsigned long color,  byte stepsize, byte maxBrightness) {
  //byte BrightRange = 255 - maxBrightness;
  //byte counter = maxBrightness/steps;
  byte red = (color & 0xFF0000) >> 16;
  byte green = (color & 0x00FF00) >> 8;
  byte blue = (color & 0x0000FF);
  for  (int bright = 0; bright <= maxBrightness; bright+=stepsize) { //Brightness max is 0, minimum (off) is 255
    for (int j = 0; j < LED_COUNT; j++) {
      collar.setPixelColor(j, color); //red,green,blue);
      collar.setBrightness(bright);
    }
    collar.show();
    delay(wait);
  }
  delay(100);
  for  (int bright = maxBrightness; bright >=0 ; bright-=stepsize) { //Brightness max is 0, minimum (off) is 255
    for (int j = 0; j < LED_COUNT; j++) {
      collar.setPixelColor(j, color); //red,green,blue);
      collar.setBrightness(bright);
    }
    collar.show();
    delay(wait);
  }
  delay(500);
}
