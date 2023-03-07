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

#include "./config.h"

namespace SDUpdaterNS
{

  struct Theme_t;

  namespace ConfigManager
  {

    // to be returned by onWaitForActionCb
    enum SDUBtnActions
    {
      SDU_BTNA_ROLLBACK =  0,
      SDU_BTNA_MENU     =  1,
      SDU_BTNB_SKIP     = -1,
      SDU_BTNC_SAVE     =  2
    };

    // callback signatures
    typedef void (*onProgressCb)( int state, int size ); // progress bar when updating/saving
    typedef void (*onMessageCb)( const String& label );  // misc info messages
    typedef void (*onErrorCb)( const String& message, unsigned long delay ); // error messages
    typedef void (*onBeforeCb)(); // called before using display
    typedef void (*onAfterCb)();  // called after using display
    typedef void (*onSplashPageCb)( const char* msg ); // lobby page
    typedef void (*onButtonDrawCb)( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor, uint16_t shadowcolor );
    typedef int  (*onWaitForActionCb)( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay ); // action trigger
    typedef void (*onConfigLoad)(); // external config loader, if set, will be called by SDUpdater constructor
    typedef void (*BtnPollCb)(); // called to poll button state e.g. like M5.update()
    typedef bool (*BtnXPressCb)(); // called when a button is pressed


    typedef void(*pollChecker)(); // M5.update()
    typedef bool(*btnAChecker)(); // M5.BtnA.wasPressed()
    typedef bool(*btnBChecker)(); // M5.BtnB.wasPressed()
    typedef bool(*btnCChecker)(); // M5.BtnC.wasPressed()

    void setup(); // overloaded in UI.hpp
    static void pollButtons(); // overloaded in UI.hpp
    static bool buttonAPressed(); // overloaded in UI.hpp
    static bool buttonBPressed(); // overloaded in UI.hpp
    static bool buttonCPressed(); // overloaded in UI.hpp
    #if defined ARDUINO_ESP32_S3_BOX
      static bool MuteChanged();
    #endif

    // button to action assignment
    struct BtnXAction
    {
      BtnXPressCb cb;
      SDUBtnActions val;
    };

    // SDUpdater config callbacks and params
    struct config_sdu_t
    {

      config_sdu_t()
      {
        setSDUBtnA( buttonAPressed ); //log_v("Attached buttonpressA");
        setSDUBtnB( buttonBPressed ); //log_v("Attached buttonpressB");
        setSDUBtnC( buttonCPressed ); //log_v("Attached buttonpressC");
        setSDUBtnChecker( pollButtons ); //log_v("Attached buttons poller");
        setDisplay();
        setButtons();
      };

      fs::FS *fs = nullptr;
      void *display = nullptr; // dereferenced display object
      Theme_t *theme = nullptr; // buttons theme
      int TFCardCsPin = -1;
      bool load_defaults = true;
      bool use_rollback = true;
      const char* labelMenu     = LAUNCHER_LABEL;
      const char* labelSkip     = SKIP_LABEL;
      const char* labelRollback = ROLLBACK_LABEL;
      const char* labelSave     = SAVE_LABEL;
      const char* binFileName   = SDU_APP_PATH;
      const char* appName       = SDU_APP_NAME;
      const char* authorName    = SDU_APP_AUTHOR;

      BtnXAction Buttons[3]     =
      {
        { nullptr, SDU_BTNA_MENU },
        { nullptr, SDU_BTNB_SKIP },
        { nullptr, SDU_BTNC_SAVE }
      };

      BtnPollCb         buttonsUpdate    = nullptr;
      pollChecker       buttonsPoll      = nullptr;
      btnAChecker       btnAPressed      = nullptr;
      btnBChecker       btnBPressed      = nullptr;
      btnCChecker       btnCPressed      = nullptr;
      onProgressCb      onProgress       = nullptr;
      onMessageCb       onMessage        = nullptr;
      onErrorCb         onError          = nullptr;
      onBeforeCb        onBefore         = nullptr;
      onAfterCb         onAfter          = nullptr;
      onSplashPageCb    onSplashPage     = nullptr;
      onButtonDrawCb    onButtonDraw     = nullptr;
      onWaitForActionCb onWaitForAction  = nullptr;

