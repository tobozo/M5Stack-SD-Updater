#pragma once
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
 * In your sketch, find the line where the core library is included:
 *
 *  // #include <M5Stack.h>
 *  // #include <M5Core2.h>
 *  // #include <ESP32-Chimera-Core.h>
 *  // #include <M5StickC.h>
 *  // #include <M5Unified.h>
 *  // #include <LovyanGFX.h>
 *
 * And add this:
 *
 *  #include <M5StackUpdater.h>
 *
 *
 * In your setup() function, find the following statements:
 *
 *   M5.begin();
 *
 * And add this:
 *
 *   checkSDUpdater( SD );
 *
 * Then do whatever you need to do (button init, timers)
 * in the setup and the loop. Your app will be ready
 * to run normally except at boot if the Button A is
 * pressed, it will load the "menu.bin" from the sd card.
 *
 * Touch UI has no buttons, this raises the problem of
 * detecting a 'pushed' state when the touch is off.
 * As a compensation, an UI will be visible for 2 seconds
 * after every ESP.restart(), and this visibility can
 * be forced in the setup :
 *
 *   checkSDUpdater( SD, MENU_BIN, 2000 );
 *
 * Headless setups can overload SDUCfg.onWaitForAction
 * See SDUCfg.setWaitForActionCb() in M5StackUpdaterConfig.h
 * to assign a your own button/sensor/whatever detection routine
 * or even issue the "update" command via serial
 *
 *   if(digitalRead(BUTTON_A_PIN) == 0) {
 *     Serial.println("Will Load menu binary");
 *     updateFromFS(SD);
 *     ESP.restart();
 *   }
 *
 *
 */
#include "gitTagVersion.h"
#include <FS.h>
#include <Update.h>
// required to store the MENU_BIN hash
#include <Preferences.h>

// inherit filesystem includes from sketch
#if __has_include(<SD.h>) || defined _SD_H_
  #define SDU_HAS_SD
  #include <SD.h>
#endif
#if __has_include(<SD_MMC.h>) || defined _SD_MMC_H_
  #define SDU_HAS_SD_MMC
  #include <SD_MMC.h>
#endif
#if __has_include(<SPIFFS.h>) || defined _SPIFFS_H_
  #define SDU_HAS_SPIFFS
  #include <SPIFFS.h>
#endif
#if __has_include(<LittleFS.h>) || defined _LiffleFS_H_
  #define SDU_HAS_LITTLEFS
  #include <LittleFS.h>
#endif
#if __has_include(<PSRamFS.h>) || defined _PSRAMFS_H_
  #include <PSRamFS.h>
#endif
#if __has_include(<LITTLEFS.h>) || defined _LIFFLEFS_H_
  // LittleFS is now part of esp32 package, the older, external version isn't supported
  #warning "A version of LittleFS is unsupported, use #include <LittleFS.h> instead, if using platformio add LittleFS(esp32)@^2.0.0 to lib_deps"
#endif

#define SDU_HAS_FS (defined SDU_HAS_SD || defined SDU_HAS_SD_MMC || defined SDU_HAS_SPIFFS || defined SDU_HAS_LITTLEFS )
#if ! HAS_FS
  //#pragma message "SDUpdater didn't detect any  preselected filesystem, will use SD as default"
  //#define SDU_HAS_SD
  //#include <SD.h>
  //TODO: implement FILE/fopen()
#endif

// required by getSketchMeta(), compareFsPartition() and copyFsPartition() methods
#include <esp_partition.h>
extern "C" {
  #include "esp_ota_ops.h"
  #include "esp_image_format.h"
}
// required to guess the reset reason
#if defined ESP_IDF_VERSION_MAJOR && ESP_IDF_VERSION_MAJOR >= 4
  #if defined CONFIG_IDF_TARGET_ESP32
    #include <esp32/rom/rtc.h>
  #elif defined CONFIG_IDF_TARGET_ESP32S2
    #include <esp32s2/rom/rtc.h>
  #elif defined CONFIG_IDF_TARGET_ESP32C3
    #include <esp32c3/rom/rtc.h>
  #elif defined CONFIG_IDF_TARGET_ESP32S3
    #include <rom/rtc.h>
  #else
    #warning "Target CONFIG_IDF_TARGET is unknown"
    #include <rom/rtc.h>
  #endif
#else
  #include <rom/rtc.h>
#endif

#define resetReason (int)rtc_get_reset_reason(0)

#include "./UI/UI.hpp" // display default methods


namespace SDUpdaterNS
{

  // provide an imperative function to avoid breaking button-based (older) versions of the M5Stack SD Updater
  void updateFromFS( fs::FS &fs, const String& fileName = MENU_BIN, const int TfCardCsPin = TFCARD_CS_PIN );
  // copy compiled sketch from flash partition to filesystem binary file
  bool saveSketchToFS(fs::FS &fs, const char* binfilename = PROGMEM {MENU_BIN}, const int TfCardCsPin = TFCARD_CS_PIN );
  // provide a rollback function for custom usages
  void updateRollBack( String message );
  // provide a conditional function to cover more devices, including headless and touch
  void checkSDUpdater( fs::FS &fs, String fileName = MENU_BIN, unsigned long waitdelay = 0, const int TfCardCsPin_ = TFCARD_CS_PIN );


  class SDUpdater
  {
    public:
      // constructor with config
      SDUpdater( config_sdu_t* _cfg );
      // legacy constructor
      SDUpdater( const int TFCardCsPin_ = TFCARD_CS_PIN );
      // check methods
      void checkSDUpdaterHeadless( String fileName, unsigned long waitdelay );
      void checkSDUpdaterHeadless( fs::FS &fs, String fileName, unsigned long waitdelay );
      void checkSDUpdaterUI( String fileName, unsigned long waitdelay );
      void checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay );
      // update methods
      void updateFromFS( const String& fileName );
      void updateFromFS( fs::FS &fs, const String& fileName = MENU_BIN );
      void updateFromStream( Stream &stream, size_t updateSize, const String& fileName );
      void doRollBack( const String& message = "" );
      // flash to SD binary replication
      bool compareFsPartition(const esp_partition_t* src1, fs::File* src2, size_t length);
      bool copyFsPartition(File* dst, const esp_partition_t* src, size_t length);
      bool saveSketchToFS(fs::FS &fs, const char* binfilename = PROGMEM {MENU_BIN}, bool skipIfExists = false );
      // static methods
      static void updateNVS();
      static esp_image_metadata_t getSketchMeta( const esp_partition_t* source_partition );
      // fs::File->name() changed behaviour after esp32 sdk 2.x.x
      static const char* fs_file_path( fs::File *file ) {
        #if defined ESP_IDF_VERSION_MAJOR && ESP_IDF_VERSION_MAJOR >= 4
          return file->path();
        #else
          return file->name();
        #endif
      }

    private:
      config_sdu_t* cfg;
      const char* MenuBin = MENU_BIN;
      void performUpdate( Stream &updateSource, size_t updateSize, String fileName );
      void tryRollback( String fileName );
      void _error( const String& errMsg, unsigned long waitdelay = 2000 );
      void _error( const char **errMsgs, uint8_t msgCount=1, unsigned long waitdelay=2000 );
      void _message( const String& label );
      #if defined _M5Core2_H_ // enable additional touch button support
        const bool SDUHasTouch = true;
      #else
        const bool SDUHasTouch = false;
      #endif
      bool _fs_begun = false;
      bool _fsBegin( bool report_errors = true );
      bool _fsBegin( fs::FS &fs, bool report_errors = true );
  };


}; // end namespace


using namespace SDUpdaterNS;
