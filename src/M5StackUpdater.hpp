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
#include "./misc/assets.h"
#include "./misc/types.h"
#include <FS.h>
#include <Update.h>


#define resetReason (int)rtc_get_reset_reason(0)

// inherit filesystem includes from sketch
#if defined _SD_H_
  #define SDU_HAS_SD
  #if defined _CLK && defined _MISO && defined _MOSI
    // user provided specific pinout via build extra flags
    #if !defined SDU_SPI_MODE
      #define SDU_SPI_MODE SPI_MODE3
    #endif
    #if !defined SDU_SPI_FREQ
      #define SDU_SPI_FREQ 80000000
    #endif
    #pragma message "SD has custom SPI pins"
    #define SDU_SD_BEGIN [](int csPin)->bool{ SPI.begin(_CLK, _MISO, _MOSI, csPin); SPI.setDataMode(SDU_SPI_MODE); return SD.begin(csPin, SPI, SDU_SPI_FREQ); }
  #else
    #define SDU_SD_BEGIN(csPin) SD.begin(csPin)
  #endif
#endif
#if defined _SDMMC_H_
  #define SDU_HAS_SD_MMC
  /*#include <SD_MMC.h>*/
#endif
#if defined _SPIFFS_H_
  #define SDU_HAS_SPIFFS
  /*#include <SPIFFS.h>*/
#endif
#if defined _LiffleFS_H_ || __has_include(<LittleFS.h>)
  #define SDU_HAS_LITTLEFS
  #include <LittleFS.h>
#endif


#if defined SDU_ENABLE_GZ || defined _ESP_TGZ_H || __has_include(<ESP32-targz.h>) // _TGZ_FSFOOLS_
  #define SDU_HAS_TARGZ
  #pragma message "gzip and tar support detected!"
  #include <ESP32-targz.h>

  using namespace SDUpdaterNS::UpdateInterfaceNS;

  #define F_Update GzUpdateClass::getInstance()
  #define F_UpdateEnd() (mode_z ? F_Update.endgz() : F_Update.end())
  #define F_abort() if (mode_z) F_Update.abortgz(); else F_Update.abort()
  #define F_writeStream(updateSource,updateSize) (mode_z ? F_Update.writeGzStream(updateSource,updateSize) : F_Update.writeStream(updateSource))
  #define F_canBegin( usize ) (mode_z ? F_Update.begingz(UPDATE_SIZE_UNKNOWN) : F_Update.begin(usize))
  #define F_end() (mode_z ? F_Update.endgz() : F_Update.end() )

  namespace SDUpdaterNS
  {
    namespace ConfigManager
    {
      static UpdateInterfaceNS::UpdateManagerInterface_t Iface =
      {
        .begin=[](size_t s)->bool{ return F_canBegin(s); },
        .writeStream=[](Stream &data,size_t size)->size_t{ return F_writeStream(data, size); },
        .abort=[]() { F_abort(); },
        .end=[]()->bool{ return F_end(); },
        .isFinished=[]()->bool{ return F_Update.isFinished(); },
        .canRollBack=[]()->bool{ return F_Update.canRollBack(); },
        .rollBack=[]()->bool{ return F_Update.rollBack(); },
        .onProgress=[](UpdateClass::THandlerFunction_Progress fn){ F_Update.onProgress(fn); },
        .getError=[]()->uint8_t{ return F_Update.getError(); },
        .setBinName=[]( String& fileName, Stream* stream ) {
          if( !fileName.endsWith(".gz") ) {
            log_d("Not a gz file");
            return;
          }
          mode_z = stream->peek() == 0x1f; // magic zlib byte
          log_d("compression: %s", mode_z ? "enabled" : "disabled" );
        }
      };
    };
  };

