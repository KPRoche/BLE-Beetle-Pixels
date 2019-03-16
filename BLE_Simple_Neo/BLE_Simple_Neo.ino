/*--------------------------------------------------------------------------------------------------------#
  #
  #         BLE Beetle Simple NeoPixel control
  #           A NeoPixel project by Kevin Roche
  #             designed for controlling the color of a few NeoPixels
  #             based on my Arduino BLE Belt code
  #         Controls RBG WS5812B addressable LEDs (NeoPixels) via the
  #         DFRobot BLE Beetle Arduino variant.
  #           uses the Adafruit_NeoPixel library, as well as the EEPROM library
  #         Animations are based on those in the Adafruit_NeoPixel library examples
  #
  #         NeoPixel data connection should be connected to pin D5 on the BLE Beetle
  #
  #         A BLE Terminal app can be used to send mode and color change requests to the program
  #         I had good luck with the BLE Terminal HM-10 iOs App, because I could assign commands to buttons.
  #           Once connected, the terminal must be put in DFB1 Mode (no echo) to work properly
  #         This program recognizes a semicolon (;) as the end of a command as well as carriage return or linefeed,
  #           and can recognize color requests from the RGB picker in the PLAY Bluno app.
  #          (That app sends a lot of extraneous data, I recommend using a simpler terminal, however)
  #
  #           COMMANDS: (Rainbow mode can make these progress around the color wheel)
  #               MODE:PULSE      a fade-up/fade-down effect across all LEDs
  #               MODE:ON         turn them on continously
  #
  #           Color commands do not change the animation, but change the color used by them.
  #                   color commands will deactivate the rainbow mode.
  #               <RGBLED>r,g,b;  set the LED global color via the color picker in the play bluno app
  #
  #               RGB:r,g,b       set the strip global color to an RGB value#
  #
  #             it is easier to use some presets
  #               COLOR:WHITE     set the whole strip white
  #               COLOR:IN        set the whole strip to indigo   (always looks purple to me, while violet looks pink)
  #               COLOR:BL        set the whole strip to blue
  #               COLOR:GR        set the whole strip to green
  #               COLOR:YL        set the whole strip to yellow
  #               COLOR:OG        set the whole strip to orange
  #               COLOR:RD        set the whole strip to red
  #               BRIGHT:nnn      set the pixels to brightness nnn (0-255). NOTE this may change the hue.
  #
  #           Rainbow commands trigger a color progression while the pattern runs
  #               RAINBOW:1        activate the rainbow modes
  #               RAINBOW:0        deactivate the rainbow modes. (Has no effect on rainbow fade or chase if active)
  #
  #           Sleep command: used to use the low power library, but that requires a reset or interrupt to wake back up
  #               SLEEP        set the whole strip off, but leave the program running
  #
  #
  #
  #
  #--------------------------------------------------------------------------------------------------------#*/

#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"
#include <EEPROM.h>


// constants
const int  ALL_SCANS_COUNT = 4;
const int PIN = 5;

const String myVersion = "BLE Beetle Neo Simple";

// Globals
byte ScannerTask = 0;
byte MaxTask = 2;
int LED_COUNT = 5;
int SCAN_WAIT = 20;
int commDelay = 5;
boolean rainbowMode = false;
boolean sleepMode = false;
uint32_t g_color = BLUE;
byte g_color_index = 0;
byte g_direction = true; // for splits
boolean g_debug = false;
byte g_brightness = 64;

String ble = "";

// Create an instance of the Adafruit_NeoPixel class called "lights".
// That'll be what we refer to from here on...

Adafruit_NeoPixel lights = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

//function prototypes
void clearLEDS();
void setColor(uint32_t color, byte brightness = 64);
void colorFade(uint8_t wait, unsigned long color = WHITE,  byte stepsize = 25, byte maxBrightness = 64);
uint32_t rainbowOrder(byte position);
uint32_t Wheel(byte WheelPos);


// -------------------   Bluetooth BLE functions for BLUNO (BLE Beetle) using the TI CC2540 BLE chip

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
  int new_count = 0;

  if (queue.length() == 0) {
    queue = bleRead();
  }
  if (mydebug and queue.length() > 0) {
    Serial.println("Raw Queue: " + queue);
  }
  queue.toUpperCase(); // this makes the string scans work even if you use mixed case in your terminal
  if (queue.indexOf("RGB") >= 0) { // set to work with RGB wheel in Play Bluno app
    if (queue.indexOf(">") > 0 ) {
      scratch = queue.substring(queue.indexOf(">") + 1);
    }
    else {
      scratch = queue.substring(queue.indexOf(":") + 1);
    }
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
  // This section is mode commands, controlling the patterns
  else if (queue.indexOf("MODE") >= 0) {
    if (mydebug) {
      Serial.println("Mode command detected");
    }
    scratch = queue.substring(queue.indexOf(":") + 1);
    scratch.toUpperCase();
    Serial.println("Mode command: " + scratch);
    if (scratch.indexOf("PULSE") >= 0) {
      ScannerTask = 1 ;
    }
    else if (scratch.indexOf("ON") >= 0) {
      ScannerTask = 0 ;
    }
    else {
      ScannerTask = 0;
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

//_____________________________________________



void setup() {
  Serial.begin(115200);  //initial the Serial port. BLE Beetle runs BLE and USB serial in parallel
  pinMode(13, OUTPUT);

  Serial.print(F("NeoPixel Control version "));
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

  lights.setBrightness(g_brightness);


}

void loop() {
  processBLEcmd("", g_debug);
  lights.setBrightness(g_brightness);
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
  Serial.print(F("Simple NeoPixel control "));
  Serial.println(myVersion);
  switch (ScannerTask) {
    case 0:
      setColor(g_color, g_brightness);
      break;
    case 1:
      //pulse (fade) mode
      colorFade(25, g_color, 2, g_brightness);
      break;
    case 20:
      Serial.println(F("Turning off Display Now!!"));
      //delay(300);
      clearLEDS();
      lights.show();
      //LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
      break;
    default:
      setColor(g_color, g_brightness);
  }
  delay(SCAN_WAIT);
}

//*************************** FUNCTIONS **************************

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
// Sets all LEDS to one color, but DOES NOT update the display;
// call lights.show() and to actually turn them off after this.
void setColor(uint32_t color, byte brightness)
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    lights.setPixelColor(i, color);
  }
  lights.setBrightness(brightness);
  lights.show();
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
