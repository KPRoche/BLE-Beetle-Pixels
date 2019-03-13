#include <LowPower.h>
#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"
#include <EEPROM.h>

//constants


const int SCAN_WAIT = 20;
const int  ALL_SCANS_COUNT = 4;
const int PIN = 5;

const String myVersion = "BLEeetleBelt v5";

// Globals
byte ScannerTask = 0;
byte MaxTask = 19;
int LED_COUNT = 120;
int NO_OF_EYES = 6;
int masterEyes = 2 * (LED_COUNT / 52);
int commDelay = 5;
boolean rainbowMode = false;
boolean sleepMode = false;
uint32_t g_color = BLUE;
byte g_color_index = 0;
byte g_direction = true; // for splits
boolean g_debug = false;

String ble = "";

// Create an instance of the Adafruit_NeoPixel class called "lights".
// That'll be what we refer to from here on...

Adafruit_NeoPixel lights = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

//function prototypes
void clearLEDS();
void theaterChaseRainbow(uint8_t wait);
void theaterChaseColor(uint8_t wait, unsigned long color = WHITE );
void multi_cylon(unsigned long color, byte wait, int num_eyes = 3);
void multi_meteor(unsigned long color, byte wait, int num_eyes, boolean roundabout = false);
void split_meteor(unsigned long color, byte wait, int num_eyes, boolean outward = false);
void double_meteor(unsigned long color, byte wait, int num_eyes);
void colorFade(uint8_t wait, unsigned long color = WHITE,  byte stepsize = 25, byte maxBrightness = 64);
void rainbow(byte startPosition);
uint32_t rainbowOrder(byte position);
uint32_t Wheel(byte WheelPos);
void finland( byte wait, int num_eyes);
void AllScans();
void ChangeMode();


// Read a string from the BLE serial port.
// This is rewritten to work with the new HM10 terminal app
String bleRead() {
  char newByte;
  String bleQueue;

  if (Serial.available()) {
    while (Serial.available()) {
      newByte = Serial.read();
      if (newByte == ';' || newByte == 10 || newByte == 13) {
        break;
      }
      else {
        if (newByte != ';' && newByte != 10 && newByte != 12) {
          bleQueue += String(newByte);
        }
      }
      delay(commDelay);
    }
  }
  return bleQueue;
}

