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

#include "SPIFFS.h"
#include <M5Stack.h>         // https://github.com/m5stack/M5Stack/
#ifdef M5_LIB_VERSION
  #include "utility/qrcode.h" // if M5Stack version >= 0.1.8 : qrCode from M5Stack
#else 
  #include "qrcode.h" // if M5Stack version <= 0.1.6 : qrCode from https://github.com/ricmoo/qrcode
#endif
#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater
#define tft M5.Lcd // syntax sugar, forward compat with other displays (i.e GO.Lcd)
#define M5_FS SD
//#define M5_FS SD_MMC
#define M5SAM_LIST_MAX_COUNT 255
// if "M5SAM_LIST_MAX_COUNT" gives a warning at compilation, apply this PR https://github.com/tomsuch/M5StackSAM/pull/4
// or modify M5StackSAM.h manually
#include <M5StackSAM.h>      // https://github.com/tobozo/M5StackSAM/tree/patch-1 (forked from https://github.com/tomsuch/M5StackSAM)
#include <ArduinoJson.h>     // https://github.com/bblanchon/ArduinoJson/
#include "i18n.h"            // language file
#include "assets.h"          // some artwork for the UI
#include "controls.h"        // keypad / joypad / keyboard controls
#include "downloader.h"      // binaries downloader module A.K.A YOLO Downloader
// #undef DOWNLOADER_BIN // uncomment this to get rid of the downloader (also delete the Downloader.bin dummy file)

/* 
 *  
 * /!\ When set to true, files with those extensions 
 * will be transferred to the SD Card if found on SPIFFS.
 * Directory is automatically created.
 * 
 */
bool migrateSPIFFS = false;
const uint8_t extensionsCount = 4; // change this if you add / remove an extension
String allowedExtensions[extensionsCount] = {
    // do NOT remove jpg and json or the menu will crash !!!
    "jpg", "json", "mod", "mp3"
};
String appDataFolder = "/data"; // if an app needs spiffs data, it's stored here
String launcherSignature = "Launcher.bin"; // app with name ending like this can overwrite menu.bin

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


FileInfo fileInfo[M5SAM_LIST_MAX_COUNT];
SDUpdater sdUpdater;
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
void getMeta( fs::FS &fs, String metaFileName, JSONMeta &jsonMeta );
void renderIcon( FileInfo &fileInfo );
void renderMeta( JSONMeta &jsonMeta );



void getMeta( fs::FS &fs, String metaFileName, JSONMeta &jsonMeta ) {
  File file = fs.open( metaFileName );
#if ARDUINOJSON_VERSION_MAJOR==6
  StaticJsonDocument<512> jsonBuffer;
  DeserializationError error = deserializeJson( jsonBuffer, file );
  if (error) return;
  JsonObject root = jsonBuffer.as<JsonObject>();
  if ( !root.isNull() )
#else
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);
  if ( root.success() )
#endif
  {
    jsonMeta.width  = root["width"];
    jsonMeta.height = root["height"];
    jsonMeta.authorName = root["authorName"].as<String>();
    jsonMeta.projectURL = root["projectURL"].as<String>();
    jsonMeta.credits    = root["credits"].as<String>();
  }
}


