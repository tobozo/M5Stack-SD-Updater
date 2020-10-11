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
 */

// auto-select board
#if defined( ARDUINO_M5STACK_Core2 )
  #warning M5STACK Core2 DETECTED !!
  #define PLATFORM_NAME "M5Core2"
  #define DEFAULT_REGISTRY_BOARD "m5stack"
#elif defined( ARDUINO_M5Stack_Core_ESP32 )
  #warning M5STACK CLASSIC DETECTED !!
  #define PLATFORM_NAME "M5Stack"
  #define DEFAULT_REGISTRY_BOARD "m5stack"
#elif defined( ARDUINO_M5STACK_FIRE )
  #warning M5STACK FIRE DETECTED !!
  #define PLATFORM_NAME "M5Fire"
  #define DEFAULT_REGISTRY_BOARD "m5stack"
#elif defined( ARDUINO_ODROID_ESP32 )
  #warning ODROID DETECTED !!
  #define PLATFORM_NAME "Odroid-GO"
  #define DEFAULT_REGISTRY_BOARD "odroid"
#elif defined ( ARDUINO_ESP32_DEV )
  #warning WROVER DETECTED !!
  #define DEFAULT_REGISTRY_BOARD "esp32"
  #define PLATFORM_NAME "ESP32"
#else
  #warning NOTHING DETECTED !!
  #define DEFAULT_REGISTRY_BOARD "lambda"
  #define PLATFORM_NAME "LAMBDA"
#endif


#if defined(_CHIMERA_CORE_)
  // auto-select SD source
  #define M5_FS SDUPDATER_FS
#else
  // until M5Stack clean up their flags
  #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  #define M5_FS SD
#endif

#include <sys/time.h>
#include "compile_time.h"
#include <SPIFFS.h>

#include "core.h"

#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater

#if defined(_CHIMERA_CORE_)
  #include "lgfx/utility/lgfx_qrcode.h"
  #define qrcode_getBufferSize lgfx_qrcode_getBufferSize
  #define qrcode_initText lgfx_qrcode_initText
  #define qrcode_initBytes lgfx_qrcode_initBytes
  #define qrcode_getModule lgfx_qrcode_getModule
#else
  #ifndef _QRCODE_H_
    #include "utility/qrcode.h" // from M5Stack Core
  #endif
#endif

#include "SAM.h" // altered version of https://github.com/tomsuch/M5StackSAM, maintained at https://github.com/tobozo/M5StackSAM/
#include <ArduinoJson.h>     // https://github.com/bblanchon/ArduinoJson/
#include <Preferences.h>
#include "i18n.h"            // language file
#include "assets.h"          // some artwork for the UI
#include "controls.h"        // keypad / joypad / keyboard controls
#include "fsformat.h"        // filesystem bin formats, functions, helpers
#include "downloader.h"      // binaries downloader module A.K.A YOLO Downloader
#include "partition_manager.h"

#define MAX_BRIGHTNESS 100
const unsigned long MS_BEFORE_SLEEP = 600000; // 600000 = 10mn
uint8_t brightness = MAX_BRIGHTNESS;

uint16_t appsCount = 0; // how many binary files
uint16_t appsCountProgress = 0; // progress window
bool inInfoMenu = false; // menu state machine
unsigned long lastcheck = millis(); // timer check
unsigned long lastpush = millis(); // keypad/keyboard activity
uint16_t checkdelay = 300; // timer frequency
uint16_t MenuID; // pointer to the current menu item selected
uint16_t PageID;
uint16_t Pages = 0;
uint16_t PageIndex;

int16_t scrollPointer = 0; // pointer to the scrollText position
unsigned long lastScrollRender = micros(); // timer for scrolling
String lastScrollMessage; // last scrolling string state
int16_t lastScrollOffset; // last scrolling string position


SDUpdater sdUpdater;
M5SAM M5Menu;
AppRegistry Registry;
TFT_eSprite sprite = TFT_eSprite( &tft );


