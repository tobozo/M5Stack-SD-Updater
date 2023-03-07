
#include "./UI.hpp"



#define FN_LAMBDA_VOID(x) []() { x; }
#define FN_LAMBDA_BOOL(x) []() -> bool { return x; }
#define FB_LAMBDA_FALSE   []() -> bool { return false; }



namespace SDUpdaterNS
{

  namespace ConfigManager
  {


    void config_sdu_t::setDisplay( void *ptr )
    {
      if( ptr ) { // arg provided
        log_d("Setting display with arg provided pointer");
        display = ptr;
      } else { // attempt detection
        if( DISPLAY_OBJ_PTR != nullptr ) { // provided by macro in config
          log_d("Setting display with macro provided pointer");
          display = (void*)DISPLAY_OBJ_PTR;
        } else { // previously provided by user ?
          if(!display) log_e("Display needs manual setup!!");
          else log_d("Inheriting config builtin display pointer");
        }
      }
    }


    void config_sdu_t::setButtons()
    {
      //#if defined DEFAULT_BTN_POLLER
        setSDUBtnPoller( FN_LAMBDA_VOID(DEFAULT_BTN_POLLER) );
      //#endif
      //#if defined DEFAULT_BTNA_CHECKER
        setBtnAChecker( FN_LAMBDA_BOOL(DEFAULT_BTNA_CHECKER) );
      //#endif
      //#if defined DEFAULT_BTNB_CHECKER
        setBtnBChecker( FN_LAMBDA_BOOL(DEFAULT_BTNB_CHECKER) );
      //#endif
      //#if defined DEFAULT_BTNC_CHECKER
        setBtnCChecker( FN_LAMBDA_BOOL(DEFAULT_BTNC_CHECKER) );
      //#endif
    }


    void setup()
    {
      if( !SDUCfg.display ) {
        log_d("No dislay attached, attempting to detect by context");
        SDUCfg.setDisplay( nullptr ); // auto attach default display, if any
      } else {
        log_d("Using config builtin display pointer");
      }
      //SDUCfg.setButtons();

      if( SDUCfg.display ) {
        log_v("Display SD Menu Config");
        if( SDUCfg.load_defaults ) {

          if( !SDUCfg.onProgress   )  { SDUCfg.setProgressCb(   SDU_UI::SDMenuProgressUI );  log_v("Attached onProgress");   }
          if( !SDUCfg.onMessage    )  { SDUCfg.setMessageCb(    SDU_UI::DisplayUpdateUI );   log_v("Attached onMessage");    }
          if( !SDUCfg.onError      )  { SDUCfg.setErrorCb(      SDU_UI::DisplayErrorUI );    log_v("Attached onError");      }
          if( !SDUCfg.onBefore     )  { SDUCfg.setBeforeCb(     SDU_UI::freezeTextStyle );   log_v("Attached onBefore");     }
          if( !SDUCfg.onAfter      )  { SDUCfg.setAfterCb(      SDU_UI::thawTextStyle );     log_v("Attached onAfter");      }
          if( !SDUCfg.onSplashPage )  { SDUCfg.setSplashPageCb( SDU_UI::drawSDUSplashPage ); log_v("Attached onSplashPage"); }
          if( !SDUCfg.onButtonDraw )  { SDUCfg.setButtonDrawCb( SDU_UI::drawSDUPushButton ); log_v("Attached onButtonDraw"); }
          #if defined ARDUINO_ESP32_S3_BOX
            SDUCfg.setSDUBtnA( ConfigManager::MuteChanged );   log_v("Attached Mute Read");
            SDUCfg.setSDUBtnB( nullptr );       log_v("Detached BtnB");
            SDUCfg.setSDUBtnC( nullptr );       log_v("Detached BtnC");
            SDUCfg.setLabelSkip( nullptr );     log_v("Disabled Skip");
            SDUCfg.setLabelRollback( nullptr ); log_v("Disabled Rollback");
            SDUCfg.setLabelSave( nullptr );     log_v("Disabled Save");
          #endif
          #if defined _M5STICKC_H_
            SDUCfg.setSDUBtnC( nullptr );       log_v("Detached BtnC");
          #endif

          #if defined SDU_HAS_TOUCH // default touch button support
            if ( !SDUCfg.onWaitForAction) { SDUCfg.setWaitForActionCb( TriggerSource::touchButton ); log_v("Attached onWaitForAction (touch)"); }
          #else // default momentary button support
            if ( !SDUCfg.onWaitForAction) { SDUCfg.setWaitForActionCb( TriggerSource::pushButton ); log_v("Attached onWaitForAction (button)"); }
          #endif
        }
      } else {
        log_v("Headless SD Menu Config (ptr=%d)", SDUCfg.display );
        if( SDUCfg.load_defaults ) {
          if( !SDUCfg.onProgress      ) { SDUCfg.setProgressCb(      SDU_UI::SDMenuProgressHeadless );      log_v("Attached onProgress"); }
          if( !SDUCfg.onMessage       ) { SDUCfg.setMessageCb(       SDU_UI::DisplayUpdateHeadless );       log_v("Attached onMessage"); }
          if( !SDUCfg.onWaitForAction ) { SDUCfg.setWaitForActionCb( TriggerSource::serial );           log_v("Attached onWaitForAction (serial)"); }
        }
      }
    }

