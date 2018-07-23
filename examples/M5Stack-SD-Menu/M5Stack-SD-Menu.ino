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
 * As SD Card mounting can be a hassle, using the ESP32 Sketch data 
 * uploader is also possible. Any file sent using this method will
 * automatically be copied onto the SD Card on next restart.
 * This includes .bin, .json, .jpg and .mod files.
 * 
 * The menu will list all available apps on the sdcard and load them 
 * on demand. 
 * 
 * Once you're finished with the loaded app, push reset+BTN_A and it 
 * loads the menu again. Rinse and repeat.
 *  
 * Obviously none of those apps will embed this menu, instead they
 * must include and implement the M5Stack SD Loader Snippet, a lighter
 * version of the loader dedicated to loading the menu.
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
 * 
 */
 
#include "SPIFFS.h"
#include <M5Stack.h>             // https://github.com/m5stack/M5Stack/
#ifdef M5_LIB_VERSION
  #include "utility/qrcode.h" // if M5Stack version >= 0.1.8 : qrCode from M5Stack
#else 
  #include "qrcode.h" // if M5Stack version <= 0.1.6 : qrCode from https://github.com/ricmoo/qrcode
#endif 
#include "M5StackUpdater.h"      // https://github.com/tobozo/M5Stack-SD-Updater
#include <M5StackSAM.h>          // https://github.com/tomsuch/M5StackSAM
#include <ArduinoJson.h>         // https://github.com/bblanchon/ArduinoJson/
#include "i18n.h"                // language file
#include "assets.h" // some artwork for the UI
#include "controls.h"            // keypad / joypad / keyboard controls



#define MAX_FILES 255 // this affects memory



/* 
 * Files with those extensions will be transferred to the SD Card
 * if found on SPIFFS.
 * Directory is automatically created.
 * 
 */
const uint8_t extensionsCount = 4; // change this if you add / remove an extension
String allowedExtensions[extensionsCount] = {
    // do NOT remove jpg and json or the menu will crash !!!
    "jpg", "json", "mod", "mp3"
};
String appDataFolder = "/data"; // if an app needs spiffs data, it's stored here


/* Storing json meta file information r */
struct JSONMeta {
  int width; // app image width
  int height; // app image height
  String authorName = "";
  String projectURL = "";
  String credits = ""; // scroll this ?
  // TODO: add more interesting properties
};

/* filenames cache structure */
struct FileInfo {
  String fileName;  // the binary name
  String metaName;  // a json file with all meta info on the binary
  String iconName;  // a jpeg image representing the binary
  String faceName;  // a jpeg image representing the author
  uint32_t fileSize;
  bool hasIcon = false;
  bool hasMeta = false;
  bool hasFace = false; // github avatar
  bool hasData = false; // app requires a spiffs /data folder
  JSONMeta jsonMeta;
};

SDUpdater sdUpdater;
FileInfo fileInfo[MAX_FILES];
M5SAM M5Menu;

uint16_t appsCount = 0; // how many binary files
bool inInfoMenu = false; // menu state machine
unsigned long lastcheck = millis(); // timer check
unsigned long lastpush = millis(); // keypad/keyboard activity
uint16_t checkdelay = 300; // timer frequency
uint16_t MenuID; // pointer to the current menu item selected
int16_t scrollPointer = 0; // pointer to the scrollText position
unsigned long lastScrollRender = micros(); // timer for scrolling
String lastScrollMessage; // last scrolling string state
int16_t lastScrollOffset; // last scrolling string position

/* vMicro compliance, see https://github.com/tobozo/M5Stack-SD-Updater/issues/5#issuecomment-386749435 */
void getMeta(String metaFileName, JSONMeta &jsonMeta);
void renderIcon(FileInfo &fileInfo);
void renderMeta(JSONMeta &jsonMeta);



void getMeta(String metaFileName, JSONMeta &jsonMeta) {
  File file = SD.open(metaFileName);
#if ARDUINOJSON_VERSION_MAJOR==6
  StaticJsonDocument<512> jsonBuffer;
  DeserializationError error = deserializeJson(jsonBuffer, file);
  if (error) return;
  JsonObject root = jsonBuffer.as<JsonObject>();
  if (!root.isNull())
#else
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);
  if (root.success())
#endif
  {
    jsonMeta.width  = root["width"];
    jsonMeta.height = root["height"];
    jsonMeta.authorName = root["authorName"].as<String>();
    jsonMeta.projectURL = root["projectURL"].as<String>();
    jsonMeta.credits    = root["credits"].as<String>();
  }
}


