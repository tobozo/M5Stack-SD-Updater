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

// inherit includes from sketch
#if __has_include(<SD.h>) || defined _SD_H_
  #include <SD.h>
#endif
#if __has_include(<SD_MMC.h>) || defined _SD_MMC_H_
  #include <SD_MMC.h>
#endif
#if __has_include(<SPIFFS.h>) || defined _SPIFFS_H_
  #include <SPIFFS.h>
#endif
#if __has_include(<LittleFS.h>) || defined _LiffleFS_H_
  #include <LittleFS.h>
#endif
#if __has_include(<LITTLEFS.h>) || defined _LIFFLEFS_H_
  #include <LITTLEFS.h>
#endif
#if __has_include(<PSRamFS.h>) || defined _PSRAMFS_H_
  #include <PSRamFS.h>
#endif

#include "M5StackUpdaterConfig.h"
#include "M5StackUpdaterUI.h"




class SDUpdater
{
  public:
    SDUpdater( config_sdu_t* _cfg ) : cfg(_cfg) {
      if( SDUCfgLoader ) SDUCfgLoader();
      else SetupSDMenuConfig();
      _fs_begun = _fsBegin( false );
      //if( cfg->fs == nullptr ) log_w("No filesystem selected in constructor!");
    };
    // legacy constructor
    SDUpdater( const int TFCardCsPin_ = TFCARD_CS_PIN ) {
      //log_d("SDUpdater base mode on CS pin(%d)", TFCardCsPin_ );
      //SDUCfg.setSDU( this ); // attach this to SDUCfg.sdUpdater
      SDUCfg.setCSPin( TFCardCsPin_ );
      cfg = &SDUCfg;
      if( SDUCfgLoader ) SDUCfgLoader();
      else SetupSDMenuConfig();
      _fs_begun = _fsBegin( false );
      //if( cfg->fs == nullptr ) log_w("No filesystem selected in constructor!");
    };
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
    bool _fsBegin( bool report_errors = true )
    {
      if( cfg->fs != nullptr ) return _fsBegin( *cfg->fs, report_errors );
      _error( "No filesystem selected" );
      return false;
    }
    bool _fsBegin( fs::FS &fs, bool report_errors = true )
    {
      if( _fs_begun ) return true;
      bool mounted = false;
      const char* msg[] = {nullptr, "ABORTING"};
      #if defined _SPIFFS_H_
        log_d("Checking for SPIFFS Support");
        if( &fs == &SPIFFS ) {
          if( !SPIFFS.begin() ){
            msg[0] = "SPIFFS MOUNT FAILED";
            if( report_errors ) _error( msg, 2 );
            return false;
          } else { log_d("SPIFFS Successfully mounted"); }
          mounted = true;
        }
      #endif
      #if defined (_LITTLEFS_H_)
        log_d("Checking for LittleFS Support");
        if( &fs == &LittleFS ) {
          if( !LittleFS.begin() ){
            msg[0] = "LittleFS MOUNT FAILED";
            if( report_errors ) _error( msg, 2 );
            return false;
          } else { log_d("LittleFS Successfully mounted"); }
          mounted = true;
        }
      #endif
      #if defined (_SD_H_)
        log_d(" Checking for SD Support (pin #%d)",  cfg->TFCardCsPin );
        if( &fs == &SD ) {
          if( !SD.begin( cfg->TFCardCsPin ) ) {
            msg[0] = String("SD MOUNT FAILED (pin #" + String(cfg->TFCardCsPin) + ")").c_str();
            if( report_errors ) _error( msg, 2 );
            return false;
          } else { log_d( "SD Successfully mounted (pin #%d)", cfg->TFCardCsPin ); }
          mounted = true;
        }
      #endif
      #if defined (_SDMMC_H_)
        log_d(" Checking for SD_MMC Support");
        if( &fs == &SD_MMC ) {
          if( !SD_MMC.begin() ){
            msg[0] = "SD_MMC FAILED";
            if( report_errors ) _error( msg, 2 );
            return false;
          } else { log_d( "SD_MMC Successfully mounted"); }
          mounted = true;
        }
      #endif
      #if __has_include(<PSRamFS.h>) || defined _PSRAMFS_H_
        log_d(" Checking for PSRamFS Support");
        if( &fs == &PSRamFS ) {
          if( !PSRamFS.begin() ){
            msg[0] = "PSRamFS FAILED";
            if( report_errors ) _error( msg, 2 );
            return false;
          } else { log_d( "PSRamFS Successfully mounted"); }
          mounted = true;
        }
      #endif
      return mounted;
    }
};





// provide an imperative function to avoid breaking button-based (older) versions of the M5Stack SD Updater
__attribute__((unused)) static void updateFromFS( fs::FS &fs, const String& fileName = MENU_BIN, const int TfCardCsPin = TFCARD_CS_PIN )
{
  SDUCfg.setFS( &fs );
  SDUpdater sdUpdater( TfCardCsPin );
  sdUpdater.updateFromFS( fs, fileName );
}


// copy compiled sketch from flash partition to filesystem binary file
__attribute__((unused)) static bool saveSketchToFS(fs::FS &fs, const char* binfilename = PROGMEM {MENU_BIN}, const int TfCardCsPin = TFCARD_CS_PIN )
{
  SDUCfg.setFS( &fs );
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
__attribute__((unused)) static void checkSDUpdater( fs::FS &fs, String fileName = MENU_BIN, unsigned long waitdelay = 0, const int TfCardCsPin_ = TFCARD_CS_PIN )
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
      // case 21:  log_d("USB_UART_CHIP_RESET"); waitdelay=2000; break;// Various reset reasons for ESP32-S3
      // case 22:  log_d("USB_JTAG_CHIP_RESET"); waitdelay=2000; break;// Various reset reasons for ESP32-S3
      // case 24:  log_d("JTAG_RESET"); waitdelay=2000; break;         // Various reset reasons for ESP32-S3

      default : log_d("NO_MEAN"); waitdelay=100;
    }
  }

  log_n("Booting with reset reason: %d", resetReason );

  SDUCfg.setCSPin( TfCardCsPin_ );
  SDUCfg.setFS( &fs );
  SDUpdater sdUpdater( &SDUCfg );

  #if defined USE_DISPLAY
    sdUpdater.checkSDUpdaterUI( fileName, waitdelay );
  #else
    if( waitdelay <=100 ) waitdelay = 2000;
    sdUpdater.checkSDUpdaterHeadless( fileName, waitdelay );
  #endif
}
