#ifndef _M5UPDATER_UI_
#define _M5UPDATER_UI_

// LGFX complains when using old TFT_eSPI syntax
// but this sketch must be driver agnostic
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "assets.h"

#define ROLLBACK_LABEL "Rollback" // reload app from the "other" OTA partition
#define LAUNCHER_LABEL "Launcher" // load Launcher (typically menu.bin)
#define SKIP_LABEL     "Skip >>|" // resume normal operations (=no action taken)
#define BTN_HINT_MSG   "SD-Updater Options"

#if defined _CHIMERA_CORE_ || defined _M5STICKC_H_ || defined _M5STACK_H_ || defined _M5Core2_H_ || defined LGFX_ONLY
  #if defined LGFX_ONLY
    // WARN: this library assumes a 'tft' object exists
    #ifndef SDU_GFX
      #define undef_tft
      static LGFX *SDuGFX = nullptr;
      #define SDU_GFX reinterpret_cast<LGFX&>(*SDuGFX)
      static void setSDUGfx( LGFX *gfx )
      {
        SDuGFX = gfx;
        SDU.setGfx( gfx );
      }
    #endif
  #else
    #ifndef SDU_GFX
      #define undef_tft
      #define SDU_GFX M5.Lcd // can be either M5.Lcd from M5Core.h, M5Core2.h or ESP32-Chimera-Core.h
    #endif
  #endif

  #ifdef ARDUINO_ODROID_ESP32 // odroid has 4 buttons under the TFT
    #define BUTTON_WIDTH 60
    #define BUTTON_HWIDTH BUTTON_WIDTH/2 // 30
    #define BUTTON_HEIGHT 28
    static int16_t SDUButtonsXOffset[4] = {
      1, 72, 188, 260
    };
    static int16_t SDUButtonsYOffset[4] = {
      0, 0, 0, 0
    };
  #else // assuming landscape mode /w 320x240 display
    #define BUTTON_WIDTH 60
    #define BUTTON_HWIDTH BUTTON_WIDTH/2 // 30
    #define BUTTON_HEIGHT 28
    static int16_t SDUButtonsXOffset[3] = {
      31, 126, 221
    };
    static int16_t SDUButtonsYOffset[3] = {
      0, 0, 0
    };
  #endif

  struct BtnStyle {
    uint16_t BorderColor;
    uint16_t FillColor;
    uint16_t TextColor;
  };

  struct BtnStyles {
    BtnStyle Load = { TFT_ORANGE,                          SDU_GFX.color565( 0xaa, 0x00, 0x00), SDU_GFX.color565( 0xdd, 0xdd, 0xdd) };
    BtnStyle Skip = { SDU_GFX.color565( 0x11, 0x11, 0x11), SDU_GFX.color565( 0x33, 0x88, 0x33), SDU_GFX.color565( 0xee, 0xee, 0xee) };
    uint16_t height          = BUTTON_HEIGHT;
    uint16_t width           = BUTTON_WIDTH;
    uint16_t hwidth          = BUTTON_HWIDTH;
    uint8_t  FontSize        = 1; // buttons font size
    uint8_t  MsgFontSize     = 2; // welcome message font size
    uint16_t MsgFontFolor[2] = {TFT_WHITE, TFT_BLACK}; // foreground, background
  };

  static BtnStyles DefaultBtnStyle;
  static BtnStyles *userBtnStyle = nullptr;

  static void SDMenuProgressUI( int state, int size );
  static void DisplayUpdateUI( const String& label );
  static void DisplayErrorUI( const String& msg, unsigned long wait );

  static void rollBackUI()
  {
    if( Update.canRollBack() ) {
      SDU.onMessage( "RE-LOADING APP" );
      for( uint8_t i=1; i<50; i++ ) {
        SDU.onProgress( i, 100 );
      }
      Update.rollBack();
      for( uint8_t i=50; i<=100; i++ ) {
        SDU.onProgress( i, 100 );
      }
      ESP.restart();
    } else {
      log_n("Cannot rollback: the other OTA partition doesn't seem to be populated or valid");
      SDU.onError("Cannot rollback", 2000);
    }
  }

  struct SDUTextStyle_T // somehow redundant with LGFX's textstyle_t
  {
    uint8_t textsize      = 0;
    uint8_t textdatum     = 0;
    uint32_t textcolor    = 0;
    uint32_t textbgcolor  = 0;
  };
  static SDUTextStyle_T SDUTextStyle;

  static void freezeTextStyle()
  {
    #if defined LGFX_ONLY
      SDUTextStyle.textcolor   = SDU_GFX.getTextStyle().fore_rgb888;
      SDUTextStyle.textbgcolor = SDU_GFX.getTextStyle().back_rgb888;
      SDUTextStyle.textdatum   = SDU_GFX.getTextStyle().datum;
      SDUTextStyle.textsize    = SDU_GFX.getTextStyle().size_x;
    #else
      SDUTextStyle.textsize    = SDU_GFX.textsize;
      SDUTextStyle.textdatum   = SDU_GFX.textdatum;
      SDUTextStyle.textcolor   = SDU_GFX.textcolor;
      SDUTextStyle.textbgcolor = SDU_GFX.textbgcolor;
    #endif

    log_d("Froze textStyle, size: %d, datum: %d, color: %d, bgcolor: %d", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
  }

  static void thawTextStyle()
  {
    SDU_GFX.setTextSize( SDUTextStyle.textsize);
    SDU_GFX.setTextDatum( SDUTextStyle.textdatum );
    SDU_GFX.setTextColor( SDUTextStyle.textcolor , SDUTextStyle.textbgcolor );
    #ifndef  _M5STICKC_H_
      SDU_GFX.setFont( nullptr );
    #endif
    SDU_GFX.setCursor(0,0);
    SDU_GFX.fillScreen(TFT_BLACK);
    log_d("Thawed textStyle, size: %d, datum: %d, color: %d, bgcolor: %d", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
  }

  __attribute__((unused))
  static void drawSDUSplashPage( const char* msg )
  {
    BtnStyles *bs = ( userBtnStyle == nullptr ) ? &DefaultBtnStyle : userBtnStyle;
    SDU_GFX.setTextColor( bs->MsgFontFolor[0], bs->MsgFontFolor[1] );
    SDU_GFX.setTextSize( bs->MsgFontSize );
    //SDU_GFX.setCursor( SDU_GFX.width()/2, 0 );
    SDU_GFX.setTextDatum( TC_DATUM );
    SDU_GFX.setTextFont( 0 );
    SDU_GFX.drawString( msg, SDU_GFX.width()/2, 0 );
    #if defined SDU_APP_NAME
      SDU_GFX.drawString( SDU_APP_NAME, SDU_GFX.width()/2, 32 );
    #endif
  }

  __attribute__((unused))
  static void drawSDUPushButton( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor )
  {
    BtnStyles *bs = ( userBtnStyle == nullptr ) ? &DefaultBtnStyle : userBtnStyle;
    SDU_GFX.setTextColor( textcolor, fillcolor );
    SDU_GFX.setTextSize( bs->FontSize );

    uint32_t bx = SDUButtonsXOffset[position];
    uint32_t by = SDU_GFX.height() - bs->height - 2 - SDUButtonsYOffset[position];

    SDU_GFX.fillRoundRect( bx, by, bs->width, bs->height, 3, fillcolor );
    SDU_GFX.drawRoundRect( bx, by, bs->width, bs->height, 3, outlinecolor );

    SDU_GFX.setTextDatum( MC_DATUM );
    SDU_GFX.setTextFont( 2 );
    SDU_GFX.drawString( label, bx+bs->width/2, by+bs->height/2 );
  }


  #if defined LGFX_ONLY
    __attribute__((unused))
    static int assertStartUpdateFromPushButton( char* labelLoad, char* labelSkip, unsigned long waitdelay )
    {
      // Dummy function, use setAssertTrigger( fn ) to override
      return -1;
    }
  #else // M5.BtnA/BtnB/BtnC support
    __attribute__((unused))
    static int assertStartUpdateFromPushButton( char* labelLoad, char* labelSkip, unsigned long waitdelay )
    {
      if( waitdelay > 100 ) {
        SDU.onBefore();
        SDU.onSplashPage( BTN_HINT_MSG );
        BtnStyles btns;
        SDU.onButtonDraw( labelLoad, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor );
        SDU.onButtonDraw( labelSkip, 1, btns.Skip.BorderColor, btns.Skip.FillColor, btns.Skip.TextColor );
      }
      auto msec = millis();
      do {
        M5.update();
        if( M5.BtnA.isPressed() ) return 1;
        if( M5.BtnB.isPressed() ) return 0;
      } while (millis() - msec < waitdelay);
      return -1;
    }
  #endif


  #if defined HAS_TOUCH || defined _M5Core2_H_ // ESP32-Chimera-Core / LGFX / M5Core2 touch support

    #include "M5StackUpdaterUITouch.h"

  #endif // HAS_TOUCH


  static void loadCfg()
  {
    if( SDU.onProgress == nullptr)      SDU.onProgress      = SDMenuProgressUI;
    if( SDU.onMessage == nullptr)       SDU.onMessage       = DisplayUpdateUI;
    if( SDU.onError == nullptr)         SDU.onError         = DisplayErrorUI;
    if( SDU.onBefore == nullptr)        SDU.onBefore        = freezeTextStyle;
    if( SDU.onAfter == nullptr)         SDU.onAfter         = thawTextStyle;
    if( SDU.onSplashPage == nullptr)    SDU.onSplashPage    = drawSDUSplashPage;
    if( SDU.onButtonDraw == nullptr)    SDU.onButtonDraw    = drawSDUPushButton;
    //if( SDU.onWaitForAction == nullptr) SDU.onWaitForAction = SDUpdaterAssertTrigger;
    #if defined HAS_TOUCH || defined _M5Core2_H_ // default touch button support
      SDU.onWaitForAction = assertStartUpdateFromTouchButton;
    #else // default momentary button support
      SDU.onWaitForAction = assertStartUpdateFromPushButton;
    #endif
  }


  static void checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay, const int TFCardCsPin )
  {

    log_n("Booting with reset reason: %d", resetReason );

    bool isRollBack = true;
    if( fileName != "" ) {
      isRollBack = false;
    }

    #if defined HAS_TOUCH || defined _M5Core2_H_ // default touch button support
      // TODO: see if "touch/press" detect on boot is possible (spoil : NO)
      // TODO: read signal from other (external?) buttons
      bool draw = true;
      //if( waitdelay == 0 ) return; // don't check for any touch/button signal if waitDelay = 0
    #else // default push buttons support
      bool draw = false;
      //if( waitdelay == 0 ) return; // don't check for any touch/button signal if waitDelay = 0
      if( waitdelay <= 100 ) {
        // no UI draw, but still attempt to detect "button is pressed on boot"
        // round up to 100ms for button debounce
        waitdelay = 100;
      } else {
        // only draw if waitdelay > 0
        draw = true;
      }
    #endif

    loadCfg();

    if( draw ) { // bring up the UI
      SDU.onBefore();
      SDU.onSplashPage( BTN_HINT_MSG );
    }

    if ( SDU.onWaitForAction( isRollBack ? (char*)ROLLBACK_LABEL : (char*)LAUNCHER_LABEL,  (char*)SKIP_LABEL, waitdelay ) == 1 ) {
      if( isRollBack == false ) {
        Serial.printf( SDU_LOAD_TPL, fileName.c_str() );
        updateFromFS( fs, fileName, TFCardCsPin );
        ESP.restart();
      } else {
        Serial.println( SDU_ROLLBACK_MSG );
        rollBackUI();
      }
    }

    if( draw ) {
      // reset text styles to avoid messing with the overlayed application
      SDU.onAfter();
    }

  }


  static void SDMenuProgressUI( int state, int size )
  {
    static int SD_UI_Progress;
    int percent = ( state * 100 ) / size;
    if( percent == SD_UI_Progress ) {
      // don't render twice the same value
      return;
    }
    /* auto &tft = M5.Lcd; */
    SD_UI_Progress = percent;
    int progress_w = 102;
    int progress_h = 20;
    int progress_x = (SDU_GFX.width() - progress_w) >> 1;
    int progress_y = (SDU_GFX.height()- progress_h) >> 1;
    if ( percent >= 0 && percent < 101 ) {
      SDU_GFX.fillRect( progress_x+1, progress_y+1, percent, 18, TFT_GREEN );
      SDU_GFX.fillRect( progress_x+1+percent, progress_y+1, 100-percent, 18, TFT_BLACK );
      Serial.print( "." );
    } else {
      SDU_GFX.fillRect( progress_x+1, progress_y+1, 100, 18, TFT_BLACK );
      Serial.println();
    }
    String percentStr = " " + String( percent ) + "% ";
    SDU_GFX.setTextDatum( TC_DATUM );
    SDU_GFX.setCursor( SDU_GFX.width() >> 1, progress_y+progress_h+5 );
    SDU_GFX.setTextFont( 0 );
    SDU_GFX.print( percentStr );
    if ( percent >= 0 && percent < 101 ) {
      Serial.print( "." );
    } else {
      Serial.println();
    }
  };



  static void DisplayUpdateUI( const String& label )
  {
    if (SDU_GFX.width() < SDU_GFX.height()) SDU_GFX.setRotation(SDU_GFX.getRotation() ^ 1);

    SDU_GFX.fillScreen( TFT_BLACK );
    SDU_GFX.setTextColor( TFT_WHITE, TFT_BLACK );
    SDU_GFX.setTextFont( 0 );
    SDU_GFX.setTextSize( 2 );
    SDU_GFX.setTextDatum( ML_DATUM );
    // attemtp to center the text
    int16_t xpos = ( SDU_GFX.width() / 2) - ( SDU_GFX.textWidth( label ) / 2 );
    if ( xpos < 0 ) {
      // try with smaller size
      SDU_GFX.setTextSize(1);
      xpos = ( SDU_GFX.width() / 2 ) - ( SDU_GFX.textWidth( label ) / 2 );
      if( xpos < 0 ) {
        // give up
        xpos = 0 ;
      }
    }
    int progress_w = 102;
    int progress_h = 20;
    int progress_x = (SDU_GFX.width() - progress_w) >> 1;
    int progress_y = (SDU_GFX.height()- progress_h) >> 1;
    SDU_GFX.setCursor( xpos, progress_y - 20 );
    SDU_GFX.print( label );
    SDU_GFX.drawRect( progress_x, progress_y, progress_w, progress_h, TFT_WHITE );
  }



  static void DisplayErrorUI( const String& msg, unsigned long wait )
  {
    SDU_GFX.fillScreen( TFT_BLACK );
    SDU_GFX.setTextColor( TFT_RED, TFT_BLACK );
    SDU_GFX.setTextFont( 0 );
    SDU_GFX.setTextSize( 2 );
    SDU_GFX.setTextDatum( ML_DATUM );
    SDU_GFX.setCursor( 10, 10 );
    SDU_GFX.print( msg );
    delay(wait);
  }


  // release 'tft' temporary define
  #if defined undef_tft
    #undef SDU_GFX
  #endif

#else
  // unknown tft context, go headless
  #undef USE_DISPLAY
#endif


#endif //  _M5UPDATER_UI_
