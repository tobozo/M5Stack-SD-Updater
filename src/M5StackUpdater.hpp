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
#include "./misc/config.h"
#include "./misc/types.h"
#include <FS.h>
#include <Update.h>


#define resetReason (int)rtc_get_reset_reason(0)

// use `#define SDU_NO_PRAGMAS` to disable duplicate pragma messages
#if !defined SDU_NO_PRAGMAS && CORE_DEBUG_LEVEL>=ARDUHAL_LOG_LEVEL_ERROR
  #define SDU_STRINGIFY(a) #a
  #define SDU_PRAGMA_MESSAGE(msg) \
    _Pragma( SDU_STRINGIFY( message msg ) )
#else
  #define SDU_PRAGMA_MESSAGE(msg)
#endif


// inherit filesystem includes from sketch

#if defined _SD_H_ || defined SDU_USE_SD
  #define SDU_HAS_SD
  #include "./FS/sd.hpp"
  #if !defined SDU_BEGIN_SD
    #define SDU_BEGIN_SD SDU_SDBegin
  #endif
#endif

#if defined _SDMMC_H_ || defined SDU_USE_SD_MMC
  #define SDU_HAS_SD_MMC
  #include "./FS/sd_mmc.hpp"
  #if !defined SDU_BEGIN_SD_MMC
    #define SDU_BEGIN_SD_MMC SDU_SD_MMC_Begin
  #endif
#endif

#if defined _SPIFFS_H_ || defined SDU_USE_SPIFFS
  #define SDU_HAS_SPIFFS
  #include "./FS/spiffs.hpp"
  #if !defined SDU_BEGIN_SPIFFS
    #define SDU_BEGIN_SPIFFS SDU_SPIFFS_Begin
  #endif
#endif

#if defined _FFAT_H_ || defined SDU_USE_FFAT
  #define SDU_HAS_FFAT
  #include "./FS/ffat.hpp"
  #if !defined SDU_BEGIN_FFat
    #define SDU_BEGIN_FFat SDU_FFat_Begin
  #endif
#endif

#if defined _LiffleFS_H_ || defined SDU_USE_LITTLEFS
  #define SDU_HAS_LITTLEFS
  #include "./FS/littlefs.hpp"
  #if !defined SDU_BEGIN_LittleFS
    #define SDU_BEGIN_LittleFS SDU_LittleFS_Begin
  #endif
#endif

#if defined _LIFFLEFS_H_ || __has_include(<LITTLEFS.h>)
  // LittleFS is now part of esp32 package, the older, external version isn't supported
  #warning "Older version of <LITTLEFS.h> is unsupported and will be ignored"
  #warning "Use builtin version with #include <LittleFS.h> instead, if using platformio add LittleFS(esp32)@^2.0.0 to lib_deps"
#endif

// Note: SdFat can't be detected using __has_include(<SdFat.h>) without creating problems downstream in the code.
//       Until this is solved, enabling SdFat is done by adding `#defined SDU_USE_SDFATFS` to the sketch before including the library.
#if defined SDU_USE_SDFATFS || defined USE_SDFATFS
  #define SDU_HAS_SDFS
  SDU_PRAGMA_MESSAGE("SDUpdater will use SdFat")
  #include "./FS/sdfat.hpp"
  #if !defined SDU_BEGIN_SDFat
    #define SDU_BEGIN_SDFat SDU_SDFat_Begin
  #endif
#endif


#if !defined SDU_HAS_SD && !defined SDU_HAS_SD_MMC && !defined SDU_HAS_SPIFFS && !defined SDU_HAS_LITTLEFS && !defined SDU_HAS_SDFS && !defined SDU_HAS_FFAT
  SDU_PRAGMA_MESSAGE("SDUpdater didn't detect any  preselected filesystem, will use SD as default")
  #define SDU_HAS_SD
  #include "./FS/sd.hpp"
  #if !defined SDU_BEGIN_SD
    #define SDU_BEGIN_SD SDU_SDBegin
  #endif
#endif

#if defined SDU_ENABLE_GZ || defined _ESP_TGZ_H || __has_include(<ESP32-targz.h>)
  #define SDU_HAS_TARGZ
  SDU_PRAGMA_MESSAGE("gzip and tar support detected!")
  #include <ESP32-targz.h>
#endif


