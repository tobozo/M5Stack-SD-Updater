[![License: MIT](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/tobozo/M5Stack-SD-Updater/blob/master/LICENSE)
[![Gitter](https://badges.gitter.im/M5Stack-SD-Updater/community.svg)](https://gitter.im/M5Stack-SD-Updater/community)
[![arduino-library-badge](https://www.ardu-badge.com/badge/M5Stack-SD-Updater.svg?)](https://www.ardu-badge.com/M5Stack-SD-Updater)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/tobozo/library/M5Stack-SD-Updater.svg?)](https://registry.platformio.org/packages/libraries/tobozo/M5Stack-SD-Updater)

![Arduino Build](https://github.com/tobozo/M5Stack-SD-Updater/actions/workflows/ArduinoBuild.yml/badge.svg?branch=master)
![Platformio Build](https://github.com/tobozo/M5Stack-SD-Updater/actions/workflows/PlatformioBuild.yml/badge.svg?branch=master)

![Library Downloads](https://img.shields.io/github/downloads/tobozo/M5Stack-SD-Updater/total)

# M5Stack-SD-Updater

<br />


[![Click to enlarge](https://github.com/PartsandCircuits/M5Stack-SD-Updater/blob/master/SDUpdaterpic.png "Click to enlarge")](https://github.com/PartsandCircuits/M5Stack-SD-Updater/blob/master/SDUpdaterpic02.png)


<br />

## ABOUT

- **M5Stack-SD-Updater is an [Platform.io](https://platformio.org/lib/show/2575/M5Stack-SD-Updater)/[Arduino library](https://www.arduinolibraries.info/libraries/m5-stack-sd-updater) for [M5Stack](http://m5stack.com/) or [Odroid-Go](https://forum.odroid.com/viewtopic.php?t=31705) to package your apps on a SD card and load them from a menu such as the [default SD Menu example](https://github.com/tobozo/M5Stack-SD-Updater/tree/master/examples/M5Stack-SD-Menu) of this project or [@lovyan03](https://github.com/lovyan03)'s [Treeview based SD Menu](https://github.com/lovyan03/M5Stack_LovyanLauncher). For a Micropython compatible version of this library, see the [M5Stack_MicroPython  custom_sdupdater](https://github.com/ciniml/M5Stack_MicroPython/tree/custom_sdupdater)**

- It is inspired by gamebuino, however it does not use a modified bootloader.

- [Video demonstration](https://www.youtube.com/watch?v=myQfeYxyc3o)

- This project by [tobozo](https://github.com/tobozo) - Demo built on M5Stack-SAM by Tom Such. Further credits listed below.

- Contributors welcome !


<br />

🏭 M5Stack-SD-Menu EXAMPLE SKETCH PREREQUISITES:
------------------------------------------------
<br />

**Micro SD Card (TF Card)** - formatted using FAT32. Max size 32 Gb.
SDCard is recommended but the SDUpdater supports other filesystems such as SdFat, SD_MMC and LittleFS (SPIFFS will soon be deprecated).

<br />

**Make sure you have the following libraries:** - they should be installed in: `~/Arduino/libraries`

- [ESP32-Chimera-Core](https://github.com/tobozo/ESP32-Chimera-Core), [LovyanGFX](https://github.com/lovyan03/LovyanGFX), [M5GFX](https://github.com/m5stack/M5GFX), [M5Unified](https://github.com/m5stack/M5Unified) or [M5Stack Core](https://github.com/m5stack/M5Stack).

- [M5Stack-SD-Updater](https://github.com/tobozo/M5Stack-SD-Updater) (this library + its examples).

- [ArduinoJSON](https://github.com/bblanchon/ArduinoJson/) (Optional, used by SD-Menu).

- [ESP32-targz](https://github.com/tobozo/ESP32-targz) (Optional if using gzipped firmwares)

All those are available in the [Arduino Library Manager](https://www.arduinolibraries.info/libraries/m5-stack-sd-updater) or by performing a [manual installation](https://www.arduino.cc/en/Guide/Libraries#toc5).


<br />


🍱 UNPACKING THE BINARIES
-------------------------


**obsolete**
~~For your own lazyness, you can use @micutil's awesome [M5Burner](https://github.com/micutil/M5Burner_Mic) and skip the next steps.~~
~~[![https://github.com/micutil/M5Burner_Mic/releases](https://raw.githubusercontent.com/micutil/M5Burner_Mic/master/images/m5burnermic128.png)](https://github.com/micutil/M5Burner_Mic/releases)~~
~~... or customize your own menu and make the installation manually :~~


**1) Open the `examples/M5Stack-SD-Update` sketch from the Arduino ID Examples menu.**

<br />

**outdated binaries**
~~**2) Download the [SD-Content :floppy_disk:](https://github.com/tobozo/M5Stack-SD-Updater/releases/download/v0.4.1/SD-Apps-Folder.zip) folder from the release page and unzip it into the root of the SD Card.** Then put the SD Card into the M5Stack. This zip file comes preloaded with [precompiled apps](https://github.com/tobozo/M5Stack-SD-Updater/tree/master/examples/M5Stack-SD-Menu/SD-Apps) and the relative meta information for the menu.~~

<br />

**2) Compile and flash the `M5Stack-SD-Menu.ino` example.** <br />
This sketch is the **menu** app. It shoul reside in the root directory of a micro SD card for persistence and also executed once.

Once flashed it will **copy itself** on OTA2 partition and on the SDCard, then rolled back and executed from the OTA2 partition.

Thanks to @Lovyan03 this self-propagation logic is very convenient: by residing on OTA2 the `menu.bin` will always be available for fast re-loading.


<br />

**3) Make application sketches compatible with the SD-Updater Menu .** <br />


The snippet of code in the `M5Stack-SDLoader-Snippet.ino` sketch can be used as a model to make any ESP32 sketch compatible with the SD-Updater menu.


 In your sketch, find the line where the core library is included:

 ```C

    // #include <M5Stack.h>
    // #include <M5Core2.h>
    // #include <LovyanGFX.h>
    // #include <M5GFX.h>
    // #include <ESP32-Chimera-Core.h>
    // #include <M5StickC.h>
    // #include <M5Unified.h>

```

 And add this after the include:

```C
    // #define SDU_ENABLE_GZ // optional: support for gzipped firmwares
    #include <M5StackUpdater.h>
```

 In your `setup()` function, find the following statements:

```C++
    M5.begin();
    // Serial.begin(115200);
```

 And add this after serial is started:

```C
    checkSDUpdater( SD );
```

 Then do whatever you need to do (button init, timer, network signal) in the setup and the loop. Your app will
 run normally except at boot (e.g. if the `Button A` is pressed), when it can load the `/menu.bin` binary from
 the filesystem, or go on with it application duties.

 ⚠️Touch UI has no buttons, this raises the problem of detecting a 'pushed' state when the touch is off.
 As a compensation, an UI lobby will be visible for 2 seconds after every `ESP.restart()`. The visibility
 of the lobby can be forced in the setup :

```C
    checkSDUpdater( SD, MENU_BIN, 2000 );
```

  Custom SD-Load scenarios can be achieved using non default values:

```C

    M5.begin();

    checkSDUpdater(
      SD,           // filesystem (SD, SD_MMC, SPIFFS, LittleFS, PSRamFS)
      MENU_BIN,     // path to binary (default = /menu.bin, empty string = rollback only)
      5000          // wait delay, (default=0, will be forced to 2000 upon ESP.restart() or with headless build )
      TFCARD_CS_PIN // optional for SD use only, usually default=4 but your mileage may vary)
    );

```


  Headless setup can bypass `onWaitForAction` lobby option with their own button/sensor/whatever detection routine.


```C

    Serial.begin( 115200 );

    if(digitalRead(BUTTON_A_PIN) == 0) {
      Serial.println("Will Load menu binary");
      updateFromFS(SD);
      ESP.restart();
    }

```

  Headless setup can also be customized in complex integrations:

```C++

    Serial.begin( 115200 );

    SDUCfg.setCSPin( TFCARD_CS_PIN );
    SDUCfg.setFS( &SD );

    // set your own button response trigger

    static int buttonState;

    SDUCfg.setSDUBtnA( []() {
      return buttonState==LOW ? true : false;
    });

    SDUCfg.setSDUBtnPoller( []() {
      buttonState = digitalRead( 16 );
    });

    // Or set your own serial input trigger
    // SDUCfg.setWaitForActionCb( mySerialActionTrigger );

    SDUpdater sdUpdater( &SDUCfg );

    sdUpdater.checkSDUpdaterHeadless( MENU_BIN, 30000 ); // wait 30 seconds for serial input

```



<br />



  Use one of following methods to get the app on the filesystem:

  - Have the app copy itself to filesystem using BtnC from the lobby or implement `saveSketchToFS( SD, "/my_application.bin" );` from an option inside your app.

  - Manually copy it to the filesystem:
    - In the Arduino IDE menu go to "Sketch / Export Compiled Binary".
    - Rename the file to remove unnecessary additions to the name. The filename will be saved as "filename.ino.esp32.bin".
      Edit the name so it reads "filename.bin". This is purely for display purposes. The file will work without this change.

<br />



⌾ SD-Updater customizations:
----------------------------


  These callback setters are populated by default but only fit the best scenario (M5Stack with display+buttons).

  ⚠️ If no supported combination of display/buttons exists, it will fall back to headless behaviour and will only accept update/rollback signals from Serial.

  As a result, any atypical setup (e.g. headless+LittleFS) should make use of those callback setters:

```C++
  SDUCfg.setCSPin       ( TFCARD_CS_PIN );      // const int
  SDUCfg.setFS          ( &FS );                // fs::FS* (SD, SD_MMC, SPIFFS, LittleFS, PSRamFS)
  SDUCfg.setProgressCb  ( myProgress );         // void (*myProgress)( int state, int size )
  SDUCfg.setMessageCb   ( myDrawMsg );          // void (*myDrawMsg)( const String& label )
  SDUCfg.setErrorCb     ( myErrorMsg );         // void (*myErrorMsg)( const String& message, unsigned long delay )
  SDUCfg.setBeforeCb    ( myBeforeCb );         // void (*myBeforeCb)()
  SDUCfg.setAfterCb     ( myAfterCb );          // void (*myAfterCb)()
  SDUCfg.setSplashPageCb( myDrawSplashPage );   // void (*myDrawSplashPage)( const char* msg )
  SDUCfg.setButtonDrawCb( myDrawPushButton );   // void (*myDrawPushButton)( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor )
  SDUCfg.setWaitForActionCb( myActionTrigger ); // int  (*myActionTrigger)( char* labelLoad, char* labelSkip, unsigned long waitdelay )
  SDUCfg.setSDUBtnPoller( myButtonPoller );     // void (*myButtonPoller)()
  SDUCfg.setSDUBtnA( myBtnAPushedcb );          // bool (*myBtnAPushedcb )()
  SDUCfg.setSDUBtnB( myBtnBPushedcb );          // bool (*myBtnBPushedcb )()
  SDUCfg.setSDUBtnC( myBtnCPushedcb );          // bool (*myBtnCPushedcb )()

```



Set custom action trigger for `update`, `rollback`, `save` and `skip` lobby options:
```C++
  // int myActionTrigger( char* labelLoad,  char* labelSkip, unsigned long waitdelay )
  // return values: 1=update, 0=rollback, -1=skip
  SDUCfg.setWaitForActionCb( myActionTrigger );

  // Or separately if a UI is available:

  static int buttonAState;
  static int buttonBState;
  static int buttonCState;

  SDUCfg.setSDUBtnPoller( []() {
    buttonAState = digitalRead( 32 );
    buttonBState = digitalRead( 33 );
    buttonCState = digitalRead( 13 );
    delay(50);
  });

  SDUCfg.setSDUBtnA( []() {
    return buttonState==LOW ? true : false;
  });

  SDUCfg.setSDUBtnB( []() {
    return buttonState==LOW ? true : false;
  });

  SDUCfg.setSDUBtnC( []() {
    return buttonState==LOW ? true : false;
  });


```

  Example:

```C++

static int myActionTrigger( char* labelLoad,  char* labelSkip, char* labelSave, unsigned long waitdelay )
{
  int64_t msec = millis();
  do {
    if( Serial.available() ) {
      String out = Serial.readStringUntil('\n');
      if(      out == "update" )  return SDU_BTNA_MENU; // load "/menu.bin"
      else if( out == "rollback") return SDU_BTNA_ROLLBACK; // rollback to other OTA partition
      else if( out == "save")     return SDU_BTNC_SAVE; // save current sketch to SD card
      else if( out == "skip" )    return SDU_BTNB_SKIP; // do nothing
      else Serial.printf("Ignored command: %s\n", out.c_str() );
    }
  } while( msec > int64_t( millis() ) - int64_t( waitdelay ) );
  return -1;
}

void setup()
{
  Serial.begin(115200);

  SDUCfg.setAppName( "My Application" );         // lobby screen label: application name
  SDUCfg.setAuthorName( "by @myself" );          // lobby screen label: application author
  SDUCfg.setBinFileName( "/MyApplication.bin" ); // if file path to bin is set for this app, it will be checked at boot and created if not exist

  SDUCfg.setWaitForActionCb( myActionTrigger );

  checkSDUpdater( SD );

}

```

Set custom progress (for filesystem operations):
```C++
  // void (*myProgress)( int state, int size )
  SDUCfg.setProgressCb( myProgress );
```

Set custom notification/warning messages emitter:
```C++
  // void (*myDrawMsg)( const String& label )
  SDUCfg.setMessageCb( myDrawMsg );
```

Set custom error messages emitter:
```C++
  // void (*myErrorMsg)( const String& message, unsigned long delay )
  SDUCfg.setErrorCb( myErrorMsg );
```

Set pre-update actions (e.g. capture display styles):
```C++
  // void (*myBeforeCb)()
  SDUCfg.setBeforeCb( myBeforeCb );
```

Set post-update actions (e.g. restore display styles):
```C++
  // void (*myAfterCb)()
  SDUCfg.setAfterCb( myAfterCb );
```

Set lobby welcome message (e.g. draw UI welcome screen):
```C++
  // void (*myDrawSplashPage)( const char* msg )
  SDUCfg.setSplashPageCb( myDrawSplashPage );
```

Set buttons drawing function (useful with Touch displays)
```C++
  // void (*myDrawPushButton)( const char* label, uint8_t buttonIndex, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor )
  SDUCfg.setButtonDrawCb( myDrawPushButton );
```

Set buttons state polling function (typically M5.update()
```C++
  // void(*myButtonPollCb)();
  SDUCfg.setSDUBtnPoller( myButtonPollCb );
```

Set each button state getter function, it must return true when the state is "pushed".
```C++
  SDUCfg.setSDUBtnA( myBtnAPushedcb ); // bool (*myBtnAPushedcb )()
  SDUCfg.setSDUBtnB( myBtnBPushedcb ); // bool (*myBtnBPushedcb )()
  SDUCfg.setSDUBtnC( myBtnCPushedcb ); // bool (*myBtnCPushedcb )()
```



<br />
<br />

📚 SD-Menu loading usage:
-------------------------

**Default behaviour:** when an app is loaded in memory, booting the M5Stack with the `Button A` pushed will load and run `menu.bin` from the filesystem (or from OTA2 if using persistence).

**Custom behaviour:** the `Button A` push event car be replaced by any other means (Touch, Serial, Network, Sensor).

Ideally that SD-Menu application should list all available apps on the sdcard and provide means to load them on demand.

For example in the SD-Menu application provided with the examples of this repository, booting the M5Stack with the `Button A` pushed will power it off.

The built-in Download utility of the SD-Menu is has been moved to the `AppStore.ino` example, as a result the menu.bin size is reduced and loads faster.
This is still being reworked though.

Along with the default SD-Menu example of this repository, some artwork/credits can be added for every uploaded binary.
The default SD-Menu application will scan for these file types:

  - .bin compiled application binary

  - .jpg image/icon (max 100x100)

  - .json file with dimensions descriptions:

  `{"width":120,"height":120,"authorName":"tobozo","projectURL":"http://short.url","credits":"** http://very.very.long.url ~~"}`


<br />

  ⚠️ The jpg/json file names must match the bin file name, case matters!
  jpg/json meta files are optional but must both be set if provided.
  The value for "credits" JSON property will be scrolled on the top of the screen while the value for *projectURL* JSON property
  will be rendered as a QR Code in the info window. It is better provide a short URL for *projectURL* so the resulting QR Code
  has more error correction.

<br />
<br />


🚫 LIMITATIONS:
---------------
- SPIFFS/LittleFS libraries limit their file names (including path) to 32 chars but 16 is recommended as it is also printed in the lobby screen.
- Long file names will eventually get covered by the jpg image, better stay under 16 chars (not including the extension).
- Short file names may be treated as 8.3 (e.g 2048.bin becomes 2048.BIN).
- FAT specifications prevent having more than 512 files on the SD Card, but this menu is limited to 256 Items anyway.


<br />

🔘 OPTIONAL:
------------

- The lobby screen at boot can be customized using `SDUCfg.setAppName`, `SDUCfg.setAuthorName` and `SDUCfg.setBinFileName`.
When set, the app name and the binary path will be visible on the lobby screen, and an extra button `Button C` labelled `Save` is added to the UI.
Pushing this button while the M5Stack is booting will create or overwrite the sketch binary to the SD Card.
This can be triggered manually by using `saveSketchToFS(SD, fileName, TFCARD_CS_PIN)`.

```C++
  SDUCfg.setAppName( "My Application" ); // lobby screen label: application name
  SDUCfg.setBinFileName( "/MyApplication.bin" ); // if file path to bin is set for this app, it will be checked at boot and created if not exist
```

- It can also be disabled (although this requires to use the early callback setters):

```C++
#define SDU_HEADLESS
#include "M5StackUpdater.h"
```


- Although not extensively tested, the default binary name to be loaded (`/menu.bin`) can be changed at compilation time by defining the `MENU_BIN` constant:

```C++
#define MENU_BIN "/my_custom_launcher.bin"
#include "M5StackUpdater.h"
```

- Gzipped firmwares are supported when `SDU_ENABLE_GZ` macro is defined or when [ESP32-targz.h](https://github.com/tobozo/ESP32-targz) was previously included.
  The firmware must have the `.gz.` extension and be a valid gzip file to trigger the decompression.

```C++
#define SDU_ENABLE_GZ // enable support for gzipped firmwares
#include "M5StackUpdater.h"

void setup()
{
  checkSDUpdater( SD, "/menu.gz", 2000 );
}
```


- The JoyPSP and [M5Stack-Faces](https://github.com/m5stack/faces) Controls for M5Stack SD Menu necessary code are now disabled in the menu example but the code stays here and can be used as a boilerplate for any other two-wires input device.


<br />

 ⚠️ KNOWN ISSUES
------------

- `SD was not declared in this scope`: make sure your `#include <SD.h>` is made *before* including `<M5StackUpdater.h>`
- Serial message `[ERROR] No filesystem selected` or `[ERROR] No valid filesystem selected`: try `SDUCfg.setFS( &SD )` prior to calling the SDUpdater.


<br /><br />



🏭 Factory Partition
--------------------

Abuse the OTA partition scheme and store up to 5 applications on the flash, plus the firmware loader.

⚠️ This scenario uses a special [firmware loader](https://github.com/tobozo/M5Stack-SD-Updater/tree/1.2.8/examples/M5Stack-FW-Menu) `M5Stack-FW-Menu`, a custom partition scheme, and a different integration of M5Stack-SD-Updater in the loadable applications.

Although it can work without the SD Card, `M5Stack-FW-Menu` can still act as a low profile replacement for the classic SD Card `/menu.bin`, and load binaries from the SD Card or other supported filesystems.


#### Requirements:

- Flash size must be 8MB or 16MB
- custom partitions.csv must have more than 2 OTA partitions followed by one factory partition (see annotated example below)
- loadable applications and firmware loader must share the same custom partitions scheme at compilation
- `SDUCfg.rollBackToFactory = true;` must be set in all loadable applications (see `Detect factory support`)

#### Custom partition scheme annotated example:

```csv
# 6 Apps + Factory
# Name,   Type, SubType,    Offset,     Size
nvs,      data, nvs,        0x9000,   0x5000
otadata,  data, ota,        0xe000,   0x2000
ota_0,    0,    ota_0,     0x10000, 0x200000  ,<< Default partition for flashing (UART, 2MB)
ota_1,    0,    ota_1,    0x210000, 0x200000  ,<< Default partition for flashing (OTA, 2MB)
ota_2,    0,    ota_2,    0x410000, 0x200000  ,<< Application (2MB)
ota_3,    0,    ota_3,    0x610000, 0x200000  ,<< Application (2MB)
ota_4,    0,    ota_4,    0x810000, 0x200000  ,<< Application (2MB)
ota_5,    0,    ota_5,    0xA10000, 0x200000  ,<< Application (2MB)
firmware, app,  factory,  0xC10000, 0x0F0000  ,<< Factory partition holding the firmware menu (960KB)
spiffs,   data, spiffs,   0xD00000, 0x2F0000  ,<< SPIFFS (2MB)
coredump, data, coredump, 0xFF0000,  0x10000
```

#### Quick Start:

- Set a custom partition scheme according to the requirements (see annotated example above)
- Compile and flash the [M5Stack-FW-Menu]https://github.com/tobozo/M5Stack-SD-Updater/tree/master/examples/M5Stack-FW-Menu)
- On first run the `M5Stack-FW-Menu` firmware loader will automatically populate the factory partition and restart from there

Then for every other app you want to store on the flash:

- Set the same custom partition scheme as `M5Stack-FW-Menu`
- Add `SDUCfg.rollBackToFactory = true;` and second argument must be empty e.g. `checkSDUpdater( SD, "", 5000, TFCARD_CS_PIN )`
- Compile your app
- Copy the binary to the SD Card (e.g. copy the bin manually or use the `Save SD` option from the app's SD-Updater lobby)
- Use `FW Menu` option from the app's SD-Updater lobby (note: **it should load the firmware loader, not the /menu.bin from the SD Card**)
- Use the firmware loader menu `Manage Partitions/Add Firmware` to copy the recently added app to one of the available slots

Note: the firmware loader can copy applications from any filesystem (SD, SD_MMC, SPIFFS, LittleFS, FFat).


#### Detect factory support

```cpp
#if M5_SD_UPDATER_VERSION_INT >= VERSION_VAL(1, 2, 8)
// New SD Updater support, requires version >=1.2.8 of https://github.com/tobozo/M5Stack-SD-Updater/
if( Flash::hasFactoryApp() ) {
  SDUCfg.rollBackToFactory = true;
  SDUCfg.setLabelMenu("FW Menu");
  SDUCfg.setLabelRollback("Save FW");
  checkFWUpdater( 5000 );
} else
#endif
{
  checkSDUpdater( SD, MENU_BIN, 5000, TFCARD_CS_PIN );
}
```







🛣 ROADMAP:
----------
- Completely detach the UI/Display/Touch/Buttons from the codebase
- Support gzipped binaries
~~- Migrate Downloader / WiFiManager to external apps~~
- esp-idf support
- Contributors welcome!

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
| 👍     | @lovyan03           | らびやん           | https://github.com/lovyan03                 |
| 👍     | @matsumo            | Matsumo          | https://github.com/matsumo                   |
| 👍     | @riraosan           | Riraosan         | https://github.com/riraosan                  |
| 👍     | @ockernuts          | ockernuts        | https://github.com/ockernuts                 |
