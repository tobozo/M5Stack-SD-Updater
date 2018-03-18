# M5Stack-SD-Updater

A helper library to package you apps on a SD card and load them from a menu.

This is inspired from gamebuino however it does not use a modified bootloader.


PREREQUISITES:

Make sure you have the following libraries
  
MSStack: [https://github.com/m5stack/M5Stack](https://github.com/m5stack/M5Stack)
  
M5StackSAM: [https://github.com/tomsuch/M5StackSAM](https://github.com/m5stack/M5Stack)
  
ArduinoJSON : [https://github.com/bblanchon/ArduinoJson/](https://github.com/bblanchon/ArduinoJson/)

QRCode : [https://github.com/ricmoo/qrcode](https://github.com/ricmoo/qrcode)
  
M5Stack-SD-Updater: this library


  Proceed to instal manually the library in your Arduino IDE.
  Open both sketch from the "examples/M5Stack-SD-Update" menu.


1) Compile the "M5Stack-SD-Menu.ino" example. This sketch is the menu app. It must be compiled once (sketch / export compiled binary) and saved on the SD Card as "menu.bin" for persistence. It should also be flashed on the M5Stack.
    

2) Merge and compile the "M5Stack-SDLoader-Snippet.ino" example. Now it is possible to embed any M5 app by implementing the 
  M5Stack SD Loader Snippet. This sketch can be used as a boilerplate to code an app from 
  scratch.

  For an existing M5 app, find the line:

      #include <M5Stack.h>
      
  And add this:
      
      #include "M5StackUpdater.h"
      
  In your setup() function, find the following statements:

      M5.begin();
      Wire.begin()

  And add this after 'Wire.begin();':

      if(digitalRead(BUTTON_A_PIN) == 0) {
        Serial.println("Will Load menu binary");
        updateFromFS(SD);
        ESP.restart();
      }

  Export the compiled binary (Sketch/Export Compiled Binary)
  and copy it to the sd card.

      
      
3) Repeat this for all applications to import


USAGE:

When an app is loaded in memory, booting the M5Stack with the BTN_A pushed will flash back the menu.bin into memory. When the menu is loaded in memory, it will list all available apps on the sdcard and load them on demand. Booting the M5Stack with the BTN_A pushed will power it off.

Some artwork can be added, the menu will scan for these file types:
  
  - .bin compiled application binary
    
  - .jpg image/icon (max 200x100)
    
  - .json file with dimensions descriptions: {"width":xxx,"height":xxx, "authorName":"tobozo","projectURL":"http://blah"} 
    

  The file names must match and case matters.
  jpg and json files are optional but must both be set if provided.
  The value for *projectURL* property will be rendered as a QR Code in the info window.


OPTIONAL:

The JoyPSP Controls for M5Stack SD Menu necessary code is disabled in the menu example.
It's using a [4 Wires PSP JoyPad breakout](https://www.google.fr/search?q=psp+joypad+breakout) on Pins 35 and 36, which are appropriate for analog reading.

The JoyPSP only handles up/down actions for the meantime
More controls will be added soon.


REFERENCES:

  [Demo](https://youtu.be/myQfeYxyc3o)
  [Another Demo with Pacman+sound](https://youtu.be/36fgNCecoEg)
  [Macsbug'article](https://macsbug.wordpress.com/2018/03/12/m5stack-sd-updater/)(JP)
 