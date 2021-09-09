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
#ifndef _M5UPDATER_UI_
#define _M5UPDATER_UI_

// LGFX complains when using old TFT_eSPI syntax
// but this sketch must be driver agnostic
//#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// headless methods
__attribute__((unused))
static int assertStartUpdateFromSerial( char* labelLoad,  char* labelSkip, unsigned long waitdelay )
{
  int64_t msec = millis();
  do {
    if( Serial.available() ) {
      String out = Serial.readStringUntil('\n');
      if(      out == "update" ) return 1;
      else if( out == "rollback") return 0;
      else if( out == "skip" ) return -1;
      else Serial.printf("Ignored command: %s\n", out.c_str() );
    }
  } while( msec > int64_t( millis() ) - int64_t( waitdelay ) );
  return -1;
}


__attribute__((unused))
static void DisplayUpdateHeadless( const String& label ) {
  // TODO: draw some fancy serial output
};


__attribute__((unused))
static void SDMenuProgressHeadless( int state, int size )
{
  static int SD_UI_Progress;
  int percent = ( state * 100 ) / size;
  if( percent == SD_UI_Progress ) {
    // don't render twice the same value
    return;
  }
  //Serial.printf("percent = %d\n", percent); // this is spammy
  SD_UI_Progress = percent;
  if ( percent >= 0 && percent < 101 ) {
    Serial.print( "." );
  } else {
    Serial.println();
  }
};



