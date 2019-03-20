#include <PinChangeInterrupt.h>
#include <PinChangeInterruptBoards.h>
#include <PinChangeInterruptPins.h>
#include <PinChangeInterruptSettings.h>

/* Party Belt Time
  Kevin Roche
  Dec 31, 2016

  hacked like crazy from my own Power Suit and Flashy Collar code
  starting from code in SparkFun WS2812 Breakout Board Example
  AND in the AdaFruit_NeoPixel strandtest example
*/

#include <LowPower.h>
#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"
#include <EEPROM.h>
#define FCVers "Flashybelt3b"

#define PIN 9
#define IRQPIN 11
#define LED_COUNT 144
#define NO_OF_EYES 6
#define SCAN_WAIT 20
#define ALL_SCANS_COUNT 4

// Create an instance of the Adafruit_NeoPixel class called "belt".
Adafruit_NeoPixel belt = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

byte ScannerTask = 0;
byte MaxTask = 11;

void clearLEDS();
void theaterChaseRainbow(uint8_t wait);
void theaterChaseColor(uint8_t wait, unsigned long color = WHITE );
void multi_cylon(unsigned long color, byte wait, int num_eyes = 3);
void multi_meteor(unsigned long color, byte wait, int num_eyes, boolean roundabout = false);
void rainbow(byte startPosition);
uint32_t rainbowOrder(byte position);
uint32_t Wheel(byte WheelPos);
void finland( byte wait, int num_eyes);
void AllScans();
void ChangeMode();
void setup()
{
  pinMode(13, OUTPUT);
  pinMode(11, INPUT_PULLUP);
  attachInterrupt(digitalPinToPCINT(IRQPIN), ChangeMode, FALLING);
  Serial.begin(9600);
  // Read the stored task from last time and modulo it
  Serial.print(F("Flashy Belt with Sleep version "));
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
  belt.begin();  // Call this to start up the LED strip.

  clearLEDS();   // This function, defined below, turns all belt off...
  belt.show();   // ...but the belt don't actually update until you call this.

  belt.setBrightness(64);


}

void loop()
{
  digitalWrite(13, HIGH);
  delay(5);
  digitalWrite(13, LOW);
  Serial.print(F("Flashy Belt with Sleep version "));
  Serial.println(FCVers);
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
      Serial.print(F("Flashy Belt with Sleep version "));
      Serial.println(FCVers);
      Serial.println(F("Starting Larson scanner"));
      // cylon function: first param is color, second is time (in ms) between cycles
      multi_cylon(INDIGO, SCAN_WAIT, NO_OF_EYES); // Indigo cylon eye!
      multi_cylon(BLUE, SCAN_WAIT, NO_OF_EYES); // Blue  multi_cylon eye!
      multi_cylon(GREEN, SCAN_WAIT, NO_OF_EYES); // GREEN  multi_cylon eye!
      multi_cylon(YELLOW, SCAN_WAIT, NO_OF_EYES); // YELLOW  multi_cylon eye!
      multi_cylon(ORANGERED, SCAN_WAIT, NO_OF_EYES); // ORANGERED  multi_cylon eye!
      multi_cylon(DARKRED, SCAN_WAIT, NO_OF_EYES); // DARKRED cylon eye!
      break;
    case 3:
      Serial.print(F("Flashy Belt with Sleep version "));
      Serial.println(FCVers);
      Serial.println(F("Starting Theater Chase"));
      theaterChaseRainbow(50);
      break;
    case 4:
      Serial.println(F("Starting meteor"));
      multi_meteor(INDIGO, SCAN_WAIT / 2, NO_OF_EYES); // Indigo meteor eye!
      multi_meteor(BLUE, SCAN_WAIT / 2, NO_OF_EYES); // BLUE  multi_meteor eye!
      multi_meteor(GREEN, SCAN_WAIT / 2, NO_OF_EYES); // GREEN  multi_meteor eye!
      multi_meteor(YELLOW, SCAN_WAIT / 2, NO_OF_EYES); // YELLOW  multi_meteor eye!
      multi_meteor(ORANGERED, SCAN_WAIT / 2, NO_OF_EYES); // ORANGERED  multi_meteor eye!
      multi_meteor(DARKRED, SCAN_WAIT / 2, NO_OF_EYES); // RED meteor eye!
      break;
    case 5:
      Serial.println(F("Starting White Cylon"));
      multi_cylon(WHITE, SCAN_WAIT, NO_OF_EYES); // WHITE cylon eye!
      break;
    case 6:
      Serial.println(F("Starting White Meteor)"));
      multi_meteor(WHITE, SCAN_WAIT / 2, NO_OF_EYES); // WHITE meteor eye!
      break;
    case 7:
      Serial.println(F("Starting Helsinki Cylon)"));
      multi_cylon(WHITE, SCAN_WAIT, NO_OF_EYES); // WHITE cylon eye!
      multi_cylon(BLUE, SCAN_WAIT, NO_OF_EYES); // BLUE meteor eye!
      break;
    case 8:
      Serial.println(F("Starting Helsinki Meteor)"));
      multi_meteor(WHITE, SCAN_WAIT / 2, NO_OF_EYES); // WHITE cylon eye!
      multi_meteor(BLUE, SCAN_WAIT / 2, NO_OF_EYES); // BLUE meteor eye!
      break;
    case 9:
      Serial.println(F("Starting Finland meteor"));
      finland( SCAN_WAIT / 2, NO_OF_EYES); // BLue and White meteor eye!
      break;
    case 10:
      Serial.println(F("Starting Theater Chase all White"));
      theaterChaseColor(50);
      break;
    case 11:
      Serial.println(F("Going to Sleep Now!"));
      delay(300);
      clearLEDS();
      belt.show();
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
      break;
    default:
      AllScans();
  }
  if (digitalRead(IRQPIN) == LOW) {
    ChangeMode();
  }
}

