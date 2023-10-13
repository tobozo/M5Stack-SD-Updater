#include "ConfigManager.hpp"

namespace SDUpdaterNS
{

  bool DigitalPinButton_t::changed()
  {
    static int lastbtnstate = digitalRead( pin );
    if( digitalRead( pin ) != lastbtnstate ) {
      lastbtnstate = 1-lastbtnstate;
      log_d("btnstate: %d", lastbtnstate );
      return true;
    }
    return false;
  }


  namespace ConfigManager
  {

    config_sdu_t::config_sdu_t() { }

    void config_sdu_t::buttonsPoll(){ if( buttonsUpdate ) buttonsUpdate(); }

    void config_sdu_t::setDisplay( void* ptr )                    { display=ptr; };
    void config_sdu_t::setCSPin( const int param )                { TFCardCsPin = param; }
    void config_sdu_t::setWaitDelay( unsigned long delay )        { waitdelay = delay; }
    void config_sdu_t::setFS( fs::FS *param )                     { fs = param; }
    void config_sdu_t::setButtonsTheme( SDU_UI::Theme_t *_theme ) { theme = _theme; }
    void config_sdu_t::setProgressCb( onProgressCb cb )           { onProgress = cb; }
    void config_sdu_t::setMessageCb( onMessageCb cb )             { onMessage = cb; }
    void config_sdu_t::setErrorCb( onErrorCb cb )                 { onError = cb; }
    void config_sdu_t::setBeforeCb( onBeforeCb cb )               { onBefore = cb; }
    void config_sdu_t::setAfterCb( onAfterCb cb )                 { onAfter = cb; }
    void config_sdu_t::setSplashPageCb( onSplashPageCb cb )       { onSplashPage = cb; }
    void config_sdu_t::setButtonDrawCb( onButtonDrawCb cb )       { onButtonDraw = cb; }
    void config_sdu_t::setWaitForActionCb( onWaitForActionCb cb ) { onWaitForAction = cb; }

    void config_sdu_t::setLabelMenu( const char* label )          { labelMenu = label; }
    void config_sdu_t::setLabelSkip( const char* label )          { labelSkip = label; }
    void config_sdu_t::setLabelRollback( const char* label )      { labelRollback = label; }
    void config_sdu_t::setLabelSave( const char* label )          { labelSave = label; }
    void config_sdu_t::setAppName( const char* name )             { appName = name; }
    void config_sdu_t::setAuthorName( const char* name )          { authorName = name; }
    void config_sdu_t::setBinFileName( const char* name )         { if( name && strstr(name, ".gz")!=NULL ) binFileName = name; }
    void config_sdu_t::useRolllback( bool use )                   { use_rollback = use; }

    void config_sdu_t::setBtnPoller( BtnPollCb cb )            { log_v("Assigning Btn Poller"); buttonsUpdate = cb; }
    void config_sdu_t::setBtnA( BtnXPressCb Btn )              { log_v("Assigning BtnA"); setBtns(SDU_BTNA_MENU, Btn ); }
    void config_sdu_t::setBtnB( BtnXPressCb Btn )              { log_v("Assigning BtnB"); setBtns(SDU_BTNB_SKIP, Btn ); }
    void config_sdu_t::setBtnC( BtnXPressCb Btn )              { log_v("Assigning BtnC"); setBtns(SDU_BTNC_SAVE, Btn ); }
    void config_sdu_t::setBtns( SDUBtnActions BtnVal, BtnXPressCb cb )
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
    //config_sdu_t SDUCfg;
    config_sdu_t SDUCfg;

  };

};