/* vMicro compliance, see https://github.com/tobozo/M5Stack-SD-Updater/issues/5#issuecomment-386749435 */
void getMeta( fs::FS &fs, String metaFileName, JSONMeta &jsonMeta );
void freeAllMeta();
void freeMeta();
void renderIcon( FileInfo &fileInfo );
void renderMeta( JSONMeta &jsonMeta );
void qrRender( String text, float sizeinpixels );



void renderScroll( String scrollText, uint8_t x = 0, uint8_t y = 0, uint16_t width = tft.width() ) {

  if( scrollText=="" ) return;

  sprite.setTextSize( 2 ); // setup text size before it's measured
  sprite.setTextDatum( ML_DATUM );
  sprite.setTextWrap( false ); // lazy way to solve a wrap bug

  sprite.setColorDepth( 1 );
  sprite.createSprite( width, BUTTON_HEIGHT );
  //sprite.fillSprite( TFT_BLACK );

  if( !scrollText.endsWith( " " )) {
    scrollText += "   ***   "; // append a space since scrolling text *will* repeat
  }
  while( sprite.textWidth( scrollText ) < width ) {
    scrollText += scrollText; // grow text to desired width
  }

  String  scrollMe = "";
  int16_t textWidth = sprite.textWidth( scrollText );
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
  while( sprite.textWidth(scrollMe) < width ) {
    for( uint8_t i=0; i<scrollText.length(); i++ ) {
      char thisChar[2];
      thisChar[0] = scrollText[i];
      thisChar[1] = '\0';
      csize = sprite.textWidth( thisChar );
      vsize+=csize;
      vpos = vsize+scrollPointer;
      if( vpos>0 && vpos<=width ) {
        scrollMe += scrollText[i];
        lastcsize = csize;
        voffset = scrollPointer%lastcsize;
        scrollOffset = voffset;
        if( sprite.textWidth(scrollMe) > width-voffset ) {
          break; // break out of the loop and out of the while
        }
      }
    }
  }
  // display trim
  while( sprite.textWidth( scrollMe ) > width-voffset ) {
    scrollMe.remove( scrollMe.length()-1 );
  }
  //scrollMe.remove( scrollMe.length()-1 ); // one last for the ride
  // only draw if things changed
  if( scrollOffset!=lastScrollOffset || scrollMe!=lastScrollMessage ) {
    sprite.setTextColor( WHITE, BLACK ); // setting background color removes the flickering effect
    sprite.setCursor( scrollOffset, BUTTON_HEIGHT/2 );
    sprite.print( scrollMe );
    sprite.setTextColor( WHITE );
    sprite.pushSprite( x, y );
  }
  sprite.deleteSprite();
  lastScrollMessage = scrollMe;
  lastScrollOffset  = scrollOffset;
  lastScrollRender  = micros();
  //lastpush          = millis();
}


/* by file info */
void renderIcon( FileInfo &fileInfo ) {
  if( !fileInfo.hasMeta() || !fileInfo.hasIcon() ) {
    return;
  }
  JSONMeta jsonMeta = fileInfo.jsonMeta;
  tft.drawJpgFile( M5_FS, fileInfo.iconName.c_str(), tft.width()-jsonMeta.width-10, (tft.height()/2)-(jsonMeta.height/2)+10/*, jsonMeta.width, jsonMeta.height, 0, 0, JPEG_DIV_NONE*/ );
}

/* by menu ID */
void renderIcon( uint16_t MenuID ) {
  renderIcon( fileInfo[MenuID] );
}

/* by file name */
void renderFace( String face ) {
  tft.drawJpgFile( M5_FS, face.c_str(), 5, 85, 120, 120, 0, 0, JPEG_DIV_NONE );
}


