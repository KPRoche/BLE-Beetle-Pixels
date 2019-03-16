![Animated GIF of belt and collar](https://github.com/KPRoche/BLE-Beetle-Belt/blob/master/images/BLE%20Beetle%20Belt-Collar.gif?raw=true)
      
# BLE-Beetle-Belt
Code for several Neopixel (WS2812B addressable LED) projects, run by  a DFRobot BLE Beetle controller

## Overview

The DFRobot BLE Beetle controller is an Arduino-compatible variant which includes a BLE (Bluetooth Low Energy) transceiver based on the 
TI CC2540 chip. 
It is part of their *Bluno* family of controllers. There are several mobile apps available for both iOS and Android devices, which means 
it fairly simple to build a wearable NeoPixel project remote controlled by your mobile device.

There is even a **Play Bluno** app available which includes an RGB color picker, but I found that it constantly barrages the BLE Beetle with
messages irrelevant to controlling a simple illuminated wearable. I had much better luck (on iOS) with the BLE Terminal HM-10 App, which 
lets you preprogram up to 25 buttons with text(ASCII) or HEX commands, as well as including a simple ASCII/HEX type-and-send terminal.

There are 3 different programs in this repository:

* **BLE_Simple_NEO** is set up to control a few (default 5) addressable LEDs simultaneously, with all set to one color. 

   Color: you can set their color via input of RGB values, or by selecting one of 7 preprogrammed color values. 
   A rainbow mode will cycle the LEDs through a color wheel of 255 hues
 
   General brightness can be set (although it may affect hue)
 
   It also offers the option of steady on, or a fade-up and down pulse effect.
   Finally, a Sleep option turns off all the LEDS, but leaves the processor up and looking for new commands.
   
* **BLEetleBelt_v6** controls a long strip (120 pixels default) designed to be mounted in a belt. (Why _ _v6 _? It took six versions before I was happy with it)

  It offers the same color options as **BLE_Simple_NEO** but has a widely different assortment of lighting effects:
  Single color effects: (which may be rotated through the color wheel via the rainbow option)
  + pulse (fade-up/fade-down)
  + "Larson scanner" (like a Cylon eye) with multiple "eyes"
  + meteor or raindrop effect   
  + split meteor effect either towards or away from center of the strip
  + double meteor effect (criss-crossing the length of the strip)
  
  + theater chase effect, either single color or with a progressive rainbow fade along the length
  + progressive rainbow fade of the entire length
  
  There are also several blue-and-white animations, an artifact of when I was assisting with the Helsinki bid to host 
  the World Science Fiction Convention:
  + white, then blue, meteor effect
  + white, then blue, cylon effect
  + white and blue simultaneous meteors
  
  The code also offers some debug and timing command options to tweak the number of eyes and speed of the other effects, 
  and to tweak the timing in the BLE command parser in the event of interference.
 
* **BLEetleCollar** offers the same options as the belt, but is optimized for a shorter (default 20) LED strip, so a belt and 
collar can be worn together and put in complementary modes.
 
All of these programs uses the Adafruit_NeoPixel library, as well as the EEPROM library (to store the last running state)
  Animations are based on those in the Adafruit_NeoPixel library examples
  
## Construction Notes  
           NeoPixel data connection should be connected to pin D5 on the BLE Beetle
  
           A BLE Terminal app can be used to send mode and color change requests to the program
           I had good luck with the BLE Terminal HM-10 iOs App, because I could assign commands to buttons.
             Once connected, the terminal must be put in DFB1 Mode (no echo) to work properly
           This program recognizes a semicolon (;) as the end of a command as well as carriage return or linefeed,
             and can recognize color requests from the RGB picker in the PLAY Bluno app.
            (That app sends a lot of extraneous data, I recommend using a simpler terminal, however)