void renderScroll( String scrollText, uint8_t x, uint8_t y, uint16_t width ) {
  if( scrollText=="" ) return;
  tft.setTextSize( 2 ); // setup text size before it's measured  
  if( !scrollText.endsWith( " " )) {
    scrollText += " "; // append a space since scrolling text *will* repeat
  }
  while( tft.textWidth( scrollText ) < width ) {
    scrollText += scrollText; // grow text to desired width
  }

  String  scrollMe = "";
  int16_t textWidth = tft.textWidth( scrollText );
  int16_t vsize = 0,
          vpos = 0,
          voffset = 0,
          scrollOffset = 0;
  uint8_t csize = 0, 
          lastcsize = 0;
  
  scrollPointer-=1;
  if( scrollPointer<-textWidth ) {
    scrollPointer = 0;
    vsize = scrollPointer;
  }
  
  while( tft.textWidth(scrollMe) < width ) {
    for( uint8_t i=0; i<scrollText.length(); i++ ) {
      char thisChar[2];
      thisChar[0] = scrollText[i];
      thisChar[1] = '\0';
      csize = tft.textWidth( thisChar );
      vsize+=csize;
      vpos = vsize+scrollPointer;
      if( vpos>x && vpos<=x+width ) {
        scrollMe += scrollText[i];
        lastcsize = csize;
        voffset = scrollPointer%lastcsize;
        scrollOffset = x+voffset;
        if( tft.textWidth(scrollMe) > width-voffset ) {
          break; // break out of the loop and out of the while
        }
      }
    }
  }

  // display trim
  while( tft.textWidth( scrollMe ) > width-voffset ) {
    scrollMe.remove( scrollMe.length()-1 );
  }

  // only draw if things changed
  if( scrollOffset!=lastScrollOffset || scrollMe!=lastScrollMessage ) {
    tft.setTextColor( WHITE, BLACK ); // setting background color removes the flickering effect
    tft.setCursor( scrollOffset, y );
    tft.print( scrollMe );
    tft.setTextColor( WHITE );
  }

  tft.setTextSize( 1 );
  lastScrollMessage = scrollMe;
  lastScrollOffset  = scrollOffset;
  lastScrollRender  = micros();
  lastpush          = millis();
}


/* by file info */
void renderIcon( FileInfo &fileInfo ) {
  if( !fileInfo.hasMeta || !fileInfo.hasIcon ) {
    return;
  }
  JSONMeta jsonMeta = fileInfo.jsonMeta;
  tft.drawJpgFile( M5_FS, fileInfo.iconName.c_str(), tft.width()-jsonMeta.width-10, (tft.height()/2)-(jsonMeta.height/2)+10, jsonMeta.width, jsonMeta.height, 0, 0, JPEG_DIV_NONE );
}

/* by menu ID */
void renderIcon( uint16_t MenuID ) {
  renderIcon( fileInfo[MenuID] );
}

void renderFace( String face ) {
  tft.drawJpgFile( M5_FS, face.c_str(), 5, 85, 120, 120, 0, 0, JPEG_DIV_NONE );
}


void renderMeta( JSONMeta &jsonMeta ) {
  tft.setTextSize( 1 );
  tft.setTextColor( WHITE );
  tft.setCursor( 10, 35 );
  tft.print( fileInfo[MenuID].fileName );
  tft.setCursor( 10, 70 );
  tft.print( String( fileInfo[MenuID].fileSize ) + String( FILESIZE_UNITS ) );
  tft.setCursor( 10, 50 );
  
  if( jsonMeta.authorName!="" && jsonMeta.projectURL!="" ) { // both values provided
    tft.print( AUTHOR_PREFIX );
    tft.print( jsonMeta.authorName );
    tft.print( AUTHOR_SUFFIX );
    qrRender( jsonMeta.projectURL, 160 );
  } else if( jsonMeta.projectURL!="" ) { // only projectURL
    tft.print( jsonMeta.projectURL );
    qrRender( jsonMeta.projectURL, 160 );
  } else { // only authorName
    tft.drawCentreString( jsonMeta.authorName,tft.width()/2,(tft.height()/2)-25,2 );
  }
}



/* give up on redundancy and ECC to produce less and bigger squares */
uint8_t getLowestQRVersionFromString( String text, uint8_t ecc ) {
  if(ecc>3) return 4; // fail fast
  uint16_t len = text.length();
  uint8_t QRMaxLenByECCLevel[4][3] = {
    // https://www.qrcode.com/en/about/version.html  
    { 41, 77, 127 }, // L
    { 34, 63, 101 }, // M
    { 27, 48, 77 },  // Q
    { 17, 34, 58 }   // H
  };
  for( uint8_t i=0; i<3; i++ ) {
    if( len <= QRMaxLenByECCLevel[ecc][i] ) {
      return i+1;
    }
  }
  // there's no point in doing higher with M5Stack's display
  return 4;
}