#else
  namespace SDUpdaterNS
  {
    namespace ConfigManager
    {
      static UpdateInterfaceNS::UpdateManagerInterface_t Iface =
      {
        .begin=[](size_t s)->bool{ return Update.begin(s); },
        .writeStream=[](Stream &data,size_t size)->size_t{ return Update.writeStream(data); },
        .abort=[]() { Update.abort(); },
        .end=[]()->bool{ return Update.end(); },
        .isFinished=[]()->bool{ return Update.isFinished(); },
        .canRollBack=[]()->bool{ return Update.canRollBack(); },
        .rollBack=[]()->bool{ return Update.rollBack(); },
        .onProgress=[](UpdateClass::THandlerFunction_Progress fn){ Update.onProgress(fn); },
        .getError=[]()->uint8_t{ return Update.getError(); },
        .setBinName=[](String&fileName, Stream* stream){
          if( fileName.endsWith(".gz") ) {
            log_e("Gz file detected but gz support is disabled!");
          }
        }
      };
    };
  }
#endif


#if __has_include(<LITTLEFS.h>) || defined _LIFFLEFS_H_
  // LittleFS is now part of esp32 package, the older, external version isn't supported
  #warning "Older version of <LITTLEFS.h> is unsupported, use builtin version with #include <LittleFS.h> instead, if using platformio add LittleFS(esp32)@^2.0.0 to lib_deps"
#endif

#define SDU_HAS_FS (defined SDU_HAS_SD || defined SDU_HAS_SD_MMC || defined SDU_HAS_SPIFFS || defined SDU_HAS_LITTLEFS )

#if ! SDU_HAS_FS
  #define SDU_HAS_SD
  #if defined USE_SDFATFS
    #pragma message "SDUpdater will use SdFat"
    #undef __has_include
    #include <misc/sdfat32fs_wrapper.h>
    #define __has_include
    #define SDU_SD_BEGIN SDU_SDFatBegin
  #else
    #pragma message "SDUpdater didn't detect any  preselected filesystem, will use SD as default"
    #include <SD.h>
    #define SDU_SD_BEGIN(csPin) SD.begin(csPin)
  #endif

#endif


#if !defined SDU_NO_AUTODETECT

  #if !defined SDU_HEADLESS
    #define SDU_USE_DISPLAY // any detected display is default enabled unless SDU_HEADLESS mode is selected
  #endif

  #define HAS_M5_API // This is M5Stack Updater, assume it has the M5.xxx() API, will be undef'd otherwise

  #if defined _CHIMERA_CORE_
    #pragma message "Chimera Core detected"
    //#include <ESP32-Chimera-Core.hpp>
    #define SDU_Sprite LGFX_Sprite
    #define SDU_DISPLAY_TYPE M5Display*
    #define SDU_DISPLAY_OBJ_PTR &M5.Lcd
    #define SDU_TouchButton LGFX_Button
    #define HAS_LGFX
    // ESP32-Chimera-Core creates the HAS_TOUCH macro when the selected display supports it
    #if !defined SDU_HAS_TOUCH && /*ARDUINO_M5STACK_Core2 ||*/ defined HAS_TOUCH
      #define SDU_HAS_TOUCH
    #endif
  #elif defined _M5STACK_H_
    #pragma message "M5Stack.h detected"
    //#include <M5Stack.h>
    #define SDU_Sprite TFT_eSprite
    #define SDU_DISPLAY_TYPE M5Display*
    #define SDU_DISPLAY_OBJ_PTR &M5.Lcd
  #elif defined _M5Core2_H_
    #pragma message "M5Core2.h detected"
    //#include <M5Core2.h>
    #define SDU_Sprite TFT_eSprite
    #define SDU_DISPLAY_TYPE M5Display*
    #define SDU_DISPLAY_OBJ_PTR &M5.Lcd
    #define SDU_TouchButton TFT_eSPI_Button
    #define SDU_HAS_TOUCH // M5Core2 has implicitely enabled touch interface
  #elif defined _M5STICKC_H_
    #pragma message "M5StickC.h detected"
    //#include <M5StickC.h>
    #define SDU_Sprite TFT_eSprite
    #define SDU_DISPLAY_TYPE M5Display*
    #define SDU_DISPLAY_OBJ_PTR &M5.Lcd
  #elif defined __M5UNIFIED_HPP__
    #pragma message "M5Unified.h detected"
    //#include <M5Unified.hpp>
    #define SDU_Sprite LGFX_Sprite
    #define SDU_DISPLAY_TYPE M5GFX*
    #define SDU_DISPLAY_OBJ_PTR &M5.Display
    #define SDU_TouchButton LGFX_Button
    #define HAS_LGFX
    #if !defined SDU_HAS_TOUCH && defined ARDUINO_M5STACK_Core2
      #define SDU_HAS_TOUCH
    #endif
  #else
    #pragma message "No display driver detected"
    //#error "No display driver detected"
    #define SDU_DISPLAY_OBJ_PTR nullptr
    //#define SDU_DISPLAY_TYPE void*
    //#define SDU_Sprite void*
    #undef SDU_USE_DISPLAY
    #undef HAS_M5_API
  #endif
