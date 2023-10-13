#pragma once

#include <stdint.h>
#include <functional>
#include <vector>
#include <Stream.h>
#include <WString.h>

#define FN_LAMBDA_VOID(x) []() { }
#define FN_LAMBDA_BOOL(x) []() -> bool { return x; }
#define FN_LAMBDA_FALSE   []() -> bool { return false; }

namespace SDUpdaterNS
{

  namespace UpdateInterfaceNS
  {
    [[maybe_unused]] static bool mode_z = false;
    typedef std::function<void(size_t, size_t)> THandlerFunction_Progress;
    struct UpdateManagerInterface_t
    {
      //typedef void    (*THandlerFunction_Progress)(size_t, size_t);
      typedef bool    (*begin_t)(size_t);
      typedef size_t  (*writeStream_t)(Stream &data,size_t size);
      typedef void    (*abort_t)();
      typedef bool    (*end_t)();
      typedef bool    (*isFinished_t)();
      typedef bool    (*canRollBack_t)();
      typedef bool    (*rollBack_t)();
      typedef void    (*onProgress_t)(THandlerFunction_Progress fn);
      typedef uint8_t (*getError_t)();
      typedef void    (*setBinName_t)(String& fileName, Stream* stream);
      public:
        begin_t begin;
        writeStream_t writeStream;
        abort_t abort;
        end_t end;
        isFinished_t isFinished;
        canRollBack_t canRollBack;
        rollBack_t rollBack;
        onProgress_t onProgress;
        getError_t getError;
        setBinName_t setBinName;
    };
  };


  namespace TriggerSource
  {

    enum TriggerSources_t
    {
      SDU_TRIGGER_SERIAL,     // headless
      SDU_TRIGGER_PUSHBUTTON, // Push Button (GPIO or user provided)
      SDU_TRIGGER_TOUCHBUTTON // Touche Button (using LGFX/eSPI touch driver)
    };

    struct triggerMap_t;

    typedef void(*triggerInitCb)( triggerMap_t* trigger ); // start listening to trigger source (preinit)
    typedef bool (*triggerActionCb)( triggerMap_t* trigger, uint32_t msec ); // listen to trigger source and return true if one was found
    typedef void(*triggerDeinitCb)( triggerMap_t* trigger, int ret ); // stop listening to trigger source (deinit)

    struct triggerMap_t
    {
      triggerMap_t() { };
      triggerMap_t(TriggerSources_t _source, const char*_labelLoad,const char*_labelSkip,const char*_labelSave,triggerInitCb _init,triggerActionCb _get, triggerDeinitCb _final)
      : source(_source), labelLoad(_labelLoad),labelSkip(_labelSkip),labelSave(_labelSave),init(_init),get(_get),finalize(_final) { };
      TriggerSources_t source = SDU_TRIGGER_SERIAL;
      const char* labelLoad = nullptr;
      const char* labelSkip = nullptr;
      const char* labelSave = nullptr;
      unsigned long waitdelay = 5000;
      int ret = -1;
      void *sharedptr = nullptr;
      triggerInitCb init;
      triggerActionCb get;
      triggerDeinitCb finalize;
    };

    //[[maybe_unused]] static int serial( char* labelLoad,  char* labelSkip, char* labelSave, unsigned long waitdelay=5000  );
    //[[maybe_unused]] static int pushButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay=5000  );
    //[[maybe_unused]] static int touchButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay=5000 );

  }



  namespace ConfigManager
  {
    // values to be returned by onWaitForActionCb
    enum SDUBtnActions
    {
      SDU_BTNA_ROLLBACK =  0, // rollback = load the other OTA partition
      SDU_BTNA_MENU     =  1, // menu = load /menu.bin from SD or rollback if partitions match
      SDU_BTNB_SKIP     = -1, // skip = leave the lobby and start the current app
      SDU_BTNC_SAVE     =  2, // save = copy the current firmware to the filesystem
      SDU_BTN_NONE      = -2  // no activity
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

    struct BtnXAction
    {
      BtnXPressCb cb; // external callback, returns true when the button was pressed
      bool changed() { return cb ? cb() : false; } // trigger checker
      SDUBtnActions val; // button value to return when action is triggered
      bool enabled;
    };

    // abstract filesystem config
    struct FS_Config_t
    {
      const char* name;
      void *fsPtr;
      void *cfgPtr;
    };


  };

  // directly callable (no poll, no pinMode) button
  struct DigitalPinButton_t
  {
    DigitalPinButton_t( const int _pin ) : pin(_pin) { } // constructor
    const int pin; // GPIO Pin
    bool changed(); // trigger checker
  };



  namespace SDU_UI
  {

    struct SplashPageElementStyle_t
    {
      const uint16_t textColor;
      const uint16_t bgColor;
      const void* fontInfo; // holds font size + font face
      const uint16_t textDatum;
      const uint16_t colorStart; // gradient color start
      const uint16_t colorEnd;   // gradient color end
    };

    struct ProgressBarStyle_t
    {
      const int      width;
      const int      height;
      const bool     clipText;
      const uint16_t borderColor;
      const uint16_t fillColor;

