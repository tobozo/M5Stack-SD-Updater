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
    BtnStyle Load = { TFT_ORANGE,                      SDU_GFX.color565( 0xaa, 0x00, 0x00), SDU_GFX.color565( 0xdd, 0xdd, 0xdd) };
    BtnStyle Skip = { SDU_GFX.color565( 0x11, 0x11, 0x11), SDU_GFX.color565( 0x33, 0x88, 0x33), SDU_GFX.color565( 0xee, 0xee, 0xee) };
    uint16_t height          = BUTTON_HEIGHT;
    uint16_t width           = BUTTON_WIDTH;
    uint16_t hwidth          = BUTTON_HWIDTH;
    uint8_t  FontSize        = 1; // buttons font size
    uint8_t  MsgFontSize     = 2; // welcome message font size
    uint16_t MsgFontFolor[2] = {TFT_WHITE, TFT_BLACK}; // foreground, background
  };

  BtnStyles DefaultBtnStyle;
  static BtnStyles *userBtnStyle = nullptr;

  static void SDMenuProgressUI( int state, int size );
  //static void checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay = 2000  );
  static void DisplayUpdateUI( const String& label );
  //static void SDMenuProgressUI( int state, int size );

  static void rollBackUI()
  {
    if( Update.canRollBack() ) {
      DisplayUpdateUI( "RE-LOADING APP" );
      for( uint8_t i=1; i<50; i++ ) {
        SDMenuProgressUI( i, 100 );
      }
      for( uint8_t i=50; i<=100; i++ ) {
        SDMenuProgressUI( i, 100 );
      }
      Update.rollBack();
      ESP.restart();
    } else {
      log_n("Cannot rollback: the other OTA partition doesn't seem to be populated or valid");
      // TODO: better error hint (e.g. message) on the display
      SDU_GFX.fillScreen( TFT_RED );
      delay(1000);
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
    #if __has_include(<LovyanGFX.h>)
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
  static void drawSDUMessage()
  {
    BtnStyles *bs = ( userBtnStyle == nullptr ) ? &DefaultBtnStyle : userBtnStyle;
    SDU_GFX.setTextColor( bs->MsgFontFolor[0], bs->MsgFontFolor[1] );
    SDU_GFX.setTextSize( bs->MsgFontSize );
    //SDU_GFX.setCursor( SDU_GFX.width()/2, 0 );
    SDU_GFX.setTextDatum( TC_DATUM );
    SDU_GFX.setTextFont( 0 );
    SDU_GFX.drawString( BTN_HINT_MSG, SDU_GFX.width()/2, 0 );
  }
  __attribute__((unused))
  static void drawSDUPushButton( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor ) {
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
    //SDU_GFX.drawCentreString( label, SDUButtonsXOffset[position] + bs->hwidth, SDU_GFX.height() - bs->height + 4 - SDUButtonsYOffset[position], 2 );
  }


  #if defined LGFX_ONLY
    __attribute__((unused))
    static int assertStartUpdateFromPushButton( char* labelLoad, char* labelSkip, unsigned long waitdelay )
    {
      // Dummy function, see checkSDUpdaterUI() or its caller to override
      return -1;
    }
  #else // M5.BtnA/BtnB/BtnC support
    __attribute__((unused))
    static int assertStartUpdateFromPushButton( char* labelLoad, char* labelSkip, unsigned long waitdelay )
    {
      if( waitdelay > 100 ) {
        freezeTextStyle();
        drawSDUMessage();
        BtnStyles btns;
        drawSDUPushButton( labelLoad, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor );
        drawSDUPushButton( labelSkip, 1, btns.Skip.BorderColor, btns.Skip.FillColor, btns.Skip.TextColor );
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


  #if defined HAS_TOUCH || defined _M5Core2_H_ // ESP32-Chimera-Core (TODO: add LGFX_ONLY) touch support

    #if defined _M5Core2_H_
      // use TFT_eSPI_Touch emulation from M5Core2.h
      #define SDU_TouchButton TFT_eSPI_Button
    #else
      // use TouchButton/LGFX_Button from ESP32-Chimera-Core/LovyanGFX
      #define SDU_TouchButton TouchButton
    #endif

    struct TouchStyles {
      int padx    = 4,                                    // buttons padding X
          pady    = 5,                                    // buttons padding Y
          marginx = 4,                                    // buttons margin X
          marginy = 4,                                    // buttons margin Y
          x1      = marginx + SDU_GFX.width()/4,              // button 1 X position
          x2      = marginx+SDU_GFX.width()-SDU_GFX.width()/4,    // button 2 X position
          y       = SDU_GFX.height()/2,                       // buttons Y position
          w       = (SDU_GFX.width()/2)-(marginx*2),          // buttons width
          h       = SDU_GFX.height()/4,                       // buttons height
          icon_x  = marginx+12,                           // icon (button 1) X position
          icon_y  = y-8,                                  // icon (button 1) Y position
          pgbar_x = SDU_GFX.width()/2+(marginx*2)+(padx*2)-1, // progressbar X position
          pgbar_y = (y+h/2)+(marginy*2)-1,                // progressbar Y position
          pgbar_w = w-(marginx*4)-(padx*4),               // progressbar width
          btn_fsize = (SDU_GFX.width()>240?2:1)               // touch buttons font size
      ;
      BtnStyle Load = { TFT_ORANGE,                      SDU_GFX.color565( 0xaa, 0x00, 0x00), SDU_GFX.color565( 0xdd, 0xdd, 0xdd) };
      BtnStyle Skip = { SDU_GFX.color565( 0x11, 0x11, 0x11), SDU_GFX.color565( 0x33, 0x88, 0x33), SDU_GFX.color565( 0xee, 0xee, 0xee) };
    };

    struct TouchButtonWrapper
    {
      bool iconRendered = false;

      void handlePressed( SDU_TouchButton *btn, bool pressed, uint16_t x, uint16_t y)
      {
        if (pressed && btn->contains(x, y)) {
          log_d("Press at [%d:%d]", x, y );
          btn->press(true); // tell the button it is pressed
        } else {
          if( pressed ) {
            log_d("Outside Press at [%d:%d]", x, y );
          }
          btn->press(false); // tell the button it is NOT pressed
        }
      }

      void handleJustPressed( SDU_TouchButton *btn, const char* label )
      {
        if ( btn->justPressed() ) {
          btn->drawButton(true, label);
          pushIcon( label );
        }
      }

      bool justReleased( SDU_TouchButton *btn, bool pressed, const char* label )
      {
        bool ret = false;
        if ( btn->justReleased() && (!pressed)) {
          // callable
          ret = true;
        } else if ( btn->justReleased() && (pressed)) {
          // state change but not callable
          ret = false;
        } else {
          // no change, no need to draw
          return false;
        }
        btn->drawButton(false, label);
        pushIcon( label );
        return ret;
      }

      void pushIcon(const char* label)
      {
        if( strcmp( label, LAUNCHER_LABEL ) == 0 || strcmp( label, ROLLBACK_LABEL ) == 0 )
        {
          TouchStyles bs;
          auto IconSprite = TFT_eSprite( &SDU_GFX );
          IconSprite.createSprite(15,16);
          IconSprite.drawJpg(sdUpdaterIcon15x16_jpg, sdUpdaterIcon15x16_jpg_len, 0,0, 15, 16);
          IconSprite.pushSprite( bs.icon_x, bs.icon_y, SDU_GFX.color565( 0x01, 0x00, 0x80 ) );
          IconSprite.deleteSprite();
        }
      }
    }; // end struct TouchButtonWrapper

    static int assertStartUpdateFromTouchButton( char* labelLoad, char* labelSkip, unsigned long waitdelay=5000 )
    {
      /* auto &tft = M5.Lcd; */
      if( waitdelay == 0 ) return -1;
      // chimera core any-touch support + buttons

      static SDU_TouchButton *LoadBtn = new SDU_TouchButton();
      static SDU_TouchButton *SkipBtn = new SDU_TouchButton();

      TouchButtonWrapper tbWrapper;
      TouchStyles ts;

      #if defined _M5Core2_H_
        LoadBtn->setFont(nullptr);
        SkipBtn->setFont(nullptr);
      #endif

      LoadBtn->initButton(
        &SDU_GFX,
        ts.x1, ts.y,  ts.w, ts.h,
        ts.Load.BorderColor, ts.Load.FillColor, ts.Load.TextColor,
        labelLoad, ts.btn_fsize
      );
      SkipBtn->initButton(
        &SDU_GFX,
        ts.x2, ts.y,  ts.w, ts.h,
        ts.Skip.BorderColor, ts.Skip.FillColor, ts.Skip.TextColor,
        labelSkip, ts.btn_fsize
      );

      LoadBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);
      SkipBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);

      LoadBtn->drawButton();
      SkipBtn->drawButton();

      LoadBtn->press(false);
      SkipBtn->press(false);

      uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
      bool ispressed = false;
      int retval = -1; // return status

      SDU_GFX.drawFastHLine( ts.pgbar_x, ts.pgbar_y, ts.pgbar_w-1, TFT_WHITE );

      auto msectouch = millis();
      do {

        if( tbWrapper.iconRendered == false ) {
          tbWrapper.pushIcon( labelLoad );
          tbWrapper.iconRendered = true;
        }
        tbWrapper.handlePressed( LoadBtn, ispressed, t_x, t_y );
        tbWrapper.handlePressed( SkipBtn, ispressed, t_x, t_y );
        tbWrapper.handleJustPressed( LoadBtn, labelLoad );
        tbWrapper.handleJustPressed( SkipBtn, labelSkip );

        if( tbWrapper.justReleased( LoadBtn, ispressed, labelLoad ) ) {
          retval = 1;
          log_n("LoadBTN Pressed at [%d:%d]!", t_x, t_y);
          break;
        }
        if( tbWrapper.justReleased( SkipBtn, ispressed, labelSkip ) ) {
          retval = 0;
          log_n("SkipBTN Pressed at [%d:%d]!", t_x, t_y);
          break;
        }

        #if __has_include("lgfx/v1/Touch.hpp") // LovyanGFX
          lgfx::touch_point_t tp;
          uint16_t number = SDU_GFX.getTouch(&tp, 1);
          t_x = tp.x;
          t_y = tp.y;
          ispressed = number > 0;
        #else // M5Core2.h
          ispressed = SDU_GFX.getTouch(&t_x, &t_y);
        #endif

        float barprogress = float(millis() - msectouch) / float(waitdelay);
        int linewidth = float(ts.pgbar_w) * barprogress;
        if( linewidth > 0 ) {
          int linepos = ts.pgbar_w - ( linewidth +1 );
          uint16_t grayscale = 255 - (192*barprogress);
          SDU_GFX.drawFastHLine( ts.pgbar_x,         ts.pgbar_y, ts.pgbar_w-linewidth-1, SDU_GFX.color565( grayscale, grayscale, grayscale ) );
          SDU_GFX.drawFastHLine( ts.pgbar_x+linepos, ts.pgbar_y, 1,                      TFT_BLACK );
        }

      } while (millis() - msectouch < waitdelay);
      return retval;
    }

  #endif // HAS_TOUCH

  static void checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay, const int TFCardCsPin )
  {
    /* auto &tft = M5.Lcd; */
    log_n("Booting with reset reason: %d", resetReason );

    bool isRollBack = true;
    if( fileName != "" ) {
      isRollBack = false;
    }


    #if defined HAS_TOUCH || defined _M5Core2_H_ // default touch button support
      if( SDUpdaterAssertTrigger==nullptr ) setAssertTrigger( &assertStartUpdateFromTouchButton );
      // TODO: see if "touch/press" detect on boot is possible (spoil : NO)
      // TODO: read signal from other (external?) buttons
      bool draw = true;
      if( waitdelay == 0 ) return; // don't check for any touch/button signal if waitDelay = 0
    #else // default push buttons suppor
      if( !SDUpdaterAssertTrigger ) setAssertTrigger( &assertStartUpdateFromPushButton );
      bool draw = false;
      if( waitdelay == 0 ) return; // don't check for any touch/button signal if waitDelay = 0
      if( waitdelay <= 100 ) {
        // no UI draw, but attempt to detect "button is pressed on boot"
        // also force some minimal delay
        waitdelay = 100;
      } else {
        // only draw if waitdelay > 0
        draw = true;
      }
    #endif

    if( draw ) { // bring up the UI
      // TODO: UI draw callback ?
      freezeTextStyle();
      drawSDUMessage();
    }

    if ( SDUpdaterAssertTrigger( isRollBack ? (char*)ROLLBACK_LABEL : (char*)LAUNCHER_LABEL,  (char*)SKIP_LABEL, waitdelay ) == 1 ) {
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
      thawTextStyle();
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
    /* auto &tft = M5.Lcd; */
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

  // release 'tft' temporary define
  #if defined undef_tft
    #undef SDU_GFX
  #endif

#else
  // unknown tft context, go headless
  #undef USE_DISPLAY
#endif


#endif //  _M5UPDATER_UI_