#if defined USE_DISPLAY
  #include "assets.h"

  // display driver selector
  #if defined LGFX_ONLY
    #ifndef SDU_GFX
      // recast from pointer
      #define undef_tft
      static LGFX *SDuGFX = nullptr;
      #define SDU_GFX reinterpret_cast<LGFX&>(*SDuGFX)
      static void setSDUGfx( LGFX *gfx )
      {
        SDuGFX = gfx;
      }
    #endif
  #else
    #ifndef SDU_GFX
      #define undef_tft
      // recast from reference, M5.Lcd can be either from Chimera-Core or M5Cores (or even TFT_eSPI)
      #define SDU_GFX M5.Lcd // can be either M5.Lcd from M5Core.h, M5Core2.h or ESP32-Chimera-Core.h
    #endif
  #endif

  // theme selector
  #ifdef ARDUINO_ODROID_ESP32 // Odroid-GO has 4 buttons under the TFT
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

  struct SDUTextStyle_T // somehow redundant with LGFX's textstyle_t
  {
    bool frozen           = false;
    uint8_t textsize      = 0;
    uint8_t textdatum     = 0;
    uint32_t textcolor    = 0;
    uint32_t textbgcolor  = 0;
  };
  static SDUTextStyle_T SDUTextStyle;

  static void freezeTextStyle()
  {
    if( SDUTextStyle.frozen ) {
      // log_v("can't freeze twice, thaw first !");
      return;
    }
    #if defined LGFX_ONLY
      SDUTextStyle.textcolor   = SDU_GFX.getTextStyle().fore_rgb888;
      SDUTextStyle.textbgcolor = SDU_GFX.getTextStyle().back_rgb888;
      SDUTextStyle.textdatum   = SDU_GFX.getTextStyle().datum;
      SDUTextStyle.textsize    = SDU_GFX.getTextStyle().size_x;
    #else // Chimera-Core, TFT_eSPI, M5Stack, M5Core2
      SDUTextStyle.textsize    = SDU_GFX.textsize;
      SDUTextStyle.textdatum   = SDU_GFX.textdatum;
      SDUTextStyle.textcolor   = SDU_GFX.textcolor;
      SDUTextStyle.textbgcolor = SDU_GFX.textbgcolor;
    #endif

    SDUTextStyle.frozen = true;
    log_d("Froze textStyle, size: %d, datum: %d, color: 0x%08x, bgcolor: 0x%08x", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
  }

  static void thawTextStyle()
  {
    SDU_GFX.setTextSize( SDUTextStyle.textsize);
    SDU_GFX.setTextDatum( SDUTextStyle.textdatum );
    SDU_GFX.setTextColor( SDUTextStyle.textcolor , SDUTextStyle.textbgcolor );
    #ifndef _M5STICKC_H_
      SDU_GFX.setFont( nullptr );
    #endif
    SDU_GFX.setCursor(0,0);
    SDU_GFX.fillScreen(TFT_BLACK);
    log_d("Thawed textStyle, size: %d, datum: %d, color: 0x%08x, bgcolor: 0x%08x", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
    SDUTextStyle.frozen = false;
  }

  __attribute__((unused))
  static void drawSDUSplashPage( const char* msg )
  {
    BtnStyles *bs = ( userBtnStyle == nullptr ) ? &DefaultBtnStyle : userBtnStyle;
    SDU_GFX.setTextColor( bs->MsgFontFolor[0], bs->MsgFontFolor[1] );
    SDU_GFX.setTextSize( bs->MsgFontSize );
    SDU_GFX.setTextDatum( TL_DATUM );
    SDU_GFX.setTextFont( 0 );
    SDU_GFX.setTextColor( TFT_WHITE );
    //SDU_GFX.drawJpg( sdUpdaterIcon15x16_jpg, sdUpdaterIcon15x16_jpg_len, 1, 0, 15, 16 );
    SDU_GFX.drawString( msg, 24, 0 );
    #if defined SDU_APP_NAME
      SDU_GFX.setTextColor( TFT_LIGHTGREY );
      //SDU_GFX.drawJpg( sdUpdaterIcon15x16_jpg, sdUpdaterIcon15x16_jpg_len, 1, 24, 15, 16 );
      SDU_GFX.drawString( SDU_APP_NAME, 24, 24 );
    #endif
    #if defined SDU_APP_PATH
      SDU_GFX.setTextColor( TFT_DARKGREY );
      //SDU_GFX.drawJpg( sdUpdaterIcon15x16_jpg, sdUpdaterIcon15x16_jpg_len, 1, 48, 15, 16 );
      SDU_GFX.drawString( SDU_APP_PATH, 24, 48 );
    #endif
    //SDU_GFX.drawJpg( sdUpdaterIcon32x40_jpg, sdUpdaterIcon32x40_jpg_len, (SDU_GFX.width()/2)-16, (SDU_GFX.height()/2)-20, 32, 40 );
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
      // Dummy function, use SDUCfg.onWaitForAction( fn ) to override
      return -1;
    }
  #else // M5.BtnA/BtnB/BtnC support
    __attribute__((unused))
    static int assertStartUpdateFromPushButton( char* labelLoad, char* labelSkip, unsigned long waitdelay )
    {
      if( waitdelay > 100 ) {
        SDUCfg.onBefore();
        SDUCfg.onSplashPage( BTN_HINT_MSG );
        BtnStyles btns;
        SDUCfg.onButtonDraw( labelLoad, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor );
        SDUCfg.onButtonDraw( labelSkip, 1, btns.Skip.BorderColor, btns.Skip.FillColor, btns.Skip.TextColor );
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


  static void SDMenuProgressUI( int state, int size )
  {
    static int SD_UI_Progress;

    int progress_w = 102;
    int progress_h = 20;
    int progress_x = (SDU_GFX.width() - progress_w) >> 1;
    int progress_y = (SDU_GFX.height()- progress_h) >> 1;

    if( state == -1 && size == -1 ) {
      // clear progress bar
      SDU_GFX.fillRect( progress_x, progress_y, progress_w, progress_h, TFT_BLACK );
    } else {
      SDU_GFX.drawRect( progress_x, progress_y, progress_w, progress_h, TFT_WHITE );
    }

    int percent = ( state * 100 ) / size;
    if( percent == SD_UI_Progress ) {
      // don't render twice the same value
      return;
    }
    SD_UI_Progress = percent;

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
    SDU_GFX.setTextFont( 0 );
    SDU_GFX.drawString( percentStr, SDU_GFX.width() >> 1, progress_y+progress_h+5 );

    if ( percent >= 0 && percent < 101 ) {
      Serial.print( "." );
    } else {
      Serial.println();
    }
  };



  static void DisplayUpdateUI( const String& label )
  {
    //log_d("Entering DisplayUpdateUI");

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



  static void SetupSDMenuConfig()
  {
    if( SDUCfg.load_defaults ) {

      if( !SDUCfg.onProgress   ) SDUCfg.setProgressCb(   SDMenuProgressUI );  log_d("Attaching onProgress");
      if( !SDUCfg.onMessage    ) SDUCfg.setMessageCb(    DisplayUpdateUI );   log_d("Attaching onMessage");
      if( !SDUCfg.onError      ) SDUCfg.setErrorCb(      DisplayErrorUI );    log_d("Attaching onError");
      if( !SDUCfg.onBefore     ) SDUCfg.setBeforeCb(     freezeTextStyle );   log_d("Attaching onBefore");
      if( !SDUCfg.onAfter      ) SDUCfg.setAfterCb(      thawTextStyle );     log_d("Attaching onAfter");
      if( !SDUCfg.onSplashPage ) SDUCfg.setSplashPageCb( drawSDUSplashPage ); log_d("Attaching onSplashPage");
      if( !SDUCfg.onButtonDraw ) SDUCfg.setButtonDrawCb( drawSDUPushButton ); log_d("Attaching onButtonDraw");

      #if defined HAS_TOUCH || defined _M5Core2_H_ // default touch button support
        if ( !SDUCfg.onWaitForAction) SDUCfg.setWaitForActionCb( assertStartUpdateFromTouchButton ); log_d("Attaching onWaitForAction (touch)");
      #else // default momentary button support
        if ( !SDUCfg.onWaitForAction) SDUCfg.setWaitForActionCb( assertStartUpdateFromPushButton ); log_d("Attaching onWaitForAction (button)");
      #endif
    }
    #if defined LGFX_ONLY
      if( SDuGFX == nullptr ) {
        log_e("No LGFX driver found, use setSDUGfx( &tft ) to load it");
      }
    #endif
  }


  // release 'tft' temporary define
  #if defined undef_tft
    #undef SDU_GFX
  #endif

#else // headless

  static void SetupSDMenuConfig()
  {
    if( SDUCfg.load_defaults ) {
      if( !SDUCfg.onProgress      ) { SDUCfg.setProgressCb(      SDMenuProgressHeadless );      log_d("Attaching onProgress"); }
      if( !SDUCfg.onMessage       ) { SDUCfg.setMessageCb(       DisplayUpdateHeadless );       log_d("Attaching onMessage"); }
      if( !SDUCfg.onWaitForAction ) { SDUCfg.setWaitForActionCb( assertStartUpdateFromSerial ); log_d("Attaching onWaitForAction (serial)"); }
    }
  }

#endif //#ifdef USE_DISPLAY


#endif //  _M5UPDATER_UI_