#endif


#if defined HAS_M5_API // SDUpdater can use M5.update(), and M5.Btnx API
  #define DEFAULT_BTN_POLLER M5.update()
  //#if defined ARDUINO_ESP32_S3_BOX // S3Box only has mute button and Touch
    //#define DEFAULT_BTNA_CHECKER S3MuteButton.changed() // use SDUpdater::DigitalPinButton_t
    //#define DEFAULT_BTNB_CHECKER false
    //#define DEFAULT_BTNC_CHECKER false
  //#else
    #define DEFAULT_BTNA_CHECKER M5.BtnA.isPressed()
    #define DEFAULT_BTNB_CHECKER M5.BtnB.isPressed()
    #if defined _M5STICKC_H_ // M5StickC has no BtnC
      #define DEFAULT_BTNC_CHECKER false
    #else
      #define DEFAULT_BTNC_CHECKER M5.BtnC.isPressed()
    #endif
  //#endif
#else
  // SDUpdater will use Serial as trigger source
  #if !defined SDU_NO_AUTODETECT
    #pragma message "No M5 API found"
  #endif
  #define DEFAULT_BTN_POLLER nullptr
  #define DEFAULT_BTNA_CHECKER false
  #define DEFAULT_BTNB_CHECKER false
  #define DEFAULT_BTNC_CHECKER false
#endif


#if !defined SDU_TRIGGER_SOURCE_DEFAULT
  #if defined SDU_HAS_TOUCH
    #pragma message "Trigger source: Touch Button"
    #define SDU_TRIGGER_SOURCE_DEFAULT TriggerSource::SDU_TRIGGER_TOUCHBUTTON
  #elif defined HAS_M5_API
    #pragma message "Trigger source: Push Button"
    #define SDU_TRIGGER_SOURCE_DEFAULT TriggerSource::SDU_TRIGGER_PUSHBUTTON
  #else
    #pragma message "Trigger source: Serial"
    #define SDU_TRIGGER_SOURCE_DEFAULT TriggerSource::SDU_TRIGGER_SERIAL
  #endif
#endif


#include "./ConfigManager/ConfigManager.hpp"
#include "./SDUpdater/SDUpdater_Class.hpp"
#include "./UI/common.hpp"


#if defined SDU_USE_DISPLAY
  #pragma message "Attached UI"
  #define SDU_GFX ((SDU_DISPLAY_TYPE)(SDUCfg.display)) // macro for UI.hpp
  #include "./UI/UI.hpp"
  #if defined SDU_HAS_TOUCH
    #pragma message "Attached Touch support"
    #include "./UI/Touch.hpp"
  #endif
#else
  #define SDU_GFX ((void*)(SDUCfg.display)) // macro for UI.hpp
#endif


namespace SDUpdaterNS
{
  namespace ConfigManager
  {

