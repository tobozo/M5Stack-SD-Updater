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
 *   - [sketch name].json file with dimensions descriptions: {"width":xxx,"height":xxx,"authorName":"tobozo", "projectURL":"http://blah"} 
 * The file names must be the same (case matters!) and left int heir relevant folders.
 * For this you will need to create two folders on the root of the SD:
 *   /jpg
 *   /json
 * jpg and json are optional but must both be set if provided.
 * 
 * 
 */

#include <M5Stack.h>             // https://github.com/m5stack/M5Stack/
#include "M5StackUpdater.h"      // https://github.com/tobozo/M5Stack-SD-Updater
#include <M5StackSAM.h>          // https://github.com/tomsuch/M5StackSAM
#include <ArduinoJson.h>         // https://github.com/bblanchon/ArduinoJson/
#include "assets.h"              // some artwork for the UI
#include "qrcode.h"              // https://github.com/ricmoo/qrcode


#define MAX_FILES 255 // this affects memory

/* 
 * 
 * PSP JoyPad control plugin is default disabled
 * but provided as an example for custom controls.
 * 
 */
#define USE_PSP_JOY true
#ifdef USE_PSP_JOY
  #include "joyPSP.h"
#endif


/* Storing json meta file information r */
struct JSONMeta {
  int width;
  int height;
  String authorName = "";
  String projectURL = "";
  // TODO: add more interesting properties
};

/* filenames cache structure */
struct FileInfo {
  String fileName;  // the binary name
  String metaName;  // a json file with all meta info on the binary
  String iconName;  // a jpeg image representing the binary
  uint32_t fileSize;
  bool hasIcon = false;
  bool hasMeta = false;
  JSONMeta jsonMeta;
};


FileInfo fileInfo[MAX_FILES];
M5SAM M5Menu;

uint16_t appsCount = 0; // how many binary files
bool inmenu = false; // menu state machine
unsigned long lastcheck = millis(); // timer check
uint16_t checkdelay = 300; // timer frequency
uint16_t MenuID; // pointer to the current menu item selected
int16_t scrollPointer = 0; // pointer to the scrollText position
unsigned long lastScrollRender = millis(); // timer for scrolling
String lastScrollMessage; // last scrolling string state
int16_t lastScrollOffset; // last scrolling string position


void getMeta(String metaFileName, JSONMeta &jsonMeta) {
  File file = SD.open(metaFileName);
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);
  if (root.success()) {
    jsonMeta.width  = root["width"];
    jsonMeta.height = root["height"];
    jsonMeta.authorName = root["authorName"].as<String>();
    jsonMeta.projectURL = root["projectURL"].as<String>();
  }
}


void renderScroll(String &scrollText, uint8_t x, uint8_t y, uint16_t width) {
  if(scrollText=="") return;
  unsigned long now = millis();
  if(lastScrollRender+60>millis()) return;
  M5.Lcd.setTextSize(2);
  //lastScrollRender = now;
  int16_t textWidth = M5.Lcd.textWidth(scrollText);
  int16_t vsize = 0;
  String scrollMe = "";
  int16_t vpos;
  uint8_t csize, lastcsize;
  
  scrollPointer-=1;
  if(scrollPointer<-textWidth) {
    scrollPointer = 0 + M5.Lcd.textWidth(" ");
    vsize = scrollPointer;
  }
  
  while( M5.Lcd.textWidth(scrollMe) < width ) {
    scrollMe +=" ";
    for(uint8_t i=0;i<scrollText.length();i++) {
      char thisChar[2];
      thisChar[0] = scrollText[i];
      thisChar[1] = '\0';
      csize = M5.Lcd.textWidth(thisChar);
      vsize+=csize;
      vpos = vsize+scrollPointer;
      
      if(vpos>x && vpos<=x+width) {
        scrollMe += scrollText[i];
        lastcsize = csize;
      }
    }
  }

  int16_t voffset = scrollPointer%lastcsize;
  int16_t scrollOffset = x+voffset;

  // trim if necessary
  while( M5.Lcd.textWidth(scrollMe) > width-voffset ) {
    scrollMe.remove(scrollMe.length()-1);
  }

  // only draw if things changed
  if(scrollOffset!=lastScrollOffset || scrollMe!=lastScrollMessage) {
    // delete last message
    M5.Lcd.setCursor(lastScrollOffset, y);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.print(lastScrollMessage );
    // write new message  
    M5.Lcd.setCursor(scrollOffset, y);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.print(scrollMe);
  }

  M5.Lcd.setTextSize(1);
  lastScrollMessage = scrollMe;
  lastScrollOffset  = scrollOffset;
  lastScrollRender  = millis();
}


/* by menu ID */
void renderIcon(uint16_t MenuID) {
  renderIcon(fileInfo[MenuID]);
}

/* by file info */
void renderIcon(FileInfo &fileInfo) {
  if(!fileInfo.hasMeta || !fileInfo.hasIcon) {
    return;
  }
  JSONMeta jsonMeta = fileInfo.jsonMeta;
  M5.Lcd.drawJpgFile(SD, fileInfo.iconName.c_str(), M5.Lcd.width()-jsonMeta.width-10, (M5.Lcd.height()/2)-(jsonMeta.height/2)+10, jsonMeta.width, jsonMeta.height, 0, 0, JPEG_DIV_NONE);
}


