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
#include "gitTagVersion.h"
#include <esp_partition.h>
extern "C" {
  #include "esp_ota_ops.h"
  #include "esp_image_format.h"
}

// #define SD_ENABLE_SPIFFS_COPY // enable SD <=> SPIFFS copy functions, from outside this library

// board selection helpers:
//   #if defined( ARDUINO_ESP32_DEV )
//   #if defined( ARDUINO_ODROID_ESP32 )
//   #if defined( ARDUINO_M5Stack_Core_ESP32 )
//   #if defined( ARDUINO_M5STACK_FIRE )
//   #if defined( ARDUINO_M5STACK_Core2 )
//   #if defined( ARDUINO_M5Stick_C )

#define SDUPDATER_SD_FS 0
#define SDUPDATER_SD_MMC_FS 1
#define SDUPDATER_SPIFFS_FS 2

#ifndef MENU_BIN
  #define MENU_BIN "/menu.bin"
#endif
#ifndef DATA_DIR
  #define DATA_DIR "/data"
#endif

#define USE_DISPLAY

#if defined( ARDUINO_ODROID_ESP32 ) // Odroid-GO
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS
#elif defined( ARDUINO_M5Stack_Core_ESP32 ) // M5Stack Classic
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS
#elif defined( ARDUINO_M5STACK_FIRE ) // M5Stack Fire
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS
#elif defined( ARDUINO_M5STACK_Core2 ) // M5Stack Core2
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS
#elif defined( ARDUINO_M5Stick_C ) // M5StickC
  #define SD_UPDATER_FS_TYPE SDUPDATER_SPIFFS_FS
#elif defined( ARDUINO_ESP32_DEV ) // ESP32 Wrover Kit
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_MMC_FS
#else
  #warning "No valid display detected, enabling headless mode"
  #undef USE_DISPLAY // headless setup, progress will be rendered in Serial
  #undef SD_ENABLE_SPIFFS_COPY // disable SD/SD_MMC <=> SPIFFS copy functions
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS // default behaviour = use SD
/*
  // or your custom board setup
  #include <FS.h>
  #include <TFT_eSPI.h>
  extern TFT_eSPI tft; // make sure 'tft' exists outside this library
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_MMC_FS // bind to SD_MMC

*/
#endif

#ifdef USE_DISPLAY
 #if !defined( _M5STACK_H_ ) && !defined( _M5STICKC_H_ ) && !defined( _M5Core2_H_ )
  #undef USE_DISPLAY
  #if defined( ARDUINO_M5Stick_C )
    #include <M5StickC.h> // load {M5StickC}-core
  #elif defined( ARDUINO_M5STACK_Core2 ) // M5Stack Core2
    #include <M5Core2.h> // load {M5Core2}-core
  #else
    #include <M5Stack.h> // load {M5Stack,ESP32-Chimera}-core
  #endif
 #endif
#endif


#if SD_UPDATER_FS_TYPE==SDUPDATER_SD_FS
  #define SDUPDATER_FS SD
  #include <SD.h>
#elif SD_UPDATER_FS_TYPE==SDUPDATER_SD_MMC_FS
  #define SDUPDATER_FS SD_MMC
  #include <SD_MMC.h>
#elif SD_UPDATER_FS_TYPE==SDUPDATER_SPIFFS_FS
  #define SDUPDATER_FS SPIFFS
  #undef SD_ENABLE_SPIFFS_COPY // disable SD/SD_MMC <=> SPIFFS copy functions
  #include <SPIFFS.h>
#else
  #error "Invalid FS type selected, must be one of: SDUPDATER_SD_FS, SDUPDATER_SD_MMC_FS, SDUPDATER_SPIFFS_FS"
#endif


#include <Update.h>
#include <Preferences.h>

// backwards compat
#define M5SDMenuProgress SDMenuProgress

class SDUpdater {
  public:
    void updateFromFS( fs::FS &fs = SDUPDATER_FS, const String& fileName = MENU_BIN );
    static void updateNVS();
    static esp_image_metadata_t getSketchMeta( const esp_partition_t* source_partition );
    static const int BACKUP_SD_TO_SPIFFS = 1;
    static const int BACKUP_SPIFFS_TO_SD = 2;
    SDUpdater( const String SPIFFS2SDFolder="" );
    String SKETCH_NAME = "";
    bool enableSPIFFS = false;
    bool SPIFFS_MOUNTED = false;
    #if defined( SD_ENABLE_SPIFFS_COPY )
      void copyFile( String sourceName, int dir );
      void copyFile( String sourceName, fs::FS &sourceFS, int dir );
      void copyFile( fs::File &sourceFile, int dir );
      void copyFile( String sourceName, fs::FS &sourceFS, String destName, fs::FS &destinationFS );
      void copyFile( fs::File &sourceFile, String destName, fs::FS &destinationFS );
      void copyDir( int direction );
      void copyDir( const char * dirname, uint8_t levels, int direction );
      void copyDir(fs::FS &sourceFS, const char * dirname, uint8_t levels, int direction );
      void makePathToFile( String destName, fs::FS destinationFS );
      String gnu_basename( String path );
      bool SPIFFSFormat();
      bool SPIFFSisEmpty();
    #endif

