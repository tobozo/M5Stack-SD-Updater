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
 * In your sketch, find the line where the core library is included:
 *
 *  // #include <M5Stack.h>
 *  // #include <M5Core2.h>
 *  // #include <ESP32-Chimera-Core.h>
 *  // #include <M5StickC.h>
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
 *   checkSDUpdater();
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
#include <esp_partition.h>
extern "C" {
  #include "esp_ota_ops.h"
  #include "esp_image_format.h"
}
#include <FS.h>
#include <Update.h>
#if defined ESP_IDF_VERSION_MAJOR && ESP_IDF_VERSION_MAJOR >= 4
  #if defined CONFIG_IDF_TARGET_ESP32
    #include <esp32/rom/rtc.h>
  #elif defined CONFIG_IDF_TARGET_ESP32S2
    #include <esp32s2/rom/rtc.h>
  #elif defined CONFIG_IDF_TARGET_ESP32C3
    #include <esp32c3/rom/rtc.h>
  #else
    #error Target CONFIG_IDF_TARGET is not supported
  #endif
#else
  #include <rom/rtc.h>
#endif

#include <Preferences.h>
#define resetReason (int)rtc_get_reset_reason(0)

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

#define ROLLBACK_LABEL "Rollback" // reload app from the "other" OTA partition
#define LAUNCHER_LABEL "Launcher" // load Launcher (typically menu.bin)
#define SKIP_LABEL     "Skip >>|" // resume normal operations (=no action taken)
#define BTN_HINT_MSG   "SD-Updater Options"

#define SDU_LOAD_TPL   "Will Load menu binary : %s\n"
#define SDU_ROLLBACK_MSG "Will Roll back"

#ifndef MENU_BIN
  #define MENU_BIN "/menu.bin"
#endif
#ifndef DATA_DIR
  #define DATA_DIR "/data"
#endif

#if !defined(TFCARD_CS_PIN) // override this from your sketch
 #define TFCARD_CS_PIN SS
#endif

#if !defined SDU_HEADLESS && (defined _CHIMERA_CORE_ || defined _M5STICKC_H_ || defined _M5STACK_H_ || defined _M5Core2_H_ || defined LGFX_ONLY)
  #define USE_DISPLAY // #undef this to force headless mode
#endif

// auto selection of default mode, don't edit this, use either aliased class instead
#define SDUpdater SDUpdater_Base

#if defined( ARDUINO_ODROID_ESP32 )         // Odroid-GO
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS
#elif defined( ARDUINO_M5Stack_Core_ESP32 ) // M5Stack Classic
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS
#elif defined( ARDUINO_M5STACK_FIRE )       // M5Stack Fire
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS
#elif defined( ARDUINO_M5STACK_Core2 )      // M5Core2
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS
#elif defined( ARDUINO_M5Stick_C )          // M5StickC
  // TODO: use __has_include to determine is SPIFFS, LITTLEFS, or FFAT
  #define SD_UPDATER_FS_TYPE SDUPDATER_SPIFFS_FS
#elif defined( ARDUINO_ESP32_DEV ) || defined( ARDUINO_ESP32_WROVER_KIT ) // ESP32 Wrover Kit
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_MMC_FS
#elif defined( ARDUINO_TTGO_T1 )            // TTGO T1
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_MMC_FS
#elif defined( ARDUINO_LOLIN_D32_PRO )      // LoLin D32 Pro
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS
#elif defined( ARDUINO_T_Watch )            // TWatch, all model
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS
#else
  #warning "No valid combination of Device+Display+SD detected, inheriting defaults"
  //#undef USE_DISPLAY // headless setup, progress will be rendered in Serial
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_FS // default behaviour = use SD
/*
  // or your custom board setup
  #include <FS.h>
  #include <TFT_eSPI.h>
  extern TFT_eSPI tft; // make sure 'tft' exists outside this library
  #define SD_UPDATER_FS_TYPE SDUPDATER_SD_MMC_FS // bind to SD_MMC

*/
#endif

#if defined( M5STACK_SD )
  #define SDUPDATER_FS M5STACK_SD
#elif SD_UPDATER_FS_TYPE==SDUPDATER_SD_FS
  #define SDUPDATER_FS SD
  #include <SD.h>
#elif SD_UPDATER_FS_TYPE==SDUPDATER_SD_MMC_FS
  #define SDUPDATER_FS SD_MMC
  #include <SD_MMC.h>
#elif SD_UPDATER_FS_TYPE==SDUPDATER_SPIFFS_FS
  #define SDUPDATER_FS SPIFFS
  #include <SPIFFS.h>
#else
  #error "Invalid FS type selected, must be one of: SDUPDATER_SD_FS, SDUPDATER_SD_MMC_FS, SDUPDATER_SPIFFS_FS"
#endif

// backwards compat
#define M5SDMenuProgress SDUCfg.onProgress

#include "M5StackUpdaterConfig.h"
#include "M5StackUpdaterUI.h"