void processBLEcmd(String queue = "", boolean mydebug = false) {
  //# global g_color;
  String scratch;
  unsigned int r, g, b;

  if (queue.length() == 0) {
    queue = bleRead();
  }
  if (mydebug and queue.length() > 0) {
    Serial.println("Raw Queue: " + queue);
  }

  if (queue.indexOf("RGB") >= 0) {
    scratch = queue.substring(queue.indexOf(":") + 1);
    if (scratch == "0,0,0") {
      Serial.println("LED control deactivated");
    }
    else {
      String calcString;
      calcString = queue.substring(queue.indexOf(":") + 1, queue.indexOf(","));
      Serial.println("r: " + calcString);
      r = calcString.toInt();
      calcString = queue.substring(queue.indexOf(",") + 1, queue.lastIndexOf(","));
      Serial.println("g: " + calcString);
      g = calcString.toInt();
      calcString = queue.substring(queue.lastIndexOf(",") + 1);
      b = calcString.toInt();
      Serial.println("b: " + calcString);
      g_color = lights.Color(r, g, b);
      Serial.println("LED Color Command: " + scratch);
      if (mydebug) {
        Serial.println(String(r) + "," + String(g) + ","  + String(b));
        Serial.println(g_color, HEX);
      }
    }
  }
  else if (queue.indexOf("MODE") >= 0) {
    if (mydebug) {
      Serial.println("Mode command detected");
    }
    scratch = queue.substring(queue.indexOf(":") + 1);
    scratch.toUpperCase();
    Serial.println("Mode command: " + scratch);
    if (scratch.indexOf("RBFADE") >= 0) {
      ScannerTask = 1;
      rainbowMode = false;
    }
    else if (scratch.indexOf("CYLON") >= 0) {
      if (rainbowMode) {
        ScannerTask = 2;
      }
      else {
        ScannerTask = 12;
      }
    }

    else if (scratch.indexOf("THEA") >= 0) {
      if (rainbowMode) {
        ScannerTask = 3;
      }
      else {
        ScannerTask = 13;
      }
    }
    else if (scratch.indexOf("MET") >= 0) {
      Serial.println("Activating Meteor Mode with Rainbow" + String(rainbowMode));
      if (rainbowMode) {
        ScannerTask = 4;
      }
      else {
        ScannerTask = 14;
      }
    }
    else if (scratch.indexOf("PULSE") >= 0) {
      ScannerTask = 5 ;
    }
    else if (scratch.indexOf("SPLIT") >= 0) {
      g_direction = ! g_direction;
      ScannerTask = 6 ;
    }
    else if (scratch.indexOf("DB") >= 0) {
      ScannerTask = 7;
      rainbowMode = false;
    }

    else if (scratch.indexOf("HC") >= 0) {
      ScannerTask = 8 ;
      rainbowMode = false;
    }
    else if (scratch.indexOf("HM") >= 0) {
      ScannerTask = 9;
      rainbowMode = false;
    }
    else if (scratch.indexOf("FL") >= 0) {
      ScannerTask = 10;
      rainbowMode = false;
    }
  }



  else if (queue.indexOf("COLOR") >= 0) {
    scratch = queue.substring(queue.indexOf(":") + 1);
    scratch.toUpperCase();
    if (scratch.indexOf("WHITE") >= 0) {
      g_color = WHITE;
      rainbowMode = false;
    }
    else if (scratch.indexOf("IN") >= 0) {
      g_color = INDIGO;
      rainbowMode = false;
    }
    else if (scratch.indexOf("BL") >= 0) {
      g_color = BLUE;
      rainbowMode = false;
    }
    else if (scratch.indexOf("GR") >= 0) {
      g_color = GREEN;
      rainbowMode = false;
    }
    else if (scratch.indexOf("YL") >= 0) {
      g_color = YELLOW;
      rainbowMode = false;
    }
    else if (scratch.indexOf("OG") >= 0) {
      g_color = ORANGERED;
      rainbowMode = false;
    }
    else if (scratch.indexOf("RD") >= 0) {
      g_color = DARKRED;
      rainbowMode = false;
    }
    Serial.print("Color command: " + scratch + ":" );
    Serial.println(g_color, HEX);
  }
  else if (queue.indexOf("RAINBOW") >= 0) {
    if (queue.indexOf("1") > 0) {
      rainbowMode = true;
    }
    else {
      rainbowMode = false;
    }
    Serial.print("Rainbow command: ");
    Serial.println(rainbowMode);
  }
  else if (queue.indexOf("SLEEP") >= 0) {
    if (mydebug) {
      Serial.println("Sleep mode request detected");
    }
    scratch = queue.substring(queue.indexOf(":") + 1);
    if (mydebug) {
      Serial.println("Sleep switch " + scratch);
    }
    if (scratch.toInt() > 0) {
      sleepMode = true;
      ScannerTask = 20;
    }
    else {
      sleepMode = false;
    }
    Serial.print("Sleep command: ");
    Serial.println(sleepMode);
  }
  else if (queue.indexOf("DEBUG") >= 0) {
    if (mydebug) {
      Serial.println("Debug mode request detected");
    }
    scratch = queue.substring(queue.indexOf(":") + 1);
    if (scratch.toInt() > 0) {
      g_debug = true;
    }
    else {
      g_debug = false;
    }
    Serial.print("Sleep command: ");
    Serial.println(sleepMode);
  }
  EEPROM.write(0, ScannerTask);
}

void setup() {
  Serial.begin(115200);  //initial the Serial
  pinMode(13, OUTPUT);

  Serial.print(F("Flashy Belt with Sleep version "));
  Serial.println(myVersion);
  ScannerTask = EEPROM.read(0);
  Serial.print(F("Stored ScannerTask: "));
  Serial.println(ScannerTask);
  ScannerTask++;
  if (ScannerTask > MaxTask) {
    ScannerTask = 0;
  }
  // store it to eeprom
  EEPROM.write(0, ScannerTask);
  lights.begin();  // Call this to start up the LED strip.
  clearLEDS();   // This function, defined below, turns all lights off...
  lights.show();   // ...but the lights don't actually update until you call this.

  lights.setBrightness(64);


}