    #if defined ARDUINO_ESP32_S3_BOX
      // this is temporary and may change later
      static bool MuteChanged()
      {
        static int lastbtnstate = digitalRead( GPIO_NUM_1 );
        if( digitalRead( GPIO_NUM_1 ) != lastbtnstate ) {
          lastbtnstate = 1-lastbtnstate;
          log_d("btnstate: %d", lastbtnstate );
          return true;
        }
        return false;
      }
    #endif

  }; // end namespace config



  namespace TriggerSource
  {

    // headless method
    int serial( char* labelLoad,  char* labelSkip, char* labelSave, unsigned long waitdelay )
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

    // display methods
    #if defined USE_DISPLAY
      int pushButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay )
      {
        if( waitdelay > 100 ) {
          SDUCfg.onBefore();
          SDUCfg.onSplashPage( BTN_HINT_MSG );
          const BtnStyles_t btns;
          if( SDUCfg.Buttons[0].cb) SDUCfg.onButtonDraw( labelLoad, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor, btns.Load.ShadowColor );
          if( SDUCfg.Buttons[1].cb) SDUCfg.onButtonDraw( labelSkip, 1, btns.Skip.BorderColor, btns.Skip.FillColor, btns.Skip.TextColor, btns.Skip.ShadowColor );
          if( SDUCfg.binFileName != nullptr && SDUCfg.Buttons[2].cb ) {
            SDUCfg.onButtonDraw( labelSave, 2, btns.Save.BorderColor, btns.Save.FillColor, btns.Save.TextColor, btns.Save.ShadowColor );
          }
        }
        auto msec = millis();
        uint32_t progress = 0, progressOld = 1;
        if( SDUCfg.onProgress ) SDUCfg.onProgress( 100, 100 );

        SDU_UI::loaderAnimator_t loadingAnimation;
        loadingAnimation.init( SDU_GFX );

        int ret = -1;

        //if(!SDUCfg.buttonsUpdate) log_w("No button poller found in SDUCfg, does M5.update() run in another task ?");

        do {
          //if( SDUCfg.buttonsUpdate ) SDUCfg.buttonsUpdate();
          SDUCfg.buttonsPoll();

          for( int i=0; i<3; i++ ) {
            if( SDUCfg.Buttons[i].cb && SDUCfg.Buttons[i].cb() ) {
              log_v("SDUCfg.Buttons[%d] was triggered", i);
              ret = SDUCfg.Buttons[i].val; goto _endAssert;
            }
          }
          if( SDUCfg.onProgress   ) {
            float barprogress = float(millis() - msec) / float(waitdelay);
            progress = 100- (100 * barprogress);
            if (progressOld != progress) {
              progressOld = progress;
              SDUCfg.onProgress( (uint8_t)progress, 100 );
            }
          }
          loadingAnimation.animate();
        } while (millis() - msec < waitdelay);

        _endAssert:

        if( SDUCfg.onProgress ) SDUCfg.onProgress( 0, 100 );
        loadingAnimation.deinit();
        if( ret > -1 ) { // wait for button release
          log_v("Waiting for Button #%d to be released", ret );
          while( SDUCfg.Buttons[ret].cb && SDUCfg.Buttons[ret].cb() ) {
            //if( SDUCfg.buttonsUpdate ) SDUCfg.buttonsUpdate();
            SDUCfg.buttonsPoll();
            vTaskDelay(10);
          }
        }
        return ret;
      }

