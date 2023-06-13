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
// #pragma GCC push_options
// #pragma GCC optimize ("Os")
#pragma once


// TODO: moved USE_DOWNLOADER features to "AppStore.ino"
// auto-select board
#if defined( ARDUINO_M5STACK_Core2 )
  #pragma message "M5STACK Core2 detected"
  #define PLATFORM_NAME "M5Core2"
  #define DEFAULT_REGISTRY_BOARD "m5core2"
  //#define USE_DOWNLOADER
#elif defined( ARDUINO_M5Stack_Core_ESP32 )
  #pragma message "M5STACK CLASSIC detected"
  #define PLATFORM_NAME "M5Stack"
  #define DEFAULT_REGISTRY_BOARD "m5stack"
  //#define USE_DOWNLOADER // moved to AppStore.ino
#elif defined( ARDUINO_M5STACK_FIRE )
  #pragma message "M5STACK FIRE detected"
  #define PLATFORM_NAME "M5Fire"
  #define DEFAULT_REGISTRY_BOARD "m5fire"
  //#define USE_DOWNLOADER
#elif defined( ARDUINO_ODROID_ESP32 )
  #pragma message "ODROID detected"
  #define PLATFORM_NAME "Odroid-GO"
  #define DEFAULT_REGISTRY_BOARD "odroid"
#elif defined ( ARDUINO_ESP32_DEV ) || defined( ARDUINO_LOLIN_D32_PRO )
  #pragma message "WROVER OR LOLIN_D32_PRO detected"
  #define DEFAULT_REGISTRY_BOARD "esp32"
  #define PLATFORM_NAME "ESP32"
  //#define USE_DOWNLOADER
#elif defined( ARDUINO_ESP32_S3_BOX )
  #pragma message "ESP32_S3_BOX detected"
  #define DEFAULT_REGISTRY_BOARD "esp32s3"
  #define PLATFORM_NAME "S3Box"
#elif defined ARDUINO_M5STACK_CORES3
  #pragma message "M5Sack CoreS3 detected"
  #define DEFAULT_REGISTRY_BOARD "cores3"
  #define PLATFORM_NAME "CoreS3"
#elif defined( ARDUINO_M5STACK_ATOM_AND_TFCARD )
  #pragma message "M5Stack ATOM detected"
  #define DEFAULT_REGISTRY_BOARD "m5atom"
  #define PLATFORM_NAME "M5 ATOM(matrix/lite)"
#else
  #pragma message "NOTHING detected"
  #define DEFAULT_REGISTRY_BOARD "lambda"
  #define PLATFORM_NAME "LAMBDA"
#endif


//#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
//#define M5_FS SD

#include "core.h"

#if !defined(ARDUINO_M5STACK_ATOM_AND_TFCARD)

#else
  //#include <SD.h>
  //#include "LGFX_8BIT_CVBS.h"
  //#include <Button2.h>
  //Button2 button;//for G39

  //#define USE_DISPLAY
  //#define LGFX_ONLY
  //#define TFCARD_CS_PIN -1

  //static LGFX_8BIT_CVBS tft;
  //#define LGFX LGFX_8BIT_CVBS
  //#define BUTTON_WIDTH 60
  //#define SDU_APP_NAME "Application Launcher"
  // #define SDU_NO_AUTODETECT           // Disable autodetect (only works with <M5xxx.h> and <Chimera> cores)
  // #define SDU_USE_DISPLAY             // Enable display functionalities (lobby, buttons, progress loader)
  // #define HAS_LGFX                    // Display UI will use LGFX API (without this it will be tft_eSPI API)
  // #define SDU_TouchButton LGFX_Button // Set button renderer
  // #define SDU_Sprite LGFX_Sprite
  // #define SDU_DISPLAY_TYPE M5Display*
  // #define SDU_DISPLAY_OBJ_PTR &tft
  // #define SDU_TRIGGER_SOURCE_DEFAULT TriggerSource::SDU_TRIGGER_PUSHBUTTON // Attach push buttons as trigger source


  //#include <M5StackUpdater.h>

  //static LGFX_Sprite sprite(&tft);
  //fs::SDFS &M5_FS(SD);

  //Button2 button;//for G39

#endif

#include <sys/time.h>
#include "compile_time.h"
//#include <SPIFFS.h>

#if defined(_CHIMERA_CORE_) || defined(ARDUINO_M5STACK_ATOM_AND_TFCARD) || __has_include("lgfx/utility/lgfx_qrcode.h")
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


