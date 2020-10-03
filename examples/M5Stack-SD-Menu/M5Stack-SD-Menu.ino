/*
 *
 * M5Stack SD Menu
 * Project Page: https://github.com/tobozo/M5Stack-SD-Updater
 *
 * Copyright 2019 tobozo http://github.com/tobozo
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files ("M5Stack SD Updater"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * This sketch is the menu application. It must be compiled once
 * (sketch / export compiled binary) and saved on the SD Card as
 * "menu.bin" for persistence, and initially flashed on the M5Stack.
 *
 * If the SD is blank, or no "menu.bin" exists, it will attempt to
 * self-replicate on the filesystem and create the minimal necessary
 * directory structure.
 *
 * The very insecure YOLO Downloader is now part of this menu and can
 * take care of downloading the lastest binaries from the registry.
 *
 * As SD Card mounting can be a hassle, using the ESP32 Sketch data
 * uploader is also possible. Any file sent using this method will
 * automatically be copied onto the SD Card on next restart.
 * This includes .bin, .json, .jpg and .mod files.
 * To enable this feature, set "migrateSPIFFS" to false
 *
 * The menu will list all available apps on the sdcard and load them
 * on demand.
 *
 * Once you're finished with the loaded app, push reset+BTN_A and it
 * loads the menu again. Rinse and repeat.
 *
 * Most of those apps will not embed this launcher, instead they should
 * include and implement the M5Stack SD Loader Snippet, a lighter version
 * of the loader dedicated to load and launch the menu.
 *
 * Usage: Push BTN_A on boot calls the menu (in app) or powers off the
 * M5Stack (in menu)
 *
 * Accepted file types on the SD:
 *   - [sketch name].bin the Arduino binary
 *   - [sketch name].jpg an image (max 200x100 but smaller is better)
 *   - [sketch name].json file with dimensions descriptions: {"width":xxx,"height":xxx,"authorName":"tobozo", "projectURL":"http://blah"}
 *
 * The file names must be the same (case matters!) and left int heir relevant folders.
 * For this you will need to create two folders on the root of the SD:
 *   /jpg
 *   /json
 * jpg and json are optional but must both be set if provided.
 *
 *
 * Persistence and fast loading:
 * - To speed up things when reloading the menu, an Update.canRollback() test is done first
 * - Loader partition information (digest, size) is saved into NVS for additional consistency
 * - Any binary named "xxxLauncher" (e.g. LovyanLauncher) will become the default launcher if selected and loaded from the menu
 *
 *
 */

#include "menu.h"


void setup() {
  #if defined(_CHIMERA_CORE_)
    M5.begin(true, false, true, false, false); // bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable, bool ScreenShotEnable
  #else
    M5.begin(); // bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable, bool ScreenShotEnable
  #endif

  // suggest rollback
  //checkSDUpdater( M5_FS, "", 2000 );
  checkSDUpdater(
    SD,           // filesystem (default=SD)
    "",           // path to binary (default = /menu.bin, empty = rollback only)
    0,            // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // (usually default=4 but your mileage may vary)
  );

  #if defined(_CHIMERA_CORE_)
    // debug I2C
    //Wire.begin(SDA, SCL);
    //M5.I2C.scan();
  #endif


  //WiFi.onEvent(WiFiEvent); // helps debugging WiFi problems with the Serial console
  UISetup(); // UI init and check if a SD exists

  doFSChecks(); // replicate on SD and app1 partition, scan data folder, load registry
  doFSInventory(); // enumerate apps and render menu
}


void loop() {

  HIDMenuObserve();
  sleepTimer();

}