      //const uint8_t  fontNumber;
      const void* fontInfo; // holds font size + font face

      const uint8_t  textDatum;

      //const uint8_t  textSize;

      const uint16_t textColor;
      const uint16_t bgColor;
    };

    struct BtnStyle_t
    {
      BtnStyle_t( const uint16_t _BorderColor, const uint16_t _FillColor, const uint16_t _TextColor, const uint16_t _ShadowColor ) :
        BorderColor(_BorderColor), FillColor(_FillColor), TextColor(_TextColor), ShadowColor(_ShadowColor) { }
      const uint16_t BorderColor;
      const uint16_t FillColor;
      const uint16_t TextColor;
      const uint16_t ShadowColor;
    };

    struct SDUTextStyle_t // somehow redundant with LGFX's textstyle_t
    {
      bool frozen           = false;
      uint8_t textsize      = 0;
      uint8_t textdatum     = 0;
      uint32_t textcolor    = 0;
      uint32_t textbgcolor  = 0;
    };

    struct BtnStyles_t
    {
      BtnStyles_t() { };
      BtnStyles_t(
        const BtnStyle_t _Load, const BtnStyle_t _Skip, const BtnStyle_t _Save,
        const uint16_t _height, const uint16_t _width, const uint16_t _hwidth,
        const void* _BtnfontInfo, const void* _MsgfontInfo,
        const uint16_t _MsgFontColor[2]
      ) :
        Load(_Load), Skip(_Skip), Save(_Save),
        height(_height), width(_width), hwidth(_hwidth),
        BtnFontInfo(_BtnfontInfo), MsgFontInfo(_MsgfontInfo)
      {
        MsgFontColor[0] = _MsgFontColor[0];
        MsgFontColor[1] = _MsgFontColor[1];
      }
      // 16bit colors:       Border  Fill    Text    Shadow
      const BtnStyle_t Load{ 0x73AE, 0x630C, 0xFFFF, 0x0000 };
      const BtnStyle_t Skip{ 0x73AE, 0x4208, 0xFFFF, 0x0000 };
      const BtnStyle_t Save{ 0x73AE, 0x2104, 0xFFFF, 0x0000 };
      const uint16_t height{28};
      const uint16_t width{68};
      const uint16_t hwidth{34};
      const void* BtnFontInfo; // holds font size + font face
      const void* MsgFontInfo; // holds font size + font face
      uint16_t MsgFontColor[2]{0xFFFF, 0x0000}; // foreground, background
    };


    // animation played in the lobby while waiting for an action trigger
    struct loaderAnimator_t
    {
      loaderAnimator_t() { };
      void init();
      void animate();
      void deinit();
    };


    // Load/Skip/Save default Touch Button styles
    struct TouchStyles
    {
      TouchStyles();
      ~TouchStyles();
      int padx     {0}, // buttons padding X
          pady     {0}, // buttons padding Y
          marginx  {0}, // buttons margin X
          marginy  {0}, // buttons margin Y
          x1       {0}, // button 1 X position
          x2       {0}, // button 2 X position
          x3       {0}, // button 3 X position
          y        {0}, // buttons Y position
          w        {0}, // buttons width
          h        {0}, // buttons height
          y1       {0}, // button3 y position
          icon_x   {0}, // icon (button 1) X position
          icon_y   {0}, // icon (button 1) Y position
          pgbar_x  {0}, // progressbar X position
          pgbar_y  {0}, // progressbar Y position
          pgbar_w  {0}, // progressbar width
          btn_fsize{0}  // touch buttons font size
      ;
      BtnStyle_t *Load{nullptr};
      BtnStyle_t *Skip{nullptr};
      BtnStyle_t *Save{nullptr};
    };

    // Theme holding all the Button styles
    struct Theme_t
    {
      Theme_t() { };
      Theme_t(
        const BtnStyles_t*        _buttons,
        SplashPageElementStyle_t* _SplashTitleStyle,
        SplashPageElementStyle_t* _SplashAppNameStyle,
        SplashPageElementStyle_t* _SplashAuthorNameStyle,
        SplashPageElementStyle_t* _SplashAppPathStyle,
        ProgressBarStyle_t      * _ProgressStyle
      ) :
        buttons              (_buttons              ),
        SplashTitleStyle     (_SplashTitleStyle     ),
        SplashAppNameStyle   (_SplashAppNameStyle   ),
        SplashAuthorNameStyle(_SplashAuthorNameStyle),
        SplashAppPathStyle   (_SplashAppPathStyle   ),
        ProgressStyle        (_ProgressStyle        )
      { };

      const BtnStyles_t* buttons{nullptr};
      SplashPageElementStyle_t* SplashTitleStyle     {nullptr};
      SplashPageElementStyle_t* SplashAppNameStyle   {nullptr};
      SplashPageElementStyle_t* SplashAuthorNameStyle{nullptr};
      SplashPageElementStyle_t* SplashAppPathStyle   {nullptr};
      ProgressBarStyle_t *ProgressStyle{nullptr};
    };

  }


};