void renderScroll(String scrollText, uint8_t x, uint8_t y, uint16_t width) {
  if(scrollText=="") return;
  M5.Lcd.setTextSize(2); // setup text size before it's measured  
  if(!scrollText.endsWith(" ")) {
    scrollText += " "; // append a space since scrolling text *will* repeat
  }
  while(M5.Lcd.textWidth(scrollText)<width) {
    scrollText += scrollText; // grow text to desired width
  }

  String  scrollMe = "";
  int16_t textWidth = M5.Lcd.textWidth(scrollText);
  int16_t vsize = 0,
          vpos = 0,
          voffset = 0,
          scrollOffset = 0;
  uint8_t csize = 0, 
          lastcsize = 0;
  
  scrollPointer-=1;
  if(scrollPointer<-textWidth) {
    scrollPointer = 0;
    vsize = scrollPointer;
  }
  
  while( M5.Lcd.textWidth(scrollMe) < width ) {
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
        voffset = scrollPointer%lastcsize;
        scrollOffset = x+voffset;
        if(M5.Lcd.textWidth(scrollMe) > width-voffset) {
          break; // break out of the loop and out of the while
        }
      }
    }
  }

  // display trim
  while( M5.Lcd.textWidth(scrollMe) > width-voffset ) {
    scrollMe.remove(scrollMe.length()-1);
  }

  // only draw if things changed
  if(scrollOffset!=lastScrollOffset || scrollMe!=lastScrollMessage) {
    M5.Lcd.setTextColor(WHITE,BLACK); // setting background color removes the flickering effect
    M5.Lcd.setCursor(scrollOffset, y);
    M5.Lcd.print(scrollMe);
    M5.Lcd.setTextColor(WHITE);
  }

  M5.Lcd.setTextSize(1);
  lastScrollMessage = scrollMe;
  lastScrollOffset  = scrollOffset;
  lastScrollRender  = micros();
  lastpush          = millis();
}


/* by file info */
void renderIcon(FileInfo &fileInfo) {
  if(!fileInfo.hasMeta || !fileInfo.hasIcon) {
    return;
  }
  JSONMeta jsonMeta = fileInfo.jsonMeta;
  M5.Lcd.drawJpgFile(SD, fileInfo.iconName.c_str(), M5.Lcd.width()-jsonMeta.width-10, (M5.Lcd.height()/2)-(jsonMeta.height/2)+10, jsonMeta.width, jsonMeta.height, 0, 0, JPEG_DIV_NONE);
}

/* by menu ID */
void renderIcon(uint16_t MenuID) {
  renderIcon(fileInfo[MenuID]);
}

void renderFace(String face) {
  M5.Lcd.drawJpgFile(SD, face.c_str(), 5, 85, 120, 120, 0, 0, JPEG_DIV_NONE);
}


void renderMeta(JSONMeta &jsonMeta) {
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 35);
  M5.Lcd.print(fileInfo[MenuID].fileName);
  M5.Lcd.setCursor(10, 70);
  M5.Lcd.print(String(fileInfo[MenuID].fileSize) + String(FILESIZE_UNITS));
  M5.Lcd.setCursor(10, 50);
  
  if(jsonMeta.authorName!="" && jsonMeta.projectURL!="" ) { // both values provided
    M5.Lcd.print(AUTHOR_PREFIX);
    M5.Lcd.print(jsonMeta.authorName);
    M5.Lcd.print(AUTHOR_SUFFIX);
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
  uint8_t version = 4;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, 0, text.c_str());

  uint8_t thickness = sizeinpixels / qrcode.size;
  uint16_t lineLength = qrcode.size * thickness;
  uint8_t xOffset = ((M5.Lcd.width() - (lineLength)) / 2) + 70;
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