void renderMeta( JSONMeta &jsonMeta ) {

  sprite.setTextSize( 1 );
  sprite.setTextDatum( TL_DATUM );
  sprite.setTextColor( WHITE, BLACK );
  sprite.setTextWrap( false );

  sprite.setColorDepth( 1 );
  sprite.createSprite( (tft.width() / 2)-20, 5*sprite.fontHeight() );
  sprite.setCursor( 0, 0 );
  sprite.println( fileInfo[MenuID].fileName );
  sprite.println();

  if( jsonMeta.authorName!="" && jsonMeta.projectURL!="" ) { // both values provided
    sprite.print( AUTHOR_PREFIX );
    sprite.print( jsonMeta.authorName );
    sprite.println( AUTHOR_SUFFIX );
    sprite.println();
    qrRender( jsonMeta.projectURL, 160 );
  } else if( jsonMeta.projectURL!="" ) { // only projectURL
    sprite.println( jsonMeta.projectURL );
    sprite.println();
    qrRender( jsonMeta.projectURL, 160 );
  } else { // only authorName
    sprite.println( jsonMeta.authorName );
    sprite.println();
  }

  sprite.println( String( fileInfo[MenuID].fileSize ) + String( FILESIZE_UNITS ) );
  sprite.pushSprite( 5, 35, TFT_BLACK );
  sprite.deleteSprite();

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
      return i+2;
    }
  }
  // there's no point in doing higher with M5Stack's display
  return 4;
}


