/*
 *
 * M5Stack SD Loader Snippet
 * Project Page: https://github.com/tobozo/M5Stack-SD-Updater
 *
 * Copyright 2018 tobozo http://github.com/tobozo
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
 * To be used with M5Stack SD Menu https://github.com/tobozo/M5Stack-SD-Updater
 * This sketch is useless without a precompiled "menu.bin" saved on the SD Card.
 * You may compile menu.bin from the M5Stack-SD-Menu sketch.
 *
 * Just use this sketch as your boilerplate and code your app over it, then
 * compile it and put the binary on the sdcard. See M5Stack-SD-Menu for more
 * info on the acceptable file formats.
 *
 * When this app is in memory, booting the M5Stack with the Button A pushed will
 * flash back the menu.bin into memory.
 *
 */
/*
#if defined( ARDUINO_M5Stack_Core_ESP32 ) || defined( ARDUINO_M5STACK_FIRE ) // M5Stack Classic/Fire
  #include <M5Stack.h>
  // #include <ESP32-Chimera-Core.h>
#elif defined( ARDUINO_M5STACK_Core2 ) // M5Stack Core2
  #include <M5Core2.h>
#elif defined( ARDUINO_M5Stick_C ) // M5StickC
  #include <M5StickC.h>
#else
  #include <ESP32-Chimera-Core.h> // any other ESP32 device with SD
#endif
*/
#include <ESP32-targz.h> // optional: https://github.com/tobozo/ESP32-targz
#include <ESP32-Chimera-Core.h>
//#include <M5Stack.h>
// #define SDU_HEADLESS

#define SDU_APP_NAME "M5Stack SDLoader Snippet"
#define SDU_APP_PATH "/MY_SKETCH.bin"

#include <M5StackUpdater.h>

void setup()
{
  M5.begin();

  M5.Lcd.fillRect( 10, 10, 100, 100, TFT_BLUE );
  delay(1000);

  Serial.println("Welcome to the SD-Updater minimal example!");
  Serial.println("Now checking if a button was pushed during boot ...");

  SDUCfg.setLabelMenu("<< Menu");        // BtnA label: load menu.bin
  SDUCfg.setLabelSkip("Launch");         // BtnB label: skip the lobby countdown and run the app
  SDUCfg.setLabelSave("Save");           // BtnC label: save the sketch to the SD
  SDUCfg.setAppName( SDU_APP_NAME );     // Lobby screen label: application name
  SDUCfg.setBinFileName( SDU_APP_PATH ); // If file path to bin is set for this app, it will be checked at boot and created if not exist

  // checkSDUpdater( SD );
  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    5000,         // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // (usually default=4 but your mileage may vary)
  );
  Serial.println("Nope, will run the sketch normally");
  M5.Lcd.print("M5Stack SD Loader test");
}


void loop()
{
  M5.update();
  // provide means to copy the sketch to filesystem
  if( M5.BtnB.pressedFor( 1000 ) ) {
    Serial.println("Will copy this sketch to filesystem");
    if( saveSketchToFS( SD, SDU_APP_PATH, TFCARD_CS_PIN ) ) {
      Serial.println("Copy successful !");
    } else {
      Serial.println("Copy failed !");
    }
  }

}
