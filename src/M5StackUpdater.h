#ifndef __M5STACKUPDATER_H
#define __M5STACKUPDATER_H
/*
 *
 * M5Stack SD Updater
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
 * (Note to self: remember it is shared by both contexts before 
 * messing with it!) 
 * 
 * This code is used by the menu but must also be included in
 * any app that will be compiled and copied the sd card.
 * 
 * 
 * In your sketch, find the line where M5 library is included:
 * 
 *   #include <M5Stack.h>
 *  
 * And add this:
 * 
 *  #include "M5StackUpdater.h"
 *  SDUpdater sdUpdater;
 *  
 * 
 * In your setup() function, find the following statements:
 * 
 *   M5.begin();
 *   Wire.begin()
 * 
 * And add this after 'Wire.begin();':
 * 
 *   if(digitalRead(BUTTON_A_PIN) == 0) {
 *     Serial.println("Will Load menu binary");
 *     sdUpdater.updateFromFS(SD);
 *     ESP.restart();
 *   }
 * 
 * And do whatever you need to do (button init, timers)
 * in the setup and the loop. Your app will be ready 
 * to run normally except at boot if the Button A is 
 * pressed, it will load the "menu.bin" from the sd card
 * 
 */

#include <M5Stack.h>
#include <Update.h>
#define MENU_BIN F("/menu.bin")

class SDUpdater {
  public: 
    void updateFromFS(fs::FS &fs, String fileName = MENU_BIN );
    static void M5SDMenuProgress(int state, int size);
    void displayUpdateUI(String label);
  private:
    void performUpdate(Stream &updateSource, size_t updateSize, String fileName);
};

/* don't break older versions of the M5Stack SD Updater */
static void updateFromFS(fs::FS &fs, String fileName = MENU_BIN ) {
  SDUpdater sdUpdater;
  sdUpdater.updateFromFS(fs, fileName);
}

#endif