#if !defined SDU_NO_AUTODETECT // lobby/buttons

  #if !defined SDU_HEADLESS
    #define SDU_USE_DISPLAY // any detected display is default enabled unless SDU_HEADLESS mode is selected
  #endif

  #define HAS_M5_API // This is M5Stack Updater, assume it has the M5.xxx() API, will be undef'd otherwise

  #if defined _CHIMERA_CORE_
    SDU_PRAGMA_MESSAGE("Chimera Core detected")
    #define SDU_Sprite LGFX_Sprite
    #define SDU_DISPLAY_TYPE M5Display*
    #define SDU_DISPLAY_OBJ_PTR &M5.Lcd
    #define SDU_TouchButton LGFX_Button
    #define HAS_LGFX
    // ESP32-Chimera-Core creates the HAS_TOUCH macro when the selected display supports it
    #if !defined SDU_HAS_TOUCH && defined HAS_TOUCH
      #define SDU_HAS_TOUCH
    #endif
  #elif defined _M5STACK_H_
    SDU_PRAGMA_MESSAGE("M5Stack.h detected")
    #define SDU_Sprite TFT_eSprite
    #define SDU_DISPLAY_TYPE M5Display*
    #define SDU_DISPLAY_OBJ_PTR &M5.Lcd
  #elif defined _M5Core2_H_
    SDU_PRAGMA_MESSAGE("M5Core2.h detected")
    #define SDU_Sprite TFT_eSprite
    #define SDU_DISPLAY_TYPE M5Display*
    #define SDU_DISPLAY_OBJ_PTR &M5.Lcd
    #define SDU_TouchButton TFT_eSPI_Button
    #define SDU_HAS_TOUCH // M5Core2 has implicitely enabled touch interface
  #elif defined _M5CORES3_H_
    SDU_PRAGMA_MESSAGE("M5Core3.h detected")
    #define SDU_Sprite TFT_eSprite
    #define SDU_DISPLAY_TYPE M5Display*
    #define SDU_DISPLAY_OBJ_PTR &M5.Lcd
    #define SDU_TouchButton TFT_eSPI_Button
    #define SDU_HAS_TOUCH // M5Core3 has implicitely enabled touch interface
  #elif defined _M5STICKC_H_
    SDU_PRAGMA_MESSAGE("M5StickC.h detected")
    #define SDU_Sprite TFT_eSprite
    #define SDU_DISPLAY_TYPE M5Display*
    #define SDU_DISPLAY_OBJ_PTR &M5.Lcd
  #elif defined __M5UNIFIED_HPP__
    SDU_PRAGMA_MESSAGE("M5Unified.h detected")
    #define SDU_Sprite LGFX_Sprite
    #define SDU_DISPLAY_TYPE M5GFX*
    #define SDU_DISPLAY_OBJ_PTR &M5.Display
    #define SDU_TouchButton LGFX_Button
    #define HAS_LGFX
    #if !defined SDU_HAS_TOUCH && (defined ARDUINO_M5STACK_Core2 || defined ARDUINO_M5STACK_CORE2 || defined ARDUINO_M5STACK_CORES3 )
      #define SDU_HAS_TOUCH
    #endif
  #else
    #if defined SDU_USE_DISPLAY
      SDU_PRAGMA_MESSAGE(message "No display driver detected")
      #undef SDU_USE_DISPLAY
    #endif
    #define SDU_DISPLAY_OBJ_PTR nullptr
    //#define SDU_DISPLAY_TYPE void*
    //#define SDU_Sprite void*
    #undef HAS_M5_API
  #endif
#endif


#if defined HAS_M5_API // SDUpdater can use M5.update(), and M5.Btnx API
  #define DEFAULT_BTN_POLLER M5.update()
  #define DEFAULT_BTNA_CHECKER M5.BtnA.isPressed()
  #define DEFAULT_BTNB_CHECKER M5.BtnB.isPressed()
  #if defined _M5STICKC_H_ // M5StickC has no BtnC
    #define DEFAULT_BTNC_CHECKER false
  #else
    #define DEFAULT_BTNC_CHECKER M5.BtnC.isPressed()
  #endif
#else
  // SDUpdater will use Serial as trigger source
  #if !defined SDU_NO_AUTODETECT && !defined SDU_HEADLESS
    SDU_PRAGMA_MESSAGE(message "No M5 API found")
  #endif
  #define DEFAULT_BTN_POLLER nullptr
  #define DEFAULT_BTNA_CHECKER false
  #define DEFAULT_BTNB_CHECKER false
  #define DEFAULT_BTNC_CHECKER false
#endif

// dispatch predefined triggers
#if !defined SDU_TRIGGER_SOURCE_DEFAULT
  #if defined SDU_HAS_TOUCH
    SDU_PRAGMA_MESSAGE("Trigger source: Touch Button")
    #define SDU_TRIGGER_SOURCE_DEFAULT TriggerSource::SDU_TRIGGER_TOUCHBUTTON
  #elif defined HAS_M5_API
    SDU_PRAGMA_MESSAGE("Trigger source: Push Button")
    #define SDU_TRIGGER_SOURCE_DEFAULT TriggerSource::SDU_TRIGGER_PUSHBUTTON
  #else
    SDU_PRAGMA_MESSAGE("Trigger source: Serial")
    #define SDU_TRIGGER_SOURCE_DEFAULT TriggerSource::SDU_TRIGGER_SERIAL
  #endif