#if defined USE_DOWNLOADER
  #include "downloader.h"      // binaries downloader module A.K.A YOLO Downloader
#endif

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

M5SAM M5Menu;
#if defined USE_DOWNLOADER
  AppRegistry Registry;
#endif


/* vMicro compliance, see https://github.com/tobozo/M5Stack-SD-Updater/issues/5#issuecomment-386749435 */
//void getMeta( fs::FS &fs, String metaFileName, JSONMeta &jsonMeta );
void freeAllMeta();
void freeMeta();
void renderIcon( FileInfo &fileInfo );
void renderMeta( JSONMeta &jsonMeta );
//void qrRender( String text, float sizeinpixels, int xOffset=-1, int yOffset=-1 );
void qrRender( SDU_DISPLAY_TYPE gfx, String text, int posX, int posY, uint32_t width, uint32_t height );

void progressBar(SDU_DISPLAY_TYPE tft, int x, int y, int w, int h, uint8_t val, uint16_t color, uint16_t bgcolor)
{
  tft->drawRect(x, y, w, h, color);
  if (val > 100) val = 100;
  if (val == 0) tft->fillRect(x + 1, y + 1, w - 2, h - 2, bgcolor);
  else {
    int fillw = (w * (((float)val) / 100.0)) - 2;
    tft->fillRect(x + 1, y + 1, fillw - 2, h - 2, color);
    tft->fillRect(x + fillw + 1, y + 1, w - fillw - 2, h - 2, bgcolor);
  }
}

static String heapState()
{
  log_i("\nRAM SIZE:\t%.2f KB\nFREE RAM:\t%.2f KB\nMAX ALLOC:\t%.2f KB",
    ESP.getHeapSize() / 1024.0,
    ESP.getFreeHeap() / 1024.0,
    ESP.getMaxAllocHeap() / 1024.0
  );
  return "";
}

void gotoSleep()
{
  Serial.println( GOTOSLEEP_MESSAGE );
  #ifdef ARDUINO_M5STACK_Core2
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0); // gpio39 == touch INT
    //M5.Axp.DeepSleep();
  #else
    #if !defined _CHIMERA_CORE_ || defined HAS_POWER || defined HAS_IP5306
      //M5.setWakeupButton( BUTTON_B_PIN );
      //M5.powerOFF();
    #else
    #endif
  #endif
  delay(100);
  //M5.Lcd.fillScreen(TFT_BLACK);
  //M5.Lcd.sleep();
  //M5.Lcd.waitDisplay();
  esp_deep_sleep_start();
}

void renderScroll( String scrollText, uint8_t x = 0, uint8_t y = 0, uint16_t width = tft.width() )
{
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
    sprite.setTextColor( TFT_WHITE, TFT_BLACK ); // setting background color removes the flickering effect
    sprite.setCursor( scrollOffset, BUTTON_HEIGHT/2 );
    sprite.print( scrollMe );
    sprite.setTextColor( TFT_WHITE );
    sprite.pushSprite( x, y );
  }
  sprite.deleteSprite();
  lastScrollMessage = scrollMe;
  lastScrollOffset  = scrollOffset;
  lastScrollRender  = micros();
  //lastpush          = millis();
}


/* by file info */
void renderIcon( FileInfo &fileInfo )
{
  if( !fileInfo.hasMeta() || !fileInfo.hasIcon() ) {
    return;
  }
  JSONMeta jsonMeta = fileInfo.jsonMeta;
  log_d("[%d] Will render icon %s at[%d:%d]", ESP.getFreeHeap(), fileInfo.iconName.c_str(), tft.width()-jsonMeta.width-10, (tft.height()/2)-(jsonMeta.height/2)+10 );

  fs::File iconFile = M5_FS.open( fileInfo.iconName.c_str()  );
  if( !iconFile ) return;
  tft.drawJpg( &iconFile, 190, 60, 110, 110 );
  iconFile.close();
  //tft.drawJpgFile( M5_FS, fileInfo.iconName.c_str(), tft.width()-jsonMeta.width-10, (tft.height()/2)-(jsonMeta.height/2)+10/*, jsonMeta.width, jsonMeta.height, 0, 0, JPEG_DIV_NONE*/ );
}

/* by menu ID */
void renderIcon( uint16_t MenuID )
{
  renderIcon( fileInfo[MenuID] );
}

