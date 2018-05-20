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
 */

#include "M5StackUpdater.h"


void SDUpdater::displayUpdateUI(String label) {
  M5.Lcd.setBrightness(100);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf(label.c_str());
  M5.Lcd.drawRect(110, 130, 102, 20, WHITE);
}


void SDUpdater::M5SDMenuProgress(int state, int size) {
  int percent = (state*100) / size;
  Serial.printf("percent = %d\n", percent);
  uint16_t x = M5.Lcd.getCursorX();
  uint16_t y = M5.Lcd.getCursorY();

  M5.Lcd.setTextColor(WHITE, BLACK);

  if (percent > 0 && percent < 101) {
    M5.Lcd.fillRect(111, 131, percent, 18, GREEN);
    M5.Lcd.fillRect(111+percent, 131, 100-percent, 18, BLACK);
  } else {
    M5.Lcd.fillRect(111, 131, 100, 18, BLACK);
  }
  M5.Lcd.setCursor(150, 155);
  M5.Lcd.print(String(percent) + "% "); // trailing space is important
  M5.Lcd.setCursor(x, y);
}

// perform the actual update from a given stream
void SDUpdater::performUpdate(Stream &updateSource, size_t updateSize, String fileName) {
   displayUpdateUI("LOADING " + fileName);
   Update.onProgress(M5SDMenuProgress);
   if (Update.begin(updateSize)) {
      size_t written = Update.writeStream(updateSource);
      if (written == updateSize) {
         Serial.println("Written : " + String(written) + " successfully");
      } else {
         Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      }
      if (Update.end()) {
         Serial.println("OTA done!");
         if (Update.isFinished()) {
            Serial.println("Update successfully completed. Rebooting.");
         } else {
            Serial.println("Update not finished? Something went wrong!");
         }
      } else {
         Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
   } else {
      Serial.println("Not enough space to begin OTA");
   }
}

// check given FS for valid menu.bin and perform update if available
void SDUpdater::updateFromFS(fs::FS &fs, String fileName) {
  // Thanks to Macbug for the hint, my old ears couldn't hear the buzzing :-)
  // See Macbug's excellent article on this tool:
  // https://macsbug.wordpress.com/2018/03/12/m5stack-sd-updater/
  dacWrite(25, 0); // turn speaker signal off
  // Also thanks to @Kongduino for a complementary way to turn off the speaker:
  // https://twitter.com/Kongduino/status/980466157701423104
  ledcDetachPin(25); // detach DAC
  File updateBin = fs.open(fileName);
  if (updateBin) {
    if(updateBin.isDirectory()){
      Serial.println("Error, this is not a file");
      updateBin.close();
      return;
    }
    size_t updateSize = updateBin.size();
    if (updateSize > 0) {
      Serial.println("Try to start update");
      performUpdate(updateBin, updateSize, fileName);
    } else {
       Serial.println("Error, file is empty");
    }
    updateBin.close();
  } else {
    Serial.println("Could not load binary from sd root");
  }
}