void qrRender( String text, float sizeinpixels ) {
  // see https://github.com/Kongduino/M5_QR_Code/blob/master/M5_QRCode_Test.ino
  // Create the QR code
  QRCode qrcode;

  uint8_t ecc = 0; // QR on TFT can do without ECC
  uint8_t version = getLowestQRVersionFromString( text, ecc );
  uint8_t qrcodeData[qrcode_getBufferSize( version )];
  qrcode_initText( &qrcode, qrcodeData, version, ecc, text.c_str() );

  uint8_t thickness = sizeinpixels / qrcode.size;
  uint16_t lineLength = qrcode.size * thickness;
  uint8_t xOffset = ( ( tft.width() - ( lineLength ) ) / 2 ) + 70;
  uint8_t yOffset =  ( tft.height() - ( lineLength ) ) / 2;

  tft.fillRect( xOffset-5, yOffset-5, lineLength+10, lineLength+10, WHITE );
  
  for ( uint8_t y = 0; y < qrcode.size; y++ ) {
    // Each horizontal module
    for ( uint8_t x = 0; x < qrcode.size; x++ ) {
      bool q = qrcode_getModule( &qrcode, x, y );
      if (q) {
        tft.fillRect( x * thickness + xOffset, y * thickness + yOffset, thickness, thickness, TFT_BLACK );
      }
    }
  }
}