class SDUpdater_Base
{
  public:
    SDUpdater_Base( config_sdu_t* _cfg ) : cfg(_cfg) {
      cfg->setSDU( this ); // attach this to SDUCfg.sdUpdater
      if( SDUCfgLoader ) SDUCfgLoader();
      else SetupSDMenuConfig();
    };
    SDUpdater_Base( const int TFCardCsPin_ = TFCARD_CS_PIN ) {
      log_d("SDUpdate base mode on CS pin(%d)", TFCardCsPin_ );
      SDUCfg.setSDU( this ); // attach this to SDUCfg.sdUpdater
      SDUCfg.setCSPin( TFCardCsPin_ );
      cfg = &SDUCfg;
      if( SDUCfgLoader ) SDUCfgLoader();
      else SetupSDMenuConfig();
    };
    void updateFromFS( fs::FS &fs = SDUPDATER_FS, const String& fileName = MENU_BIN );
    void doRollBack( const String& message = "" );
    static void updateNVS();
    static esp_image_metadata_t getSketchMeta( const esp_partition_t* source_partition );
    String SKETCH_NAME = "";

    // flash to SD binary replication
    bool compareFsPartition(const esp_partition_t* src1, fs::File* src2, size_t length);
    bool copyFsPartition(File* dst, const esp_partition_t* src, size_t length);
    bool saveSketchToFS(fs::FS &fs, const char* binfilename = PROGMEM {MENU_BIN} );

    // fs::File->name() changed behaviour after esp32 sdk 2.x.x
    const char* fs_file_path( fs::File *file ) {
      #if defined ESP_IDF_VERSION_MAJOR && ESP_IDF_VERSION_MAJOR >= 4
        return file->path();
      #else
        return file->name();
      #endif
    }

    config_sdu_t* cfg;

    void checkSDUpdaterHeadless( fs::FS &fs, String fileName, unsigned long waitdelay );
    void checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay );

  private:
    void performUpdate( Stream &updateSource, size_t updateSize, String fileName );
    void tryRollback( String fileName );
};


// provide an imperative function to avoid breaking button-based (older) versions of the M5Stack SD Updater
__attribute__((unused)) static void updateFromFS( fs::FS &fs = SDUPDATER_FS, const String& fileName = MENU_BIN, const int TfCardCsPin = TFCARD_CS_PIN )
{
  SDUpdater sdUpdater( TfCardCsPin );
  sdUpdater.updateFromFS( fs, fileName );
}


// copy compiled sketch from flash partition to filesystem binary file
__attribute__((unused)) static bool saveSketchToFS(fs::FS &fs, const char* binfilename = PROGMEM {MENU_BIN}, const int TfCardCsPin = TFCARD_CS_PIN )
{
  SDUpdater sdUpdater( TfCardCsPin );
  return sdUpdater.saveSketchToFS( fs, binfilename );
}


// provide a rollback function for custom usages
__attribute__((unused)) static void updateRollBack( String message )
{
  SDUpdater sdUpdater;
  sdUpdater.doRollBack( message );
}


// provide a conditional function to cover more devices, including headless and touch
__attribute__((unused)) static void checkSDUpdater( fs::FS &fs = SDUPDATER_FS, String fileName = MENU_BIN, unsigned long waitdelay = 0, const int TfCardCsPin_ = TFCARD_CS_PIN )
{
  if( waitdelay == 0 ) {
    // check for reset reset reason
    switch( resetReason ) {
      //case 1 : log_d("POWERON_RESET");break;                  // 1, Vbat power on reset
      //case 3 : log_d("SW_RESET");break;                       // 3, Software reset digital core
      //case 4 : log_d("OWDT_RESET");break;                     // 4, Legacy watch dog reset digital core
      //case 5 : log_d("DEEPSLEEP_RESET");break;                // 5, Deep Sleep reset digital core
      //case 6 : log_d("SDIO_RESET");break;                     // 6, Reset by SLC module, reset digital core
      //case 7 : log_d("TG0WDT_SYS_RESET");break;               // 7, Timer Group0 Watch dog reset digital core
      //case 8 : log_d("TG1WDT_SYS_RESET");break;               // 8, Timer Group1 Watch dog reset digital core
      //case 9 : log_d("RTCWDT_SYS_RESET");break;               // 9, RTC Watch dog Reset digital core
      //case 10 : log_d("INTRUSION_RESET");break;               // 10, Instrusion tested to reset CPU
      //case 11 : log_d("TGWDT_CPU_RESET");break;               // 11, Time Group reset CPU
      case 12 : log_d("SW_CPU_RESET"); waitdelay=2000; break;   // 12, Software reset CPU
      //case 13 : log_d("RTCWDT_CPU_RESET");break;              // 13, RTC Watch dog Reset CPU
      //case 14 : log_d("EXT_CPU_RESET");break;                 // 14, for APP CPU, reseted by PRO CPU
      //case 15 : log_d("RTCWDT_BROWN_OUT_RESET");break;        // 15, Reset when the vdd voltage is not stable
      case 16 : log_d("RTCWDT_RTC_RESET"); waitdelay=500; break;// 16, RTC Watch dog reset digital core and rtc module
      default : log_d("NO_MEAN"); waitdelay=100;
    }
  }

  log_n("Booting with reset reason: %d", resetReason );

  SDUCfg.setCSPin( TfCardCsPin_ );
  SDUpdater sdUpdater( &SDUCfg );

  #if defined USE_DISPLAY
    sdUpdater.checkSDUpdaterUI( fs, fileName, waitdelay );
  #else
    sdUpdater.checkSDUpdaterHeadless( fs, fileName, waitdelay );
  #endif
}


#endif
