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
#include <ESP32-Chimera-Core.h>
#include <M5StackUpdater.h>

void setup() {

  M5.begin();

  Serial.println("Welcome to the SD-Updater minimal example!");
  Serial.println("Now checking if a button was pushed during boot ...");

  checkSDUpdater();

  Serial.println("Nope, will run the sketch normally");

}

void loop() {

}