void getFileInfo( fs::FS &fs, File &file ) {
  String fileName   = file.name();
  uint32_t fileSize = file.size();
  time_t t= file.getLastWrite();
  struct tm * tmstruct = localtime(&t);
  char fileDate[64] = "1980-01-01 00:07:20";
  sprintf(fileDate, "%04d-%02d-%02d %02d:%02d:%02d",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
  if( (tmstruct->tm_year)+1900 < 2000 ) {
    // time is not set
  }
  Serial.println( "[" + String(fileDate) + "]" + String( DEBUG_FILELABEL ) + fileName );
  
  fileInfo[appsCount].fileName = fileName;
  fileInfo[appsCount].fileSize = fileSize;

  String currentIconFile = "/jpg" + fileName;
  currentIconFile.replace( ".bin", ".jpg" );
  if( fs.exists( currentIconFile.c_str() ) ) {
    fileInfo[appsCount].hasIcon = true;
    fileInfo[appsCount].iconName = currentIconFile;
  }
  currentIconFile.replace( ".jpg", "_gh.jpg" );
  if( fs.exists( currentIconFile.c_str() ) ) {
    fileInfo[appsCount].hasFace = true;
    fileInfo[appsCount].faceName = currentIconFile;
  }
  String currentDataFolder = appDataFolder + fileName;
  currentDataFolder.replace( ".bin", "" );
  if( fs.exists( currentDataFolder.c_str() ) ) {
    fileInfo[appsCount].hasData = true; // TODO: actually use this feature
  }
  
  String currentMetaFile = "/json" + fileName;
  currentMetaFile.replace( ".bin", ".json" );
  if( fs.exists(currentMetaFile.c_str() ) ) {
    fileInfo[appsCount].hasMeta = true;
    fileInfo[appsCount].metaName = currentMetaFile;
  }
  
  if( fileInfo[appsCount].hasMeta == true ) {
    getMeta( fs, fileInfo[appsCount].metaName, fileInfo[appsCount].jsonMeta );
  }
  
}


void listDir( fs::FS &fs, const char * dirName, uint8_t levels ){
  Serial.printf( String( DEBUG_DIRNAME ).c_str(), dirName );

  File root = fs.open( dirName );
  if( !root ){
    log_e( DEBUG_DIROPEN_FAILED );
    return;
  }
  if( !root.isDirectory() ){
    log_e( DEBUG_NOTADIR );
    return;
  }

  File file = root.openNextFile();
  while( file ){
    if( file.isDirectory() ){
      log_d( DEBUG_DIRLABEL );
      log_d( file.name() );
      if( levels ){
        listDir( fs, file.name(), levels -1 );
      }
    } else {
      if(   String( file.name() )!=MENU_BIN // ignore menu
         && String( file.name() ).endsWith( ".bin" ) // ignore files not ending in ".bin"
         && !String( file.name() ).startsWith( "/." ) ) { // ignore dotfiles (thanks to https://twitter.com/micutil)
        getFileInfo( fs, file );
        appsCount++;
        if( appsCount >= M5SAM_LIST_MAX_COUNT-1 ) {
          //Serial.println( String( DEBUG_IGNORED ) + file.name() );
          log_w( DEBUG_ABORTLISTING );
          break; // don't make M5Stack list explode
        }
      } else {
        // ignored files
        log_d( String( DEBUG_IGNORED ) + file.name() );
      }
    }
    file = root.openNextFile();
  }
  if( fs.exists( MENU_BIN ) ) {
    file = fs.open( MENU_BIN );
    getFileInfo( fs, file );
    appsCount++;
  } else {
    log_w( "[WARNING] No %s file found\n", MENU_BIN );
  }
}


/* bubble sort filenames */
void aSortFiles() {
  bool swapped;
  FileInfo temp;
  String name1, name2;
  do {
    swapped = false;
    for( uint16_t i=0; i<appsCount-1; i++ ) {
      name1 = fileInfo[i].fileName[0];
      name2 = fileInfo[i+1].fileName[0];
      if( name1==name2 ) {
        name1 = fileInfo[i].fileName[1];
        name2 = fileInfo[i+1].fileName[1];
        if( name1==name2 ) {
          name1 = fileInfo[i].fileName[2];
          name2 = fileInfo[i+1].fileName[2];        
        } else {
          // give it up :-)
        }
      }

      if ( name1 > name2 || name1==MENU_BIN ) {
        temp = fileInfo[i];
        fileInfo[i] = fileInfo[i + 1];
        fileInfo[i + 1] = temp;
        swapped = true;
      }
    }
  } while ( swapped );
}


void buildM5Menu() {
  M5Menu.clearList();
  M5Menu.setListCaption( MENU_SUBTITLE );
  for( uint16_t i=0; i < appsCount; i++ ) {
    String shortName = fileInfo[i].fileName.substring(1);
    shortName.replace( ".bin", "" );
    
    if( shortName=="menu" ) {
      shortName = ABOUT_THIS_MENU;
    }
    
    M5Menu.addList( shortName );
  }
}



bool modalConfirm( String question, String title, String body ) {
  bool response = false;
  M5Menu.windowClr();
  M5Menu.drawAppMenu( question , "YES", "NO", "CANCEL");
  tft.setTextSize( 1 );
  tft.setTextColor( WHITE );
  tft.drawCentreString( title, 160, 50, 1 );
  tft.setCursor( 0, 72 );
  tft.print( body );
  
  tft.drawJpg(caution_jpg, caution_jpg_len, 224, 136, 64, 46 );
  HIDSignal hidState = UI_INERT;

  while( hidState==UI_INERT ) {
    delay( 100 );
    M5.update();
    hidState = getControls();
  }

  switch( hidState ) {
    //case UI_UP
    case UI_INFO:
      response = true;
    break;
    case UI_LOAD:
    case UI_DOWN:
    default:
      // already false
      M5Menu.drawAppMenu( MENU_TITLE, MENU_BTN_INFO, MENU_BTN_LOAD, MENU_BTN_NEXT );
      M5Menu.showList();
      renderIcon( MenuID );
    break;

  }
  M5.update();
  return response;
}




void menuUp() {
  MenuID = M5Menu.getListID();
  if( MenuID > 0 ) {
    MenuID--;
  } else {
    MenuID = appsCount-1;
  }
  MenuID = M5Menu.getListID();
  M5Menu.setListID( MenuID );

  if( fileInfo[ MenuID ].fileName.endsWith( launcherSignature ) ) {
    M5Menu.drawAppMenu( MENU_TITLE, MENU_BTN_INFO, MENU_BTN_SET, MENU_BTN_NEXT );
  } else {
    M5Menu.drawAppMenu( MENU_TITLE, MENU_BTN_INFO, MENU_BTN_LOAD, MENU_BTN_NEXT );
  }

  M5Menu.showList();
  renderIcon( MenuID );
  inInfoMenu = false;
  lastpush = millis();
}


void menuDown() {

  if(MenuID<appsCount-1){
    MenuID++;
  } else {
    MenuID = 0;
  }

  M5Menu.nextList( false );
  MenuID = M5Menu.getListID();

  if( fileInfo[ MenuID ].fileName.endsWith( launcherSignature ) ) {
    M5Menu.drawAppMenu( MENU_TITLE, MENU_BTN_INFO, MENU_BTN_SET, MENU_BTN_NEXT );
  } else {
    M5Menu.drawAppMenu( MENU_TITLE, MENU_BTN_INFO, MENU_BTN_LOAD, MENU_BTN_NEXT );
  }

  M5Menu.showList();

  renderIcon( MenuID );
  inInfoMenu = false;
  lastpush = millis();
}


void menuInfo() {
  inInfoMenu = true;
  M5Menu.windowClr();
  renderMeta( fileInfo[MenuID].jsonMeta );
  if( fileInfo[MenuID].hasFace ) {
    renderFace( fileInfo[MenuID].faceName );
  }
  lastpush = millis();
}


void menuMeta() {
  inInfoMenu = false;
  M5Menu.drawAppMenu( MENU_TITLE, MENU_BTN_INFO, MENU_BTN_LOAD, MENU_BTN_NEXT );
  M5Menu.showList();
  MenuID = M5Menu.getListID();
  renderIcon( MenuID );
  lastpush = millis();
}


/* 
 *  Scan SPIFFS for binaries and move them onto the SD Card
 *  TODO: create an app manager for the SD Card
 */
void scanDataFolder() {
  /* check if mandatory folders exists and create if necessary */

  // data folder
  if( !M5_FS.exists( appDataFolder ) ) {
    M5_FS.mkdir( appDataFolder );
  }


  for( uint8_t i=0; i<extensionsCount; i++ ) {
    String dir = "/" + allowedExtensions[i];
    if( !M5_FS.exists( dir ) ) {
      M5_FS.mkdir( dir );
    }
  }

  if( !migrateSPIFFS ) {
    return;
  }
  
  Serial.println( DEBUG_SPIFFS_SCAN );

  if( !SPIFFS.begin() ){
    Serial.println( DEBUG_SPIFFS_MOUNTFAILED );
  } else {
    File root = SPIFFS.open( "/" );
    if( !root ){
      Serial.println( DEBUG_DIROPEN_FAILED );
    } else {
      if( !root.isDirectory() ){
        Serial.println( DEBUG_NOTADIR );
      } else {
        File file = root.openNextFile();
        Serial.println( file.name() );
        String fileName = file.name();
        String destName = "";
        if( fileName.endsWith( ".bin" ) ) {
          destName = fileName;
        }
        // move allowed file types to their own folders
        for( uint8_t i=0; i<extensionsCount; i++)  {
          String ext = "." + allowedExtensions[i];
          if( fileName.endsWith( ext ) ) {  
            destName = "/" + allowedExtensions[i] + fileName;
          }
        }

        if( destName!="" ) {
          sdUpdater.displayUpdateUI( String( MOVINGFILE_MESSAGE ) + fileName );
          size_t fileSize = file.size();
          File destFile = M5_FS.open( destName, FILE_WRITE );
          
          if( !destFile ){
            Serial.println( DEBUG_SPIFFS_WRITEFAILED) ;
          } else {
            static uint8_t buf[512];
            size_t packets = 0;
            Serial.println( String( DEBUG_FILECOPY ) + fileName );
            
            while( file.read( buf, 512) ) {
              destFile.write( buf, 512 );
              packets++;
              sdUpdater.SDMenuProgress( (packets*512)-511, fileSize );
            }

            destFile.close();
            Serial.println();
            Serial.println( DEBUG_FILECOPY_DONE );
            SPIFFS.remove( fileName );
            Serial.println( DEBUG_WILL_RESTART );
            delay( 500 );
            ESP.restart();
          }
        } else {
          Serial.println( DEBUG_NOTHING_TODO );
        }
      }
    }
  }
}


#define SPI_FLASH_SEC_STEP8 SPI_FLASH_SEC_SIZE / 4

static esp_image_metadata_t getSketchMeta( const esp_partition_t* running ) {
  esp_image_metadata_t data;
  if ( !running ) return data;
  const esp_partition_pos_t running_pos  = {
    .offset = running->address,
    .size = running->size,
  };
  data.start_addr = running_pos.offset;
  esp_image_verify( ESP_IMAGE_VERIFY, &running_pos, &data );
  return data;
}


bool replaceMenu( fs::FS &fs, FileInfo &info) {
  if(!replaceItem( fs, info.fileName, String(MENU_BIN) ) ) {
    return false;
  }
  if( info.hasIcon ) {
    replaceItem( fs, info.iconName, "/jpg/menu.jpg" );
  }
  if( info.hasFace ) {
    replaceItem( fs, info.faceName, "/jpg/menu_gh.jpg" );
  }
  if( info.hasMeta ) {
    replaceItem( fs, info.metaName, "/json/menu.json" );
  }
  return true;
}


bool replaceItem( fs::FS &fs, String SourceName, String  DestName) {
  if( !fs.exists( SourceName ) ) {
    Serial.printf("Source file %s does not exists !\n", SourceName.c_str() );
    return false;
  }
  fs.remove( DestName );
  fs::File source = fs.open( SourceName );
  if( !source ) {
    Serial.printf("Failed to open source file %s\n", SourceName.c_str() );
    return false;
  }
  fs::File dest = fs.open( DestName, FILE_WRITE );
  if( !dest ) {
    Serial.printf("Failed to open dest file %s\n", DestName );
    return false;
  }
  uint8_t buf[4096]; // 4K buffer should be enough to fast-copy the file
  uint8_t dot = 0;
  size_t fileSize = source.size();
  size_t n;
  while ((n = source.read(buf, sizeof(buf))) > 0) {
    Serial.print(".");
    if(dot++%64==0) {
      Serial.println();
      sdUpdater.SDMenuProgress( (dot*4096)-4095, fileSize );
    }
    dest.write(buf, n);
  }
  dest.close();
  source.close();
  return true;
}



void dumpSketchToFS( fs::FS &fs, const char* fileName ) {
  const esp_partition_t* source_partition = esp_ota_get_running_partition();
  const char* label = "Current running partition";
  size_t fileSize;

  {
    File destFile = fs.open( fileName, FILE_WRITE );
    if( !destFile ) {
      Serial.printf( "Can't open %s\n", fileName );
      return;
    }
    fileSize = destFile.size();
    destFile.close();
  }

  esp_image_metadata_t sketchMeta = getSketchMeta( source_partition );
  uint32_t sketchSize = sketchMeta.image_len;

  Preferences preferences;
  preferences.begin( "sd-menu" );
  uint32_t menuSize = preferences.getInt( "menusize", 0 );
  uint8_t image_digest[32];
  preferences.getBytes( "digest", image_digest, 32 );
  preferences.end();

  if( menuSize==sketchSize ) {
    bool match = true;
    for( uint8_t i=0; i<32; i++ ) {
      if( image_digest[i]!=sketchMeta.image_digest[i] ) {
        Serial.println( "NONVSMATCH" );
        match = false;
        break;
      }
    }
    if( match ) {
      Serial.printf( "%s size (%d bytes) and hashes match %s's expected data from NVS: %d, no replication necessary\n", label, sketchSize, fileName, menuSize );
      return;
    }
  }
 
  Serial.printf( "%s (%d bytes) differs from %s's expected NVS size: %d, overwriting\n", label, sketchSize, fileName, fileSize );
  static uint8_t spi_rbuf[SPI_FLASH_SEC_STEP8];

  Serial.printf( " [INFO] Writing %s ...\n", fileName );
  File destFile = fs.open( fileName, FILE_WRITE );
  uint32_t bytescounter = 0;
  for ( uint32_t base_addr = source_partition->address; base_addr < source_partition->address + sketchSize; base_addr += SPI_FLASH_SEC_STEP8 ) {
    memset( spi_rbuf, 0, SPI_FLASH_SEC_STEP8 );
    spi_flash_read( base_addr, spi_rbuf, SPI_FLASH_SEC_STEP8 );
    destFile.write( spi_rbuf, SPI_FLASH_SEC_STEP8 );
    bytescounter++;
    if( bytescounter%128==0 ) {
      Serial.println( "." );
    } else {
      Serial.print( "." );
    }
  }
  Serial.println();
  destFile.close();

  preferences.begin( "sd-menu", false );
  preferences.putInt( "menusize", sketchSize );
  preferences.putBytes( "digest", sketchMeta.image_digest, 32 );
  preferences.end();

}



void setup() {
  Serial.begin( 115200 );
  Serial.println( WELCOME_MESSAGE );
  Serial.print( INIT_MESSAGE );
  M5.begin();
  //tft.begin();
  
  if( digitalRead( BUTTON_A_PIN ) == 0 ) {
    Serial.println( GOTOSLEEP_MESSAGE );
    M5.setWakeupButton( BUTTON_B_PIN );
    M5.powerOFF();
  }
  
  tft.setBrightness(100);

  lastcheck = millis();
  bool toggle = true;
  tft.drawJpg(disk01_jpg, 1775, (tft.width()-30)/2, 100);
  tft.setTextSize(1);
  int16_t posx = ( tft.width() / 2 ) - ( tft.textWidth( SD_LOADING_MESSAGE ) / 2 );
  if( posx <0 ) posx = 0;
  tft.setCursor( posx, 136 );
  tft.print( SD_LOADING_MESSAGE );

  tft.setTextSize( 2 );
  while( !M5_FS.begin( TFCARD_CS_PIN ) ) {
    // TODO: make a more fancy animation
    unsigned long now = millis();
    toggle = !toggle;
    uint16_t color = toggle ? BLACK : WHITE;
    tft.setCursor( 10,100 );
    tft.setTextColor( color );
    tft.print( INSERTSD_MESSAGE );
    if( toggle ) {
      tft.drawJpg( disk01_jpg, 1775, (tft.width()-30)/2, 100 );
      delay( 300 );
    } else {
      tft.drawJpg( disk00_jpg, 1775, (tft.width()-30)/2, 100 );
      delay( 500 );
    }
    // go to sleep after a minute, no need to hammer the SD Card reader
    if( lastcheck + 60000 < now ) {
      Serial.println( GOTOSLEEP_MESSAGE );
      M5.setWakeupButton( BUTTON_B_PIN );
      M5.powerOFF();
    }
  }

  tft.setTextColor( WHITE );
  tft.setTextSize( 1 );
  tft.clear();

  sdUpdater.SDMenuProgress( 10, 100 );

  // do SD / SPIFFS health checks
  scanDataFolder();

  if( !M5_FS.exists( MENU_BIN ) ) {
    Serial.printf("File %s does not exist, copying current sketch\n", MENU_BIN);
    dumpSketchToFS( M5_FS, MENU_BIN );
  }

  #ifdef DOWNLOADER_BIN
  if( !M5_FS.exists( DOWNLOADER_BIN ) ) {
    // create a dummy file just to enable the feature
    fs::File dummyDownloader = M5_FS.open( DOWNLOADER_BIN, FILE_WRITE);
    dummyDownloader.print("blah");
    dummyDownloader.close();
  }
  #endif

  sdUpdater.SDMenuProgress( 20, 100 );
  listDir(M5_FS, "/", 0);
  sdUpdater.SDMenuProgress( 30, 100 );
  aSortFiles();
  sdUpdater.SDMenuProgress( 40, 100 );
  buildM5Menu();

  #ifdef USE_PSP_JOY
    initJoyPad();
  #endif
  #ifdef USE_FACES_GAMEBOY
    initKeypad();
  #endif

  // TODO: animate loading screen
  tft.clear();

  for( uint8_t i=50; i<=80; i++ ) {
    sdUpdater.SDMenuProgress( i, 100 );
  }

  sdUpdater.SDMenuProgress( 100, 100 );
  
  M5Menu.drawAppMenu( MENU_TITLE, MENU_BTN_INFO, MENU_BTN_LOAD, MENU_BTN_NEXT );
  M5Menu.showList();
  renderIcon(0);
  inInfoMenu = false;
  lastcheck = millis();
  lastpush = millis();
  checkdelay = 300;

}


#define MAX_BRIGHTNESS 100
const unsigned long MS_BEFORE_SLEEP = 600000; // 600000 = 10mn 
uint8_t brightness = MAX_BRIGHTNESS;


void loop() {

  HIDSignal hidState = getControls();

  if( hidState!=UI_INERT && brightness != MAX_BRIGHTNESS ) {
    // some activity occured, restore brightness
    Serial.println(".. !!! Waking up !!"); 
    brightness = MAX_BRIGHTNESS;
    tft.setBrightness( brightness );
  }

  switch( hidState ) {
    case UI_DOWN:
      menuDown();
    break;
    case UI_UP:
      menuUp();
    break;
    case UI_INFO:
      if( !inInfoMenu ) {
        menuInfo();
      } else {
        menuMeta();
      }
    break;
    case UI_LOAD:
      #ifdef DOWNLOADER_BIN
      if( fileInfo[MenuID].fileName == String( DOWNLOADER_BIN ) ) {
        if( modalConfirm( DOWNLOADER_MODAL_NAME, DOWNLOADER_MODAL_TITLE, DOWNLOADER_MODAL_BODY ) ) {
          updateAll();
          ESP.restart();
        } else {
          // action cancelled or refused by user
          return;
        }
      }
      #endif
    
      if( fileInfo[MenuID].fileName.endsWith( launcherSignature ) ) {
        Serial.printf("Will overwrite current %s with a copy of %s\n", MENU_BIN, fileInfo[MenuID].fileName.c_str() );
        if( replaceMenu( M5_FS, fileInfo[MenuID] ) ) {
        } else {
          Serial.println("Failed to overwrite ?????");
          return;
        }
      }
      sdUpdater.updateFromFS( M5_FS, fileInfo[ M5Menu.getListID() ].fileName );
      ESP.restart();
    break;
    default:
    case UI_INERT:
      if( inInfoMenu ) {
        // !! scrolling text also prevents sleep mode !!
        renderScroll( fileInfo[MenuID].jsonMeta.credits, 0, 5, 320 );
      }
    break;
  }

  M5.update();
  
  // go to sleep if nothing happens for a while
  if( lastpush + MS_BEFORE_SLEEP < millis() ) {
    // slowly dim the screen first
    if( brightness > 1 ) {
      brightness--;
      if( brightness %10 == 0 ) {
        Serial.print("(\".¬.\") ");
      }
      if( brightness %30 == 0 ) {
        Serial.print(" Yawn... ");
      }
      if( brightness %7 == 0 ) {
        Serial.print(" .zzZzzz. ");
      }
      tft.setBrightness( brightness );
      lastpush = millis() - (MS_BEFORE_SLEEP - brightness*10); // exponential dimming effect
      return;
    }
    Serial.println( GOTOSLEEP_MESSAGE );
    M5.setWakeupButton( BUTTON_B_PIN );
    M5.powerOFF();
  }
}
