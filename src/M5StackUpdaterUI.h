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


// headless methods
__attribute__((unused))
static int assertStartUpdateFromSerial( char* labelLoad,  char* labelSkip, char* labelSave, unsigned long waitdelay )
{
  int64_t msec = millis();
  do {
    if( Serial.available() ) {
      String out = Serial.readStringUntil('\n');
      if(      out == "update" ) return 1;
      else if( out == "rollback") return 0;
      else if( out == "skip" ) return -1;
      else if( out == "save" ) return 2;
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
      #define HAS_LGFX // LFGX family cores/drivers (ESP32-Chimera-Core, LovyanGFX, M5GFX, M5Unified)
      #define SDUSprite LGFX_Sprite
    #endif
  #else
    #ifndef SDU_GFX
      #define undef_tft
      // recast from reference, M5.Lcd can be either from Chimera-Core or M5Cores (or even TFT_eSPI)
      #if defined __M5UNIFIED_HPP__
        #define SDU_GFX M5.Display // M5Unified has a different namespace but LGFX compatible API
        #define HAS_LGFX // LFGX family cores/drivers (ESP32-Chimera-Core, LovyanGFX, M5GFX, M5Unified)
        #define SDUSprite LGFX_Sprite
      #else
        #define SDU_GFX M5.Lcd // can be either M5.Lcd from M5Core.h, M5Core2.h, M5StickC.h or ESP32-Chimera-Core.h
        #if defined _CHIMERA_CORE_H_
          #define HAS_LGFX // LFGX family cores/drivers (ESP32-Chimera-Core, LovyanGFX, M5GFX, M5Unified)
          #define SDUSprite LGFX_Sprite
        #else
          #define SDUSprite TFT_eSprite
        #endif
      #endif
    #endif
  #endif

  // theme selector
  #ifdef ARDUINO_ODROID_ESP32 // Odroid-GO has 4 buttons under the TFT
    #define BUTTON_WIDTH 60
    #define BUTTON_HWIDTH BUTTON_WIDTH/2 // button half width
    #define BUTTON_HEIGHT 28
    static int16_t SDUButtonsXOffset[4] = {
      1, 72, 188, 260
    };
    static int16_t SDUButtonsYOffset[4] = {
      0, 0, 0, 0
    };
  #else // assuming landscape mode /w 320x240 display
    #define BUTTON_WIDTH 60
    #define BUTTON_HWIDTH BUTTON_WIDTH/2 // button half width
    #define BUTTON_HEIGHT 28
    static int16_t SDUButtonsXOffset[3] = {
      31, 126, 221
    };
    static int16_t SDUButtonsYOffset[3] = {
      0, 0, 0
    };
  #endif


  struct SplashPageElementStyle
  {
    uint16_t textColor;
    uint16_t bgColor;
    uint16_t fontSize;
    uint16_t textDatum;
    uint16_t colorStart; // gradient color start
    uint16_t colorEnd;   // gradient color end
  };

  struct ProgressBarStyle
  {
    int      width;
    int      height;
    bool     clipText;
    uint16_t borderColor;
    uint16_t fillColor;
    uint8_t  fontNumber;
    uint8_t  textDatum;
    uint8_t  textSize;
    uint16_t textColor;
    uint16_t bgColor;
  };

  struct BtnStyle
  {
    uint16_t BorderColor;
    uint16_t FillColor;
    uint16_t TextColor;
  };

  struct SDUTextStyle_T // somehow redundant with LGFX's textstyle_t
  {
    bool frozen           = false;
    uint8_t textsize      = 0;
    uint8_t textdatum     = 0;
    uint32_t textcolor    = 0;
    uint32_t textbgcolor  = 0;
  };

  struct BtnStyles
  {
    BtnStyle Load = { TFT_ORANGE,                          SDU_GFX.color565( 0xaa, 0x00, 0x00), SDU_GFX.color565( 0xdd, 0xdd, 0xdd) };
    BtnStyle Skip = { SDU_GFX.color565( 0x11, 0x11, 0x11), SDU_GFX.color565( 0x33, 0x88, 0x33), SDU_GFX.color565( 0xee, 0xee, 0xee) };
    BtnStyle Save = { TFT_ORANGE,                          TFT_BLACK,                           TFT_WHITE };
    uint16_t height          = BUTTON_HEIGHT;
    uint16_t width           = BUTTON_WIDTH;
    uint16_t hwidth          = BUTTON_HWIDTH;
    uint8_t  FontSize        = 1; // buttons font size
    uint8_t  MsgFontSize     = 2; // welcome message font size
    uint16_t MsgFontFolor[2] = {TFT_WHITE, TFT_BLACK}; // foreground, background
  };

  static BtnStyles DefaultBtnStyle;
  static BtnStyles *userBtnStyle = nullptr;

  static SplashPageElementStyle SplashTitleStyle   = { TFT_BLACK,     TFT_WHITE, 2, MC_DATUM, TFT_LIGHTGREY, TFT_DARKGREY };
  static SplashPageElementStyle SplashAppNameStyle = { TFT_LIGHTGREY, TFT_BLACK, 2, BC_DATUM, 0, 0 };
  static SplashPageElementStyle SplashAppPathStyle = { TFT_DARKGREY,  TFT_BLACK, 1, TC_DATUM, 0, 0 };

  static ProgressBarStyle ProgressStyle = {
    200,        // width
    8,          // height
    true,       // clip "xx%" text
    TFT_LIGHTGREY,  // border color
    TFT_DARKGREY,  // fill color
    0,          // font number
    TC_DATUM,   // text alignment
    1,          // text size
    TFT_WHITE,  // text color
    TFT_BLACK   // text bgcolor
  };

  static void SDMenuProgressUI( int state, int size );
  static void DisplayUpdateUI( const String& label );
  static void DisplayErrorUI( const String& msg, unsigned long wait );
  static SDUTextStyle_T SDUTextStyle;

  static void freezeTextStyle()
  {
    if( SDUTextStyle.frozen ) {
      // log_v("can't freeze twice, thaw first !");
      return;
    }
    #if defined HAS_LGFX // LFGX family cores/drivers (ESP32-Chimera-Core, LovyanGFX, M5GFX, M5Unified)
      SDUTextStyle.textcolor   = SDU_GFX.getTextStyle().fore_rgb888;
      SDUTextStyle.textbgcolor = SDU_GFX.getTextStyle().back_rgb888;
      SDUTextStyle.textdatum   = SDU_GFX.getTextStyle().datum;
      SDUTextStyle.textsize    = SDU_GFX.getTextStyle().size_x;
    #else // TFT_eSPI.h, M5Stack.h, M5Core2.h
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
    #if defined HAS_LGFX // LFGX family cores/drivers (ESP32-Chimera-Core, LovyanGFX, M5GFX, M5Unified)
      SDU_GFX.setFont( nullptr );
    #endif
    SDU_GFX.setCursor(0,0);
    SDU_GFX.fillScreen(TFT_BLACK);
    log_d("Thawed textStyle, size: %d, datum: %d, color: 0x%08x, bgcolor: 0x%08x", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
    SDUTextStyle.frozen = false;
  }

  __attribute__((unused))
  static void drawSDUSplashElement( const char* msg, int32_t x, int32_t y, SplashPageElementStyle *style )
  {
    SDU_GFX.setTextSize( style->fontSize );
    SDU_GFX.setTextDatum( style->textDatum );
    uint8_t lineHeight = SDU_GFX.fontHeight()*1.8;

    #if defined HAS_LGFX
      if( style->colorStart == style->colorEnd ) {
        SDU_GFX.fillRect( 0, y, SDU_GFX.width(), lineHeight, style->bgColor );
      } else {
        for( int i=y; i<y+lineHeight; i++ ) {
          SDU_GFX.drawGradientHLine( 0, i, SDU_GFX.width(), style->colorStart, style->colorEnd );
        }
      }
    #else
      SDU_GFX.fillRect( 0, y, SDU_GFX.width(), lineHeight, style->bgColor );
    #endif

    if( style->textColor != TFT_DARKGREY ) {
      // drop shadow first
      SDU_GFX.setTextColor( TFT_DARKGREY );
      SDU_GFX.drawString( msg, x+1, 1+y+lineHeight/2 );
    }
    // now draw text
    SDU_GFX.setTextColor( style->textColor );
    SDU_GFX.drawString( msg, x, y+lineHeight/2 );
  }

  __attribute__((unused))
  static void drawSDUSplashPage( const char* msg )
  {
    int32_t centerX = SDU_GFX.width()/2;
    SDU_GFX.setTextFont( 1 );

    drawSDUSplashElement( msg, centerX, 0, &SplashTitleStyle );

    if( SDUCfg.appName != nullptr ) {
      drawSDUSplashElement( SDUCfg.appName, centerX, SDU_GFX.height()/4, &SplashAppNameStyle  );
    }
    if( SDUCfg.binFileName != nullptr ) {
      int32_t posY = (SDU_GFX.height() >> 1)-( ProgressStyle.height+2+SDU_GFX.fontHeight() );
      drawSDUSplashElement( String("File name: " + String(&SDUCfg.binFileName[1])).c_str(), centerX, posY, &SplashAppPathStyle );
    }
    //SDU_GFX.drawJpg( sdUpdaterIcon32x40_jpg, sdUpdaterIcon32x40_jpg_len, (centerX)-16, (SDU_GFX.height()/2)-20, 32, 40 );
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


  #if defined LGFX_ONLY // has no push button logic
    __attribute__((unused))
    static int assertStartUpdateFromPushButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay )
    {
      // Dummy function, use SDUCfg.onWaitForAction( fn ) to override
      return -1;
    }
  #else // M5.BtnA/BtnB/BtnC support
    __attribute__((unused))
    static int assertStartUpdateFromPushButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay )
    {
      if( waitdelay > 100 ) {
        SDUCfg.onBefore();
        SDUCfg.onSplashPage( BTN_HINT_MSG );
        BtnStyles btns;
        SDUCfg.onButtonDraw( labelLoad, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor );
        SDUCfg.onButtonDraw( labelSkip, 1, btns.Skip.BorderColor, btns.Skip.FillColor, btns.Skip.TextColor );
        if( SDUCfg.binFileName != nullptr ) {
          SDUCfg.onButtonDraw( labelSave, 2, btns.Save.BorderColor, btns.Save.FillColor, btns.Save.TextColor );
        }
      }
      auto msec = millis();
      auto lastdraw = millis();
      uint32_t progress = 0, progressOld = 1;
      if( SDUCfg.onProgress ) SDUCfg.onProgress( 100, 100 );

      SDUSprite *sprite = new SDUSprite( &SDU_GFX );
      sprite->createSprite( 32, 32 );

      float angle = 0;
      int ret = -1;
      do {
        M5.update();
        if( M5.BtnA.isPressed() ) { ret = 1; break; }
        if( M5.BtnB.isPressed() ) { ret = 0; break; }
        if( SDUCfg.binFileName != nullptr ) {
          // copy binary to SD
          #ifndef _M5STICKC_H_ // M5StickC has not BtnC
            if( M5.BtnC.isPressed() ) { ret = 2; break; }// Force copy bin
          #endif
        }

        if( SDUCfg.onProgress   ) {
          float barprogress = float(millis() - msec) / float(waitdelay);
          progress = 100- (100 * barprogress);
          if (progressOld != progress) {
            progressOld = progress;
            SDUCfg.onProgress( (uint8_t)progress, 100 );
          }
        }
        #if defined HAS_LGFX
          angle = sin( float(millis())/500.0 )*180.0; // 1/2 round per second
          sprite->clear();
          sprite->pushImageRotateZoom(sprite->width()/2, sprite->height()/2, 7.5, 8, angle, 1, 1, 15, 16, sdUpdaterIcon15x16_raw);
          sprite->pushSprite( SDU_GFX.width()/2-sprite->width()/2, SDU_GFX.height()*.75-sprite->height() );
          lastdraw = millis();
        #endif
      } while (millis() - msec < waitdelay);
      if( SDUCfg.onProgress ) SDUCfg.onProgress( 0, 100 );
      sprite->deleteSprite();
      return ret;
    }
  #endif


  #if defined _M5Core2_H_ // M5Core2.h additional touch support, will soon be deprecated

    #include "M5StackUpdaterUITouch.h"

  #endif


  static void SDMenuProgressUI( int state, int size )
  {
    static int SD_UI_Progress;

    int posX = (SDU_GFX.width() - (ProgressStyle.width+2)) >> 1;
    int posY = (SDU_GFX.height()- (ProgressStyle.height+2)) >> 1;

    if( state <=0 || size <=0 ) {
      // clear progress bar
      SDU_GFX.fillRect( posX, posY, ProgressStyle.width+2, ProgressStyle.height+2, ProgressStyle.bgColor );
    } else {
      // draw frame
      SDU_GFX.drawRect( posX, posY, ProgressStyle.width+2, ProgressStyle.height+2, ProgressStyle.borderColor );
    }

    int offset = ( state * ProgressStyle.width ) / size;
    int percent = ( state * 100 ) / size;
    if( offset == SD_UI_Progress ) {
      // don't render twice the same value
      return;
    }
    SD_UI_Progress = offset;

    if ( offset >= 0 && offset <= ProgressStyle.width ) {
      SDU_GFX.fillRect( posX+1,        posY+1, offset,                     ProgressStyle.height, ProgressStyle.fillColor );
      SDU_GFX.fillRect( posX+1+offset, posY+1, ProgressStyle.width-offset, ProgressStyle.height, ProgressStyle.bgColor );
    } else {
      SDU_GFX.fillRect( posX+1,        posY+1, ProgressStyle.width,        ProgressStyle.height, ProgressStyle.bgColor );
    }
    if( ProgressStyle.clipText ) {
      String percentStr = " " + String( percent ) + "% ";
      SDU_GFX.setTextFont( ProgressStyle.fontNumber );
      SDU_GFX.setTextDatum( ProgressStyle.textDatum );
      SDU_GFX.setTextSize( ProgressStyle.textSize );
      SDU_GFX.setTextColor( ProgressStyle.textColor, ProgressStyle.bgColor );
      SDU_GFX.drawString( percentStr, SDU_GFX.width() >> 1, posY+ProgressStyle.height+SDU_GFX.fontHeight() );
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
    //int progress_w = 102;
    //int progress_h = 20;
    int posX = (SDU_GFX.width() - ProgressStyle.width+2) >> 1;
    int posY = (SDU_GFX.height()- ProgressStyle.height+2) >> 1;
    SDU_GFX.setCursor( xpos, posY - 20 );
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

      #if defined SDU_HAS_TOUCH // default touch button support
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

  // release 'SDU_GFX' temporary define
  #if defined undef_tft
    #undef SDU_GFX
  #endif

#else // USE_DISPLAY is undefined => headless

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