    inline void* config_sdu_t::getCompilationTimeDisplay()
    {
      #if defined SDU_DISPLAY_OBJ_PTR
        return (void*)SDU_DISPLAY_OBJ_PTR;
      #else
        return nullptr;
      #endif
    }


    inline void* config_sdu_t::getRunTimeDisplay()
    {
      return (void*)SDU_GFX;
    }


    inline void config_sdu_t::useBuiltinTouchButton()
    {
      using namespace TriggerSource;
      using namespace SDU_UI;
      triggers = new triggerMap_t( SDU_TRIGGER_TOUCHBUTTON, labelMenu, labelSkip, labelRollback, triggerInitTouch, triggerActionTouch, triggerFinalizeTouch );
    }

    inline void config_sdu_t::useBuiltinPushButton()
    {
      using namespace TriggerSource;
      using namespace SDU_UI;
      triggers = new triggerMap_t( SDU_TRIGGER_PUSHBUTTON, labelMenu, labelSkip, labelRollback, triggerInitButton, triggerActionButton, triggerFinalizeButton );
    }

    inline void config_sdu_t::useBuiltinSerial()
    {
      using namespace TriggerSource;
      using namespace SDU_UI;
      triggers = new TriggerSource::triggerMap_t( SDU_TRIGGER_SERIAL, labelMenu, labelSkip, labelRollback, triggerInitSerial, triggerActionSerial, triggerFinalizeSerial );
    }



    inline void config_sdu_t::setDefaults()
    {
      using namespace TriggerSource;
      using namespace SDU_UI;
      fsChecker=hasFS;

      TriggerSources_t triggerSource = SDU_TRIGGER_SOURCE_DEFAULT; // default

      if(!buttonsUpdate) setBtnPoller( FN_LAMBDA_VOID(DEFAULT_BTN_POLLER) );
      if( !Buttons[0].cb ) setBtnA( FN_LAMBDA_BOOL(DEFAULT_BTNA_CHECKER) );
      if( !Buttons[1].cb ) setBtnB( FN_LAMBDA_BOOL(DEFAULT_BTNB_CHECKER) );
      if( !Buttons[2].cb ) setBtnC( FN_LAMBDA_BOOL(DEFAULT_BTNC_CHECKER) );


      if( !labelMenu      && LAUNCHER_LABEL ) labelMenu     = LAUNCHER_LABEL;
      if( !labelSkip      && SKIP_LABEL     ) labelSkip     = SKIP_LABEL;
      if( !labelRollback  && ROLLBACK_LABEL ) labelRollback = ROLLBACK_LABEL;
      if( !labelSave      && SAVE_LABEL     ) labelSave     = SAVE_LABEL;
      if( !binFileName    && SDU_APP_PATH   ) binFileName   = SDU_APP_PATH;
      if( !appName        && SDU_APP_NAME   ) appName       = SDU_APP_NAME;
      if( !authorName     && SDU_APP_AUTHOR ) authorName    = SDU_APP_AUTHOR;

      // detect display
      if( display ) {
        log_d("Found display driver set by user");
      } else if( getCompilationTimeDisplay() ) {
        log_d("Found display driver set by macro");
        setDisplay( getCompilationTimeDisplay() );
      } else if( getRunTimeDisplay() ) {
        log_d("Found display driver set by config");
        setDisplay( getRunTimeDisplay() );
      } else {
        log_w("No display driver found :-(" );
      }

      // attach default callbacks
      if( display ) {

        #if defined SDU_USE_DISPLAY
          if( !onProgress   )  { setProgressCb(   SDMenuProgressUI );  log_d("Attached onProgress");   }
          if( !onMessage    )  { setMessageCb(    DisplayUpdateUI );   log_d("Attached onMessage");    }
          if( !onError      )  { setErrorCb(      DisplayErrorUI );    log_d("Attached onError");      }
          if( !onBefore     )  { setBeforeCb(     freezeTextStyle );   log_d("Attached onBefore");     }
          if( !onAfter      )  { setAfterCb(      thawTextStyle );     log_d("Attached onAfter");      }
          if( !onSplashPage )  { setSplashPageCb( drawSDUSplashPage ); log_d("Attached onSplashPage"); }
          if( !onButtonDraw )  { setButtonDrawCb( drawSDUPushButton ); log_d("Attached onButtonDraw"); }
        #endif

        #if defined ARDUINO_ESP32_S3_BOX
          //setSDUBtnA( ConfigManager::MuteChanged );   log_v("Attached Mute Read");
          //setSDUBtnA( ConfigManager::S3MuteButtonChanged );    log_v("Attached Mute Read");
          // setBtnB( nullptr );       log_d("Detached BtnB");
          // setBtnC( nullptr );       log_d("Detached BtnC");
          // setLabelSkip( nullptr );     log_d("Disabled Skip");
          // setLabelRollback( nullptr ); log_d("Disabled Rollback");
          // setLabelSave( nullptr );     log_d("Disabled Save");
        #endif
        #if defined _M5STICKC_H_
          setBtnC( nullptr );       log_d("Detached BtnC");
        #endif

      } else {

        if( !onProgress ) { setProgressCb( SDMenuProgressHeadless ); log_d("Attached onProgress"); }
        if( !onMessage  ) { setMessageCb( DisplayUpdateHeadless );   log_d("Attached onMessage"); }
        triggerSource = SDU_TRIGGER_SERIAL; // no display detected, fallback to serial

      }

      if( !onWaitForAction) { setWaitForActionCb( actionTriggered ); log_d("Attached onWaitForAction(any)"); }

      if( !triggers ) {
        switch( triggerSource ) {
          case SDU_TRIGGER_PUSHBUTTON:  useBuiltinPushButton();  log_d("Attaching trigger source: Push Button");break;
          case SDU_TRIGGER_TOUCHBUTTON: useBuiltinTouchButton(); log_d("Attaching trigger source: Touch Button");break;
          default:
          case SDU_TRIGGER_SERIAL:      useBuiltinSerial();      log_d("Attaching trigger source: Serial"); break;
        }
      }
    }


