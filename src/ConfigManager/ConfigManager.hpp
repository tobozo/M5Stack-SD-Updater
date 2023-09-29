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
 */
#pragma once

#include "./misc/config.h"
#include "./misc/types.h"
#include <FS.h>


namespace SDUpdaterNS
{

  class SDUpdater;

  namespace ConfigManager
  {

    #if defined ARDUINO_ESP32_S3_BOX
      static DigitalPinButton_t S3MuteButton(GPIO_NUM_1);
      bool S3MuteButtonChanged();
    #endif

    typedef bool (*fsCheckerCb)( SDUpdater* sdu, fs::FS &fs, bool report_errors );

    extern void setup();
    extern bool hasFS( SDUpdater *sdu, fs::FS &fs, bool report_errors );
    extern UpdateInterfaceNS::UpdateManagerInterface_t *GetUpdateInterface();

    using namespace UpdateInterfaceNS;

    // SDUpdater config callbacks and params
    struct config_sdu_t
    {
      config_sdu_t();
      fs::FS *fs = nullptr;
      //FS_Config_t *fsConfig = nullptr;
      bool mounted = false;
      bool fs_begun = false;
      void *display = nullptr; // dereferenced display object
      void* getCompilationTimeDisplay();
      void* getRunTimeDisplay();
      void useBuiltinPushButton();
      void useBuiltinTouchButton();
      void useBuiltinSerial();
      SDU_UI::Theme_t *theme = nullptr; // buttons theme
      TriggerSource::triggerMap_t *triggers = nullptr;
      int TFCardCsPin = -1;
      unsigned long waitdelay = 5000;
      //bool load_defaults = true;
      bool use_rollback = true;
      bool rollBackToFactory = false;
      const char* labelMenu     = LAUNCHER_LABEL;
      const char* labelSkip     = SKIP_LABEL;
      const char* labelRollback = ROLLBACK_LABEL;
      const char* labelSave     = SAVE_LABEL;
      const char* binFileName   = SDU_APP_PATH;
      const char* appName       = SDU_APP_NAME;
      const char* authorName    = SDU_APP_AUTHOR;

      BtnXAction Buttons[3]     =
      {
        { nullptr, SDU_BTNA_MENU, true },
        { nullptr, SDU_BTNB_SKIP, true },
        { nullptr, SDU_BTNC_SAVE, true }
      };

      BtnPollCb         buttonsUpdate    = nullptr;
      onProgressCb      onProgress       = nullptr;
      onMessageCb       onMessage        = nullptr;
      onErrorCb         onError          = nullptr;
      onBeforeCb        onBefore         = nullptr;
      onAfterCb         onAfter          = nullptr;
      onSplashPageCb    onSplashPage     = nullptr;
      onButtonDrawCb    onButtonDraw     = nullptr;
      onWaitForActionCb onWaitForAction  = nullptr;
      fsCheckerCb       fsChecker        = nullptr;

      void setDefaults();
      void setDisplay( void* ptr=nullptr );

      void buttonsPoll();

      void setCSPin( const int param );
      void setWaitDelay( unsigned long waitdelay );
      void setFS( fs::FS *param );
      void setButtonsTheme( SDU_UI::Theme_t *_theme );
      void setProgressCb( onProgressCb cb );
      void setMessageCb( onMessageCb cb );
      void setErrorCb( onErrorCb cb );
      void setBeforeCb( onBeforeCb cb );
      void setAfterCb( onAfterCb cb );
      void setSplashPageCb( onSplashPageCb cb );
      void setButtonDrawCb( onButtonDrawCb cb );
      void setWaitForActionCb( onWaitForActionCb cb );

      void setLabelMenu( const char* label );
      void setLabelSkip( const char* label );
      void setLabelRollback( const char* label );
      void setLabelSave( const char* label );
      void setAppName( const char* name );
      void setAuthorName( const char* name );
      void setBinFileName( const char* name );
      void useRolllback( bool use );

      void setBtnPoller( BtnPollCb cb );
      void setBtnA( BtnXPressCb Btn );
      void setBtnB( BtnXPressCb Btn );
      void setBtnC( BtnXPressCb Btn );
      void setBtns( SDUBtnActions BtnVal, BtnXPressCb cb );

      // some methods were renamed
      [[deprecated("use setBtnPoller()")]] void setSDUBtnPoller( BtnPollCb cb ) { setBtnPoller(cb); }
      [[deprecated("use setBtnA()")]]      void setSDUBtnA( BtnXPressCb Btn ) { setBtnA( Btn ); }
      [[deprecated("use setBtnB()")]]      void setSDUBtnB( BtnXPressCb Btn ) { setBtnB( Btn ); }
      [[deprecated("use setBtnC()")]]      void setSDUBtnC( BtnXPressCb Btn ) { setBtnC( Btn ); }

    };

    // override this from sketch
    [[maybe_unused]] static onConfigLoad SDUCfgLoader = nullptr;

    extern config_sdu_t SDUCfg;
  };

  using ConfigManager::SDUCfg;

}; // end namespace