/* by file name */
void renderFace( String face )
{
  log_d("[%d] Will render face %s", ESP.getFreeHeap(), face.c_str() );
  fs::File iconFile = M5_FS.open( face.c_str()  );
  if( !iconFile ) return;
  tft.drawJpg( &iconFile, 5, 85, 120, 120 );
  iconFile.close();
  //tft.drawJpgFile( M5_FS, face.c_str(), 5, 85/*, 120, 120, 0, 0, JPEG_DIV_NONE*/ );
}


void renderMeta( JSONMeta &jsonMeta )
{
  sprite.setTextSize( 1 );
  sprite.setTextDatum( TL_DATUM );
  sprite.setTextColor( TFT_WHITE, TFT_BLACK );
  sprite.setTextWrap( false );

  sprite.setColorDepth( 1 );
  sprite.createSprite( (tft.width() / 2)-20, 5*sprite.fontHeight() );
  sprite.setCursor( 0, 0 );
  sprite.println( fileInfo[MenuID].fileName );
  sprite.println();

  log_d("Rendering meta");

  if( jsonMeta.authorName!="" && jsonMeta.projectURL!="" ) { // both values provided
    log_d("Rendering QRCode+author");
    sprite.print( AUTHOR_PREFIX );
    sprite.print( jsonMeta.authorName );
    sprite.println( AUTHOR_SUFFIX );
    sprite.println();
    qrRender( SDU_DISPLAY_OBJ_PTR, jsonMeta.projectURL, 155, 45, 150, 150 );
  } else if( jsonMeta.projectURL!="" ) { // only projectURL
    log_d("Rendering QRCode");
    sprite.println( jsonMeta.projectURL );
    sprite.println();
    qrRender( SDU_DISPLAY_OBJ_PTR, jsonMeta.projectURL, 155, 45, 150, 150 );
  } else { // only authorName
    log_d("Rendering Authorname");
    sprite.println( jsonMeta.authorName );
    sprite.println();
  }

  sprite.println( String( fileInfo[MenuID].fileSize ) + String( FILESIZE_UNITS ) );
  sprite.pushSprite( 5, 35, TFT_BLACK );
  sprite.deleteSprite();
}


/* give up on redundancy and ECC to produce less and bigger squares */
uint8_t getLowestQRVersionFromString( String text, uint8_t ecc )
{
  #define QR_MAX_VERSION 9
  if(ecc>QR_MAX_VERSION) return QR_MAX_VERSION; // fail fast
  uint16_t len = text.length();
  uint8_t QRMaxLenByECCLevel[4][QR_MAX_VERSION] = {
    // https://www.qrcode.com/en/about/version.html
    // Handling version 1-9 only since there's no point with M5Stack's 320x240 display (next version is at 271)
    { 17, 32, 53, 78, 106, 134, 154, 192, 230 }, // L
    { 14, 26, 45, 62, 84,  106, 122, 152, 180 }, // M
    { 11, 20, 32, 46, 60,  74,  86,  108, 130 }, // Q
    { 7,  14, 24, 34, 44,  58,  64,  84,  98  }  // H
  };
  for( uint8_t i=0; i<QR_MAX_VERSION; i++ ) {
    if( len <= QRMaxLenByECCLevel[ecc][i] ) {
      log_d("string len=%d bytes, fits in version %d / ecc %d (max=%d)", len, i+1, ecc, QRMaxLenByECCLevel[ecc][i] );
      return i+1;
    }
  }
  log_e("String length exceeds output parameters");
  return QR_MAX_VERSION;
}


void qrRender( SDU_DISPLAY_TYPE gfx, String text, int posX, int posY, uint32_t width, uint32_t height )
{
  // see https://github.com/Kongduino/M5_QR_Code/blob/master/M5_QRCode_Test.ino
  // Create the QR code
  QRCode qrcode;

  uint8_t ecc = 0; // QR on TFT can do with minimal ECC
  uint8_t version = getLowestQRVersionFromString( text, ecc );

  uint8_t qrcodeData[lgfx_qrcode_getBufferSize( version )];
  lgfx_qrcode_initText( &qrcode, qrcodeData, version, ecc, text.c_str() );

  uint32_t gridSize  = (qrcode.size + 4);   // margin: 2 dots
  uint32_t dotWidth  = width / gridSize;    // rounded
  uint32_t dotHeight = height / gridSize;   // rounded
  uint32_t realWidth  = dotWidth*gridSize;  // recalculated
  uint32_t realHeight = dotHeight*gridSize; // recalculated
  if( realWidth > width || realHeight > height ) {
    log_e("Can't fit QR with gridsize(%d),dotSize(%d) =>[%dx%d] => [%dx%d]", gridSize, dotWidth, realWidth, realHeight, width, height );
    return;
  } else {
    log_d("Rendering QR Code '%s' (%d bytes) on version #%d on [%dx%d] => [%dx%d] grid", text.c_str(), text.length(), version, qrcode.size, qrcode.size, realWidth, realHeight );
  }

  uint8_t marginX = (width  - qrcode.size*dotWidth)/2;
  uint8_t marginY = (height - qrcode.size*dotHeight)/2;

  gfx->fillRect( posX, posY, width, height, TFT_WHITE );

  for ( uint8_t y = 0; y < qrcode.size; y++ ) {
    // Each horizontal module
    for ( uint8_t x = 0; x < qrcode.size; x++ ) {
      bool q = lgfx_qrcode_getModule( &qrcode, x, y );
      if (q) {
        gfx->fillRect( x*dotWidth +posX+marginX, y*dotHeight +posY+marginY, dotWidth, dotHeight, TFT_BLACK );
      }
    }
  }
}


