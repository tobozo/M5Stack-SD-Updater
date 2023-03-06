#pragma once

#include "../M5StackUpdaterConfig.h"
#include "UI/types.h"
#include "touch.hpp" // touch default methods


namespace SDUpdaterNS
{

  namespace UI
  {
    void SDMenuProgressUI( int state, int size );
    void DisplayUpdateUI( const String& label );
    void DisplayErrorUI( const String& msg, unsigned long wait );
    void freezeTextStyle();
    void thawTextStyle();
    void drawSDUSplashPage( const char* msg );
    void drawSDUPushButton( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor, uint16_t shadowcolor );

    void DisplayUpdateHeadless( const String& label );
    void SDMenuProgressHeadless( int state, int size );
  };

  namespace TriggerSource
  {
    int pushButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay );
    int touchButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay=5000 );
    int serial( char* labelLoad,  char* labelSkip, char* labelSave, unsigned long waitdelay );
  };


  namespace config
  {

    [[maybe_unused]]
    void setup()
    {
      SDUCfg.setDisplay(); // auto attach default display, if any

      if( SDUCfg.display ) {
        log_v("Display SD Menu Config");
        if( SDUCfg.load_defaults ) {

          if( !SDUCfg.Buttons[0].cb ) { SDUCfg.setSDUBtnA( config::buttonAPressed ); log_v("Attached buttonpressA"); }
          if( !SDUCfg.Buttons[1].cb ) { SDUCfg.setSDUBtnB( config::buttonBPressed ); log_v("Attached buttonpressB"); }
          if( !SDUCfg.Buttons[2].cb ) { SDUCfg.setSDUBtnC( config::buttonCPressed ); log_v("Attached buttonpressC"); }

          if( !SDUCfg.buttonsUpdate ) { SDUCfg.setSDUBtnPoller( config::pollButtons ); log_v("Attached buttons poller"); }
          if( !SDUCfg.onProgress   )  { SDUCfg.setProgressCb(   UI::SDMenuProgressUI );  log_v("Attached onProgress");   }
          if( !SDUCfg.onMessage    )  { SDUCfg.setMessageCb(    UI::DisplayUpdateUI );   log_v("Attached onMessage");    }
          if( !SDUCfg.onError      )  { SDUCfg.setErrorCb(      UI::DisplayErrorUI );    log_v("Attached onError");      }
          if( !SDUCfg.onBefore     )  { SDUCfg.setBeforeCb(     UI::freezeTextStyle );   log_v("Attached onBefore");     }
          if( !SDUCfg.onAfter      )  { SDUCfg.setAfterCb(      UI::thawTextStyle );     log_v("Attached onAfter");      }
          if( !SDUCfg.onSplashPage )  { SDUCfg.setSplashPageCb( UI::drawSDUSplashPage ); log_v("Attached onSplashPage"); }
          if( !SDUCfg.onButtonDraw )  { SDUCfg.setButtonDrawCb( UI::drawSDUPushButton ); log_v("Attached onButtonDraw"); }
          #if defined ARDUINO_ESP32_S3_BOX
            SDUCfg.setSDUBtnA( config::MuteChanged );   log_v("Attached Mute Read");
            SDUCfg.setSDUBtnB( nullptr );       log_v("Detached BtnB");
            SDUCfg.setSDUBtnC( nullptr );       log_v("Detached BtnC");
            SDUCfg.setLabelSkip( nullptr );     log_v("Disabled Skip");
            SDUCfg.setLabelRollback( nullptr ); log_v("Disabled Rollback");
            SDUCfg.setLabelSave( nullptr );     log_v("Disabled Save");
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
          if( !SDUCfg.onProgress      ) { SDUCfg.setProgressCb(      UI::SDMenuProgressHeadless );      log_v("Attached onProgress"); }
          if( !SDUCfg.onMessage       ) { SDUCfg.setMessageCb(       UI::DisplayUpdateHeadless );       log_v("Attached onMessage"); }
          if( !SDUCfg.onWaitForAction ) { SDUCfg.setWaitForActionCb( TriggerSource::serial );           log_v("Attached onWaitForAction (serial)"); }
        }
      }
    }

    void pollButtons()
    {
      if(SDUCfg.buttonsUpdate) SDUCfg.buttonsUpdate();
    }

    bool buttonAPressed()
    {
      if(SDUCfg.btnAPressed) return SDUCfg.btnAPressed();
      else return false;
    }

    bool buttonBPressed()
    {
      if(SDUCfg.btnBPressed) return SDUCfg.btnBPressed();
      else return false;
    }

    bool buttonCPressed()
    {
      #if defined _M5STICKC_H_
        return false;
      #else
        if(SDUCfg.btnCPressed) return SDUCfg.btnCPressed();
      #endif
      return false;
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

  };

};