    inline bool hasFS( SDUpdater* sdu, fs::FS &fs, bool report_errors )
    {
      assert(sdu);
      bool mounted = sdu->cfg->mounted; // inherit config mount state as default (can be triggered by rollback)
      const char* msg[] = {nullptr, "ABORTING"};
      #if defined _SPIFFS_H_
        if( &fs == &SPIFFS ) {
          if( !SPIFFS.begin() ){
            msg[0] = "SPIFFS MOUNT FAILED";
            if( report_errors ) sdu->_error( msg, 2 );
            return false;
          } else { log_d("SPIFFS Successfully mounted"); }
          mounted = true;
        }
      #endif
      #if defined (_LITTLEFS_H_)
        if( &fs == &LittleFS ) {
          if( !LittleFS.begin() ){
            msg[0] = "LittleFS MOUNT FAILED";
            if( report_errors ) sdu->_error( msg, 2 );
            return false;
          } else { log_d("LittleFS Successfully mounted"); }
          mounted = true;
        }
      #endif
      #if defined (_SD_H_)
        if( &fs == &SD ) {
          if( ! SDU_SD_BEGIN(sdu->cfg->TFCardCsPin) ) {
            msg[0] = String("SD MOUNT FAILED (pin #" + String(sdu->cfg->TFCardCsPin) + ")").c_str();
            if( report_errors ) sdu->_error( msg, 2 );
            return false;
          } else {
            log_d("[%d] SD Successfully mounted (pin #%d)", ESP.getFreeHeap(), sdu->cfg->TFCardCsPin );
          }
          mounted = true;
        }
      #endif
      #if defined (_SDMMC_H_)
        if( &fs == &SD_MMC ) {
          if( !SD_MMC.begin() ){
            msg[0] = "SD_MMC FAILED";
            if( report_errors ) sdu->_error( msg, 2 );
            return false;
          } else { log_d( "SD_MMC Successfully mounted"); }
          mounted = true;
        }
      #endif
      #if defined USE_SDFATFS
        using namespace ConfigManager;
        if( SDU_SdFatFsPtr ) {
          if( !SDU_SD_BEGIN( SDU_SdSpiConfigPtr ) ) {
            msg[0] = String("SDFat MOUNT FAILED ").c_str();
            if( report_errors ) sdu->_error( msg, 2 );
            return false;
          }
          log_d("[%d] SDFat Successfully mounted (pin #%d)", ESP.getFreeHeap(), SDU_SdSpiConfigPtr->csPin );
          mounted = true;
        }
      #endif
      return mounted;
    }

  };