void qrRender( String text, float sizeinpixels ) {
  // see https://github.com/Kongduino/M5_QR_Code/blob/master/M5_QRCode_Test.ino
  // Create the QR code
  QRCode qrcode;

  uint8_t ecc = 0; // QR on TFT can do with minimal ECC
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


void listDir( fs::FS &fs, const char * dirName, uint8_t levels, bool process ){
  log_i( DEBUG_DIRNAME, dirName );
  File root = fs.open( dirName );
  if( !root ){
    log_e( "%s", DEBUG_DIROPEN_FAILED );
    return;
  }
  if( !root.isDirectory() ){
    log_e( "%s", DEBUG_NOTADIR );
    return;
  }
  File file = root.openNextFile();
  while( file ){
    if( file.isDirectory() ){
      log_d( "%s %s", DEBUG_DIRLABEL, file.name() );
      if( levels ){
        listDir( fs, file.name(), levels -1, process );
      }
    } else {
      if( isValidAppName( file.name() ) ) {
        if( process ) {
          getFileInfo( fileInfo[appsCount], fs, file );
          if( appsCountProgress > 0 ) {
            float progressRatio = ((((float)appsCount+1.0) / (float)appsCountProgress) * 80.00)+20.00;
            tft.progressBar( 110, 112, 100, 20, progressRatio);
            tft.fillRect( 0, 140, tft.width(), 16, TFT_BLACK);
            tft.drawString( fileInfo[appsCount].displayName(), 160, 148, 2);
          }
        }
        appsCount++;
        if( appsCount >= M5SAM_LIST_MAX_COUNT-1 ) {
          //Serial.println( String( DEBUG_IGNORED ) + file.name() );
          log_w( "%s", DEBUG_ABORTLISTING );
          break; // don't make M5Stack list explode
        }
      } else {
        if( String( file.name() ).endsWith(".tmp") || String( file.name() ).endsWith(".pcap") ) {
          fs.remove( file.name() );
          log_d( "%s %s", DEBUG_CLEANED, file.name() );
        } else {
          log_d( "%s %s", DEBUG_IGNORED, file.name() );
        }
      }
    }
    file = root.openNextFile();
  }
  if( fs.exists( MENU_BIN ) ) {
    file = fs.open( MENU_BIN );
    if( process ) {
      getFileInfo( fileInfo[appsCount], fs, file );
      if( appsCountProgress > 0 ) {
        float progressRatio = ((((float)appsCount+1.0) / (float)appsCountProgress) * 80.00)+20.00;
        tft.progressBar( 110, 112, 100, 20, progressRatio);
        tft.fillRect( 0, 140, tft.width(), 16, TFT_BLACK);
        tft.drawString( fileInfo[appsCount].displayName(), 160, 148, 2);
      }
    }
    appsCount++;
  } else {
    log_w( "[WARNING] No %s file found\n", MENU_BIN );
  }
}


/*
 *  bubble sort filenames
 *  '32' is based on SD Card filename limitations
 */
void aSortFiles( uint8_t depth_level=32 ) {
  bool swapped;
  FileInfo temp;
  String name1, name2;
  do {
    swapped = false;
    for( uint16_t i=0; i<appsCount-1; i++ ) {
      name1 = fileInfo[i].fileName[0];
      name2 = fileInfo[i+1].fileName[0];
      if( name1==name2 ) {
        uint8_t depth = 0;
        while( depth <= depth_level ) {
          depth++;
          if( depth > fileInfo[i].fileName.length() || depth > fileInfo[i+1].fileName.length() ) {
            // end of filename
            break;
          }
          name1 = fileInfo[i].fileName[depth];
          name2 = fileInfo[i+1].fileName[depth];
          if( name1==name2 ) {
            continue;
          } else {
            break;
          }
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

  PageID = 0;
  Pages = appsCount / M5Menu.listPagination;
  if( appsCount % M5Menu.listPagination != 0 ) Pages++;
  PageIndex = 0;
  M5Menu.clearList();
  M5Menu.setListCaption( MENU_SUBTITLE );
  for( uint16_t i=0; i < appsCount; i++ ) {
    if( fileInfo[i].shortName() == "menu" ) {
      M5Menu.addList( ABOUT_THIS_MENU );
    } else {
      M5Menu.addList( fileInfo[i].displayName() );
    }
  }
}

void drawM5Menu( bool renderButtons = false ) {
  const char* paginationTpl = "Page %d / %d";
  char paginationStr[64];
  PageID = MenuID / M5Menu.listPagination;
  PageIndex = MenuID % M5Menu.listPagination;
  sprintf(paginationStr, paginationTpl, PageID+1, Pages);
  M5Menu.setListCaption( paginationStr );
  if( renderButtons ) {
    #if defined(ARDUINO_ODROID_ESP32) && defined(_CHIMERA_CORE_)
      M5Menu.drawAppMenu( MENU_TITLE, MENU_BTN_INFO, MENU_SCREENSHOT, MENU_BTN_PAGE, MENU_BTN_NEXT );
    #else
      M5Menu.drawAppMenu( MENU_TITLE, MENU_BTN_INFO, MENU_BTN_PAGE, MENU_BTN_NEXT );
    #endif
    tft.drawJpg(sd_updater15x16_jpg, sd_updater15x16_jpg_len, 296, 6, 15, 16);
    drawSDUpdaterChannel();
  }
  M5Menu.showList();
  renderIcon( MenuID );
  inInfoMenu = false;
  lastpush = millis();
}


void pageDown() {
  if( PageID < Pages -1 ) {
    PageID++;
    MenuID = (PageID * M5Menu.listPagination) -1;
    M5Menu.setListID( MenuID );
    M5Menu.nextList();
    MenuID = M5Menu.getListID();
  } else {
    PageID = 0;
    MenuID = 0;
  }
  M5Menu.setListID( MenuID );
  drawM5Menu( inInfoMenu );
}

void pageUp() {
  if( PageID > 0 ) {
    PageID--;
    MenuID -= M5Menu.listPagination;
    M5Menu.setListID( MenuID );
    drawM5Menu( inInfoMenu );
  }
}



void menuUp() {
  MenuID = M5Menu.getListID();
  if( MenuID > 0 ) {
    if( (MenuID - 1)%M5Menu.listPagination==0 ) {
      MenuID += (M5Menu.listPagination-1);
    } else {
      MenuID--;
    }
  } else {
    MenuID = appsCount-1;
  }
  M5Menu.setListID( MenuID );
  drawM5Menu( inInfoMenu );
}


void menuDown( int jumpSize = 1 ) {
  if(MenuID<appsCount-1){
    if( (MenuID + 1)%M5Menu.listPagination==0 ) {
      MenuID -= (M5Menu.listPagination-1);
    } else {
      MenuID++;
    }
  } else {
    MenuID = PageID * M5Menu.listPagination;
  }
  M5Menu.setListID( MenuID );
  drawM5Menu( inInfoMenu );
}


void menuInfo() {
  inInfoMenu = true;
  M5Menu.windowClr();
  if( MenuID == 0 ) {
    // downloader
    M5Menu.drawAppMenu( "Apps Downloader", MENU_BTN_LAUNCH, MENU_BTN_SOURCE, MENU_BTN_BACK );
    lastpush = millis();
    return;
  } else if( fileInfo[ MenuID ].fileName.endsWith( launcherSignature ) ) {
    M5Menu.drawAppMenu( String(MENU_TITLE), MENU_BTN_SET, MENU_BTN_UPDATE, MENU_BTN_BACK );
  } else {
    M5Menu.drawAppMenu( String(MENU_TITLE), MENU_BTN_LOAD, MENU_BTN_UPDATE, MENU_BTN_BACK );
  }
  drawSDUpdaterChannel();
  renderMeta( fileInfo[MenuID].jsonMeta );
  if( fileInfo[MenuID].hasFace() ) {
    renderFace( fileInfo[MenuID].faceName );
  }
  lastpush = millis();
}


void checkMenuTimeStamp() {
  File menu = M5_FS.open( MENU_BIN );
  time_t lastWrite;
  if( menu ) {
    lastWrite = menu.getLastWrite();
    menu.close();
  } else {
    lastWrite = __TIME_UNIX__;
  }

  // setting a pseudo realistic internal clock time when no NTP sync occured,
  // and before writing to the SD Card gives unacurate but better timestamps
  // than the default [1980-01-01 00:00:00]
  int epoch_time = __TIME_UNIX__; // this macro is populated at compilation time
  struct tm * tmstruct = localtime(&lastWrite);
  String timeSource = "this sketch build";
  String timeStatus;

  if( (tmstruct->tm_year)+1900 < 2000 ) {
    timeStatus = "an unreliable";
  } else {
    int tmptime = mktime(tmstruct); // epoch time ( seconds since 1st jan 1969 )
    if( tmptime > epoch_time ) {
      timeSource = "menu.bin's lastWrite";
      timeStatus = "a realistic";
      epoch_time = tmptime;
    } else {
      timeStatus = "an obsolete";
    }
  }

  log_w(DEBUG_TIMESTAMP_GUESS, timeStatus.c_str(), (tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec, timeSource.c_str() );

  timeval epoch = {epoch_time, 0};
  const timeval *tv = &epoch;
  settimeofday(tv, NULL);

  struct tm now;
  getLocalTime(&now,0);

  Serial.printf("[Hobo style] Clock set to %s source (%s): ", timeStatus.c_str(), timeSource.c_str());
  Serial.println(&now,"%B %d %Y %H:%M:%S (%A)");

}


void downloaderMenu() {


  int resp = modalConfirm( "chantool", CHANNEL_TOOL, CHANNEL_TOOL_PROMPT, CHANNEL_TOOL_TEXT, DOWNLOADER_MODAL_CHANGE, MENU_BTN_UPDATE, "WiFi" );
  // choose between updating the JSON or changing the default channel
  switch( resp ) {

    case HID_SELECT:
      resp = modalConfirm( "chanpick", CHANNEL_CHOOSER, CHANNEL_CHOOSER_PROMPT, CHANNEL_CHOOSER_TEXT, DOWNLOADER_MODAL_CHANGE, MENU_BTN_CANCEL, MENU_BTN_BACK );
      if( resp == HID_SELECT ) {
        if( Registry.pref_default_channel == "master" ) {
          Registry.pref_default_channel = "unstable";
        } else {
          Registry.pref_default_channel = "master";
        }
        registrySave( Registry, appRegistryFolder + "/" + appRegistryDefaultName );
        Serial.println("Will reload in 5 sec");
        delay(5000);
        ESP.restart();
      }
    break;

    case HID_PAGE_DOWN:
      resp = modalConfirm( "chanupd", CHANNEL_DOWNLOADER, CHANNEL_DOWNLOADER_PROMPT, CHANNEL_DOWNLOADER_TEXT, MENU_BTN_UPDATE, MENU_BTN_CANCEL, MENU_BTN_BACK );
      if( resp == HID_SELECT ) {
        // TODO: WiFi connect, wget file and save to SD
        registryFetch( Registry, appRegistryFolder + "/" + appRegistryDefaultName );
      }
    break;

    default:
      resp = modalConfirm( "appDlChooser", "WiFi Setup", "WiFi Manager", "        Start WiFi Manager ?",  "Start", MENU_BTN_CANCEL, MENU_BTN_BACK );
      if( resp == HID_SELECT ) {
        wifiManagerSetup();
        wifiManagerLoop();
        ESP.restart();
      }
    break;

  }
  drawM5Menu( inInfoMenu );
}


void updateApp( FileInfo &info ) {
  String appName = info.shortName();
  //appName.replace(".bin", "");
  //appName.replace(".BIN", "");
  //appName.replace("/", "");
  Serial.println( appName );
  updateOne( appName );
  drawM5Menu( inInfoMenu );
}


void launchApp( FileInfo &info ) {
  if( info.fileName == String( DOWNLOADER_BIN ) ) {
    if( modalConfirm( "launchapp", DOWNLOADER_MODAL_NAME, DOWNLOADER_MODAL_TITLE, DOWNLOADER_MODAL_BODY ) == HID_SELECT ) {
      updateAll();
    }
    // action cancelled or refused by user
    drawM5Menu( inInfoMenu );
    return;
  }
  if( info.fileName.endsWith( launcherSignature ) ) {
    log_w("Will overwrite current %s with a copy of %s\n", MENU_BIN, info.fileName.c_str() );
    if( replaceLauncher( M5_FS, info ) ) {
      // fine
    } else {
      log_e("Failed to overwrite %s!", info.fileName.c_str());
      return;
    }
  }
  sdUpdater.updateFromFS( M5_FS, fileInfo[MenuID].fileName );
  ESP.restart();
}


void UISetup() {

  HIDInit();
  // make sure you're using the latest from https://github.com/tobozo/M5StackSAM/
  M5Menu.listMaxLabelSize = 32; // list labels will be trimmed
  M5Menu.listPagination = 8; // 8 items per page
  M5Menu.listPageLabelsOffset = 42; // initially 80, pixels offset from top screen for list items
  M5Menu.listCaptionDatum = TR_DATUM; // initially TC_DATUM=top centered, TL_DATUM=top left (default), top/right/bottom/left
  M5Menu.listCaptionXPos = tft.width()-10; // initially M5.Lcd.width()/2, text cursor position-x for list caption
  M5Menu.listCaptionYPos = 42; // initially 45, text cursor position-x for list caption

  Serial.println( WELCOME_MESSAGE );
  Serial.println( INIT_MESSAGE );
  Serial.printf( M5_SAM_MENU_SETTINGS, M5Menu.listPagination, M5SAM_LIST_MAX_COUNT);
  Serial.printf("Has PSRam: %s\n", psramInit() ? "true" : "false");

  heapState();

  tft.setBrightness(100);
  lastcheck = millis();
  tft.drawJpg(disk01_jpg, 1775, (tft.width()-30)/2, 100);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor( TFT_WHITE, TFT_BLACK );
  tft.setTextSize( 1 );
  tft.drawString( SD_LOADING_MESSAGE, 160, 142, 1 );

  bool toggle = true;
#ifdef _CHIMERA_CORE_
  while( !M5.sd_begin() )
#else
  while( !M5_FS.begin() )
#endif
  {
    // TODO: make a more fancy animation
    toggle = !toggle;
    tft.setTextColor( toggle ? TFT_BLACK : TFT_WHITE );
    tft.drawString( INSERTSD_MESSAGE, 160, 84, 2 );
    tft.drawJpg( toggle ? disk01_jpg : disk00_jpg, 1775, (tft.width()-30)/2, 100 );
    delay( toggle ? 300 : 500 );
    // go to sleep after a minute, no need to hammer the SD Card reader
    if( lastcheck + 60000 < millis() ) {
      Serial.println( GOTOSLEEP_MESSAGE );
      #ifdef ARDUINO_M5STACK_Core2
        M5.Axp.DeepSleep();
      #else
        M5.setWakeupButton( BUTTON_B_PIN );
        M5.powerOFF();
      #endif
    }
  }
  tft.setTextDatum(TL_DATUM);

  unsigned long longPush = 10000;
  unsigned long shortPush = 5000;

  #ifdef ARDUINO_M5STACK_Core2
    tft.setCursor(0,0);
    tft.print("SDUpdater\npress BtnA");
    tft.setCursor(0,0);
    delay(500);
    M5.update();
  #endif

  if( M5.BtnB.isPressed() )
  {
    unsigned long pushStart = millis();
    unsigned long pushDuration = 0;
    drawAppMenu(); // render the menu
    M5.update();
    tft.setTextColor( WHITE, M5MENU_GREY );
    tft.setTextDatum(MC_DATUM);
    char remainingStr[32];
    while( M5.BtnB.isPressed() ) {
      pushDuration = millis() - pushStart;
      if( pushDuration > longPush ) break;
      if( pushDuration > shortPush ) {
        tft.setTextColor( WHITE, RED );
        tft.drawString( "FULL RESET", 160, 100, 2 );
        sprintf( remainingStr, "%.2f", (float)(longPush-pushDuration)/1000 );
      } else {
        tft.drawString( "TLS RESET", 160, 100, 2 );
        sprintf( remainingStr, "%.2f", (float)(shortPush-pushDuration)/1000 );
      }
      tft.drawString( remainingStr, 160, 120, 2 );
      delay(100);
      M5.update();
    }
    tft.setTextDatum(TL_DATUM);

    Serial.printf("Push duration : %d\n", (int)pushDuration );
    if( pushDuration > shortPush ) {
      // Short push at boot = cleanup /cert/ and /.registry/ folders
      cleanDir( SD_CERT_PATH );
      cleanDir( appRegistryFolder.c_str() );
    }
    if( pushDuration > longPush ) {
      int resp = modalConfirm( "cleanup", "DELETE APPS", "CAUTION! This will remove all apps and assets.", "    Obliviate?",  "DELETE", "CANCEL", "NOES!" );
      if( resp == HID_SELECT ) {
        checkMenuTimeStamp(); // set the time before cleaning up the folder
        cleanDir( "/" );
        cleanDir( "/jpg/" );
        cleanDir( "/json/" );
        drawAppMenu(); // render the menu
        copyPartition(); // restore the menu.bin file
      }
    }
    Serial.println( GOTOSLEEP_MESSAGE );
    #ifdef ARDUINO_M5STACK_Core2
      M5.Axp.DeepSleep();
    #else
      M5.setWakeupButton( BUTTON_B_PIN );
      M5.powerOFF();
    #endif

  }

}


void doFSChecks() {
  tft.setTextColor( WHITE );
  tft.setTextSize( 1 );
  tft.clear();

  // TODO: check menu.bin datetime and set
  checkMenuTimeStamp();
  checkMenuStickyPartition();

  tft.fillRect(110, 112, 100, 20,0);
  tft.progressBar( 110, 112, 100, 20, 10);

  scanDataFolder(); // do SD / SPIFFS health checks
  Registry = registryInit(); // load registry profile

  if( !M5_FS.exists( DOWNLOADER_BIN ) ) {
    if( M5_FS.exists( DOWNLOADER_BIN_VIRTUAL) ) { // rename for hoisting in the list
      M5_FS.rename( DOWNLOADER_BIN_VIRTUAL, DOWNLOADER_BIN );
    } else { // create a dummy file to enable the feature
      fs::File dummyDownloader = M5_FS.open( DOWNLOADER_BIN, FILE_WRITE);
      dummyDownloader.print("Fake Binary");
      dummyDownloader.close();
    }
  } else { // cleanup old legacy file if necessary
    if( M5_FS.exists(DOWNLOADER_BIN_VIRTUAL) ) {
      M5_FS.remove( DOWNLOADER_BIN_VIRTUAL );
    }
  }
}


void doFSInventory() {
  tft.setTextColor( WHITE );
  tft.setTextSize( 1 );
  tft.clear();
  tft.progressBar( 110, 112, 100, 20, 20);
  appsCount = 0;
  listDir(M5_FS, "/", 0, false); // count valid files first so a progress meter can be displayed
  appsCountProgress = appsCount;
  appsCount = 0;
  tft.drawJpg(sd_updater32x40_jpg, sd_updater32x40_jpg_len, (tft.width()-32)/2, 40);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Scanning SD Card", 160, 95, 2);
  listDir(M5_FS, "/", 0, true); // now retrieve files meta
  tft.setTextDatum(TL_DATUM);
  aSortFiles(); // bubble sort alphabetically
  buildM5Menu();
  drawM5Menu( true ); // render the menu
  lastcheck = millis(); // reset the timer
  lastpush = millis(); // reset the timer
  heapState();
}


void HIDMenuObserve() {

  HIDSignal hidState = getControls();

  if( hidState!=HID_INERT && brightness != MAX_BRIGHTNESS ) {
    // some activity occured, restore brightness
    Serial.println(".. !!! Waking up !!");
    brightness = MAX_BRIGHTNESS;
    tft.setBrightness( brightness );
  }
  switch( hidState ) {
    #ifdef _CHIMERA_CORE_
      case HID_SCREENSHOT:
        M5.ScreenShot.snap( "screenshot" );
      break;
    #endif
    case HID_DOWN:
      if( !inInfoMenu ) {
        menuDown();
      } else {
        drawM5Menu( inInfoMenu );
      }
    break;
    case HID_UP:
      if( inInfoMenu ) {
        drawM5Menu( inInfoMenu );
      } else {
        menuUp();
      }
    break;
    case HID_SELECT:
      if( !inInfoMenu ) {
        menuInfo();
      } else {
        launchApp( fileInfo[MenuID] );
      }
    break;
    case HID_PAGE_DOWN:
      if( inInfoMenu ) {
        // update
        if( fileInfo[MenuID].fileName == String( DOWNLOADER_BIN ) ) {
          downloaderMenu();
        } else {
          updateApp( fileInfo[MenuID] );
        }
      } else {
        pageDown();
      }
    break;
    case HID_PAGE_UP:
      if( inInfoMenu ) {
        // ignore
      } else {
        pageUp();
      }
    break;
    default:
    case HID_INERT:
      if( inInfoMenu ) { // !! scrolling text also prevents sleep mode !!
        renderScroll( fileInfo[MenuID].jsonMeta.credits );
      }
    break;
  }
  M5.update();
}


void sleepTimer() {
  if( lastpush + MS_BEFORE_SLEEP < millis() ) { // go to sleep if nothing happens for a while
    if( brightness > 1 ) { // slowly dim the screen first
      brightness--;
      if( brightness %10 == 0 ) {
        Serial.print("(\".¬.\") ");
      }
      if( brightness %30 == 0 ) {
        Serial.print(" Yawn... ");
      }
      if( brightness %7 == 0 ) {
        Serial.println(" .zzZzzz. ");
      }
      tft.setBrightness( brightness );
      lastpush = millis() - (MS_BEFORE_SLEEP - brightness*10); // exponential dimming effect
      return;
    }
    Serial.println( GOTOSLEEP_MESSAGE );
    #ifdef ARDUINO_M5STACK_Core2
      M5.Axp.DeepSleep();
    #else
      M5.setWakeupButton( BUTTON_B_PIN );
      M5.powerOFF();
    #endif
  }
}