#if defined USE_DISPLAY

  #include "assets.h"

  namespace SDUpdaterNS
  {

    #define SDU_GFX ((DISPLAY_TYPE)(SDUCfg.display))

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
      static uint16_t BUTTON_WIDTH = 68;
      static uint16_t BUTTON_HWIDTH = BUTTON_WIDTH/2; // button half width
      static uint16_t BUTTON_HEIGHT = 28;
      static int16_t SDUButtonsXOffset[3] = {
        31, 125, 219
      };
      static int16_t SDUButtonsYOffset[3] = {
        0, 0, 0
      };
    #endif


    namespace config
    {
      void* config_sdu_t::getDisplay()
      {
        #if defined LGFX_ONLY
          if(!display) log_e("Display and Buttons needs manual setup!!");
        #else
          #if defined __M5UNIFIED_HPP__
            setDisplay( (void*)&M5.Display );
          #else
            setDisplay( (void*)&M5.Lcd );
          #endif
          setSDUBtnPoller( []() { M5.update(); } );
          setBtnAChecker( []() -> bool { return M5.BtnA.isPressed(); } );
          setBtnBChecker( []() -> bool { return M5.BtnB.isPressed(); } );
          setBtnCChecker( []() -> bool {
            #if defined _M5STICKC_H_
              return false;
            #else
              return M5.BtnC.isPressed();
            #endif
          });

        #endif
        return display ? display : (void*)SDU_GFX;
      }
    };


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

    static SplashPageElementStyle_t SplashTitleStyle      = { TFT_BLACK,     TFT_WHITE, 2, MC_DATUM, TFT_LIGHTGREY, TFT_DARKGREY };
    static SplashPageElementStyle_t SplashAppNameStyle    = { TFT_LIGHTGREY, TFT_BLACK, 2, BC_DATUM, 0, 0 };
    static SplashPageElementStyle_t SplashAuthorNameStyle = { TFT_LIGHTGREY, TFT_BLACK, 2, BC_DATUM, 0, 0 };
    static SplashPageElementStyle_t SplashAppPathStyle    = { TFT_DARKGREY,  TFT_BLACK, 1, BC_DATUM, 0, 0 };

    static ProgressBarStyle_t ProgressStyle = {
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

    static Theme_t* SDUTheme = new Theme_t(&DefaultBtnStyle, &SplashTitleStyle, &SplashAppNameStyle, &SplashAuthorNameStyle, &SplashAppPathStyle, &ProgressStyle );

    static SDUTextStyle_t SDUTextStyle; // temporary style holder

    namespace UI
    {


      void freezeTextStyle()
      {
        if( SDUTextStyle.frozen ) {
          // log_v("can't freeze twice, thaw first !");
          return;
        }
        #if defined HAS_LGFX // LFGX family cores/drivers (ESP32-Chimera-Core, LovyanGFX, M5GFX, M5Unified)
          SDUTextStyle.textcolor   = SDU_GFX->getTextStyle().fore_rgb888;
          SDUTextStyle.textbgcolor = SDU_GFX->getTextStyle().back_rgb888;
          SDUTextStyle.textdatum   = SDU_GFX->getTextStyle().datum;
          SDUTextStyle.textsize    = SDU_GFX->getTextStyle().size_x;
        #else // TFT_eSPI.h, M5Stack.h, M5Core2.h
          SDUTextStyle.textsize    = SDU_GFX->textsize;
          SDUTextStyle.textdatum   = SDU_GFX->textdatum;
          SDUTextStyle.textcolor   = SDU_GFX->textcolor;
          SDUTextStyle.textbgcolor = SDU_GFX->textbgcolor;
        #endif

        SDUTextStyle.frozen = true;
        log_d("Froze textStyle, size: %d, datum: %d, color: 0x%08x, bgcolor: 0x%08x", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
      }

      void thawTextStyle()
      {
        SDU_GFX->setTextSize( SDUTextStyle.textsize);
        SDU_GFX->setTextDatum( SDUTextStyle.textdatum );
        SDU_GFX->setTextColor( SDUTextStyle.textcolor , SDUTextStyle.textbgcolor );
        #if defined HAS_LGFX // LFGX family cores/drivers (ESP32-Chimera-Core, LovyanGFX, M5GFX, M5Unified)
          SDU_GFX->setFont( nullptr );
        #endif
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
        #if defined HAS_LGFX // draw gradient background
          if( style->colorStart == style->colorEnd ) {
            SDU_GFX->fillRect( 0, y, SDU_GFX->width(), lineHeight, style->bgColor );
          } else {
            for( int i=y; i<y+lineHeight; i++ ) {
              SDU_GFX->drawGradientHLine( 0, i, SDU_GFX->width(), style->colorStart, style->colorEnd );
            }
          }
        #else // just fill the background
          SDU_GFX->fillRect( 0, y, SDU_GFX->width(), lineHeight, style->bgColor );
        #endif
        drawTextShadow( msg, x, y+lineHeight/2, style->textColor, TFT_DARKGREY );
      }

      void drawSDUSplashPage( const char* msg )
      {
        int32_t centerX = SDU_GFX->width() >> 1;

        #if defined HAS_LGFX
          auto fontSize = SDU_GFX->getTextSizeX();
          SDU_GFX->setTextSize( fontSize*2.0 );
          uint8_t lineHeightBig = SDU_GFX->fontHeight();
          SDU_GFX->setTextFont( fontSize );
          uint8_t lineHeightSmall = SDU_GFX->fontHeight();
        #else
          //int fontSize = 1;//SDU_GFX->textWidth("0");
          uint8_t lineHeightBig = 14;
          uint8_t lineHeightSmall = 8;
        #endif

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
        const BtnStyles_t *bs = SDUTheme->buttons; // ( SDUTheme.userBtnStyle == nullptr ) ? &DefaultBtnStyle : SDUTheme.userBtnStyle;
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


    };



    namespace TriggerSource
    {

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

        #if defined HAS_LGFX
          SDUSprite *sprite = new SDUSprite( SDU_GFX );
          sprite->createSprite( 32, 32 );
        #endif

        int ret = -1;

        if(!SDUCfg.buttonsUpdate) log_w("No button poller found in SDUCfg, does M5.update() run in another task ?");

        do {
          if( SDUCfg.buttonsUpdate ) SDUCfg.buttonsUpdate();
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
          #if defined HAS_LGFX // this effect needs pushImageRotateZoom support
            float angle = sin( float(millis())/500.0 )*180.0; // 1/2 round per second
            sprite->clear();
            sprite->pushImageRotateZoom(sprite->width()/2, sprite->height()/2, 7.5, 8, angle, 1, 1, 15, 16, sdUpdaterIcon15x16_raw);
            sprite->pushSprite( SDU_GFX->width()/2-sprite->width()/2, SDU_GFX->height()*.75-sprite->height() );
          #endif
        } while (millis() - msec < waitdelay);

        _endAssert:

        if( SDUCfg.onProgress ) SDUCfg.onProgress( 0, 100 );
        #if defined HAS_LGFX
          sprite->deleteSprite();
        #endif
        if( ret > -1 ) { // wait for button release
          log_v("Waiting for Button #%d to be released", ret );
          while( SDUCfg.Buttons[ret].cb && SDUCfg.Buttons[ret].cb() ) {
            if( SDUCfg.buttonsUpdate ) SDUCfg.buttonsUpdate();
            vTaskDelay(10);
          }
        }
        return ret;
      }

    };

    // release 'SDU_GFX' temporary define
    #if defined undef_tft
      #undef SDU_GFX
    #endif

    #undef SDU_GFX

  };


#else

  namespace SDUpdaterNS
  {

    // namespace config
    // {
    //   void* config_sdu_t::getDisplay() { return nullptr; }
    // };

    // headless methods
    namespace TriggerSource
    {
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
    };


    namespace UI
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

    };

  };

#endif //  defined USE_DISPLAY