    void (*SDMenuProgress)( int state, int size ) = []( int state, int size )
    {
      static int SD_UI_Progress;
      int percent = ( state * 100 ) / size;
      if( percent == SD_UI_Progress ) {
        // don't render twice the same value
        return;
      }
      //Serial.printf("percent = %d\n", percent); // this is spammy
      SD_UI_Progress = percent;
#if defined USE_DISPLAY
      auto &tft = M5.Lcd;
      int progress_w = 102;
      int progress_h = 20;
      int progress_x = (tft.width() - progress_w) >> 1;
      int progress_y = (tft.height()- progress_h) >> 1;
      if ( percent >= 0 && percent < 101 ) {
        tft.fillRect( progress_x+1, progress_y+1, percent, 18, TFT_GREEN );
        tft.fillRect( progress_x+1+percent, progress_y+1, 100-percent, 18, TFT_BLACK );
        Serial.print( "." );
      } else {
        tft.fillRect( progress_x+1, progress_y+1, 100, 18, TFT_BLACK );
        Serial.println();
      }
      String percentStr = " " + String( percent ) + "% ";
      tft.drawCentreString( percentStr , tft.width() >> 1, progress_y+progress_h+5, 0); // trailing space is important
#else
      if ( percent >= 0 && percent < 101 ) {
        Serial.print( "." );
      } else {
        Serial.println();
      }
#endif
    };

    void (*displayUpdateUI)( const String& label ) = []( const String& label ){};

  private:
    void performUpdate( Stream &updateSource, size_t updateSize, String fileName );
    void tryRollback( String fileName );
};

/* don't break older versions of the M5Stack SD Updater */
__attribute__((unused)) static void updateFromFS( fs::FS &fs = SDUPDATER_FS, const String& fileName = MENU_BIN )
{
  SDUpdater sdUpdater;
#if defined USE_DISPLAY
  auto &tft = M5.Lcd;
  if (tft.width() < tft.height()) tft.setRotation(tft.getRotation() ^ 1);
  sdUpdater.displayUpdateUI = []( const String& label )
  {
    auto &tft = M5.Lcd;
    tft.fillScreen( TFT_BLACK );
    tft.setTextColor( TFT_WHITE, TFT_BLACK );
    tft.setTextFont( 0 );
    tft.setTextSize( 2 );
    // attemtp to center the text
    int16_t xpos = ( tft.width() / 2) - ( tft.textWidth( label ) / 2 );
    if ( xpos < 0 ) {
      // try with smaller size
      tft.setTextSize(1);
      xpos = ( tft.width() / 2 ) - ( tft.textWidth( label ) / 2 );
      if( xpos < 0 ) {
        // give up
        xpos = 0 ;
      }
    }

    int progress_w = 102;
    int progress_h = 20;
    int progress_x = (tft.width() - progress_w) >> 1;
    int progress_y = (tft.height()- progress_h) >> 1;
    tft.setCursor( xpos, progress_y - 20 );
    tft.print( label );
    tft.drawRect( progress_x, progress_y, progress_w, progress_h, TFT_WHITE );
  };

  sdUpdater.SDMenuProgress = []( int state, int size )
  {
    static int SD_UI_Progress;
    int percent = ( state * 100 ) / size;
    if( percent == SD_UI_Progress ) {
      // don't render twice the same value
      return;
    }
    //Serial.printf("percent = %d\n", percent); // this is spammy
    SD_UI_Progress = percent;
    auto &tft = M5.Lcd;
    int progress_w = 102;
    int progress_h = 20;
    int progress_x = (tft.width() - progress_w) >> 1;
    int progress_y = (tft.height()- progress_h) >> 1;
    if ( percent >= 0 && percent < 101 ) {
      tft.fillRect( progress_x+1, progress_y+1, percent, 18, TFT_GREEN );
      tft.fillRect( progress_x+1+percent, progress_y+1, 100-percent, 18, TFT_BLACK );
      Serial.print( "." );
    } else {
      tft.fillRect( progress_x+1, progress_y+1, 100, 18, TFT_BLACK );
      Serial.println();
    }
    String percentStr = " " + String( percent ) + "% ";
    tft.drawCentreString( percentStr , tft.width() >> 1, progress_y+progress_h+5, 0); // trailing space is important
  };
#endif
  sdUpdater.updateFromFS( fs, fileName );
}

#if defined USE_DISPLAY
__attribute__((unused)) static void checkSDUpdater( fs::FS &fs = SDUPDATER_FS, String fileName = MENU_BIN ) {
  auto &tft = M5.Lcd;
  tft.setCursor(0,0);
  tft.print("SDUpdater\npress BtnA");
  tft.setCursor(0,0);
  delay(500);
  M5.update();
  if (M5.BtnA.isPressed()) {
    Serial.println("Will Load menu binary");
    updateFromFS( fs, fileName );
    ESP.restart();
  }
  tft.fillScreen(TFT_BLACK);
}
#endif

#endif
