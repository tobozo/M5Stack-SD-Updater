/*
 * 
 * M5Stack SD Menu
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
 * This sketch is the menu application. It must be compiled once 
 * (sketch / export compiled binary) and saved on the SD Card as 
 * "menu.bin" for persistence, and initially flashed on the M5Stack.
 * 
 * It will list all available apps on the sdcard and load them on 
 * demand. Once you're finished with the app, you reset+BTN_A and
 * it loads the menu again. Rinse and repeat.
 *  
 * Obviously none of those apps will embed this menu, instead they
 * must include and implement the M5Stack SD Loader Snippet, a lighter
 * version of the loader dedicated to loading the menu.
 * 
 * Usage: Push BTN_A on boot calls the menu (in app) or powers the M5Stack off (in menu)
 * 
 * Accepted file types on the sd:
 *   - [sketch name].bin the Arduino binary
 *   - [sketch name].jpg an image (max 200x100 but smaller is better)
 *   - [sketch name].json file with dimensions descriptions: {"width":xxx,"height":xxx} 
 * The file names must be the same (case matters!).
 * jpg and json are optional but must both be set if provided.
 * 
 * 
 */

#include <M5Stack.h>        // https://github.com/m5stack/M5Stack/
#include "M5StackUpdater.h" // https://github.com/tobozo/M5Stack-SD-Updater
#include <M5StackSAM.h>     // https://github.com/tomsuch/M5StackSAM
#include <ArduinoJson.h>    // https://github.com/bblanchon/ArduinoJson/

#define MAX_FILES 255 // this affects memory

/* filenames cache structure */
struct FileInfo {
  String fileName;  // heap heap heap
  uint32_t fileSize;
  bool hasIcon = false;
  bool hasMeta = false;
};
/* these are the types we're looking for */
struct JSONMeta {
  int width;
  int height;
};

JSONMeta jsonMeta;
FileInfo fileInfo[MAX_FILES];
M5SAM M5Menu;

uint16_t appsCount = 0;

/* not sure that was really necessary */
bool fileExists(fs::FS &fs, const char * fileName) {
  File file = fs.open(fileName);
  if(!file){
    return false;
  }
  return true;
}

/* recursivity hasn't been tested yet, create folders at your own risks */
void listDir(fs::FS &fs, const char * dirName, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirName);

  File root = fs.open(dirName);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      String fileName = file.name();
      if(fileName!="/menu.bin" && fileName.endsWith(".bin")) {
        Serial.println("  FILE: " + fileName );
        String currentIconFile = fileName;
        String currentMetaFile = fileName;
        currentIconFile.replace(".bin", ".jpg");
        currentMetaFile.replace(".bin", ".json");
        fileInfo[appsCount].fileName = fileName;
        fileInfo[appsCount].fileSize = file.size();
        if(fileExists(fs, currentIconFile.c_str())) {
          fileInfo[appsCount].hasIcon = true;
        }
        if(fileExists(fs, currentIconFile.c_str())) {
          fileInfo[appsCount].hasMeta = true;
        }
        appsCount++;
      } else {
        // ignored files
        Serial.println("  IGNORED FILE: " + fileName );
      }
    }
    file = root.openNextFile();
  }
}


/* bubble sort */
void aSortFiles() {
  bool swapped;
  FileInfo temp;
  String name1, name2;
  do {
    swapped = false;
    for(uint16_t i=0; i<appsCount-1; i++ ) {
      name1 = fileInfo[i].fileName[0];
      name2 = fileInfo[i+1].fileName[0];
      if(name1==name2) {
        name1 = fileInfo[i].fileName[1];
        name2 = fileInfo[i+1].fileName[1];
        if(name1==name2) {
          name1 = fileInfo[i].fileName[2];
          name2 = fileInfo[i+1].fileName[2];        
        } else {
          // give it up :-)
        }
      }

      if (name1 > name2) {
        temp = fileInfo[i];
        fileInfo[i] = fileInfo[i + 1];
        fileInfo[i + 1] = temp;
        swapped = true;
      }
    }
  } while (swapped);
}


void buildM5Menu() {
  M5Menu.clearList();
  M5Menu.setListCaption("Applications");
  for(uint16_t i=0;i<appsCount;i++) {
    String shortName = fileInfo[i].fileName.substring(1);
    shortName.replace(".bin", "");
    M5Menu.addList(shortName);
  }
}


void doM5Menu() {
  M5Menu.drawAppMenu(F("SD CARD LOADER"),F("INFO"),F("LOAD"),F(">"));
  M5Menu.showList();
  bool inmenu = false;
  while(!M5.BtnB.wasPressed()){
    if(M5.BtnC.wasPressed()){
      M5Menu.nextList();
    }
    if(M5.BtnA.wasPressed()){
      if(!inmenu) {
        inmenu = true;
        M5Menu.windowClr();
        // file must be jpeg and json must contain dimensions as follows:  {"width":11,"height":22}
        if( fileInfo[ M5Menu.getListID() ].hasMeta && fileInfo[ M5Menu.getListID() ].hasIcon) {
          String metaFile = fileInfo[ M5Menu.getListID() ].fileName;
          metaFile.replace(".bin", ".json");
          File file = SD.open(metaFile);
          StaticJsonBuffer<64> jsonBuffer;
          JsonObject &root = jsonBuffer.parseObject(file);
          if (root.success()) {
            jsonMeta.width  = root["width"];
            jsonMeta.height = root["height"];
            if(jsonMeta.width>0 && jsonMeta.height>0) {
              String iconFile = fileInfo[ M5Menu.getListID() ].fileName;
              iconFile.replace(".bin", ".jpg");
              M5.Lcd.drawJpgFile(SD, iconFile.c_str(), (M5.Lcd.width()/2)-(jsonMeta.width/2), 30, jsonMeta.width, jsonMeta.height, 0, 0, JPEG_DIV_NONE);
            }
          }
        }
        M5.Lcd.drawCentreString("File Name: "+fileInfo[ M5Menu.getListID() ].fileName,M5.Lcd.width()/2,(M5.Lcd.height()/2)-10,2);
        M5.Lcd.drawCentreString("File Size: "+String(fileInfo[ M5Menu.getListID() ].fileSize),M5.Lcd.width()/2,(M5.Lcd.height()/2)+10,2);
      } else {
        inmenu = false;
        M5Menu.showList();
      }
    }
    M5.update();
  }
  updateFromFS(SD, fileInfo[ M5Menu.getListID() ].fileName);
  ESP.restart();
}


void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to the M5Stack SD Menu Loader!");
  Serial.print("M5Stack initializing...");
  M5.begin();
  Wire.begin();
  if(digitalRead(BUTTON_A_PIN) == 0) {
    M5.setWakeupButton(BUTTON_B_PIN);
    M5.powerOFF();
  }
  listDir(SD, "/", 0);
  aSortFiles();
  buildM5Menu();
}


void loop() {
  doM5Menu();
}