//*************************** FUNCTIONS **************************
//ChangeMode Increments the scanner mode without requiring a power cycle
void ChangeMode() {
  Serial.print("Interrupt Request Detected...");
  Serial.print("Stored ScannerTask: ");
  Serial.println(ScannerTask);
  ScannerTask++;
  if (ScannerTask > MaxTask) {
    ScannerTask = 0;
  }
  Serial.print("New ScannerTask: ");
  Serial.println(ScannerTask);
  // store it to eeprom
  EEPROM.write(0, ScannerTask);

}




//*********************************************************************************************
// Sets all LEDS to off, but DOES NOT update the display;
// call belt.show() and to actually turn them off after this.
void clearLEDS()
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    belt.setPixelColor(i, 0);
  }
}


//*********************************************************************************************

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j += 4) { // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < belt.numPixels(); i = i + 3) {
        belt.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on

      }
      belt.show();
      delay(wait);
      for (int i = 0; i < belt.numPixels(); i = i + 3) {
        belt.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}
//Theatre-style crawling lights with rainbow effect
void theaterChaseColor(uint8_t wait, unsigned long color = WHITE ) {

  for (int q = 0; q < 3; q++) {
    for (int i = 0; i < belt.numPixels(); i = i + 3) {
      belt.setPixelColor(i + q, color); //turn every third pixel on
    }
    belt.show();
    delay(wait);
    for (int i = 0; i < belt.numPixels(); i = i + 3) {
      belt.setPixelColor(i + q, 0);      //turn every third pixel off

    }
  }

}
//**********************************************************************************
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
  Serial.print("number of eyes: ");
  Serial.println(num_eyes);
  // Start at closest LED, and move to the outside
  for (int i = 0; i <= gap; i++)
  {
    clearLEDS();
    for (int e = 0; e < num_eyes; e++)
    {
      belt.setPixelColor(i + e * gap , red, green, blue);  // Set the bright middle eye
    }

    // Now set two eyes to each side to get progressively dimmer

    for (int j = 1; j < 3; j++)
    {
      if (i - j >= 0) {
        for (int e = 0; e < num_eyes; e++)
        {
          belt.setPixelColor(e * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
        }
      }
      if (i - j <= LED_COUNT) {
        for (int e = 0; e < num_eyes; e++)
        {
          belt.setPixelColor(e * gap + i + j, red / (weight * j), green / (weight * j), blue / (weight * j));
        }
      }
    }
    belt.show();  // Turn the belt on
    delay(wait);  // Delay for visibility
  }
  // Now we go back to where we came. Do the same thing.
  for (int i = gap - 2; i >= 1; i--)
  {
    clearLEDS();
    for (int e = 0; e < num_eyes; e++)
    {
      belt.setPixelColor(e * gap + i, red, green, blue);
    }
    for (int j = 1; j < 3; j++)
    {
      if (i - j >= 0) {
        for (int e = 0; e < num_eyes; e++)
        {
          belt.setPixelColor(e * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
        }
      }
      if (i - j <= LED_COUNT) {
        for (int e = 0; e < num_eyes; e++)
        {
          belt.setPixelColor(i + j, red / (weight * j), green / (weight * j), blue / (weight * j));
        }
      }
    }
    belt.show();
    delay(wait);
  }
}






//****************************
// Implements a little cascade with a fade effect
void multi_meteor(unsigned long color, byte wait, int num_eyes, boolean roundabout)
{
  const byte weight = 4;
  // It'll be easier to decrement each of these colors individually
  // so we'll split them out of the 24-bit color value
  byte red = (color & 0xFF0000) >> 16;
  byte green = (color & 0x00FF00) >> 8;
  byte blue = (color & 0x0000FF);

  Serial.print("number of eyes: ");
  Serial.println(num_eyes);

  int gap = LED_COUNT / num_eyes;
  // Start at closest LED, and move to the outside
  for (int i = 0; i <= gap; i++)
  {
    clearLEDS();
    for (int g = 0; g < num_eyes; g++) {
      belt.setPixelColor(i + g * gap, red, green, blue); // Set the bright middle eye
      // Now set four eyes after to get progressively dimmer
      for (int j = 1; j < 5; j++)
      {
        if (i - j > 0)
          for (int k = 0; k < num_eyes; k++) {
            belt.setPixelColor(k * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
          }
      }
      belt.show();  // Turn the belt on

      delay(wait / 2); // Delay for visibility
    }
  }
}





//*********************************************************************************************
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
    belt.setPixelColor(i, rainbowOrder((rainbowScale * (i + startPosition)) % 192));

  }
  belt.show();

}