void listDir( fs::FS &fs, const char * dirName, uint8_t levels, bool process )
{
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
  tft.setFont( &Font2 );
  while( file ){
    if( file.isDirectory() ){
      log_d( "%s %s", DEBUG_DIRLABEL, SDUpdater::fs_file_path(&file) );
      if( levels ){
        listDir( fs, SDUpdater::fs_file_path(&file), levels -1, process );
      }
    } else {
      if( isValidAppName( SDUpdater::fs_file_path(&file) ) ) {
        if( process ) {
          FileInfo newFile = FileInfo();
          fileInfo.push_back(newFile);
          newFile.srcfs.fs = &fs;
          getFileInfo( fileInfo[appsCount], &file );
          if( appsCountProgress > 0 ) {
            float progressRatio = ((((float)appsCount+1.0) / (float)appsCountProgress) * 80.00)+20.00;
            progressBar( SDU_DISPLAY_OBJ_PTR, 110, 112, 100, 20, progressRatio);
            tft.fillRect( 0, 140, tft.width(), 16, TFT_BLACK);
            tft.drawString( fileInfo[appsCount].displayName(), 160, 148 );
          }
        }
        appsCount++;
        if( appsCount >= M5SAM_LIST_MAX_COUNT-1 ) {
          //Serial.println( String( DEBUG_IGNORED ) + SDUpdater::fs_file_path(&file) );
          log_w( "%s", DEBUG_ABORTLISTING );
          break; // don't make M5Stack list explode
        }
      } else {
        if( String( SDUpdater::fs_file_path(&file) ).endsWith(".tmp") || String( SDUpdater::fs_file_path(&file) ).endsWith(".pcap") ) {
          fs.remove( SDUpdater::fs_file_path(&file) );
          log_d( "%s %s", DEBUG_CLEANED, SDUpdater::fs_file_path(&file) );
        } else {
          log_d( "%s %s", DEBUG_IGNORED, SDUpdater::fs_file_path(&file) );
        }
      }
    }
    file = root.openNextFile();
  }
  if( fs.exists( MENU_BIN ) ) {
    file = fs.open( MENU_BIN );
    if( process ) {
      FileInfo newFile = FileInfo();
      newFile.srcfs.fs = &fs;
      fileInfo.push_back(newFile);
      getFileInfo( fileInfo[appsCount], &file );
      if( appsCountProgress > 0 ) {
        float progressRatio = ((((float)appsCount+1.0) / (float)appsCountProgress) * 80.00)+20.00;
        progressBar( SDU_DISPLAY_OBJ_PTR, 110, 112, 100, 20, progressRatio);
        tft.fillRect( 0, 140, tft.width(), 16, TFT_BLACK);
        tft.drawString( fileInfo[appsCount].displayName(), 160, 148 );
      }
    }
    appsCount++;
  } else {
    log_w( "[WARNING] No %s file found\n", MENU_BIN );
  }
}


/*
 *  bubble sort filenames
 *  '32' is based on SPIFFS filename limitations
 */