void renderMeta(JSONMeta &jsonMeta) {
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, (M5.Lcd.height()/2)-25);
  M5.Lcd.print(fileInfo[MenuID].fileName);
  M5.Lcd.setCursor(10, (M5.Lcd.height()/2)+10);
  M5.Lcd.print(String(fileInfo[MenuID].fileSize) + " bytes");
  M5.Lcd.setCursor(10,(M5.Lcd.height()/2)-10);  
  
  if(jsonMeta.authorName!="" && jsonMeta.projectURL!="" ) { // both values provided
    M5.Lcd.print("By ");
    M5.Lcd.print(jsonMeta.authorName);
    qrRender(jsonMeta.projectURL, 180);
  } else if(jsonMeta.projectURL!="") { // only projectURL
    M5.Lcd.print(jsonMeta.projectURL);
    qrRender(jsonMeta.projectURL, 180);
  } else { // only authorName
    M5.Lcd.drawCentreString(jsonMeta.authorName,M5.Lcd.width()/2,(M5.Lcd.height()/2)-25,2);
  }
}


void qrRender(String text, float sizeinpixels) {
  // see https://github.com/Kongduino/M5_QR_Code/blob/master/M5_QRCode_Test.ino
  // Create the QR code
  QRCode qrcode;
  uint8_t version = 6;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, 0, text.c_str());

  uint8_t thickness = sizeinpixels / qrcode.size;
  uint16_t lineLength = qrcode.size * thickness;
  uint8_t xOffset = ((M5.Lcd.width() - (lineLength)) / 2) + 60;
  uint8_t yOffset = (M5.Lcd.height() - (lineLength)) / 2;

  M5.Lcd.fillRect(xOffset-5, yOffset-5, lineLength+10, lineLength+10, WHITE);
  
  for (uint8_t y = 0; y < qrcode.size; y++) {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcode.size; x++) {
      bool q = qrcode_getModule(&qrcode, x, y);
      if (q) {
        M5.Lcd.fillRect(x * thickness + xOffset, y * thickness + yOffset, thickness, thickness, TFT_BLACK);
      }
    }
  }
  
}


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
        
        fileInfo[appsCount].fileName = fileName;
        fileInfo[appsCount].fileSize = file.size();

        String currentIconFile = "/jpg" + fileName;
        currentIconFile.replace(".bin", ".jpg");
        if(SD.exists(currentIconFile.c_str())) {
          fileInfo[appsCount].hasIcon = true;
          fileInfo[appsCount].iconName = currentIconFile;
        }
        String currentMetaFile = "/json" + fileName;
        currentMetaFile.replace(".bin", ".json");
        if(SD.exists(currentMetaFile.c_str())) {
          fileInfo[appsCount].hasMeta = true;
          fileInfo[appsCount].metaName = currentMetaFile;
        }
        if(fileInfo[appsCount].hasMeta == true) {
          getMeta(fileInfo[appsCount].metaName, fileInfo[appsCount].jsonMeta);
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


/* bubble sort filenames */
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
  renderIcon(0);
  inmenu = false;
  lastcheck = millis();
  checkdelay = 300;
  
  while(!M5.BtnB.wasPressed()){
    
    MenuID = M5Menu.getListID();

    #ifdef USE_PSP_JOY
      handleJoyPad();
    #endif
    
    if(M5.BtnC.wasPressed()){
      M5Menu.drawAppMenu(F("SD CARD LOADER"),F("INFO"),F("LOAD"),F(">"));
      M5Menu.nextList();
      MenuID = M5Menu.getListID();
      renderIcon(MenuID);
      inmenu = false;
    }
    
    if(M5.BtnA.wasPressed()){
      if(!inmenu) {
        inmenu = true;
        M5Menu.windowClr();
        renderMeta(fileInfo[MenuID].jsonMeta);
      } else {
        inmenu = false;
        M5Menu.drawAppMenu(F("SD CARD LOADER"),F("INFO"),F("LOAD"),F(">"));
        M5Menu.showList();
        renderIcon(MenuID);
      }
    }

    if(inmenu) {
      // do scroll text
      renderScroll(fileInfo[MenuID].jsonMeta.projectURL, 0, 5, 320);
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

  // Thanks to Macbug for the hint, my old ears couldn't hear 
  // the buzzing :-) 
  dacWrite(25, 0); // turn speaker off
  // See Macbug's excellent article on this tool:
  // https://macsbug.wordpress.com/2018/03/12/m5stack-sd-updater/
  
  if(digitalRead(BUTTON_A_PIN) == 0) {
    M5.setWakeupButton(BUTTON_B_PIN);
    M5.powerOFF();
  }
  
  M5.Lcd.setBrightness(100);
  M5.Lcd.setTextSize(2);

  lastcheck = millis();
  bool toggle = true;

  while(!SD.begin(TFCARD_CS_PIN)) {
    // TODO: make a more fancy animation
    unsigned long now = millis();
    toggle = !toggle;
    uint16_t color = toggle ? BLACK : WHITE;
    M5.Lcd.setCursor(10,100);
    M5.Lcd.setTextColor(color);
    M5.Lcd.print("Insert SD");
    if(toggle) {
      M5.Lcd.drawJpg(disk01_jpg, 1775, 160, 100);
      delay(300);
    } else {
      M5.Lcd.drawJpg(disk00_jpg, 1775, 160, 100);
      delay(500);
    }

    // go to sleep after a minute
    if(lastcheck+60000<now) {
      Serial.println("Will go to sleep after waiting too long for SD Card insertion (meh)");
      M5.setWakeupButton(BUTTON_B_PIN);
      M5.powerOFF();
    }
  }

  // TODO: animate loading screen
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  
  listDir(SD, "/", 0);
  aSortFiles();
  buildM5Menu();

  // TODO: implement other controls
  #ifdef USE_PSP_JOY
    initJoyPad();
  #endif
  
}


void loop() {
  doM5Menu();
}