void getFileInfo(File &file) {
  String fileName   = file.name();
  uint32_t fileSize = file.size();
  Serial.println(String(DEBUG_FILELABEL) + fileName );
  
  fileInfo[appsCount].fileName = fileName;
  fileInfo[appsCount].fileSize = fileSize;

  String currentIconFile = "/jpg" + fileName;
  currentIconFile.replace(".bin", ".jpg");
  if(SD.exists(currentIconFile.c_str())) {
    fileInfo[appsCount].hasIcon = true;
    fileInfo[appsCount].iconName = currentIconFile;
  }
  currentIconFile.replace(".jpg", "_gh.jpg");
  if(SD.exists(currentIconFile.c_str())) {
    fileInfo[appsCount].hasFace = true;
    fileInfo[appsCount].faceName = currentIconFile;
  }
  String currentDataFolder = appDataFolder + fileName;
  currentDataFolder.replace(".bin", "");
  if(SD.exists(currentDataFolder.c_str())) {
    fileInfo[appsCount].hasData = true;
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
}


void listDir(fs::FS &fs, const char * dirName, uint8_t levels){
  Serial.printf(String(DEBUG_DIRNAME).c_str(), dirName);

  File root = fs.open(dirName);
  if(!root){
    Serial.println(DEBUG_DIROPEN_FAILED);
    return;
  }
  if(!root.isDirectory()){
    Serial.println(DEBUG_NOTADIR);
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print(DEBUG_DIRLABEL);
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      if(String(file.name())!=MENU_BIN && String(file.name()).endsWith(".bin")) {
        getFileInfo( file );
        appsCount++;
      } else {
        // ignored files
        Serial.println(String(DEBUG_IGNORED) + file.name() );
      }
    }
    file = root.openNextFile();
  }
  file = fs.open(MENU_BIN);
  getFileInfo( file );
  appsCount++;
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

      if (name1 > name2 || name1==MENU_BIN) {
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
  M5Menu.setListCaption(MENU_SUBTITLE);
  for(uint16_t i=0;i<appsCount;i++) {
    String shortName = fileInfo[i].fileName.substring(1);
    shortName.replace(".bin", "");
    
    if(shortName=="menu") {
      shortName = ABOUT_THIS_MENU;
    }
    
    M5Menu.addList(shortName);
  }
}


void menuUp() {
  MenuID = M5Menu.getListID();
  if(MenuID>0) {
    MenuID--;
  } else {
    MenuID = appsCount-1;
  }
  M5Menu.setListID(MenuID);
  M5Menu.drawAppMenu(MENU_TITLE, MENU_BTN_INFO, MENU_BTN_LOAD, MENU_BTN_NEXT);
  M5Menu.showList();
  renderIcon(MenuID);
  inInfoMenu = false;
  lastpush = millis();
}


void menuDown() {
  M5Menu.drawAppMenu(MENU_TITLE, MENU_BTN_INFO, MENU_BTN_LOAD, MENU_BTN_NEXT);
  M5Menu.nextList();
  MenuID = M5Menu.getListID();
  renderIcon(MenuID);
  inInfoMenu = false;
  lastpush = millis();
}


void menuInfo() {
  inInfoMenu = true;
  M5Menu.windowClr();
  renderMeta(fileInfo[MenuID].jsonMeta);
  if(fileInfo[MenuID].hasFace) {
    renderFace(fileInfo[MenuID].faceName);
  }
  lastpush = millis();
}


void menuMeta() {
  inInfoMenu = false;
  M5Menu.drawAppMenu(MENU_TITLE, MENU_BTN_INFO, MENU_BTN_LOAD, MENU_BTN_NEXT);
  M5Menu.showList();
  MenuID = M5Menu.getListID();
  renderIcon(MenuID);
  lastpush = millis();
}


/* 
 *  Scan SPIFFS for binaries and move them onto the SD Card
 *  TODO: create an app manager for the SD Card
 */
void scanDataFolder() {
  Serial.println(DEBUG_SPIFFS_SCAN);
  /* check if mandatory folders exists and create if necessary */

  // data folder
  if(!SD.exists(appDataFolder)) {
    SD.mkdir(appDataFolder);
  }


  for(uint8_t i=0;i<extensionsCount;i++) {
    String dir = "/" + allowedExtensions[i];
    if(!SD.exists(dir)) {
      SD.mkdir(dir);
    }
  }
  
  if(!SPIFFS.begin()){
    Serial.println(DEBUG_SPIFFS_MOUNTFAILED);
  } else {
    File root = SPIFFS.open("/");
    if(!root){
      Serial.println(DEBUG_DIROPEN_FAILED);
    } else {
      if(!root.isDirectory()){
        Serial.println(DEBUG_NOTADIR);
      } else {
        File file = root.openNextFile();
        Serial.println(file.name());
        String fileName = file.name();
        String destName = "";
        if(fileName.endsWith(".bin")) {
          destName = fileName;
        }
        // move allowed file types to their own folders
        for(uint8_t i=0;i<extensionsCount;i++) {
          String ext = "." + allowedExtensions[i];
          if(fileName.endsWith(ext)) {  
            destName = "/" + allowedExtensions[i] + fileName;
          }
        }

        if(destName!="") {
          sdUpdater.displayUpdateUI(String(MOVINGFILE_MESSAGE) + fileName);
          size_t fileSize = file.size();
          File destFile = SD.open(destName, FILE_WRITE);
          
          if(!destFile){
            Serial.println(DEBUG_SPIFFS_WRITEFAILED);
          } else {
            static uint8_t buf[512];
            size_t packets = 0;
            Serial.println(String(DEBUG_FILECOPY) + fileName);
            
            while(file.read(buf, 512)) {
              destFile.write(buf, 512);
              packets++;
              sdUpdater.M5SDMenuProgress((packets*512)-511, fileSize);
            }
            destFile.close();
            Serial.println();
            Serial.println(DEBUG_FILECOPY_DONE);
            SPIFFS.remove( fileName );
            Serial.println(DEBUG_WILL_RESTART);
            delay(500);
            ESP.restart();
          }
        } else {
          Serial.println(DEBUG_NOTHING_TODO);
        }
      }
    }
  }
}



void setup() {
  Serial.begin(115200);
  Serial.println(WELCOME_MESSAGE);
  Serial.print(INIT_MESSAGE);
  M5.begin();
  Wire.begin();
  // Thanks to Macbug for the hint, my old ears couldn't hear the buzzing :-) 
  // See Macbug's excellent article on this tool:
  // https://macsbug.wordpress.com/2018/03/12/m5stack-sd-updater/
  dacWrite(25, 0); // turn speaker signal off
  // Also thanks to @Kongduino for a complementary way to turn off the speaker:
  // https://twitter.com/Kongduino/status/980466157701423104
  ledcDetachPin(25); // detach DAC
  
  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println(GOTOSLEEP_MESSAGE);
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
    M5.Lcd.print(INSERTSD_MESSAGE);
    if(toggle) {
      M5.Lcd.drawJpg(disk01_jpg, 1775, 160, 100);
      delay(300);
    } else {
      M5.Lcd.drawJpg(disk00_jpg, 1775, 160, 100);
      delay(500);
    }
    // go to sleep after a minute, no need to hammer the SD Card reader
    if(lastcheck+60000<now) {
      Serial.println(GOTOSLEEP_MESSAGE);
      M5.setWakeupButton(BUTTON_B_PIN);
      M5.powerOFF();
    }
  }

  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);

  // scan for SPIFFS files waiting to be moved onto the SD Card
  scanDataFolder();
  
  listDir(SD, "/", 0);
  aSortFiles();
  buildM5Menu();

  #ifdef USE_PSP_JOY
    initJoyPad();
  #endif
  #ifdef USE_FACES_GAMEBOY
    initKeypad();
  #endif

  // TODO: animate loading screen
  /* fake loading progress, looks kool ;-) */
  for(uint8_t i=1;i<100;i++) {
    sdUpdater.M5SDMenuProgress(i, 100);
  }

  M5Menu.drawAppMenu(MENU_TITLE, MENU_BTN_INFO, MENU_BTN_LOAD, MENU_BTN_NEXT);
  M5Menu.showList();
  renderIcon(0);
  inInfoMenu = false;
  lastcheck = millis();
  lastpush = millis();
  checkdelay = 300;

}


void loop() {

  HIDSignal hidState = getControls();

  switch(hidState) {
    case UI_DOWN:
      menuDown();
    break;
    case UI_UP:
      menuUp();
    break;
    case UI_INFO:
      if(!inInfoMenu) {
        menuInfo();
      } else {
        menuMeta();
      }
    break;
    case UI_LOAD:
      sdUpdater.updateFromFS(SD, fileInfo[ M5Menu.getListID() ].fileName);
      ESP.restart();
    break;
    default:
    case UI_INERT:
      if(inInfoMenu) {
        // !! scrolling text also prevents sleep mode !!
        renderScroll(fileInfo[MenuID].jsonMeta.credits, 0, 5, 320);
      }
    break;
  }

  M5.update();
  
  // go to sleep after 10 minutes if nothing happens
  if(lastpush+600000<millis()) {
    Serial.println(GOTOSLEEP_MESSAGE);
    M5.setWakeupButton(BUTTON_B_PIN);
    M5.powerOFF();
  }

}

