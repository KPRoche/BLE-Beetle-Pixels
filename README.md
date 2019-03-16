![Animated GIF of belt and collar](https://github.com/KPRoche/BLE-Beetle-Belt/blob/master/images/BLE%20Beetle%20Belt-Collar.gif?raw=true)
      
# BLE-Beetle-Belt
Code for several Neopixel (WS2812B addressable LED) projects, run by  a DFRobot BLE Beetle controller

## Overview

<img src="https://github.com/KPRoche/BLE-Beetle-Belt/blob/master/images/BLEBeetle.jpg" align=right> The DFRobot BLE Beetle controller is an Arduino-compatible variant which includes a BLE (Bluetooth Low Energy) transceiver based on the 
TI CC2540 chip.  The Beetle is quite compact, 28.8mm X 33.1mm (1.13" x 1.30") and has the bonus of two sets of power terminals 
(V+ and GND) which are connected to the V+ and 0V pins in its micro-USB connector, so LEDs can be easily connected to power, 
bypassing the voltage regulator for the controller chip. (Getting to those voltages can be fussy on other small Arduino variants).
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
   
* **BLEetleBelt_v6** controls a long strip (120 pixels default) designed to be mounted in a belt. (Why *v6*? It took six versions before I was happy with it)

  It offers the same color options as **BLE_Simple_NEO** but has a widely different assortment of lighting effects:
  Single color effects: (which may be rotated through the color wheel via the rainbow option)
  + pulse (fade-up/fade-down)
  + "Larson scanner" (like a Cylon eye) with multiple "eyes"
  + meteor or raindrop effect   <img src="https://github.com/KPRoche/BLE-Beetle-Belt/blob/master/images/split-in.gif" align=right width='100px' alt-text='inward split meteor animation'><img src="https://github.com/KPRoche/BLE-Beetle-Belt/blob/master/images/split-out.gif" align=right alt-text='outward split meTeor animation' width='100px'>
  + split meteor effect either towards or away from center of the strip 
  + double meteor effect (criss-crossing the length of the strip)<img src="https://github.com/KPRoche/BLE-Beetle-Belt/blob/master/images/double-meteor.gif" align=right alt-text='double meteor animation' width='100px'>
  
  
  
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

The program presumes the NeoPixel data connection will be connected to pin D5 on the BLE Beetle. I like to use the 3-pin JST connectors
for NeoPixels, so the controller can be disconnected from the lighting harness for reprogrammng.
While rated for 5V (easily provided by a USB power bank), the BLE Beetle can also happily run on the 3.7 VDC provided by a LiPo
(lithium-polymer) battery. Most NeoPixels will also happily run at this voltage, which makes powering small wearables like the collar
version of this project easier.
The plus and minus terminals are easy to find as their holes are in the shape of a **+** and a **-**. Here are some photos 
showing examples of wiring for USB and LiPo use.

<img src="https://github.com/KPRoche/BLE-Beetle-Belt/blob/master/images/IMG_3531.jpg" width='250px' align='left' alt-text='two BLE beetles showing output wiring for NeoPixels. One (short wires) includes a connector for LiPo power connection' >
<img src="https://github.com/KPRoche/BLE-Beetle-Belt/blob/master/images/IMG_3535.jpg" width='250px' align alt-text='close-up of two BLE beetles showing output wiring for NeoPixels. One (short wires) includes a connector for LiPo power connection' >
<br>
<img src="https://github.com/KPRoche/BLE-Beetle-Belt/blob/master/images/IMG_3532.jpg" width='250px' align = 'left' alt-text='close-up of BLE Beetle wired for NeoPixels and LiPo power connection'>
<img src="https://github.com/KPRoche/BLE-Beetle-Belt/blob/master/images/IMG_3534.jpg" width='250px' align = 'right' alt-text='close-up of BLE Beetle wired for NeoPixels, no extra power connection'>

## Software notes 
A BLE Terminal app can be used to send mode and color change requests to the program
I had good luck with the BLE Terminal HM-10 iOs App, because I could assign commands to buttons.
Once connected, the terminal must be put in DFB1 Mode (no echo) to work properly
This program recognizes a semicolon (;) as the end of a command as well as carriage return or linefeed,
and can recognize color requests from the RGB picker in the PLAY Bluno app.
(That app sends a lot of extraneous data, I recommend using a simpler terminal, however)