void aSortFiles( uint8_t depth_level=32 )
{
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


void buildM5Menu()
{
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


void drawM5Menu( bool renderButtons = false )
{
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
    tft.drawJpg(sdUpdaterIcon15x16_jpg, sdUpdaterIcon15x16_jpg_len, 296, 6, 15, 16);
    if( factory_partition ) {
      tft.drawJpg(flashUpdaterIcon16x16_jpg, flashUpdaterIcon16x16_jpg_len, 8, 6, 16, 16);
    }
    #if defined USE_DOWNLOADER
      drawSDUpdaterChannel();
    #endif
  }
  M5Menu.showList();
  renderIcon( MenuID );
  inInfoMenu = false;
  lastpush = millis();
}


void pageDown()
{
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


void pageUp()
{
  if( PageID > 0 ) {
    PageID--;
    MenuID -= M5Menu.listPagination;
    M5Menu.setListID( MenuID );
    drawM5Menu( inInfoMenu );
  }
}



void menuUp()
{
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


void menuDown( int jumpSize = 1 )
{
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


void menuInfo()
{
  inInfoMenu = true;
  M5Menu.windowClr();

  #ifdef USE_DOWNLOADER
    if( MenuID == 0 ) {
      // downloader
      M5Menu.drawAppMenu( "Apps Downloader", MENU_BTN_LAUNCH, MENU_BTN_SOURCE, MENU_BTN_BACK );
      lastpush = millis();
      return;
    } else
  #endif
  if( fileInfo[ MenuID ].fileName.endsWith( launcherSignature ) ) {
    M5Menu.drawAppMenu( String(MENU_TITLE), MENU_BTN_SET, MENU_BTN_UPDATE, MENU_BTN_BACK );
  } else {
    M5Menu.drawAppMenu( String(MENU_TITLE), MENU_BTN_LOAD, MENU_BTN_UPDATE, MENU_BTN_BACK );
  }
  #if defined USE_DOWNLOADER
    drawSDUpdaterChannel();
  #endif
  renderMeta( fileInfo[MenuID].jsonMeta );
  if( fileInfo[MenuID].hasFace() ) {
    renderFace( fileInfo[MenuID].faceName );
  }
  lastpush = millis();
}


void checkMenuTimeStamp()
{
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

#if defined USE_DOWNLOADER

void downloaderMenu()
{
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
      #if defined USE_WIFI_MANAGER
        resp = modalConfirm( "appDlChooser", "WiFi Setup", "WiFi Manager", "        Start WiFi Manager ?",  "Start", MENU_BTN_CANCEL, MENU_BTN_BACK );
        if( resp == HID_SELECT ) {
          wifiManagerSetup();
          wifiManagerLoop();
          ESP.restart();
        }
      #endif
    break;

  }
  drawM5Menu( inInfoMenu );
}


void updateApp( FileInfo &info )
{
  String appName = info.shortName();
  //appName.replace(".bin", "");
  //appName.replace(".BIN", "");
  //appName.replace("/", "");
  Serial.println( appName );
  updateOne( appName );
  drawM5Menu( inInfoMenu );
}

#endif


void launchApp( FileInfo &info )
{
  #if defined USE_DOWNLOADER
    if( info.fileName == String( DOWNLOADER_BIN ) ) {
      if( modalConfirm( "launchapp", DOWNLOADER_MODAL_NAME, DOWNLOADER_MODAL_TITLE, DOWNLOADER_MODAL_BODY ) == HID_SELECT ) {
        updateAll();
      }
      // action cancelled or refused by user
      drawM5Menu( inInfoMenu );
      return;
    }
  #endif
  if( info.fileName.endsWith( launcherSignature ) ) {
    log_w("Will overwrite current %s with a copy of %s\n", MENU_BIN, info.fileName.c_str() );
    if( replaceLauncher( M5_FS, info ) ) {
      // fine
    } else {
      log_e("Failed to overwrite %s!", info.fileName.c_str());
      return;
    }
  }
  SDUpdaterNS::updateFromFS( M5_FS, fileInfo[MenuID].fileName );
  //updateFromFS( M5_FS, fileInfo[MenuID].fileName, TFCARD_CS_PIN );
  ESP.restart();
}


void UISetup()
{
  initFileInfo();
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

  //lsPart();

  tft.setBrightness(100);

  #if defined HAS_LGFX // reset scroll position
    log_d("Resetting scroll position");
    tft.setScrollRect(0, 0, tft.width(), tft.height() );
    tft.startWrite();
    tft.writecommand(0x37); // ILI934x/ST778x VSCRSADD Vertical scrolling pointer
    tft.writedata(0>>8);
    tft.writedata(0);
    tft.endWrite();
  #endif

  lastcheck = millis();
  tft.drawJpg(disk01_jpg, 1775, (tft.width()-30)/2, 100);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor( TFT_WHITE, TFT_BLACK );
  tft.setTextSize( 1 );
  tft.setFont( &Font0 );
  tft.drawString( SD_LOADING_MESSAGE, 160, 142 );

  //M5.update();

  bool toggle = true;
#ifdef _CHIMERA_CORE_
  while( !M5.sd_begin() )
#else
  while( !M5_FS.begin(4) )
#endif
  {
    // TODO: make a more fancy animation
    toggle = !toggle;
    tft.setTextColor( toggle ? TFT_BLACK : TFT_WHITE );
    tft.setFont( &Font2 );
    tft.drawString( INSERTSD_MESSAGE, 160, 84 );
    tft.drawJpg( toggle ? disk01_jpg : disk00_jpg, 1775, (tft.width()-30)/2, 100 );
    delay( toggle ? 300 : 500 );
    // go to sleep after a minute, no need to hammer the SD Card reader
    if( lastcheck + 60000 < millis() ) {
      gotoSleep();
    }
  }
  tft.setTextDatum(TL_DATUM);

  #if defined USE_DOWNLOADER

    unsigned long longPush = 10000;
    unsigned long shortPush = 5000;

    if( M5.BtnB.isPressed() )
    {
      unsigned long pushStart = millis();
      unsigned long pushDuration = 0;
      drawAppMenu(); // render the menu
      M5.update();
      tft.setTextColor( TFT_WHITE, M5MENU_GREY );
      tft.setTextDatum(MC_DATUM);
      tft.setFont( &Font2 );
      char remainingStr[32];
      while( M5.BtnB.isPressed() ) {
        pushDuration = millis() - pushStart;
        if( pushDuration > longPush ) break;
        if( pushDuration > shortPush ) {
          tft.setTextColor( TFT_WHITE, TFT_RED );
          tft.drawString( "FULL RESET", 160, 100 );
          sprintf( remainingStr, "%.2f", (float)(longPush-pushDuration)/1000 );
        } else {
          tft.drawString( "TLS RESET", 160, 100 );
          sprintf( remainingStr, "%.2f", (float)(shortPush-pushDuration)/1000 );
        }
        tft.drawString( remainingStr, 160, 120 );
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
          #if !defined HAS_RTC
            checkMenuTimeStamp(); // set the time before cleaning up the folder
          #endif
          cleanDir( "/" );
          cleanDir( "/jpg/" );
          cleanDir( "/json/" );
          drawAppMenu(); // render the menu
          copyPartition(); // restore the menu.bin file
        }
      }
      gotoSleep();
    }
  #endif

      //Serial.println("Going to factory in 5s");
      //delay(5000);
      //loadFactory();
      //ESP.restart();



}


void doFSChecks()
{
  tft.setTextColor( TFT_WHITE );
  tft.setTextSize( 1 );
  tft.clear();

  #if !defined HAS_RTC
    checkMenuTimeStamp();
  #endif

  factory_partition = Flash::getFactoryPartition();

  if( ! factory_partition ) {
    // propagate to SD and OTA1
    checkMenuStickyPartition();
  }

  tft.fillRect(110, 112, 100, 20,0);
  progressBar( SDU_DISPLAY_OBJ_PTR, 110, 112, 100, 20, 10);

  scanDataFolder(); // create necessary folders

  #if defined USE_DOWNLOADER
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
  #endif

}


void doFSInventory()
{
  tft.setTextColor( TFT_WHITE );
  tft.setTextSize( 1 );
  tft.clear();
  progressBar( SDU_DISPLAY_OBJ_PTR, 110, 112, 100, 20, 20);
  appsCount = 0;
  listDir(M5_FS, "/", 0, false); // count valid files first so a progress meter can be displayed
  appsCountProgress = appsCount;
  appsCount = 0;
  tft.drawJpg(sdUpdaterIcon32x40_jpg, sdUpdaterIcon32x40_jpg_len, (tft.width()-32)/2, 40);
  tft.setTextDatum(MC_DATUM);
  tft.setFont( &Font2 );
  tft.drawString("Scanning SD Card", 160, 95 );
  listDir(M5_FS, "/", 0, true); // now retrieve files meta
  tft.setTextDatum(TL_DATUM);
  aSortFiles(); // bubble sort alphabetically

  if( factory_partition ) {
    // TODO: insert partitions from NVS
  }

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
    #if defined _CHIMERA_CORE_ && defined USE_SCREENSHOTS
      case HID_SCREENSHOT:
        M5.ScreenShot->snap( "screenshot" );
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
      #if defined USE_DOWNLOADER
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
      #else
        pageDown();
      #endif
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
  //M5.update();
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
    gotoSleep();
  }
}

//#pragma GCC pop_options