#endif

// now that all the contextual flags are created, load the SDUpdater stack
#include "./ConfigManager/ConfigManager.hpp"
//#include "./NVS/NVSUtils.hpp"
#include "./SDUpdater/Update_Interface.hpp"
#include "./SDUpdater/SDUpdater_Class.hpp"
#include "./UI/common.hpp"

#if defined SDU_USE_DISPLAY // load the lobby and button decorations if applicable
  SDU_PRAGMA_MESSAGE("Attached UI")
  #define SDU_GFX ((SDU_DISPLAY_TYPE)(SDUCfg.display)) // type-casted display macro for UI.hpp
  #include "./UI/UI.hpp"
  #if defined SDU_HAS_TOUCH // load touch helpers if applicable
    SDU_PRAGMA_MESSAGE("Attached Touch support")
    #include "./UI/Touch.hpp"
  #endif
#else // bind null display to SDUCfg.display
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
      SDUCfg.triggers = new triggerMap_t( SDU_TRIGGER_TOUCHBUTTON, labelMenu, labelSkip, labelRollback, triggerInitTouch, triggerActionTouch, triggerFinalizeTouch );
    }

    inline void config_sdu_t::useBuiltinPushButton()
    {
      using namespace TriggerSource;
      using namespace SDU_UI;
      SDUCfg.triggers = new triggerMap_t( SDU_TRIGGER_PUSHBUTTON, labelMenu, labelSkip, labelRollback, triggerInitButton, triggerActionButton, triggerFinalizeButton );
    }

    inline void config_sdu_t::useBuiltinSerial()
    {
      using namespace TriggerSource;
      using namespace SDU_UI;
      SDUCfg.triggers = new TriggerSource::triggerMap_t( SDU_TRIGGER_SERIAL, labelMenu, labelSkip, labelRollback, triggerInitSerial, triggerActionSerial, triggerFinalizeSerial );
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
          if( !onProgress   )  { setProgressCb(   SDMenuProgressUI );  log_v("Attached onProgress");   }
          if( !onMessage    )  { setMessageCb(    DisplayUpdateUI );   log_v("Attached onMessage");    }
          if( !onError      )  { setErrorCb(      DisplayErrorUI );    log_v("Attached onError");      }
          if( !onBefore     )  { setBeforeCb(     freezeTextStyle );   log_v("Attached onBefore");     }
          if( !onAfter      )  { setAfterCb(      thawTextStyle );     log_v("Attached onAfter");      }
          if( !onSplashPage )  { setSplashPageCb( drawSDUSplashPage ); log_v("Attached onSplashPage"); }
          if( !onButtonDraw )  { setButtonDrawCb( drawSDUPushButton ); log_v("Attached onButtonDraw"); }
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

        if( !onProgress ) { setProgressCb( SDMenuProgressHeadless ); log_v("Attached onProgress"); }
        if( !onMessage  ) { setMessageCb( DisplayUpdateHeadless );   log_v("Attached onMessage"); }
        triggerSource = SDU_TRIGGER_SERIAL; // no display detected, fallback to serial

      }

      if( !onWaitForAction) { setWaitForActionCb( actionTriggered ); log_v("Attached onWaitForAction(any)"); }

      if( !triggers ) {
        switch( triggerSource ) {
          case SDU_TRIGGER_PUSHBUTTON:  useBuiltinPushButton();  log_d("Attaching trigger source: Push Button");break;
          case SDU_TRIGGER_TOUCHBUTTON: useBuiltinTouchButton(); log_d("Attaching trigger source: Touch Button");break;
          default:
          case SDU_TRIGGER_SERIAL:      useBuiltinSerial();      log_d("Attaching trigger source: Serial"); break;
        }
      }
    }



    // logic block generator for hasFS() filesystem detection/init
    #define SDU_MOUNT_ANY_FS_IF( cond, begin_cb, name )      \
      if( cond ) {                                          \
        if( !begin_cb ){                                    \
          msg[0] = name " MOUNT FAILED";                    \
          if( report_errors ) sdu->_error( msg, 2 );        \
          return false;                                     \
        } else { log_d("%s Successfully mounted", name); }  \
        mounted = true;                                     \
      }                                                     \

    // function call generator to SDU_MOUNT_ANY_FS_IF( condition, begin-callback, filesystem-name )
    #define SDU_MOUNT_FS_IF( fsobj ) SDU_MOUNT_ANY_FS_IF( &fs == &fsobj, SDU_BEGIN_##fsobj (SDU_CONFIG_##fsobj ), #fsobj );

    inline bool hasFS( SDUpdater* sdu, fs::FS &fs, bool report_errors=true )
    {
      assert(sdu);
      bool mounted = sdu->cfg->mounted; // inherit config mount state as default (can be triggered by rollback)
      const char* msg[] = {nullptr, "ABORTING"};
      #if defined SDU_HAS_SPIFFS // _SPIFFS_H_
        SDU_MOUNT_FS_IF( SPIFFS );
      #endif
      #if defined SDU_HAS_LITTLEFS // _LITTLEFS_H_
        SDU_MOUNT_FS_IF( LittleFS );
      #endif
      #if defined SDU_HAS_FFAT //_FFAT_H_
        SDU_MOUNT_FS_IF( FFat );
      #endif
      #if defined SDU_HAS_SD // _SD_H_
        SDU_SD_CONFIG_GET()->csPin = sdu->cfg->TFCardCsPin;
        SDU_MOUNT_FS_IF( SD );
      #endif
      #if defined SDU_HAS_SD_MMC // _SDMMC_H_
        //SDU_SD_MMC_CONFIG_GET()->busCfg.freq = 40000000;
        SDU_MOUNT_FS_IF( SD_MMC );
      #endif
      #if defined SDU_HAS_SDFS
        SDU_MOUNT_ANY_FS_IF( &fs==ConfigManager::SDU_SdFatFsPtr, SDU_BEGIN_SDFat(ConfigManager::SDU_SdSpiConfigPtr), "SDFat" );
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
  inline void checkSDUpdater( fs::FS *fsPtr, String fileName, unsigned long waitdelay, const int TfCardCsPin_ )
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
    SDUCfg.setFS( fsPtr );
    SDUCfg.setWaitDelay( waitdelay );

    // if( !fsPtr ) SDUCfg.Buttons[2].enabled = false; // disable "Save SD/Rollback" button

    SDUpdater sdUpdater( &SDUCfg );

    if( SDUCfg.display != nullptr ) {
      sdUpdater.checkUpdaterUI( fileName );
    } else {
      if( SDUCfg.waitdelay <=100 ) SDUCfg.waitdelay = 2000;
      sdUpdater.checkUpdaterHeadless( fileName );
    }
  }



  inline void checkFWUpdater( unsigned long waitdelay=5000 )
  {
    return checkSDUpdater( SDUCfg.fs, "", waitdelay, SDUCfg.TFCardCsPin );
  }



  // provide a conditional function to cover more devices, including headless and touch
  inline void checkSDUpdater( fs::FS &fs, String fileName, unsigned long waitdelay, const int TfCardCsPin_ )
  {
    return checkSDUpdater( &fs, fileName, waitdelay, TfCardCsPin_ );
  }


  // inline void checkSDUpdater( ConfigManager::FS_Config_t &cfg, String fileName, unsigned long waitdelay )
  // {
  //   SDUCfg.fsConfig = &cfg;
  //   SDUpdater sdUpdater( &SDUCfg );
  //
  //   if( SDUCfg.display != nullptr ) {
  //     sdUpdater.checkSDUpdaterUI( fileName, waitdelay );
  //   } else {
  //     if( waitdelay <=100 ) waitdelay = 2000;
  //     sdUpdater.checkSDUpdaterHeadless( fileName, waitdelay );
  //   }
  // }



  #if defined SDU_HAS_SDFS

    inline void checkSDUpdater( SdFs &sd, String fileName=MENU_BIN, unsigned long waitdelay=0, SdSpiConfig *SdFatCfg=nullptr )
    {
      if( !SdFatCfg ) {
        // load default config
        auto cfg = SdSpiConfig( SDUCfg.TFCardCsPin, SHARED_SPI, SD_SCK_MHZ(25) );
        SdFatCfg = (SdSpiConfig*)malloc( sizeof(SdSpiConfig) + 1 );
        void *DstPtr = SdFatCfg;
        void *SrcPtr = &cfg;
        memcpy( DstPtr, SrcPtr, sizeof(SdSpiConfig) );
      }
      ConfigManager::SDU_SdSpiConfigPtr = SdFatCfg;
      ConfigManager::SDU_SdFatPtr = &sd;
      ConfigManager::SDU_SdFatFsPtr = getSdFsFs(sd);
      checkSDUpdater( ConfigManager::SDU_SdFatFsPtr, fileName, waitdelay, SdFatCfg->csPin );
    }

  #endif



};


using namespace SDUpdaterNS;