  inline void updateFromFS( const String& fileName )
  {
    if( !SDUCfg.fs ) {
      log_e("NO FILESYSTEM");
      return;
    }
    SDUpdater sdUpdater( &SDUCfg );
    sdUpdater.updateFromFS( fileName );
    ESP.restart();
  }


  // provide an imperative function to avoid breaking button-based (older) versions of the M5Stack SD Updater
  inline void updateFromFS( fs::FS &fs, const String& fileName, const int TfCardCsPin )
  {
    SDUCfg.setFS( &fs );
    SDUCfg.setCSPin( TfCardCsPin );
    SDUpdater sdUpdater( &SDUCfg );
    sdUpdater.updateFromFS( fs, fileName );
    ESP.restart();
  }


  // copy compiled sketch from flash partition to filesystem binary file
  inline bool saveSketchToFS(fs::FS &fs, const char* binfilename, const int TfCardCsPin )
  {
    SDUCfg.setFS( &fs );
    SDUCfg.setCSPin( TfCardCsPin );
    SDUpdater sdUpdater( &SDUCfg );
    return sdUpdater.saveSketchToFS( fs, binfilename );
  }


  // provide a rollback function for custom usages
  inline void updateRollBack( String message )
  {
    bool wasmounted = SDUCfg.mounted;
    SDUCfg.mounted = true;
    SDUpdater sdUpdater( &SDUCfg );
    sdUpdater.doRollBack( message );
    SDUCfg.mounted = wasmounted;
  }


  // provide a conditional function to cover more devices, including headless and touch
  inline void checkSDUpdater( fs::FS &fs, String fileName, unsigned long waitdelay, const int TfCardCsPin_ )
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

    if( SDUCfg.display != nullptr ) {
      sdUpdater.checkSDUpdaterUI( fileName, waitdelay );
    } else {
      if( waitdelay <=100 ) waitdelay = 2000;
      sdUpdater.checkSDUpdaterHeadless( fileName, waitdelay );
    }
  }


  #if defined USE_SDFATFS

    inline void checkSDUpdater( SdFs &sd, String fileName=MENU_BIN, unsigned long waitdelay=0, SdSpiConfig *SdFatCfg=nullptr )
    {
      if( !SdFatCfg ) {
        // load default config
        auto cfg = SdSpiConfig( SDUCfg.TFCardCsPin, SHARED_SPI, SD_SCK_MHZ(25) );
        SdFatCfg = (SdSpiConfig*)malloc( sizeof(SdSpiConfig) + 1 );
        memcpy( SdFatCfg, &cfg, sizeof(SdSpiConfig) );
      }
      ConfigManager::SDU_SdSpiConfigPtr = SdFatCfg;
      ConfigManager::SDU_SdFatPtr = &sd;
      ConfigManager::SDU_SdFatFsPtr = getSdFsFs(sd);
      checkSDUpdater( *ConfigManager::SDU_SdFatFsPtr, fileName, waitdelay, SdFatCfg->csPin );
    }

  #endif



};


using namespace SDUpdaterNS;