void loop() {
  processBLEcmd("", g_debug);
  lights.setBrightness(64);
  Serial.println("ScannerTask: " + String(ScannerTask));
  if (rainbowMode) {
    g_color_index++;
    if (g_color_index > 255) {
      g_color_index = 0;
    }
    g_color = Wheel(g_color_index);
    Serial.print("Rainbow mode on: color ");
    Serial.println(g_color, HEX);
  }

  digitalWrite(13, HIGH);
  delay(5);
  digitalWrite(13, LOW);
  Serial.print(F("Flashy Belt with Sleep version "));
  Serial.println(myVersion);
  switch (ScannerTask) {
      //   case 0:
      //     Serial.println(F("Starting AllScans"));
      //     AllScans();
      break;
    case 1:
      Serial.println(F("Starting rainbow"));
      for (int i = LED_COUNT * 2; i >= 0; i--)
      {
        rainbow(i);
        delay(20);  // Delay between rainbow slides
      }
      break;
    case 2:
      Serial.print(F("Flashy Belt with Sleep version "));
      Serial.println(myVersion);
      Serial.println(F("Starting Larson scanner"));
      // cylon function: first param is color, second is time (in ms) between cycles
      multi_cylon(INDIGO, SCAN_WAIT, NO_OF_EYES); // Indigo cylon eye!
      multi_cylon(BLUE, SCAN_WAIT, NO_OF_EYES); // Blue  multi_cylon eye!
      multi_cylon(GREEN, SCAN_WAIT, NO_OF_EYES); // GREEN  multi_cylon eye!
      multi_cylon(YELLOW, SCAN_WAIT, NO_OF_EYES); // YELLOW  multi_cylon eye!
      multi_cylon(ORANGERED, SCAN_WAIT, NO_OF_EYES); // ORANGERED  multi_cylon eye!
      multi_cylon(DARKRED, SCAN_WAIT, NO_OF_EYES); // DARKRED cylon eye!
      break;
    case 12:
      Serial.println(F("Starting scanner"));
      multi_cylon(g_color, SCAN_WAIT, NO_OF_EYES); // Indigo cylon eye!
      break;

    case 3:
      Serial.print(F("Flashy Belt with Sleep version "));
      Serial.println(myVersion);
      Serial.println(F("Starting Theater Chase"));
      theaterChaseRainbow(50);
      break;
    case 13:
      Serial.println(F("Starting Theater Chase"));
      theaterChaseColor(50, g_color);
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
    case 14:
      Serial.println(F("Starting meteor"));
      multi_meteor(g_color, SCAN_WAIT / 2, NO_OF_EYES); // Indigo meteor eye!
      break;

    case 5:
      colorFade(25, g_color, 2);
      break;
    case 6:
      split_meteor(g_color, SCAN_WAIT / 2, 4, g_direction); // Indigo meteor eye!
      break;
    case 7:
      double_meteor(g_color, SCAN_WAIT / 2, 4);
      break;
    case 8:
      Serial.println(F("Starting Helsinki Cylon)"));
      multi_cylon(WHITE, SCAN_WAIT, NO_OF_EYES); // WHITE cylon eye!
      multi_cylon(BLUE, SCAN_WAIT, NO_OF_EYES); // BLUE meteor eye!
      break;
    case 9:
      Serial.println(F("Starting Helsinki Meteor)"));
      multi_meteor(WHITE, SCAN_WAIT / 2, NO_OF_EYES); // WHITE cylon eye!
      multi_meteor(BLUE, SCAN_WAIT / 2, NO_OF_EYES); // BLUE meteor eye!
      break;
    case 10:
      Serial.println(F("Starting Finland meteor"));
      finland( SCAN_WAIT / 2, NO_OF_EYES);
      break;

    case 20:
      Serial.println(F("Turning off Display Now!!"));
      //delay(300);
      clearLEDS();
      lights.show();
      //LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
      break;
    default:
      for (int i = LED_COUNT * 2; i >= 0; i--)
      {
        rainbow(i);
        delay(20);  // Delay between rainbow slides
      }
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
// call lights.show() and to actually turn them off after this.
void clearLEDS()
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    lights.setPixelColor(i, 0);
  }
}


