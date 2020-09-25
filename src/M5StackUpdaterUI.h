#ifndef _M5UPDATER_UI_
#define _M5UPDATER_UI_

#include "assets.h"

#define ROLLBACK_LABEL "Rollback" // reload app from the "other" OTA partition
#define LAUNCHER_LABEL "Launcher" // load Launcher (typically menu.bin)
#define SKIP_LABEL     "Skip >>|" // resume normal operations (=no action taken)
#define BTN_HINT_TPL   "SDUpdater\npress BtnA for %s"


#if defined _CHIMERA_CORE_|| defined _M5STICKC_H_ || defined _M5STACK_H_ || defined _M5Core2_H_ //|| defined LGFX_ONLY

  #if defined LGFX_ONLY
    static bool assertStartUpdateFromButton()
    {
      // Dummy function, see checkSDUpdaterUI() or its caller to override
      return false;
    }
  #else
    #ifndef tft
      #define undef_tft
      #define tft M5.Lcd
    #endif
    static bool assertStartUpdateFromButton()
    {
      M5.update();
      return M5.BtnA.isPressed();
    }
  #endif

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
      tft.fillScreen(TFT_RED);
      delay(1000);
    }
  }

  #if defined HAS_TOUCH || defined _M5Core2_H_ // ESP32-Chimera-Core (TODO: add LGFX_ONLY) touch support

    #if defined _M5Core2_H_

      #define TOUCHBUTTONS_PADX    4
      #define TOUCHBUTTONS_PADY    5
      #define TOUCHBUTTONS_MARGINX 4
      #define TOUCHBUTTONS_MARGINY 4
      #define TOUCHBUTTONS_W       (tft.width()/2)-(TOUCHBUTTONS_MARGINX*2)
      #define TOUCHBUTTONS_H       tft.height()/4
      #define TOUCHBUTTONS_X1      TOUCHBUTTONS_MARGINX // centered position
      #define TOUCHBUTTONS_X2      TOUCHBUTTONS_MARGINX+tft.width()/2 // centered position
      #define TOUCHBUTTONS_Y       tft.height()/2-(TOUCHBUTTONS_H/2)
      #define TOUCHBUTTONS_ICON_X  TOUCHBUTTONS_MARGINX+16
      #define TOUCHBUTTONS_ICON_Y  TOUCHBUTTONS_Y-8 //+ TOUCHBUTTONS_H/2 - 8
      #define TOUCH_PROGRESSBAR_X  tft.width()/2+(TOUCHBUTTONS_MARGINX*2)+(TOUCHBUTTONS_PADX*2)-1
      #define TOUCH_PROGRESSBAR_Y  tft.height()/2+(TOUCHBUTTONS_H/2)+(TOUCHBUTTONS_MARGINY*2)-1
      #define TOUCH_PROGRESSBAR_W  TOUCHBUTTONS_W-(TOUCHBUTTONS_MARGINX*4)-(TOUCHBUTTONS_PADX*4)
      #define TOUCHBUTTONS_FSIZE   (tft.width()>240?2:1)

      static int AnyTouchedButtonStatus = -1;

      static void LoadBtnTapped(TouchEvent& e)
      {
        /*
        // This creates shorthand "b" for the button in the event, so "e.button->" becomes "b."
        // (Warning: only if you're SURE there's a button with the event, otherwise will crash)
        TouchButton& b = *e.button;
        // Toggles the background between black and blue
        if (b.off.bg == BLACK) b.off.bg = BLUE; else b.off.bg = BLACK;
        b.draw();
        */
        log_w("changing status to 'LOAD'");
        AnyTouchedButtonStatus = 1;
      }

      static void SkipBtnTapped(TouchEvent& e)
      {
        AnyTouchedButtonStatus = 0;
      }



      static int AnyTouchedButtonOf( char* labelLoad, char* labelSkip, unsigned long waitdelay=5000 )
      {
        /*
        #ifndef tft
          auto &tft = M5.Lcd;
        #endif
        */
        if( waitdelay == 0 ) return -1;

        uint8_t  textsize = tft.textsize,  // Current font size multiplier
                textdatum = tft.textdatum; // Text reference datum

        // M5Core2 any-touch support + buttons
        ButtonColors ColorOn = {RED, WHITE, WHITE};
        ButtonColors ColorLoadOff = { tft.color565( 0xaa, 0x00, 0x00), tft.color565( 0xdd, 0xdd, 0xdd), TFT_ORANGE };
        ButtonColors ColorSkipOff = { tft.color565( 0x33, 0x88, 0x33), tft.color565( 0xee, 0xee, 0xee), tft.color565( 0x11, 0x11, 0x11) };

        TouchButton LoadBtn(TOUCHBUTTONS_X1, TOUCHBUTTONS_Y,  TOUCHBUTTONS_W, TOUCHBUTTONS_H, labelLoad, ColorLoadOff, ColorOn, MC_DATUM);
        TouchButton SkipBtn(TOUCHBUTTONS_X2, TOUCHBUTTONS_Y,  TOUCHBUTTONS_W, TOUCHBUTTONS_H, labelSkip, ColorSkipOff, ColorOn, MC_DATUM);

        uint8_t fsize = TOUCHBUTTONS_FSIZE;

        //LoadBtn.setTextSize( fsize );
        //SkipBtn.setTextSize( fsize );

        LoadBtn.draw();
        SkipBtn.draw();

        LoadBtn.addHandler(LoadBtnTapped, TE_TAP + TE_BTNONLY);
        SkipBtn.addHandler(SkipBtnTapped, TE_TAP + TE_BTNONLY);

        tft.drawFastHLine( TOUCH_PROGRESSBAR_X, TOUCH_PROGRESSBAR_Y, TOUCH_PROGRESSBAR_W-1, TFT_WHITE );

        auto msectouch = millis();
        do {
          M5.update();
          float barprogress = float(millis() - msectouch) / float(waitdelay);
          int linewidth = float(TOUCH_PROGRESSBAR_W) * barprogress;
          int linepos = TOUCH_PROGRESSBAR_W - ( linewidth +1 );
          uint16_t grayscale = 255 - (192*barprogress);
          tft.drawFastHLine( TOUCH_PROGRESSBAR_X,         TOUCH_PROGRESSBAR_Y, TOUCH_PROGRESSBAR_W-linewidth-1, tft.color565( grayscale, grayscale, grayscale ) );
          tft.drawFastHLine( TOUCH_PROGRESSBAR_X+linepos, TOUCH_PROGRESSBAR_Y, 1,                               TFT_BLACK );

        } while (AnyTouchedButtonStatus<0 && millis() - msectouch < waitdelay);
        tft.fillScreen(TFT_BLACK);
        // restore initial text style
        tft.setFont( nullptr );
        tft.setTextSize( textsize );
        tft.setTextDatum( textdatum );
        return AnyTouchedButtonStatus;
      }


    #else

      #define TOUCHBUTTONS_PADX    4
      #define TOUCHBUTTONS_PADY    5
      #define TOUCHBUTTONS_MARGINX 4
      #define TOUCHBUTTONS_MARGINY 4
      #define TOUCHBUTTONS_X1      TOUCHBUTTONS_MARGINX+tft.width()/4 // centered position
      #define TOUCHBUTTONS_X2      TOUCHBUTTONS_MARGINX+tft.width()-tft.width()/4 // centered position
      #define TOUCHBUTTONS_Y       tft.height()/2
      #define TOUCHBUTTONS_W       (tft.width()/2)-(TOUCHBUTTONS_MARGINX*2)
      #define TOUCHBUTTONS_H       tft.height()/4
      #define TOUCHBUTTONS_ICON_X  TOUCHBUTTONS_MARGINX+16
      #define TOUCHBUTTONS_ICON_Y  TOUCHBUTTONS_Y-8 //+ TOUCHBUTTONS_H/2 - 8
      #define TOUCH_PROGRESSBAR_X  tft.width()/2+(TOUCHBUTTONS_MARGINX*2)+(TOUCHBUTTONS_PADX*2)-1
      #define TOUCH_PROGRESSBAR_Y  (TOUCHBUTTONS_Y+TOUCHBUTTONS_H/2)+(TOUCHBUTTONS_MARGINY*2)-1
      #define TOUCH_PROGRESSBAR_W  TOUCHBUTTONS_W-(TOUCHBUTTONS_MARGINX*4)-(TOUCHBUTTONS_PADX*4)
      #define TOUCHBUTTONS_FSIZE   (tft.width()>240?2:1)

      struct TouchButtonWrapper
      {

        bool iconRendered = false;

        void handlePressed( TouchButton &btn, bool pressed, uint16_t x, uint16_t y)
        {
          if (pressed && btn.contains(x, y)) {
            btn.press(true);  // tell the button it is pressed
            //Serial.println("Pressed");
          } else {
            btn.press(false);  // tell the button it is NOT pressed
          }
        }

        void handleJustPressed( TouchButton &btn, const char* label )
        {
          if ( btn.justPressed() ) {
            btn.drawButton(true, label);
            pushIcon( label );
            //Serial.println("JustPressed");
          }
        }

        bool justReleased( TouchButton &btn, bool pressed, const char* label )
        {
          bool ret = false;
          if ( btn.justReleased() && (!pressed)) {
            log_w("Callable");
            ret = true;
          } else if ( btn.justReleased() && (pressed)) {
            ret = false;
            //Serial.println("Not callable");
          } else {
            return false;
          }
          btn.drawButton(false, label);
          pushIcon( label );
          return ret;
        }

        void pushIcon(const char* label)
        {
          if( strcmp( label, LAUNCHER_LABEL ) == 0 || strcmp( label, ROLLBACK_LABEL ) )
          {
            auto IconSprite = TFT_eSprite( &tft );
            IconSprite.createSprite(15,16);
            IconSprite.drawJpg(sdUpdaterIcon15x16_jpg, sdUpdaterIcon15x16_jpg_len, 0,0, 15, 16);
            IconSprite.pushSprite( TOUCHBUTTONS_ICON_X, TOUCHBUTTONS_ICON_Y, TFT_BLACK );
            IconSprite.deleteSprite();
          }
        }

      };

      static int AnyTouchedButtonOf( char* labelLoad, char* labelSkip, unsigned long waitdelay=5000 )
      {
        /*
        #ifndef tft
          auto &tft = M5.Lcd;
        #endif
        */
        if( waitdelay == 0 ) return -1;

        uint8_t  textsize = tft.textsize,  // Current font size multiplier
                textdatum = tft.textdatum; // Text reference datum
        // chimera core any-touch support + buttons
        TouchButton LoadBtn;
        TouchButton SkipBtn;
        TouchButtonWrapper tbWrapper;

        LoadBtn.initButton(
          &tft,
          TOUCHBUTTONS_X1, TOUCHBUTTONS_Y,  TOUCHBUTTONS_W, TOUCHBUTTONS_H,  // x, y, w, h
          TFT_ORANGE, // Outline
          tft.color565( 0xaa, 0x00, 0x00), // Fill
          tft.color565( 0xdd, 0xdd, 0xdd), // Text
          labelLoad, TOUCHBUTTONS_FSIZE    // label, fontsize
        );
        SkipBtn.initButton(
          &tft,
          TOUCHBUTTONS_X2, TOUCHBUTTONS_Y, TOUCHBUTTONS_W, TOUCHBUTTONS_H,  // x, y, w, h
          tft.color565( 0x11, 0x11, 0x11), // Outline
          tft.color565( 0x33, 0x88, 0x33), // Fill
          tft.color565( 0xee, 0xee, 0xee), // Text
          labelSkip, TOUCHBUTTONS_FSIZE    // label, fontsize
        );

        LoadBtn.setLabelDatum(TOUCHBUTTONS_PADX, TOUCHBUTTONS_PADY, MC_DATUM);
        SkipBtn.setLabelDatum(TOUCHBUTTONS_PADX, TOUCHBUTTONS_PADY, MC_DATUM);

        LoadBtn.drawButton();
        SkipBtn.drawButton();

        uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
        bool ispressed = false;
        int retval = -1; // return status

        tft.drawFastHLine( TOUCH_PROGRESSBAR_X, TOUCH_PROGRESSBAR_Y, TOUCH_PROGRESSBAR_W-1, TFT_WHITE );

        auto msectouch = millis();
        do {
          ispressed = tft.getTouch(&t_x, &t_y);

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
            break;
          }
          if( tbWrapper.justReleased( SkipBtn, ispressed, labelSkip ) ) {
            retval = 0;
            break;
          }

          float barprogress = float(millis() - msectouch) / float(waitdelay);
          int linewidth = float(TOUCH_PROGRESSBAR_W) * barprogress;
          int linepos = TOUCH_PROGRESSBAR_W - ( linewidth +1 );
          uint16_t grayscale = 255 - (192*barprogress);
          tft.drawFastHLine( TOUCH_PROGRESSBAR_X,         TOUCH_PROGRESSBAR_Y, TOUCH_PROGRESSBAR_W-linewidth-1, tft.color565( grayscale, grayscale, grayscale ) );
          tft.drawFastHLine( TOUCH_PROGRESSBAR_X+linepos, TOUCH_PROGRESSBAR_Y, 1,                               TFT_BLACK );

        } while (millis() - msectouch < waitdelay);
        tft.fillScreen(TFT_BLACK);
        // restore initial text style
        tft.setFont( nullptr );
        tft.setTextSize( textsize );
        tft.setTextDatum( textdatum );
        return retval;
      }

    #endif // !defined _M5Core2_H_
  #endif // HAS_TOUCH



  static void checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay  )
  {
    /*
    #ifndef tft
      auto &tft = M5.Lcd;
    #endif
    */

    log_n("Booting with reset reason: %d", resetReason );

    bool isRollBack = true;
    if( fileName != "" ) {
      isRollBack = false;
    }

    #if defined HAS_TOUCH || defined _M5Core2_H_
      // bring up Touch UI as there are no buttons to click with
      tft.setTextColor( TFT_WHITE, TFT_BLACK );
      //tft.setTextFont( 0 );
      tft.setTextSize( TOUCHBUTTONS_FSIZE );
      tft.setCursor( 160, 0 );
      tft.setTextDatum( TC_DATUM );
      tft.drawString( "SD-Updater Options", tft.width()/2, 0 );
      // todo: figure out a way to freeze/restore text settings
      tft.setTextDatum( TL_DATUM );
      tft.setCursor(0,0);

      if( AnyTouchedButtonOf( isRollBack ? (char*)ROLLBACK_LABEL : (char*)LAUNCHER_LABEL,  (char*)SKIP_LABEL, waitdelay) == 1 ) {
        if( isRollBack == false ) {
          Serial.print("Will Load menu binary : ");
          Serial.println( fileName );
          updateFromFS( fs, fileName );
          ESP.restart();
        } else {
          Serial.println("Will Roll back");
          rollBackUI();
        }
        return;
      }
    #else
      // show a hint
      tft.setCursor(0,0);
      if( isRollBack ) {
        tft.printf( BTN_HINT_TPL, ROLLBACK_LABEL );
      } else {
        tft.printf( BTN_HINT_TPL, LAUNCHER_LABEL );
      }
      tft.setCursor(0,0);

      auto msec = millis();
      do {
        if ( assertStartUpdateFromButton() ) {
          if( isRollBack == false ) {
            Serial.println("Will Load menu binary");
            updateFromFS( fs, fileName );
            ESP.restart();
          } else {
            Serial.println("Will Roll back");
            rollBackUI();
          }

        }
      } while (millis() - msec < waitdelay);
      tft.fillScreen(TFT_BLACK);
    #endif
  }



  static void SDMenuProgressUI( int state, int size )
  {
    static int SD_UI_Progress;
    int percent = ( state * 100 ) / size;
    if( percent == SD_UI_Progress ) {
      // don't render twice the same value
      return;
    }/*
    #ifndef tft
      auto &tft = M5.Lcd;
    #endif
    */
    //Serial.printf("percent = %d\n", percent); // this is spammy
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
    /*
    #ifndef tft
      auto &tft = M5.Lcd;
    #endif
    */
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


  #if defined undef_tft
    #undef tft
  #endif

#else

  #undef USE_DISPLAY

#endif




#endif //  _M5UPDATER_UI_
