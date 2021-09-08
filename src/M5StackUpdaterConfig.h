
// callback signatures
typedef void (*onProgressCb)( int state, int size );
typedef void (*onMessageCb)( const String& label );
typedef void (*onErrorCb)( const String& message, unsigned long delay );
typedef void (*onBeforeCb)();
typedef void (*onAfterCb)();
typedef void (*onSplashPageCb)( const char* msg );
typedef void (*onButtonDrawCb)( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor );
typedef int  (*onWaitForActionCb)( char* labelLoad, char* labelSkip, unsigned long waitdelay );
typedef void (*onConfigLoad)();

// SDUpdater config callbacks and params
struct config_sdu_t
{
  void * gfx =  nullptr; // optional pointer for custom callbacks
  void * sdUpdater = nullptr; // mandatory pointer to SDUpdater instance
  int TFCardCsPin = TFCARD_CS_PIN;
  bool load_defaults = true;

  onProgressCb      onProgress       = nullptr;
  onMessageCb       onMessage        = nullptr;
  onErrorCb         onError          = nullptr;
  onBeforeCb        onBefore         = nullptr;
  onAfterCb         onAfter          = nullptr;
  onSplashPageCb    onSplashPage     = nullptr;
  onButtonDrawCb    onButtonDraw     = nullptr;
  onWaitForActionCb onWaitForAction  = nullptr;

  void setSDU( void *param)                       { sdUpdater = param; }
  void setCSPin( const int param )                { TFCardCsPin = param; }
  void setGfx( void *param )                      { gfx = param; };
  void setProgressCb( onProgressCb cb )           { onProgress = cb; }
  void setMessageCb( onMessageCb cb )             { onMessage = cb; }
  void setErrorCb( onErrorCb cb )                 { onError = cb; }
  void setBeforeCb( onBeforeCb cb )               { onBefore = cb; }
  void setAfterCb( onAfterCb cb )                 { onAfter = cb; }
  void setSplashPageCb( onSplashPageCb cb )       { onSplashPage = cb; }
  void setButtonDrawCb( onButtonDrawCb cb )       { onButtonDraw = cb; }
  void setWaitForActionCb( onWaitForActionCb cb ) { onWaitForAction = cb; }

};

static onConfigLoad SDUCfgLoader = nullptr;
static config_sdu_t SDUCfg;

