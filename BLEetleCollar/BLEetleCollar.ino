/*--------------------------------------------------------------------------------------------------------#
  #
  #         BLE Beetle Flashy Collar v1
  #           A wearable NeoPixel project by Kevin Roche
  #             designed for a short strip of NeoPixels to match the BLEetleBelt project
  #             based on my Arduino Mini Flashy Collar code
  #         Animates a  strip of RBG WS5812B addressable LEDs (NeoPixels) via the
  #         DFRobot BLE Beetle Arduino variant.
  #           uses the Adafruit_NeoPixel library, as well as the EEPROM library
 #         Animations are based on those in the Adafruit_NeoPixel library examples
 #
  #
  #
  #         NeoPixel data connection should be connected to pin D5 on the BLE Beetle
  #
  #         A BLE Terminal app can be used to send mode and color change requests to the program
  #         I had good luck with the BLE Terminal HM-10 iOs App, because I could assign commands to buttons.
  #           Once connected, the terminal must be put in DFB1 Mode (no echo) to work properly
  #
  #           COMMANDS:
  #             Mode commands change the animation pattern
  #               MODE:RBFADE     sets the strip to a continuous end-to-end rainbow fade
  #               MODE:CYLON      a multi-spot "Larsen Scanner" that goes back and forth
  #               MODE:METEOR     a multi-spot "raindrop" effect that goes in one direction
  #               MODE:THEATER    a classic theater marquee chase effect
  #               MODE:PULSE      a fade-up/fade-down effect across the whole strip
  #               MODE:SPLIT      a mirrored meteor effect that either emits from or meets in the center of the strip
  #                     (successive commands reverse the direction)
  #               MODE:DB         double meteor effect flying in both directions simultaneously
  #               MODE:HC         a cylon effect that alternates between blue and white along the whole strip
  #               MODE:HM         a meteor effect that alternates between blue and white along the whole strip
  #               MODE:FL         a meteor effect that has alternate blue and white meteors at the same time
  #
  #           Color commands do not change the animation, but change the color used by them.
  #                   color commands will deactivate the rainbow mode.
  #               RGB:r,g,b       set the strip global color to an RGB value#
  #             it is easier to use some presets
  #               COLOR:WHITE     set the whole strip white
  #               COLOR:IN        set the whole strip to indigo   (always looks purple to me, while violet looks pink)
  #               COLOR:BL        set the whole strip to blue
  #               COLOR:GR        set the whole strip to green
  #               COLOR:YL        set the whole strip to yellow
  #               COLOR:OG        set the whole strip to orange
  #               COLOR:RD        set the whole strip to red
  #           Rainbow commands trigger a color progression while the pattern runs
  #               RAINBOW:1        activate the rainbow modes
  #               RAINBOW:0        deactivate the rainbow modes. (Has no effect on rainbow fade or chase if active)
  #
  #           Sleep command: used to use the low power library, but that requires a reset or interrupt to wake back up
  #               SLEEP        set the whole strip off, but leave the program running
  #
  #
  #           Configuration commands let you experiment with some of the control parameters without recompiling.
  #               E#nnnn        change the number of "eyes" for meteors and cylons from the default of 6
  #               S#nnnn        change the number of eyes in the split meteor pattern from the default of 4
  #                               Split eyes must be in multiples of 2. If an odd number is supplied it will be incremented by 1
  #               C#nn          change the comm delay from the default of 5ms to nn ms
  #               D#nn          change the pattern timing delay from the default of 20 ms to nn ms
  #
  #
  #--------------------------------------------------------------------------------------------------------#*/

#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"
#include <EEPROM.h>

//#include <LowPower.h>  Don't want to use low power mode for sleep; turns off BLE


// constants
const int ALL_SCANS_COUNT = 4;
const int PIN = 5;
const String FCVers = "BLE Beetle v1:matching belt";

