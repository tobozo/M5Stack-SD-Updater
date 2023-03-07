#pragma once

#include "../M5StackUpdater.hpp"
#include "../types.h"
#if defined USE_DISPLAY
  #include "assets.h"
#endif

namespace SDUpdaterNS
{

  namespace SDU_UI
  {
    void SDMenuProgressUI( int state, int size );
    void DisplayUpdateUI( const String& label );
    void DisplayErrorUI( const String& msg, unsigned long wait );
    void freezeTextStyle();
    void thawTextStyle();
    void drawSDUSplashPage( const char* msg );
    void drawSDUPushButton( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor, uint16_t shadowcolor );
    void fillStyledRect( SplashPageElementStyle_t *style, int32_t x, int32_t y, uint16_t width, uint16_t height );
    void adjustFontSize( uint8_t *lineHeightBig, uint8_t *lineHeightSmall );
    static void drawTextShadow( const char* text, int32_t x, int32_t y, uint16_t textcolor, uint16_t shadowcolor );
    void drawSDUSplashElement( const char* msg, int32_t x, int32_t y, SplashPageElementStyle_t *style );
    void DisplayUpdateHeadless( const String& label );
    void SDMenuProgressHeadless( int state, int size );

    #if defined USE_DISPLAY
      struct loaderAnimator_t
      {
        SDUSprite *sprite = nullptr;
        DISPLAY_TYPE display = nullptr;
        void init( DISPLAY_TYPE gfx );
        void animate();
        void deinit();
      };


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

      #if defined SDU_TouchButton
        struct TouchButtonWrapper
        {
          bool iconRendered = false;
          void handlePressed( SDU_TouchButton *btn, bool pressed, uint16_t x, uint16_t y);
          void handleJustPressed( SDU_TouchButton *btn, const char* label );
          bool justReleased( SDU_TouchButton *btn, bool pressed, const char* label );
          void pushIcon(const char* label);
        };
      #endif


    #endif

  };

  namespace TriggerSource
  {
    int pushButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay );
    int touchButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay=5000 );
    int serial( char* labelLoad,  char* labelSkip, char* labelSave, unsigned long waitdelay );
  };

  #define SDU_GFX ((DISPLAY_TYPE)(SDUCfg.display))

};

