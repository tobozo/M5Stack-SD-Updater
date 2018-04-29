[![License: MIT](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/tobozo/M5Stack-SD-Updater/blob/master/LICENSE)
[![Build Status](https://travis-ci.org/tobozo/M5Stack-SD-Updater.svg?branch=master)](https://travis-ci.org/tobozo/M5Stack-SD-Updater)


# M5Stack-SD-Updater

<br />



[![Click to enlarge](https://github.com/PartsandCircuits/M5Stack-SD-Updater/blob/master/SDUpdaterpic.png "Click to enlarge")](https://github.com/PartsandCircuits/M5Stack-SD-Updater/blob/master/SDUpdaterpic02.png)


<br />

## ABOUT

- **M5Stack-SD-Updater is an [Platform.io](https://platformio.org/lib/show/2575/M5Stack-SD-Updater)/[Arduino library](https://www.arduinolibraries.info/libraries/m5-stack-sd-updater) for [M5Stack](http://m5stack.com/) to package you apps on a SD card and load them from a menu.**

- It is inspired by gamebuino, however it does not use a modified bootloader.

- [Video demonstration](https://www.youtube.com/watch?v=myQfeYxyc3o)

- This project by [tobozo](https://github.com/tobozo) - built on M5Stack-SAM by Tom Such. Further credits listed below.

- Contributors welcome !


<br />

🏭 PREREQUISITES:
-----------------
<br />

**Micro SD Card (TF Card)** - formatted using FAT32. Max size 32 Gb.

<br />

**Make sure you have the following libraries:** - they should be installed in: ....\Documents\Arduino\libraries

- [M5Stack library](https://github.com/m5stack/M5Stack) - this is probably already installed - if not you can get it from the [Arduino Library Manager](https://www.arduinolibraries.info/libraries/m5-stack-sd-updater) or by performing a [manual installation](https://www.arduino.cc/en/Guide/Libraries#toc5)

- M5Stack-SD-Updater (this project): get it from the [Arduino Library Manager](https://www.arduinolibraries.info/libraries/m5-stack-sd-updater) or by performing a [manual installation](https://www.arduino.cc/en/Guide/Libraries#toc5)

- ArduinoJSON: [https://github.com/bblanchon/ArduinoJson/](https://github.com/bblanchon/ArduinoJson/) available in the Arduino Library Manager
- M5StackSAM: [https://github.com/tomsuch/M5StackSAM](https://github.com/tomsuch/M5StackSAM)

<br />

**If your version of [M5Stack is (0.1.7)](https://github.com/m5stack/M5Stack/releases/tag/0.1.7) or major you are set and can move on.**

<br />

**If your version of M5Stack is 0.1.6 or minor, you need to install this additional library:**

- QRCode: [https://github.com/ricmoo/qrcode](https://github.com/ricmoo/qrcode) available in the Arduino Library Manager


<br />


🍱 UNPACKING
------------

**1) Open both sketches from the "examples/M5Stack-SD-Update" menu.**

<br />

**2) Download the [SD-Content :floppy_disk:](https://github.com/tobozo/M5Stack-SD-Updater/releases/download/v0.2.2/SD-Apps-Folder.zip) folder from the release page and unzip it into the root of the SD Card.** Then put the SD Card into the M5Stack. This zip file comes preloaded with [18 precompiled apps](https://github.com/tobozo/M5Stack-SD-Updater/tree/master/examples/M5Stack-SD-Menu/SD-Apps) and the relative meta information for the menu.

<br />

**3) Compile the "M5Stack-SD-Menu.ino" example.** <br />
This sketch is the menu app. It must be (a) compiled and saved to the root directory of a micro SD card for persistence and (b) flashed onto the M5Stack.

(a) In the Arduino IDE, go to Sketch / Export compiled binary , and compile the file. Rename the file "menu.bin" and copy it to the micro SD card. (b) Next, flash "menu.bin" to the M5Stack. 

 Note that you won't need to (a) copy it if you previously extracted the SD-Content folder on the SD card.

<br />

**4) Make sketches compatible with the SD-Updater Menu .** <br />


The brief bit of code in the "M5Stack-SDLoader-Snippet.ino" sketch can be used to make any Arduino compatible sketch compatible for use with the SD-Updater menu.


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

<br />




 
  Export the compiled binary ( on the Arduino IDE menu go to:  Sketch / Export Compiled Binary ).
  
  (Optional) Rename the file to remove unnecessary additions to the name. The filename will be saved as "filename.ino.esp32.bin". Edit the name so it reads "filename.bin". This is purely for display purposes. The file will work without this change.
  
  Use one of following methods to get the app on the M5Stack:

  - Manually copy it to the sd card
  
  - Existing installations (menu.bin already copied and loaded on the M5Stack): clear the content of the [examples/M5Stack-SD-Menu/data](https://github.com/tobozo/M5Stack-SD-Updater/tree/master/examples/M5Stack-SD-Menu/data) folder, copy the compiled binary there and use the [ESP32 Sketch Data Uploader](https://github.com/me-no-dev/arduino-esp32fs-plugin) utility (available from the Tools menu in the Arduino IDE).

<br />


📚 USAGE:
---------
When an app is loaded in memory, booting the M5Stack with the BTN_A pushed will flash back the menu.bin into memory. 

When the menu is loaded in memory, it will list all available apps on the sdcard and load them on demand. 

Booting the M5Stack with the BTN_A pushed will power it off.

The [ESP32 Sketch Data Uploader](https://github.com/me-no-dev/arduino-esp32fs-plugin) can be used to send .bin, jpg, json, mod and mp3 files onto the M5Stack. The menu.bin will take care of dumping them on the SD Card.

Some artwork/credits can be added for every uploaded binary, the menu will scan for these file types:
  
  - .bin compiled application binary
    
  - .jpg image/icon (max 200x100)
    
  - .json file with dimensions descriptions: 
  
  `{"width":128,"height":128,"authorName":"tobozo","projectURL":"http://short.url","credits":"** http://very.very.long.url ~~"}`



<br />

  ⚠️ The jpg/json file names must match the bin file name, case matters!
  jpg/json files are optional but must both be set if provided.
  The value for "credits" JSON property will be scrolled on the top of the screen while the value for *projectURL* JSON property will be rendered as a QR Code in the info window. It is better provide a short URL for *projectURL* so the resulting QR Code has more error correction.

<br />
<br />

🚫 LIMITATIONS:
---------------
- SD Library limits file names (including path) to 32 chars, M5StackSAM has a slightly higher limit.
- FAT specifications prevent having more than 512 files on the SD Card, but this menu is limited to 256 Items anyway.
- Long file names will eventually get covered by the jpg image, better stay under 8 chars (not including the extension).

<br />

🔘 OPTIONAL:
------------
The M5Stack automatically detects and uses the [M5Stack-Faces](https://github.com/m5stack/faces) addon (gameboy only).

The JoyPSP Controls for M5Stack SD Menu necessary code is now disabled in the menu example but the code stays here and can be used as a boilerplate for any other two-wires input device.

The JoyPSP code is optimized for a [4 Wires PSP JoyPad breakout](https://www.google.fr/search?q=psp+joypad+breakout) on Pins 35 and 36, but it shouldn't be a problem to adapt/extend to other analog joystick models.

<br />

 ⚠️ KNOWN ISSUES
------------
*qrcode.h not found*, or *duplicate declarations* errors can occur during compilation of M5Stack-SD-Menu.ino.

Reason: M5Stack recently embedded the `qrcode.h` library into their own core.
If your version of M5stack core is older than 0.1.8, Arduino IDE will probably complain.

Solution 1: choose between one of the two includes in M5Stack-SD-Menu.ino:

`#include "qrcode.h"` ← use this with M5Stack-Core 0.1.6 and older, comment out the other one

or

`#include "utilities/qrcode.h` ← use this with M5Stack-Core 0.1.7, comment out the other one

Solution 2: in your library manager, downgrade the M5Stack-SD-Menu to an earlier version (0.0.1) until you update M5Stack library

Solution 3: upgrade your M5Stack core version to 0.1.8

Compilation `#pragma` warnings/errors in the Arduino IDE can be solved by setting the debug level to `default` in the Arduino preferences window.
See [#3](https://github.com/tobozo/M5Stack-SD-Updater/issues/3) 

vMicro: currently can't compile at all, see [#5](https://github.com/tobozo/M5Stack-SD-Updater/issues/5). Looking for a solution that works with both vMicro and Arduino IDE.

<br />

🛣 ROADMAP:
----------
Not defined yet, but looking at how fast this [library landed in platform.io](https://platformio.org/lib/show/2575/M5Stack-SD-Updater), there's a possibility it will soon exist in different flavours (i.e. as an ESP-IDF component) or with more [features](https://github.com/m5stack/faces). Contributors welcome!

<br />

#️⃣  REFERENCES: 
--------------
<br />

|              |                          |                                              |
| ------------ |:------------------------ | :------------------------------------------- |
| :clapper:   | Video demonstration      | https://youtu.be/myQfeYxyc3o                 |
| :clapper:   | [Video demo of Pacman + sound](https://youtu.be/36fgNCecoEg) | [Source](https://github.com/tobozo/M5Stack-Pacman-JoyPSP) | 
| :clapper:   | [Video demo of NyanCat](https://youtu.be/Zxh2mtWwfaE) |  [Source](https://github.com/tobozo/M5Stack-NyanCat)  |
| 🎓        | [Macsbug's article on M5Stack SD-Updater](https://macsbug.wordpress.com/2018/03/12/m5stack-sd-updater/) | [🇯🇵](https://macsbug.wordpress.com/2018/03/12/m5stack-sd-updater/) [🇬🇧](https://translate.google.com/translate?hl=en&sl=ja&tl=en&u=https%3A%2F%2Fmacsbug.wordpress.com%2F2018%2F03%2F12%2Fm5stack-sd-updater%2F) (google translate)|

<br />

🙏 CREDITS:
-----------

<br />

|        |                     |                  |                                              |
| ------ |:------------------- | :--------------- | :------------------------------------------- |
| 👍     | M5Stack             | M5Stack          | https://github.com/m5stack/M5Stack           |
| 👍     | M5StackSam          | Tom Such         | https://github.com/tomsuch/M5StackSAM        |
| 👍     | ArduinoJSON         | Benoît Blanchon  | https://github.com/bblanchon/ArduinoJson/    |
| 👍     | QRCode              | Richard Moore    | https://github.com/ricmoo/qrcode             |
| 👍     | @Reaper7            | Reaper7          | https://github.com/reaper7                   |
| 👍     | @PartsandCircuits   | PartsandCircuits | https://github.com/PartsandCircuits          |
 
 