      void setDisplay( void* ptr=nullptr );// overloaded in UI.hpp
      void setButtons();// overloaded in UI.hpp

      void setCSPin( const int param )                { TFCardCsPin = param; }
      void setFS( fs::FS *param )                     { fs = param; }
      void setProgressCb( onProgressCb cb )           { onProgress = cb; }
      void setMessageCb( onMessageCb cb )             { onMessage = cb; }
      void setErrorCb( onErrorCb cb )                 { onError = cb; }
      void setBeforeCb( onBeforeCb cb )               { onBefore = cb; }
      void setAfterCb( onAfterCb cb )                 { onAfter = cb; }
      void setSplashPageCb( onSplashPageCb cb )       { onSplashPage = cb; }
      void setButtonDrawCb( onButtonDrawCb cb )       { onButtonDraw = cb; }
      void setWaitForActionCb( onWaitForActionCb cb ) { onWaitForAction = cb; }

      void setLabelMenu( const char* label )          { labelMenu = label; }
      void setLabelSkip( const char* label )          { labelSkip = label; }
      void setLabelRollback( const char* label )      { labelRollback = label; }
      void setLabelSave( const char* label )          { labelSave = label; }
      void setAppName( const char* name )             { appName = name; }
      void setAuthorName( const char* name )          { authorName = name; }
      void setBinFileName( const char* name )         { binFileName = name; }
      void useRolllback( bool use )                   { use_rollback = use; }

      void setButtonsTheme( Theme_t *_theme )         { theme = _theme; }

      void setSDUBtnPoller( BtnPollCb cb )            { buttonsUpdate = cb; }

      void setSDUBtnChecker( pollChecker cb )         { buttonsPoll = cb; }
      void setBtnAChecker( btnAChecker fn )           { btnAPressed = fn; }
      void setBtnBChecker( btnBChecker fn )           { btnBPressed = fn; }
      void setBtnCChecker( btnCChecker fn )           { btnCPressed = fn; }

      void setSDUBtnA( BtnXPressCb Btn )              { setSDUBtns(SDU_BTNA_MENU, Btn ); }
      void setSDUBtnB( BtnXPressCb Btn )              { setSDUBtns(SDU_BTNB_SKIP, Btn ); }
      void setSDUBtnC( BtnXPressCb Btn )              { setSDUBtns(SDU_BTNC_SAVE, Btn ); }
      void setSDUBtns( SDUBtnActions BtnVal, BtnXPressCb cb )
      {
        int _id = -1;
        switch( BtnVal ) {
          case SDU_BTNA_MENU: _id = 0; break;
          case SDU_BTNB_SKIP: _id = 1; break;
          case SDU_BTNC_SAVE: _id = 2; break;
          default: log_e("Invalid button val: %d", BtnVal ); return; break;
        }
        Buttons[_id].cb  = cb;
        Buttons[_id].val = BtnVal;
      }

    };

    // override this from sketch
    [[maybe_unused]] static onConfigLoad SDUCfgLoader = nullptr;
    extern config_sdu_t SDUCfg;


    void pollButtons()
    {
      if(SDUCfg.buttonsUpdate) SDUCfg.buttonsUpdate();
    }

    bool buttonAPressed()
    {
      if(SDUCfg.btnAPressed) return SDUCfg.btnAPressed();
      else return false;
    }

    bool buttonBPressed()
    {
      if(SDUCfg.btnBPressed) return SDUCfg.btnBPressed();
      else return false;
    }

    bool buttonCPressed()
    {
      if(SDUCfg.btnCPressed) return SDUCfg.btnCPressed();
      else return false;
    }


  };

  using ConfigManager::SDUCfg;

}; // end namespace