// globals
int LED_COUNT = 38;
int BELT_COUNT = 120;
int NO_OF_SPLIT = 2;
int NO_OF_EYES = 2;
int SCAN_WAIT = 20;
int commDelay = 5;
boolean rainbowMode = false;
boolean sleepMode = false;
uint32_t g_color = BLUE;
byte g_color_index = 0;
byte g_direction = true; // for splits
boolean g_debug = false;
byte g_brightness = 64;

float belt2collar = BELT_COUNT / LED_COUNT;
int scaled_wait = SCAN_WAIT * belt2collar;

// Create an instance of the Adafruit_NeoPixel class called "collar".
Adafruit_NeoPixel collar = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

byte ScannerTask = 0;
byte MaxTask = 19;

void clearLEDS();
void theaterChaseRainbow(uint8_t wait);
void theaterChaseColor(uint8_t wait, unsigned long color = WHITE );
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



// -------------------   Bluetooth BLE functions for BLUNO (BLE Beetle) using the TI CC2540 BLE chip

// Read a string from the BLE serial port.
// This is rewritten to work with the new HM10 terminal app
String bleRead() {
  char newByte;
  String bleQueue;
  // ";" is accepted as a command terminator for compatibility with the Play Bluno App
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
  int new_count = 0;

  if (queue.length() == 0) {
    queue = bleRead();
  }
  if (mydebug and queue.length() > 0) {
    Serial.println("Raw Queue: " + queue);
  }
  queue.toUpperCase(); // this makes the string scans work even if you use mixed case in your terminal
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
      g_color = collar.Color(r, g, b);
      Serial.println("LED Color Command: " + scratch);
      if (mydebug) {
        Serial.println(String(r) + "," + String(g) + ","  + String(b));
        Serial.println(g_color, HEX);
      }
    }
  }
  // This section is for special control parameters (modifying defaults)
  else if (queue.indexOf("E#") >= 0) {
    scratch = queue.substring(queue.indexOf("#") + 1);
    new_count = scratch.toInt();
    if (new_count > 0) {
      NO_OF_EYES = new_count;
    }
  }
  else if (queue.indexOf("S#") >= 0) {
    scratch = queue.substring(queue.indexOf("#") + 1);
    new_count = scratch.toInt();
    if (new_count > 0) {
      if (new_count % 2 == 1) {
        new_count++;  // increment to an even number if necessary
      }
      NO_OF_SPLIT = new_count;
    }
  }
  else if (queue.indexOf("C#") >= 0) {
    scratch = queue.substring(queue.indexOf("#") + 1);
    new_count = scratch.toInt();
    if (new_count > 0) {
      commDelay = new_count;
    }
  }
  else if (queue.indexOf("D#") >= 0) {
    scratch = queue.substring(queue.indexOf("#") + 1);
    new_count = scratch.toInt();
    if (new_count > 0) {
      SCAN_WAIT = new_count;
    }
  }
  // This section is mode commands, controlling the patterns
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
      //     if (rainbowMode) {
      //       ScannerTask = 2;
      //     }
      //     else {
      ScannerTask = 12;
      //     }
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
      //    if (rainbowMode) {
      ScannerTask = 4;
      //    }
      //     else {
      //     ScannerTask = 14;
      // }
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

  // This section is the color commands, to pick one of the preset colors
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
  // Set the brightness
  else if (queue.indexOf("BRIGHT") >= 0) {
    scratch = queue.substring(queue.indexOf(":") + 1);
    g_brightness = scratch.toInt() % 255;
  }
  // This command implements the color-wheel options
  else if (queue.indexOf("RAINBOW") >= 0) {
    if (queue.indexOf("1") > 0) {
      rainbowMode = true;
      //g_color_index = unWheel(g_color);
    }
    else {
      rainbowMode = false;
    }
    Serial.print("Rainbow command: ");
    Serial.println(rainbowMode);
  }

  // This command puts the strip into OFF mode but leaves the program running
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

  // This command enables the extra debug messages
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
  }

  EEPROM.write(0, ScannerTask);
}
//___________________________________________________________


void setup()
{
  Serial.begin(115200);  //initial the Serial port. BLE Beetle runs BLE and USB serial in parallel
  pinMode(13, OUTPUT);
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

  collar.setBrightness(g_brightness);


}

