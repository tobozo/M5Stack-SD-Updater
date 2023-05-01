/*
 *
 * M5StickC SPIFFS Loader Snippet
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
 * To be used with M5Stack SD Menu https://github.com/tobozo/M5Stack-SD-Updater
 * This sketch is useless without a precompiled "menu.bin" saved on the SD Card.
 * You may compile menu.bin from the M5Stack-SD-Menu sketch.
 *
 * Just use this sketch as your boilerplate and code your app over it, then
 * compile it and put the binary on the sdcard. See M5Stack-SD-Menu for more
 * info on the acceptable file formats.
 *
 * When this app is in memory, booting the M5StickC with the Button A pushed will
 * flash back the menu.bin into memory.
 *
 */
#include <ESP32-targz.h> // optional: https://github.com/tobozo/ESP32-targz
#include <M5StickC.h>
#include <M5StackUpdater.h>

//#define APP2

#if defined APP1
  #pragma message "Compiling app1.bin"

  void setup() {
    Serial.begin(115200);
    Serial.println("Welcome to the SPIFFS-Update example!");
    Serial.print("M5StickC initializing...");
    M5.begin();
    M5.update();
    M5.Lcd.setRotation(3);
    M5.Lcd.println("**** APP1 ****");
    M5.Lcd.println("BtnA: rollback");
    M5.Lcd.println("BtnB: app2");
  }

  void loop()
  {
    M5.update();
    if( M5.BtnA.wasPressed() ) {
      updateRollBack("Hot-Loading");
    }
    if( M5.BtnB.wasPressed() ) {
      updateFromFS( SPIFFS, "/app2.bin" );
    }
  }


#elif defined APP2

  #pragma message "Compiling app2.bin"

  void setup() {
    Serial.begin(115200);
    Serial.println("Welcome to the SPIFFS-Update example!");
    Serial.print("M5StickC initializing...");
    M5.begin();
    M5.update();
    M5.Lcd.setRotation(3);
    M5.Lcd.println("**** APP2 ****");
    M5.Lcd.println("BtnA: rollback");
    M5.Lcd.println("BtnB: app1");
  }

  void loop()
  {
    M5.update();
    if( M5.BtnA.wasPressed() ) {
      updateRollBack("Hot-Loading");
    }
    if( M5.BtnB.wasPressed() ) {
      updateFromFS( SPIFFS, "/app1.bin" );
    }
  }

#else

  #pragma message "Compiling example"

  void setup() {
    Serial.begin(115200);
    Serial.println("Welcome to the SPIFFS-Update example!");
    Serial.print("M5StickC initializing...");
    M5.begin();
    M5.update();
    M5.Lcd.setRotation(3);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("-> LAUNCHER <");

    M5.Lcd.println("BtnA: app1");
    M5.Lcd.println("BtnB: app2");
  }

  void loop()
  {
    M5.update();
    if( M5.BtnA.wasPressed() ) {
      updateFromFS( SPIFFS, "/app1.bin" );
    }
    if( M5.BtnB.wasPressed() ) {
      updateFromFS( SPIFFS, "/app2.bin" );
    }
  }

#endif
