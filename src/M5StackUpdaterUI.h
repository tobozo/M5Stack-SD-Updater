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


#if defined _CHIMERA_CORE_ || defined _M5STICKC_H_ || defined _M5STACK_H_ || defined _M5Core2_H_ //|| defined LGFX_ONLY


  #if defined LGFX_ONLY
    static int assertStartUpdateFromButton( char* labelLoad, char* labelSkip )
    {
      // Dummy function, see checkSDUpdaterUI() or its caller to override
      return -1;
    }
  #else
    #ifndef tft
      #define undef_tft
      #define tft M5.Lcd
    #endif
    static int assertStartUpdateFromButton( char* labelLoad, char* labelSkip )
    {
      M5.update();
      if( M5.BtnA.isPressed() ) return 1;
      if( M5.BtnB.isPressed() ) return 0;
      return -1;
    }
  #endif

  #ifdef ARDUINO_ODROID_ESP32 // odroid has 4 buttons under the TFT
    #define BUTTON_WIDTH 60
    #define BUTTON_HWIDTH BUTTON_WIDTH/2 // 30
    #define BUTTON_HEIGHT 28
    static uint16_t SDUButtonsXOffset[4] = {
      1, 72, 188, 260
    };
  #else
    #define BUTTON_WIDTH 60
    #define BUTTON_HWIDTH BUTTON_WIDTH/2 // 30
    #define BUTTON_HEIGHT 28
    static uint16_t SDUButtonsXOffset[3] = {
      31, 126, 221
    };
  #endif

  struct BtnStyle {
    uint16_t BorderColor,
             FillColor,
             TextColor
    ;
  };

  struct BtnStyles {
    BtnStyle Load = { TFT_ORANGE,                      tft.color565( 0xaa, 0x00, 0x00), tft.color565( 0xdd, 0xdd, 0xdd) };
    BtnStyle Skip = { tft.color565( 0x11, 0x11, 0x11), tft.color565( 0x33, 0x88, 0x33), tft.color565( 0xee, 0xee, 0xee) };
    uint16_t height          = BUTTON_HEIGHT;
    uint16_t width           = BUTTON_WIDTH;
    uint16_t hwidth          = BUTTON_HWIDTH;
    uint8_t  FontSize        = 1; // buttons font size
    uint8_t  MsgFontSize     = 2; // welcome message font size
    uint16_t MsgFontFolor[2] = {TFT_WHITE, TFT_BLACK}; // foreground, background
  };

  static void SDMenuProgressUI( int state, int size );
  static void checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay = 2000  );
  static void DisplayUpdateUI( const String& label );
  static void SDMenuProgressUI( int state, int size );

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
      tft.fillScreen( TFT_RED );
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
    SDUTextStyle.textsize    = tft.textsize;
    SDUTextStyle.textdatum   = tft.textdatum;
    SDUTextStyle.textcolor   = tft.textcolor;
    SDUTextStyle.textbgcolor = tft.textbgcolor;
    log_d("Froze textStyle, size: %d, datum: %d, color: %d, bgcolor: %d", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
  }
  static void thawTextStyle()
  {
    tft.setTextSize( SDUTextStyle.textsize);
    tft.setTextDatum( SDUTextStyle.textdatum );
    tft.setTextColor( SDUTextStyle.textcolor , SDUTextStyle.textbgcolor );
    #ifndef  _M5STICKC_H_
      tft.setFont( nullptr );
    #endif
    tft.setCursor(0,0);
    tft.fillScreen(TFT_BLACK);
    log_d("Thawed textStyle, size: %d, datum: %d, color: %d, bgcolor: %d", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
  }

  static void drawSDUMessage()
  {
    BtnStyles bs;
    tft.setTextColor( bs.MsgFontFolor[0], bs.MsgFontFolor[1] );
    tft.setTextSize( bs.MsgFontSize );
    tft.setCursor( tft.width()/2, 0 );
    tft.setTextDatum( TC_DATUM );
    tft.setTextFont( 0 );
    tft.drawString( BTN_HINT_MSG, tft.width()/2, 0 );
  }

  static void drawSDUPushButton( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor ) {
    BtnStyles bs;
    tft.setTextColor( textcolor, fillcolor );
    tft.setTextSize( bs.FontSize );
    tft.fillRoundRect( SDUButtonsXOffset[position], tft.height() - bs.height - 2, bs.width, bs.height, 3, fillcolor );
    tft.drawRoundRect( SDUButtonsXOffset[position], tft.height() - bs.height - 2, bs.width, bs.height, 3, outlinecolor );
    tft.drawCentreString( label, SDUButtonsXOffset[position] + bs.hwidth, tft.height() - bs.height + 4, 2 );
  }


  #if defined HAS_TOUCH || defined _M5Core2_H_ // ESP32-Chimera-Core (TODO: add LGFX_ONLY) touch support

    #define SDUpdaterAssertTrigger AnyTouchedButtonOf

    #if defined _M5Core2_H_
      // use TFT_eSPI_Touch emulation from M5Core2.h
      #define SDU_TouchButton TFT_eSPI_Button
    #else
      // use native TouchButton from ESP32-Chimera-Core and methods from LGFX
      #define SDU_TouchButton TouchButton
    #endif

    struct TouchStyles {
      int padx    = 4,                                    // buttons padding X
          pady    = 5,                                    // buttons padding Y
          marginx = 4,                                    // buttons margin X
          marginy = 4,                                    // buttons margin Y
          x1      = marginx + tft.width()/4,              // button 1 X position
          x2      = marginx+tft.width()-tft.width()/4,    // button 2 X position
          y       = tft.height()/2,                       // buttons Y position
          w       = (tft.width()/2)-(marginx*2),          // buttons width
          h       = tft.height()/4,                       // buttons height
          icon_x  = marginx+12,                           // icon (button 1) X position
          icon_y  = y-8,                                  // icon (button 1) Y position
          pgbar_x = tft.width()/2+(marginx*2)+(padx*2)-1, // progressbar X position
          pgbar_y = (y+h/2)+(marginy*2)-1,                // progressbar Y position
          pgbar_w = w-(marginx*4)-(padx*4),               // progressbar width
          btn_fsize = (tft.width()>240?2:1)               // touch buttons font size
      ;
      BtnStyle Load = { TFT_ORANGE,                      tft.color565( 0xaa, 0x00, 0x00), tft.color565( 0xdd, 0xdd, 0xdd) };
      BtnStyle Skip = { tft.color565( 0x11, 0x11, 0x11), tft.color565( 0x33, 0x88, 0x33), tft.color565( 0xee, 0xee, 0xee) };
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
          auto IconSprite = TFT_eSprite( &tft );
          IconSprite.createSprite(15,16);
          IconSprite.drawJpg(sdUpdaterIcon15x16_jpg, sdUpdaterIcon15x16_jpg_len, 0,0, 15, 16);
          IconSprite.pushSprite( bs.icon_x, bs.icon_y, tft.color565( 0x01, 0x00, 0x80 ) );
          IconSprite.deleteSprite();
        }
      }

    };

    static int AnyTouchedButtonOf( char* labelLoad, char* labelSkip, unsigned long waitdelay=5000 )
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
        &tft,
        ts.x1, ts.y,  ts.w, ts.h,
        ts.Load.BorderColor, ts.Load.FillColor, ts.Load.TextColor,
        labelLoad, ts.btn_fsize
      );
      SkipBtn->initButton(
        &tft,
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

      tft.drawFastHLine( ts.pgbar_x, ts.pgbar_y, ts.pgbar_w-1, TFT_WHITE );

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

        ispressed = tft.getTouch(&t_x, &t_y);

        float barprogress = float(millis() - msectouch) / float(waitdelay);
        int linewidth = float(ts.pgbar_w) * barprogress;
        if( linewidth > 0 ) {
          int linepos = ts.pgbar_w - ( linewidth +1 );
          uint16_t grayscale = 255 - (192*barprogress);
          tft.drawFastHLine( ts.pgbar_x,         ts.pgbar_y, ts.pgbar_w-linewidth-1, tft.color565( grayscale, grayscale, grayscale ) );
          tft.drawFastHLine( ts.pgbar_x+linepos, ts.pgbar_y, 1,                      TFT_BLACK );
        }

      } while (millis() - msectouch < waitdelay);
      return retval;
    }

  #else

    #define SDUpdaterAssertTrigger assertStartUpdateFromButton

  #endif // HAS_TOUCH

  static void checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay, const int TFCardCsPin )
  {
    /* auto &tft = M5.Lcd; */
    log_n("Booting with reset reason: %d", resetReason );

    bool isRollBack = true;
    if( fileName != "" ) {
      isRollBack = false;
    }

    #if defined HAS_TOUCH || defined _M5Core2_H_
      // TODO: see if "touch/press" detect on boot is possible (spoil : NO)
      // TODO: read signal from other (external?) buttons
      if( waitdelay == 0 ) return; // don't check for touch signal if waitDelay = 0
      freezeTextStyle();
      drawSDUMessage();
      // bring up Touch UI as there are no buttons to click with
      if( SDUpdaterAssertTrigger( isRollBack ? (char*)ROLLBACK_LABEL : (char*)LAUNCHER_LABEL,  (char*)SKIP_LABEL ) == 1 ) {
        if( isRollBack == false ) {
          Serial.printf( SDU_LOAD_TPL, fileName.c_str() );
          updateFromFS( fs, fileName, TFCardCsPin );
          ESP.restart();
        } else {
          Serial.println( SDU_ROLLBACK_MSG );
          rollBackUI();
        }
      }
      thawTextStyle();
    #else
      bool draw = false;
      if( waitdelay == 0 ) {
        waitdelay = 100;
      } else {
        draw = true;
      }

      if( draw ) {
        // show a hint
        freezeTextStyle();
        drawSDUMessage();
        BtnStyles btns;
        if( isRollBack ) {
          drawSDUPushButton( ROLLBACK_LABEL, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor );
        } else {
          drawSDUPushButton( LAUNCHER_LABEL, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor );
        }
        drawSDUPushButton( SKIP_LABEL, 1, btns.Skip.BorderColor, btns.Skip.FillColor, btns.Skip.TextColor );
      }

      auto msec = millis();
      do {
        if ( assertStartUpdateFromButton( isRollBack ? (char*)ROLLBACK_LABEL : (char*)LAUNCHER_LABEL,  (char*)SKIP_LABEL ) == 1 ) {
          if( isRollBack == false ) {
            Serial.printf( SDU_LOAD_TPL, fileName.c_str() );
            updateFromFS( fs, fileName, TFCardCsPin );
            ESP.restart();
          } else {
            Serial.println( SDU_ROLLBACK_MSG );
            rollBackUI();
          }

        }
      } while (millis() - msec < waitdelay);

      if( draw ) {
        thawTextStyle();
      }
    #endif
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
    int progress_x = (tft.width() - progress_w) >> 1;
    int progress_y = (tft.height()- progress_h) >> 1;
    if ( percent >= 0 && percent < 101 ) {
      tft.fillRect( progress_x+1, progress_y+1, percent, 18, TFT_GREEN );
      tft.fillRect( progress_x+1+percent, progress_y+1, 100-percent, 18, TFT_BLACK );
      Serial.print( "." );
    } else {
      tft.fillRect( progress_x+1, progress_y+1, 100, 18, TFT_BLACK );
      Serial.println();
    }
    String percentStr = " " + String( percent ) + "% ";
    tft.drawCentreString( percentStr , tft.width() >> 1, progress_y+progress_h+5, 0); // trailing space is important
    if ( percent >= 0 && percent < 101 ) {
      Serial.print( "." );
    } else {
      Serial.println();
    }
  };



  static void DisplayUpdateUI( const String& label )
  {
    /* auto &tft = M5.Lcd; */
    if (tft.width() < tft.height()) tft.setRotation(tft.getRotation() ^ 1);

    tft.fillScreen( TFT_BLACK );
    tft.setTextColor( TFT_WHITE, TFT_BLACK );
    tft.setTextFont( 0 );
    tft.setTextSize( 2 );
    // attemtp to center the text
    int16_t xpos = ( tft.width() / 2) - ( tft.textWidth( label ) / 2 );
    if ( xpos < 0 ) {
      // try with smaller size
      tft.setTextSize(1);
      xpos = ( tft.width() / 2 ) - ( tft.textWidth( label ) / 2 );
      if( xpos < 0 ) {
        // give up
        xpos = 0 ;
      }
    }
    int progress_w = 102;
    int progress_h = 20;
    int progress_x = (tft.width() - progress_w) >> 1;
    int progress_y = (tft.height()- progress_h) >> 1;
    tft.setCursor( xpos, progress_y - 20 );
    tft.print( label );
    tft.drawRect( progress_x, progress_y, progress_w, progress_h, TFT_WHITE );
  }

  // release 'tft' temporary define
  #if defined undef_tft
    #undef tft
  #endif

#else
  // unknown tft context, go headless
  #undef USE_DISPLAY
#endif


#endif //  _M5UPDATER_UI_