      #if defined SDU_TouchButton

        int touchButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay )
        {
          /* auto &tft = M5.Lcd; */
          if( waitdelay == 0 ) return -1;
          // touch support + buttons
          SDU_TouchButton *LoadBtn = new SDU_TouchButton();
          SDU_TouchButton *SkipBtn = new SDU_TouchButton();
          SDU_TouchButton *SaveBtn = new SDU_TouchButton();

          SDU_UI::TouchButtonWrapper tbWrapper;
          SDU_UI::TouchStyles ts;

          #if !defined HAS_LGFX
            LoadBtn->setFont(nullptr);
            SkipBtn->setFont(nullptr);
            if( SDUCfg.binFileName != nullptr ) {
              SaveBtn->setFont(nullptr);
            }
          #endif

          LoadBtn->initButton(
            SDU_GFX,
            ts.x1, ts.y,  ts.w, ts.h,
            ts.Load->BorderColor, ts.Load->FillColor, ts.Load->TextColor,
            labelLoad, ts.btn_fsize
          );
          SkipBtn->initButton(
            SDU_GFX,
            ts.x2, ts.y,  ts.w, ts.h,
            ts.Skip->BorderColor, ts.Skip->FillColor, ts.Skip->TextColor,
            labelSkip, ts.btn_fsize
          );

          if( SDUCfg.binFileName != nullptr ) {
            SaveBtn->initButton(
              SDU_GFX,
              ts.x3, ts.y1,  ts.w, ts.h,
              ts.Save->BorderColor, ts.Save->FillColor, ts.Save->TextColor,
              labelSave, ts.btn_fsize
            );
            SaveBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);
            SaveBtn->drawButton();
            SaveBtn->press(false);
          }

          LoadBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);
          SkipBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);

          LoadBtn->drawButton();
          SkipBtn->drawButton();

          LoadBtn->press(false);
          SkipBtn->press(false);

          uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
          bool ispressed = false;
          int retval = -1; // return status

          SDU_GFX->drawFastHLine( ts.pgbar_x, ts.pgbar_y, ts.pgbar_w-1, TFT_WHITE );

          auto msectouch = millis();
          do {

            if( tbWrapper.iconRendered == false ) {
              tbWrapper.pushIcon( labelLoad );
              tbWrapper.iconRendered = true;
            }
            tbWrapper.handlePressed( LoadBtn, ispressed, t_x, t_y );
            tbWrapper.handlePressed( SkipBtn, ispressed, t_x, t_y );
            if( SDUCfg.binFileName != nullptr ) {
              tbWrapper.handlePressed( SaveBtn, ispressed, t_x, t_y );
            }
            tbWrapper.handleJustPressed( LoadBtn, labelLoad );
            tbWrapper.handleJustPressed( SkipBtn, labelSkip );
            if( SDUCfg.binFileName != nullptr ) {
              tbWrapper.handleJustPressed( SaveBtn, labelSave );
            }

            if( tbWrapper.justReleased( LoadBtn, ispressed, labelLoad ) ) {
              retval = 1;
              log_v("LoadBTN Pressed at [%d:%d]!", t_x, t_y);
              break;
            }
            if( tbWrapper.justReleased( SkipBtn, ispressed, labelSkip ) ) {
              retval = 0;
              log_v("SkipBTN Pressed at [%d:%d]!", t_x, t_y);
              break;
            }
            if( SDUCfg.binFileName != nullptr ) {
              if( tbWrapper.justReleased( SaveBtn, ispressed, labelSave ) ) {
                retval = 2;
                log_v("SaveBtn Pressed at [%d:%d]!", t_x, t_y);
                break;
              }
            }

            #if defined HAS_LGFX
              lgfx::touch_point_t tp;
              uint16_t number = SDU_GFX->getTouch(&tp, 1);
              t_x = tp.x;
              t_y = tp.y;
              ispressed = number > 0;
            #else // M5Core2.h / TFT_eSPI_Button syntax
              ispressed = SDU_GFX->getTouch(&t_x, &t_y);
            #endif

            float barprogress = float(millis() - msectouch) / float(waitdelay);
            int linewidth = float(ts.pgbar_w) * barprogress;
            if( linewidth > 0 ) {
              int linepos = ts.pgbar_w - ( linewidth +1 );
              uint16_t grayscale = 255 - (192*barprogress);
              SDU_GFX->drawFastHLine( ts.pgbar_x,         ts.pgbar_y, ts.pgbar_w-linewidth-1, SDU_GFX->color565( grayscale, grayscale, grayscale ) );
              SDU_GFX->drawFastHLine( ts.pgbar_x+linepos, ts.pgbar_y, 1,                      TFT_BLACK );
            }

          } while (millis() - msectouch < waitdelay);

          #if defined _M5Core2_H_
            // clean handlers
            LoadBtn->delHandlers();
            SkipBtn->delHandlers();
            SaveBtn->delHandlers();
          #endif

          delete LoadBtn;
          delete SkipBtn;
          delete SaveBtn;

          return retval;
        }

      #else // ! defined SDU_TouchButton
        int touchButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay )
        {
          return serial( labelLoad,  labelSkip, labelSave, waitdelay );
        }
      #endif // ! defined SDU_TouchButton

    #else // ! defined USE_DISPLAY
      int pushButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay )
      {
        return serial( labelLoad,  labelSkip, labelSave, waitdelay );
      }
      int touchButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay )
      {
        return serial( labelLoad,  labelSkip, labelSave, waitdelay );
      }
    #endif // ! defined USE_DISPLAY


  };


  namespace SDU_UI
  {

    void DisplayUpdateHeadless( const String& label )
    {
      // TODO: draw some fancy serial output
    };

    void SDMenuProgressHeadless( int state, int size )
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


      #ifdef ARDUINO_ODROID_ESP32 // Odroid-GO has 4 buttons under the TFT
        const uint16_t BUTTON_WIDTH = 60;
        const uint16_t BUTTON_HWIDTH = BUTTON_WIDTH/2; // button half width
        const uint16_t BUTTON_HEIGHT = 28;
        const int16_t SDUButtonsXOffset[4] = {
          1, 72, 188, 260
        };
        const int16_t SDUButtonsYOffset[4] = {
          0, 0, 0, 0
        };
      #else // assuming landscape mode /w 320x240 display
        const uint16_t BUTTON_WIDTH = 68;
        const uint16_t BUTTON_HWIDTH = BUTTON_WIDTH/2; // button half width
        const uint16_t BUTTON_HEIGHT = 28;
        const int16_t SDUButtonsXOffset[3] = {
          31, 125, 219
        };
        const int16_t SDUButtonsYOffset[3] = {
          0, 0, 0
        };
      #endif

      // default theme
      const BtnStyle_t DefaultLoadBtn{0x73AE,0x630C,TFT_WHITE,TFT_BLACK};
      const BtnStyle_t DefaultSkipBtn{0x73AE,0x630C,TFT_WHITE,TFT_BLACK};
      const BtnStyle_t DefaultSaveBtn{0x73AE,0x630C,TFT_WHITE,TFT_BLACK};
      const uint16_t DefaultMsgFontColors[2] = {TFT_WHITE, TFT_BLACK};

      const BtnStyles_t DefaultBtnStyle(
        DefaultLoadBtn, DefaultSkipBtn, DefaultSaveBtn,
        BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HWIDTH,
        1, 1, 2, DefaultMsgFontColors
      );

      SplashPageElementStyle_t SplashTitleStyle      = { TFT_BLACK,     TFT_WHITE, 2, MC_DATUM, TFT_LIGHTGREY, TFT_DARKGREY };
      SplashPageElementStyle_t SplashAppNameStyle    = { TFT_LIGHTGREY, TFT_BLACK, 2, BC_DATUM, 0, 0 };
      SplashPageElementStyle_t SplashAuthorNameStyle = { TFT_LIGHTGREY, TFT_BLACK, 2, BC_DATUM, 0, 0 };
      SplashPageElementStyle_t SplashAppPathStyle    = { TFT_DARKGREY,  TFT_BLACK, 1, BC_DATUM, 0, 0 };

      ProgressBarStyle_t ProgressStyle = {
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

      SDUTextStyle_t SDUTextStyle; // temporary style holder


      // API Specifics
      #if defined HAS_LGFX
        #define getTextFgColor(x) x->getTextStyle().fore_rgb888
        #define getTextBgColor(x) x->getTextStyle().back_rgb888
        #define getTextDatum(x)   x->getTextStyle().datum
        #define getTextSize(x)    x->getTextStyle().size_x
        #define resetFont(x)      x->setFont( nullptr )
      #else
        #define getTextFgColor(x) x->textcolor
        #define getTextBgColor(x) x->textbgcolor
        #define getTextDatum(x)   x->textdatum
        #define getTextSize(x)    x->textsize
        #define resetFont(x)      (void)0
      #endif


      void loaderAnimator_t::init( DISPLAY_TYPE gfx )
      {
        assert( gfx );
        display = gfx;
        if( !sprite ) {
          //log_d("[%d] Init animation sprite", ESP.getFreeHeap() );
          sprite = new SDUSprite( display );
        }
        sprite->createSprite( 32, 32 );
      }

      void loaderAnimator_t::animate()
      {
        if( !sprite ) return;
        #if defined HAS_LGFX
          float angle = sin( float(millis())/500.0 )*180.0; // 1/2 round per second
          sprite->clear();
          sprite->pushImageRotateZoom(sprite->width()/2, sprite->height()/2, 7.5, 8, angle, 1, 1, 15, 16, sdUpdaterIcon15x16_raw);
          sprite->pushSprite( display->width()/2-sprite->width()/2, display->height()*.75-sprite->height() );
        #endif
      }

      void loaderAnimator_t::deinit()
      {
        if( sprite ) {
          sprite->deleteSprite(); // free framebuffer
          delete(sprite);         // delete object
          sprite = nullptr;       // reset pointer
          //log_d("[%d] Deinit animation sprite", ESP.getFreeHeap() );
        }
      }

      void fillStyledRect( SplashPageElementStyle_t *style, int32_t x, int32_t y, uint16_t width, uint16_t height )
      {
        #if defined HAS_LGFX // draw gradient background
          if( style->colorStart == style->colorEnd ) {
            SDU_GFX->fillRect( x, y, width, height, style->bgColor );
          } else {
            for( int i=y; i<y+height; i++ ) {
              SDU_GFX->drawGradientHLine( x, i, width, style->colorStart, style->colorEnd );
            }
          }
        #else // just fill the background
          SDU_GFX->fillRect( x, y, width, height, style->bgColor );
        #endif
      }

      void adjustFontSize( uint8_t *lineHeightBig, uint8_t *lineHeightSmall )
      {
        #if defined HAS_LGFX
          auto fontSize = SDU_GFX->getTextSizeX();
          SDU_GFX->setTextSize( fontSize*2.0 );
          *lineHeightBig = SDU_GFX->fontHeight();
          SDU_GFX->setTextFont( fontSize );
          *lineHeightSmall = SDU_GFX->fontHeight();
        #else
          //int fontSize = 1;//SDU_GFX->textWidth("0");
          *lineHeightBig = 14;
          *lineHeightSmall = 8;
        #endif
      }


      void freezeTextStyle()
      {
        if( SDUTextStyle.frozen ) {
          // log_v("can't freeze twice, thaw first !");
          return;
        }

        SDUTextStyle.textcolor   = getTextFgColor(SDU_GFX);
        SDUTextStyle.textbgcolor = getTextBgColor(SDU_GFX);
        SDUTextStyle.textdatum   = getTextDatum(SDU_GFX);
        SDUTextStyle.textsize    = getTextSize(SDU_GFX);
        SDUTextStyle.frozen = true;
        log_d("Froze textStyle, size: %d, datum: %d, color: 0x%08x, bgcolor: 0x%08x", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
      }

      void thawTextStyle()
      {
        SDU_GFX->setTextSize( SDUTextStyle.textsize);
        SDU_GFX->setTextDatum( SDUTextStyle.textdatum );
        SDU_GFX->setTextColor( SDUTextStyle.textcolor , SDUTextStyle.textbgcolor );
        resetFont(SDU_GFX);
        SDU_GFX->setCursor(0,0);
        SDU_GFX->fillScreen(TFT_BLACK);
        log_d("Thawed textStyle, size: %d, datum: %d, color: 0x%08x, bgcolor: 0x%08x", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
        SDUTextStyle.frozen = false;
      }


      static void drawTextShadow( const char* text, int32_t x, int32_t y, uint16_t textcolor, uint16_t shadowcolor )
      {
        if( textcolor != shadowcolor ) {
          SDU_GFX->setTextColor( shadowcolor );
          SDU_GFX->drawString( text, x+1, y+1 );
        }
        SDU_GFX->setTextColor( textcolor );
        SDU_GFX->drawString( text, x, y );
      }


      void drawSDUSplashElement( const char* msg, int32_t x, int32_t y, SplashPageElementStyle_t *style )
      {
        SDU_GFX->setTextSize( style->fontSize );
        SDU_GFX->setTextDatum( style->textDatum );
        uint8_t lineHeight = SDU_GFX->fontHeight()*1.8;
        fillStyledRect( style, 0, y, SDU_GFX->width(), lineHeight );
        drawTextShadow( msg, x, y+lineHeight/2, style->textColor, TFT_DARKGREY );
      }

      void drawSDUSplashPage( const char* msg )
      {
        int32_t centerX = SDU_GFX->width() >> 1;

        uint8_t lineHeightBig   = 14;
        uint8_t lineHeightSmall = 8;

        adjustFontSize( &lineHeightBig, &lineHeightSmall );

        uint8_t appNamePosy     = lineHeightBig*1.8+lineHeightSmall;
        uint8_t authorNamePosY  = appNamePosy + lineHeightBig*1.8;
        uint8_t binFileNamePosY = authorNamePosY+lineHeightSmall*1.8;

        SDU_GFX->fillScreen(TFT_BLACK); // M5StickC does not have tft.clear()
        drawSDUSplashElement( msg, centerX, 0, &SplashTitleStyle );

        if( SDUCfg.appName != nullptr ) {
          drawSDUSplashElement( SDUCfg.appName, centerX, appNamePosy, &SplashAppNameStyle  );
        }
        if( SDUCfg.authorName != nullptr ) {
          drawSDUSplashElement( String( "By " + String(SDUCfg.authorName) ).c_str(), centerX, authorNamePosY, &SplashAuthorNameStyle  );
        }
        if( SDUCfg.binFileName != nullptr ) {
          drawSDUSplashElement( String("File name: " + String(&SDUCfg.binFileName[1])).c_str(), centerX, binFileNamePosY, &SplashAppPathStyle );
        }
        //SDU_GFX->drawJpg( sdUpdaterIcon32x40_jpg, sdUpdaterIcon32x40_jpg_len, (centerX)-16, (SDU_GFX->height()/2)-20, 32, 40 );
      }


      void drawSDUPushButton( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor, uint16_t shadowcolor )
      {
        Theme_t defaultTheme(&DefaultBtnStyle, &SplashTitleStyle, &SplashAppNameStyle, &SplashAuthorNameStyle, &SplashAppPathStyle, &ProgressStyle );

        if( !SDUCfg.theme ) {
          SDUCfg.theme = &defaultTheme;
        }

        const BtnStyles_t *bs = SDUCfg.theme->buttons; // ( SDUTheme.userBtnStyle == nullptr ) ? &DefaultBtnStyle : SDUTheme.userBtnStyle;
        uint32_t bx = SDUButtonsXOffset[position];
        uint32_t by = SDU_GFX->height() - bs->height - 2 - SDUButtonsYOffset[position];
        SDU_GFX->fillRoundRect( bx, by, bs->width, bs->height, 3, fillcolor );
        SDU_GFX->drawRoundRect( bx, by, bs->width, bs->height, 3, outlinecolor );
        SDU_GFX->setTextSize( bs->FontSize );
        SDU_GFX->setTextDatum( MC_DATUM );
        SDU_GFX->setTextFont( bs->BtnFontNumber );
        drawTextShadow( label, bx+bs->width/2, by+bs->height/2, textcolor, shadowcolor );
      }


      void SDMenuProgressUI( int state, int size )
      {
        static int SD_UI_Progress;

        int posX = (SDU_GFX->width() - (ProgressStyle.width+2)) >> 1;
        int posY = (SDU_GFX->height()- (ProgressStyle.height+2)) >> 1;

        if( state <=0 || size <=0 ) {
          // clear progress bar
          SDU_GFX->fillRect( posX, posY, ProgressStyle.width+2, ProgressStyle.height+2, ProgressStyle.bgColor );
        } else {
          // draw frame
          SDU_GFX->drawRect( posX, posY, ProgressStyle.width+2, ProgressStyle.height+2, ProgressStyle.borderColor );
        }

        int offset = ( state * ProgressStyle.width ) / size;
        int percent = ( state * 100 ) / size;
        if( offset == SD_UI_Progress ) {
          // don't render twice the same value
          return;
        }
        SD_UI_Progress = offset;

        if ( offset >= 0 && offset <= ProgressStyle.width ) {
          SDU_GFX->fillRect( posX+1,        posY+1, offset,                     ProgressStyle.height, ProgressStyle.fillColor );
          SDU_GFX->fillRect( posX+1+offset, posY+1, ProgressStyle.width-offset, ProgressStyle.height, ProgressStyle.bgColor );
        } else {
          SDU_GFX->fillRect( posX+1,        posY+1, ProgressStyle.width,        ProgressStyle.height, ProgressStyle.bgColor );
        }
        if( ProgressStyle.clipText ) {
          String percentStr = " " + String( percent ) + "% ";
          SDU_GFX->setTextFont( ProgressStyle.fontNumber );
          SDU_GFX->setTextDatum( ProgressStyle.textDatum );
          SDU_GFX->setTextSize( ProgressStyle.textSize );
          SDU_GFX->setTextColor( ProgressStyle.textColor, ProgressStyle.bgColor );
          SDU_GFX->drawString( percentStr, SDU_GFX->width() >> 1, posY+ProgressStyle.height+SDU_GFX->fontHeight() );
        }
      };


      void DisplayUpdateUI( const String& label )
      {
        if (SDU_GFX->width() < SDU_GFX->height()) SDU_GFX->setRotation(SDU_GFX->getRotation() ^ 1);

        SDU_GFX->fillScreen( TFT_BLACK );
        SDU_GFX->setTextColor( TFT_WHITE, TFT_BLACK );
        SDU_GFX->setTextFont( 0 );
        SDU_GFX->setTextSize( 2 );
        SDU_GFX->setTextDatum( ML_DATUM );
        // attemtp to center the text
        int16_t xpos = ( SDU_GFX->width() / 2) - ( SDU_GFX->textWidth( label ) / 2 );
        if ( xpos < 0 ) {
          // try with smaller size
          SDU_GFX->setTextSize(1);
          xpos = ( SDU_GFX->width() / 2 ) - ( SDU_GFX->textWidth( label ) / 2 );
          if( xpos < 0 ) {
            // give up
            xpos = 0 ;
          }
        }
        //int posX = (SDU_GFX->width() - ProgressStyle.width+2) >> 1;
        int posY = (SDU_GFX->height()- ProgressStyle.height+2) >> 1;
        SDU_GFX->setCursor( xpos, posY - 20 );
        SDU_GFX->print( label );
      }


      void DisplayErrorUI( const String& msg, unsigned long wait )
      {
        static uint8_t msgposy = 0;
        SDU_GFX->setTextSize( 2 );
        SDU_GFX->setTextFont( 0 );
        uint8_t headerHeight = SDU_GFX->fontHeight()*1.8; // = 28px
        log_v("Header height: %d", headerHeight );
        if( msgposy == 0 ) {
          SDU_GFX->fillRect( 0, headerHeight+1, SDU_GFX->width(), SDU_GFX->height()-headerHeight, TFT_BLACK );
        }
        SDU_GFX->setTextColor( TFT_RED, TFT_BLACK );
        SDU_GFX->setTextDatum( MC_DATUM );
        SDU_GFX->drawString( msg.c_str(), SDU_GFX->width()/2, msgposy+headerHeight*2 );
        msgposy += headerHeight;
        delay(wait);
      }



      TouchStyles::~TouchStyles()
      {
        if( Load ) delete Load;
        if( Skip ) delete Skip;
        if( Save ) delete Save;
      }

      TouchStyles::TouchStyles()
      {
        padx    = 4;                                    // buttons padding X
        pady    = 5;                                    // buttons padding Y
        marginx = 4;                                    // buttons margin X
        marginy = 4;                                    // buttons margin Y
        x1      = marginx + SDU_GFX->width()/4;              // button 1 X position
        x2      = marginx+SDU_GFX->width()-SDU_GFX->width()/4;    // button 2 X position
        x3      = SDU_GFX->width()/2;                         // button 3 X position
        y       = SDU_GFX->height()/2.2;                       // buttons Y position
        w       = (SDU_GFX->width()/2)-(marginx*2);          // buttons width
        h       = SDU_GFX->height()/4,                       // buttons height
        y1      = marginx*3+SDU_GFX->height()-h;               // button3 y position
        icon_x  = marginx+12;                           // icon (button 1) X position
        icon_y  = y-8;                                  // icon (button 1) Y position
        pgbar_x = SDU_GFX->width()/2+(marginx*2)+(padx*2)-1; // progressbar X position
        pgbar_y = (y+h/2)+(marginy*2)-1;                // progressbar Y position
        pgbar_w = w-(marginx*4)-(padx*4);               // progressbar width
        btn_fsize = (SDU_GFX->width()>240?2:1);               // touch buttons font size
        Load = new BtnStyle_t( (uint16_t)TFT_ORANGE,                 SDU_GFX->color565( 0xaa, 0x00, 0x00), SDU_GFX->color565( 0xdd, 0xdd, 0xdd), (uint16_t)TFT_BLACK );
        Skip = new BtnStyle_t( SDU_GFX->color565( 0x11, 0x11, 0x11), SDU_GFX->color565( 0x33, 0x88, 0x33), SDU_GFX->color565( 0xee, 0xee, 0xee), (uint16_t)TFT_BLACK );
        Save = new BtnStyle_t( (uint16_t)TFT_ORANGE,                 (uint16_t)TFT_BLACK,                  (uint16_t)TFT_WHITE,                  (uint16_t)TFT_BLACK );
      }


      #if defined SDU_TouchButton

        void TouchButtonWrapper::handlePressed( SDU_TouchButton *btn, bool pressed, uint16_t x, uint16_t y)
        {
          if (pressed && btn->contains(x, y)) {
            log_v("Press at [%d:%d]", x, y );
            btn->press(true); // tell the button it is pressed
          } else {
            if( pressed ) {
              log_v("Outside Press at [%d:%d]", x, y );
            }
            btn->press(false); // tell the button it is NOT pressed
          }
        }

        void TouchButtonWrapper::handleJustPressed( SDU_TouchButton *btn, const char* label )
        {
          if ( btn->justPressed() ) {
            btn->drawButton(true, label);
            pushIcon( label );
          }
        }

        bool TouchButtonWrapper::justReleased( SDU_TouchButton *btn, bool pressed, const char* label )
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

        void TouchButtonWrapper::pushIcon(const char* label)
        {
          if( strcmp( label, SDUCfg.labelMenu ) == 0 || strcmp( label, SDUCfg.labelRollback ) == 0 )
          {
            TouchStyles bs;
            auto IconSprite = SDUSprite( SDU_GFX );
            IconSprite.createSprite(15,16);
            IconSprite.drawJpg(sdUpdaterIcon15x16_jpg, sdUpdaterIcon15x16_jpg_len, 0,0, 15, 16);
            IconSprite.pushSprite( bs.icon_x, bs.icon_y, SDU_GFX->color565( 0x01, 0x00, 0x80 ) );
            IconSprite.deleteSprite();
          }
        }

      #endif








    #else // !defined USE_DISPLAY

      void SDMenuProgressUI( int state, int size ) {}
      void DisplayUpdateUI( const String& label ) {}
      void DisplayErrorUI( const String& msg, unsigned long wait ) {}
      void freezeTextStyle() {}
      void thawTextStyle() {}
      void drawSDUSplashPage( const char* msg ) {}
      void drawSDUPushButton( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor, uint16_t shadowcolor ) {}

    #endif // !defined USE_DISPLAY


  };

};