//*********************************************************************************************

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j += 4) { // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < lights.numPixels(); i = i + 3) {
        lights.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on

      }
      lights.show();
      delay(wait);
      for (int i = 0; i < lights.numPixels(); i = i + 3) {
        lights.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights in a single color
void theaterChaseColor(uint8_t wait, unsigned long color) {

  for (int q = 0; q < 3; q++) {
    for (int i = 0; i < lights.numPixels(); i = i + 3) {
      lights.setPixelColor(i + q, color); //turn every third pixel on
    }
    lights.show();
    delay(wait);
    for (int i = 0; i < lights.numPixels(); i = i + 3) {
      lights.setPixelColor(i + q, 0);      //turn every third pixel off

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
      lights.setPixelColor(i + e * gap , red, green, blue);  // Set the bright middle eye
    }

    // Now set two eyes to each side to get progressively dimmer

    for (int j = 1; j < 3; j++)
    {
      if (i - j >= 0) {
        for (int e = 0; e < num_eyes; e++)
        {
          lights.setPixelColor(e * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
        }
      }
      if (i - j <= LED_COUNT) {
        for (int e = 0; e < num_eyes; e++)
        {
          lights.setPixelColor(e * gap + i + j, red / (weight * j), green / (weight * j), blue / (weight * j));
        }
      }
    }
    lights.show();  // Turn the lights on
    delay(wait);  // Delay for visibility
  }
  // Now we go back to where we came. Do the same thing.
  for (int i = gap - 2; i >= 1; i--)
  {
    clearLEDS();
    for (int e = 0; e < num_eyes; e++)
    {
      lights.setPixelColor(e * gap + i, red, green, blue);
    }
    for (int j = 1; j < 3; j++)
    {
      if (i - j >= 0) {
        for (int e = 0; e < num_eyes; e++)
        {
          lights.setPixelColor(e * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
        }
      }
      if (i - j <= LED_COUNT) {
        for (int e = 0; e < num_eyes; e++)
        {
          lights.setPixelColor(i + j, red / (weight * j), green / (weight * j), blue / (weight * j));
        }
      }
    }
    lights.show();
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
      lights.setPixelColor(i + g * gap, red, green, blue); // Set the bright middle eye
      // Now set four eyes after to get progressively dimmer
      for (int j = 1; j < 5; j++)
      {
        if (i - j > 0)
          for (int k = 0; k < num_eyes; k++) {
            lights.setPixelColor(k * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
          }
      }
      lights.show();  // Turn the lights on

      delay(wait / 2); // Delay for visibility
    }
  }
}
//****************************

// Implements a little cascade with a fade effect that starts at the ends and runs to the center
void split_meteor(unsigned long color, byte wait, int num_eyes, boolean outward)
{
  const byte weight = 4;
  // It'll be easier to decrement each of these colors individually
  // so we'll split them out of the 24-bit color value
  byte red = (color & 0xFF0000) >> 16;
  byte green = (color & 0x00FF00) >> 8;
  byte blue = (color & 0x0000FF);
  int offset = 0;

  Serial.print("Split Meteor number of eyes: " + String(num_eyes) + " ");
  Serial.println("outward: " + String(outward));

  int gap = (LED_COUNT ) / num_eyes;
  // Start at outside LEDs, and move to the center
  // Start at closest LED, and move to the outside
  for (int i = 0; i <= gap; i++)
  {
    clearLEDS();
    if (outward) {
      for (int g = 0; g < num_eyes; g++) {

        if ((LED_COUNT / 2) + i + g * gap < LED_COUNT) {
          lights.setPixelColor((LED_COUNT / 2) + i + g * gap, red, green, blue); // Set the bright middle eye
        }
        if ((LED_COUNT / 2 - (i + g * gap) > 0)) {
          lights.setPixelColor(LED_COUNT / 2 - ( i + g * gap), red, green, blue); // Set the bright middle eye
        }
        // Now set four eyes after to get progressively dimmer
        for (int j = 1; j < 5; j++)
        {
          if (i - j > 0)
            for (int k = 0; k < num_eyes; k++) {
              offset = (k * gap) + i - j;
              lights.setPixelColor(LED_COUNT / 2 + offset, red / (weight * j), green / (weight * j), blue / (weight * j));
              lights.setPixelColor(LED_COUNT / 2 - offset, red / (weight * j), green / (weight * j), blue / (weight * j));
            }
        }
      }

    }
    else { // not outward
      for (int g = 0; g < num_eyes / 2; g++) {
        if (i + g * gap <= LED_COUNT / 2) {
          lights.setPixelColor(i + g * gap, red, green, blue); // Set the bright spot
          lights.setPixelColor(LED_COUNT - (i + g * gap), red, green, blue); // Set the bright spot
          for (int j = 1; j < 5; j++)
          {
            if (i - j > 0)
              for (int k = 0; k < num_eyes / 2; k++) {

                lights.setPixelColor(k * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
                lights.setPixelColor(LED_COUNT - (k * gap + i - j), red / (weight * j), green / (weight * j), blue / (weight * j));

              }
          }
        }
      }
    }
    lights.show();  // Turn the lights on

    delay(wait * 2); // Delay for visibility
  }
}


// Implements a little cascade with a fade effect that starts at the ends and runs to the center
void double_meteor(unsigned long color, byte wait, int num_eyes)
{
  const byte weight = 4;
  // It'll be easier to decrement each of these colors individually
  // so we'll split them out of the 24-bit color value
  byte red = (color & 0xFF0000) >> 16;
  byte green = (color & 0x00FF00) >> 8;
  byte blue = (color & 0x0000FF);

  Serial.print("double Meteor number of eyes: ");
  Serial.println(num_eyes);

  int gap = (LED_COUNT ) / num_eyes;
  // Start at outside LEDs, and move to the center
  for (int i = 0; i <= gap; i++)
  {
    clearLEDS();
    for (int g = 0; g < (num_eyes); g++) {

      lights.setPixelColor(i + g * gap, red, green, blue); // Set the bright middle eye
      lights.setPixelColor(LED_COUNT - (i + g * gap), red, green, blue); // Set the bright middle eye

      // Now set four eyes after to get progressively dimmer
      for (int j = 1; j < 5; j++)
      {
        if (i - j > 0)
          for (int k = 0; k < num_eyes; k++) {

            lights.setPixelColor(k * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
            lights.setPixelColor(LED_COUNT - (k * gap + i - j), red / (weight * j), green / (weight * j), blue / (weight * j));

          }
      }
      lights.show();  // Turn the lights on

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
    lights.setPixelColor(i, rainbowOrder((rainbowScale * (i + startPosition)) % 192));

  }
  lights.show();
  processBLEcmd();

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
    return lights.Color(0xFF, position * 8, 0);
  }
  else if (position < 63)  // Yellow -> Green (Green = FF, blue = 0, red goes FF->00)
  {
    position -= 31;
    return lights.Color(0xFF - position * 8, 0xFF, 0);
  }
  else if (position < 95)  // Green->Aqua (Green = FF, red = 0, blue goes 00->FF)
  {
    position -= 63;
    return lights.Color(0, 0xFF, position * 8);
  }
  else if (position < 127)  // Aqua->Blue (Blue = FF, red = 0, green goes FF->00)
  {
    position -= 95;
    return lights.Color(0, 0xFF - position * 8, 0xFF);
  }
  else if (position < 159)  // Blue->Fuchsia (Blue = FF, green = 0, red goes 00->FF)
  {
    position -= 127;
    return lights.Color(position * 8, 0, 0xFF);
  }
  else  //160 <position< 191   Fuchsia->Red (Red = FF, green = 0, blue goes FF->00)
  {
    position -= 159;
    return lights.Color(0xFF, 0x00, 0xFF - position * 8);
  }
}

//*********************************************************************************************
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return lights.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return lights.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return lights.Color(0, WheelPos * 3, 255 - WheelPos * 3);
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
        lights.setPixelColor(i + g * gap, red, green, blue); // Set the bright middle eye

      }
      // Now set four eyes after to get progressively dimmer
      for (int j = 1; j < 5; j++)
      {
        if (i - j >= 0)
          for (int k = 0; k < num_eyes; k++) {
            lights.setPixelColor(k * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));

          }
        red = (BLUE & 0xFF0000) >> 16;
        green = (BLUE & 0x00FF00) >> 8;
        blue = (BLUE & 0x0000FF);
        for (int g = 0; g < num_eyes; g = g + 2) {
          lights.setPixelColor(i + (g + 1) * gap, red, green, blue); // Set the bright middle eye

        }

        // Now set four eyes after to get progressively dimmer
        for (int j = 1; j < 5; j++)
        {
          if (i - j >= 0)
            for (int k = 0; k < num_eyes; k++) {
              lights.setPixelColor((k + 1) * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));

            }
        }
        lights.show();  // Turn the lights on

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
  for  (int bright = 0; bright <= maxBrightness; bright += stepsize) { //Brightness max is 0, minimum (off) is 255
    for (int j = 0; j < LED_COUNT; j++) {
      lights.setPixelColor(j, color); //red,green,blue);
      lights.setBrightness(bright);
    }
    lights.show();
    delay(wait);
  }
  delay(100);
  for  (int bright = maxBrightness; bright >= 0 ; bright -= stepsize) { //Brightness max is 0, minimum (off) is 255
    for (int j = 0; j < LED_COUNT; j++) {
      lights.setPixelColor(j, color); //red,green,blue);
      lights.setBrightness(bright);
    }
    lights.show();
    delay(wait);
  }
  delay(500);
}