void loop()
{
  digitalWrite(13, HIGH);
  delay(5);
  digitalWrite(13, LOW);
  processBLEcmd("", g_debug);
  collar.setBrightness(64);
  if (rainbowMode) {
    g_color_index++;
    if (g_color_index > 255) {
      g_color_index = 0;
    }
    g_color = Wheel(g_color_index);
    Serial.print("Rainbow mode on: color ");
    Serial.println(g_color, HEX);
  }
  Serial.print(F("Flashy Collar with Sleep version "));
  Serial.println(FCVers);
  // Select the task according to the settings
  switch (ScannerTask) {
      //case 0:
      Serial.println(F("Starting AllScans"));
      AllScans();
      break;
    case 1:
      Serial.println(F("Starting rainbow"));
      for (int i = LED_COUNT * 2; i >= 0; i--)
      {
        rainbow(i);
        delay(20);  // Delay between rainbow slides
      }
      break;
    // Larson scanners ("Cylon" eyes)
    case 2:
    case 12:
      Serial.println(F("Starting Larson scanner"));
      // cylon function: first param is color, second is time (in ms) between cycles
      multi_cylon(g_color, scaled_wait, NO_OF_EYES); // DARKRED cylon eye!
      break;

    //Theater Chases
    case 3:
      Serial.println(F("Starting Theater Chase"));
      theaterChaseRainbow(50);
      break;
    case 13:
      Serial.println(F("Starting Theater Chase all GREEN"));
      theaterChaseColor(50, g_color);
      break;

    // Meteor chase
    case 4: // the rainbow mode flag takes care of color rotation
    case 14:
      Serial.println(F("Starting meteor"));
      multi_meteor(g_color, scaled_wait / 2, NO_OF_EYES); // RED meteor eye!
      break;

    //pulse (fade) mode
    case 5:
      Serial.println(F("Beginning fade now!"));
      colorFade(25, g_color, 2, g_brightness);
      break;

    // New double and split meteor modes
    case 6:
      split_meteor(g_color, scaled_wait / 2, NO_OF_SPLIT, g_direction); // Indigo meteor eye!
      break;

    case 7:
      double_meteor(g_color, scaled_wait / 2, 2);
      break;


    // Helsinki (blue and white) specials
    case 8: // Finnish cylon!
      Serial.println(F("Starting Helsinki Cylon"));
      multi_cylon(WHITE, scaled_wait, NO_OF_EYES); // WHITE cylon eye!
      multi_cylon(BLUE, scaled_wait, NO_OF_EYES); // BLUE meteor eye!
      break;
    case 9:
      Serial.println(F("Starting Helsinki meteor"));
      multi_meteor(WHITE, scaled_wait / 2, NO_OF_EYES); // WHITE cylon eye!
      multi_meteor(BLUE, scaled_wait / 2, NO_OF_EYES); // BLUE meteor eye!
      break;
    case 10:
      Serial.println(F("Starting Finland meteor"));
      finland( scaled_wait / 2, 2); // BLue and White meteor eye!
      break;

    //Special Sleep (lights off) case
    case 20:
      Serial.println(F("Turning off Display Now!"));
      clearLEDS();
      collar.show();
      break;
    default:
      Serial.println(F("Starting default rainbow"));
      for (int i = LED_COUNT * 2; i >= 0; i--)
      {
        rainbow(i);
        delay(20);  // Delay between rainbow slides
      };
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

byte unWheel(byte r, byte g, byte b) {
  byte WheelPos;
  if (r == 0) {
    WheelPos = g / 3;
  }
  else if (g == 0) {
    WheelPos = b / 3;
  }
  else if (b == 0) {
    WheelPos = r / 3;
  }
  else {
    WheelPos = 0;
  }
  return WheelPos;
}

byte unWheel (uint32_t color) {
  byte r = (color & 0xFF0000) >> 16;
  byte g = (color & 0x00FF00) >> 8;
  byte b = (color & 0x0000FF);
  byte WheelPos;
  Serial.print(F("Calculating color wheel index for "));
  Serial.println(color, HEX);
  if (r == 0) {
    if (g > 0) {
      Serial.print(F("r is zero, indexing from green"));
      WheelPos = g / 3;
    }
    else {
      Serial.print(F("r and g are zero, indexing from blue"));
      WheelPos = b / 3;
    }
  }
  else if (g == 0) {
    if (b > 0) {
      Serial.print(F("g is zero, indexing from blue"));
      WheelPos =  b / 3;
    }
    else {
      Serial.print(F("g and b are zero, indexing from red"));
      WheelPos = 255 + r / 3;
    }
  }
  else if (b == 0) {
    if (r > 0) {
      Serial.print(F("b is zero, indexing from red"));
      WheelPos = r / 3;
    }
    else {
      Serial.print(F("b and r are zero, indexing from green"));
      WheelPos = 255 + g / 3;
    }
  }
  else {
    WheelPos = 0;
  }
  WheelPos = WheelPos % 255;
  Serial.println("WheelPos index is " + WheelPos);
  return WheelPos;
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
          collar.setPixelColor((LED_COUNT / 2) + i + g * gap, red, green, blue); // Set the bright middle eye
        }
        if ((LED_COUNT / 2 - (i + g * gap) > 0)) {
          collar.setPixelColor(LED_COUNT / 2 - ( i + g * gap), red, green, blue); // Set the bright middle eye
        }
        // Now set four eyes after to get progressively dimmer
        for (int j = 1; j < 5; j++)
        {
          if (i - j > 0)
            for (int k = 0; k < num_eyes; k++) {
              offset = (k * gap) + i - j;
              collar.setPixelColor(LED_COUNT / 2 + offset, red / (weight * j), green / (weight * j), blue / (weight * j));
              collar.setPixelColor(LED_COUNT / 2 - offset, red / (weight * j), green / (weight * j), blue / (weight * j));
            }
        }
      }

    }
    else { // not outward
      for (int g = 0; g < num_eyes / 2; g++) {
        if (i + g * gap <= LED_COUNT / 2) {
          collar.setPixelColor(i + g * gap, red, green, blue); // Set the bright spot
          collar.setPixelColor(LED_COUNT - (i + g * gap), red, green, blue); // Set the bright spot
          for (int j = 1; j < 5; j++)
          {
            if (i - j > 0)
              for (int k = 0; k < num_eyes / 2; k++) {

                collar.setPixelColor(k * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
                collar.setPixelColor(LED_COUNT - (k * gap + i - j), red / (weight * j), green / (weight * j), blue / (weight * j));

              }
          }
        }
      }
    }
    collar.show();  // Turn the lights on

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

      collar.setPixelColor(i + g * gap, red, green, blue); // Set the bright middle eye
      collar.setPixelColor(LED_COUNT - (i + g * gap), red, green, blue); // Set the bright middle eye

      // Now set four eyes after to get progressively dimmer
      for (int j = 1; j < 5; j++)
      {
        if (i - j > 0)
          for (int k = 0; k < num_eyes; k++) {

            collar.setPixelColor(k * gap + i - j, red / (weight * j), green / (weight * j), blue / (weight * j));
            collar.setPixelColor(LED_COUNT - (k * gap + i - j), red / (weight * j), green / (weight * j), blue / (weight * j));

          }
      }
      collar.show();  // Turn the lights on

      delay(wait / 2); // Delay for visibility
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
      collar.setPixelColor(j, color); //red,green,blue);
      collar.setBrightness(bright);
    }
    collar.show();
    delay(wait);
  }
  delay(100);
  for  (int bright = maxBrightness; bright >= 0 ; bright -= stepsize) { //Brightness max is 0, minimum (off) is 255
    for (int j = 0; j < LED_COUNT; j++) {
      collar.setPixelColor(j, color); //red,green,blue);
      collar.setBrightness(bright);
    }
    collar.show();
    delay(wait);
  }
  delay(500);
}