//*********************************************************************************************
// Input a value 0 to 191 to get a color value.
// The colors are a transition red->yellow->green->aqua->blue->fuchsia->red...
//  Adapted from Wheel function in the Adafruit_NeoPixel library example sketch
uint32_t rainbowOrder(byte position)
{
  // 6 total zones of color change:
  if (position < 31)  // Red -> Yellow (Red = FF, blue = 0, green goes 00-FF)
  {
    return belt.Color(0xFF, position * 8, 0);
  }
  else if (position < 63)  // Yellow -> Green (Green = FF, blue = 0, red goes FF->00)
  {
    position -= 31;
    return belt.Color(0xFF - position * 8, 0xFF, 0);
  }
  else if (position < 95)  // Green->Aqua (Green = FF, red = 0, blue goes 00->FF)
  {
    position -= 63;
    return belt.Color(0, 0xFF, position * 8);
  }
  else if (position < 127)  // Aqua->Blue (Blue = FF, red = 0, green goes FF->00)
  {
    position -= 95;
    return belt.Color(0, 0xFF - position * 8, 0xFF);
  }
  else if (position < 159)  // Blue->Fuchsia (Blue = FF, green = 0, red goes 00->FF)
  {
    position -= 127;
    return belt.Color(position * 8, 0, 0xFF);
  }
  else  //160 <position< 191   Fuchsia->Red (Red = FF, green = 0, blue goes FF->00)
  {
    position -= 159;
    return belt.Color(0xFF, 0x00, 0xFF - position * 8);
  }
}

//*********************************************************************************************
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return belt.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return belt.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return belt.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}



//*********************************************************************************************
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
    multi_cylon(INDIGO, SCAN_WAIT, NO_OF_EYES); // Indigo cylon eye!
    multi_cylon(BLUE, SCAN_WAIT, NO_OF_EYES); // Indigo  multi_cylon eye!
    multi_cylon(GREEN, SCAN_WAIT, NO_OF_EYES); // Indigo  multi_cylon eye!
    multi_cylon(YELLOW, SCAN_WAIT, NO_OF_EYES); // Indigo  multi_cylon eye!
    multi_cylon(ORANGERED, SCAN_WAIT, NO_OF_EYES); // Indigo  multi_cylon eye!
    multi_cylon(DARKRED, SCAN_WAIT, NO_OF_EYES); // Indigo cylon eye!
  }
  for (int reps = 0; reps < ALL_SCANS_COUNT; reps++) {
    theaterChaseRainbow(60);
  }
  for (int reps = 0; reps < ALL_SCANS_COUNT; reps++) {
    multi_meteor(INDIGO, SCAN_WAIT, NO_OF_EYES); // Indigo cylon eye!
    multi_meteor(BLUE, SCAN_WAIT, NO_OF_EYES); // Indigo  multi_meteor eye!
    multi_meteor(GREEN, SCAN_WAIT, NO_OF_EYES); // Indigo  multi_meteor eye!
    multi_meteor(YELLOW, SCAN_WAIT, NO_OF_EYES); // Indigo  multi_meteor eye!
    multi_meteor(ORANGERED, SCAN_WAIT, NO_OF_EYES); // Indigo  multi_meteor eye!
    multi_meteor(DARKRED, SCAN_WAIT, NO_OF_EYES); // Indigo cylon eye!
  }
}

//*********************************************************************************************
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
        belt.setPixelColor(i + g * gap, red, green, blue); // Set the bright middle eye

      }
      // Now set four eyes after to get progressively dimmer
      for (int j = 1; j < 5; j++)
      {
        if (i - j >= 0)
          for (int k = 0; k < num_eyes; k++) {
            belt.setPixelColor(k * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));

          }
        red = (BLUE & 0xFF0000) >> 16;
        green = (BLUE & 0x00FF00) >> 8;
        blue = (BLUE & 0x0000FF);
        for (int g = 0; g < num_eyes; g = g + 2) {
          belt.setPixelColor(i + (g + 1) * gap, red, green, blue); // Set the bright middle eye

        }

        // Now set four eyes after to get progressively dimmer
        for (int j = 1; j < 5; j++)
        {
          if (i - j >= 0)
            for (int k = 0; k < num_eyes; k++) {
              belt.setPixelColor((k + 1) * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));

            }
        }
        belt.show();  // Turn the collar on

        delay(wait);  // Delay for visibility
      }
    }
  }
}
