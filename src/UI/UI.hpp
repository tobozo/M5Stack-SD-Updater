#pragma once

#include "../misc/assets.h"
#include "../misc/types.h"
#include "../UI/common.hpp"

namespace SDUpdaterNS
{


  namespace SDU_UI
  {

    #ifdef ARDUINO_ODROID_ESP32 // Odroid-GO has 4 buttons under the TFT
      static const uint16_t BUTTON_WIDTH = 60;
      static const uint16_t BUTTON_HWIDTH = BUTTON_WIDTH/2; // button half width
      static const uint16_t BUTTON_HEIGHT = 28;
      static const int16_t SDUButtonsXOffset[4] = {
        1, 72, 188, 260
      };
      static const int16_t SDUButtonsYOffset[4] = {
        0, 0, 0, 0
      };
    #else // assuming landscape mode /w 320x240 display
      static const uint16_t BUTTON_WIDTH = 68;
      static const uint16_t BUTTON_HWIDTH = BUTTON_WIDTH/2; // button half width
      static const uint16_t BUTTON_HEIGHT = 28;
      static const int16_t SDUButtonsXOffset[3] = {
        31, 125, 219
      };
      static const int16_t SDUButtonsYOffset[3] = {
        0, 0, 0
      };
    #endif



    #if defined HAS_LGFX
      //#define fontInto_t fontInfo_t
      struct fontInfo_t
      {
        const lgfx::IFont* font;
        const float fontSize;
      };
      fontInfo_t Font0Size1 = {&Font0, 1};
      fontInfo_t Font0Size2 = {&Font0, 2};
      fontInfo_t Font2Size1 = {&Font2, 1};
    #else
      //#define fontInto_t basic_fontInfo_t
      struct fontInfo_t
      {
        const uint8_t  fontNumber;
        const uint8_t  fontSize;
      };
      fontInfo_t Font0Size1 = {0, 1};
      fontInfo_t Font0Size2 = {0, 2};
      fontInfo_t Font2Size1 = {2, 1};
    #endif



    // default theme
    static const BtnStyle_t DefaultLoadBtn{0x73AE,0x630C,TFT_WHITE,TFT_BLACK};
    static const BtnStyle_t DefaultSkipBtn{0x73AE,0x630C,TFT_WHITE,TFT_BLACK};
    static const BtnStyle_t DefaultSaveBtn{0x73AE,0x630C,TFT_WHITE,TFT_BLACK};
    static const uint16_t DefaultMsgFontColors[2] = {TFT_WHITE, TFT_BLACK};

    static const BtnStyles_t DefaultBtnStyle(
      DefaultLoadBtn, DefaultSkipBtn, DefaultSaveBtn,
      BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HWIDTH,
      &Font0Size1, &Font0Size2, DefaultMsgFontColors
    );

    static SplashPageElementStyle_t SplashTitleStyle      = { TFT_BLACK,     TFT_WHITE, &Font0Size2, MC_DATUM, TFT_LIGHTGREY, TFT_DARKGREY };
    static SplashPageElementStyle_t SplashAppNameStyle    = { TFT_LIGHTGREY, TFT_BLACK, &Font0Size2, BC_DATUM, 0, 0 };
    static SplashPageElementStyle_t SplashAuthorNameStyle = { TFT_LIGHTGREY, TFT_BLACK, &Font0Size2, BC_DATUM, 0, 0 };
    static SplashPageElementStyle_t SplashAppPathStyle    = { TFT_DARKGREY,  TFT_BLACK, &Font0Size1, BC_DATUM, 0, 0 };

    static ProgressBarStyle_t ProgressStyle = {
      200,        // width
      8,          // height
      true,       // clip "xx%" text
      TFT_LIGHTGREY,  // border color
      TFT_DARKGREY,  // fill color
      &Font0Size1,
      //0,          // font number
      TC_DATUM,   // text alignment
      //1,          // text size
      TFT_WHITE,  // text color
      TFT_BLACK   // text bgcolor
    };

    static SDUTextStyle_t SDUTextStyle; // temporary style holder


    // API Specifics
    #if defined HAS_LGFX // using LovyanGFX or M5GFX

      #define setFontInfo(x, i) {auto info=(fontInfo_t*)i;if(info->font){x->setFont((lgfx::IFont*)info->font);}x->setTextSize(info->fontSize);}
      #define getTextFgColor(x) x->getTextStyle().fore_rgb888
      #define getTextBgColor(x) x->getTextStyle().back_rgb888
      #define getTextDatum(x)   x->getTextStyle().datum
      #define getTextSize(x)    x->getTextStyle().size_x
      #define resetFont(x)      x->setFont( nullptr )
      static SDU_Sprite *animSpr = nullptr;

      inline void loaderAnimator_t::init()
      {
        assert( SDU_GFX );
        //display = SDU_GFX;
        if( !animSpr ) {
          //log_d("[%d] Init animation sprite", ESP.getFreeHeap() );
          animSpr = new SDU_Sprite( SDU_GFX );
        }
        animSpr->createSprite( 32, 32 );
      }

      inline void loaderAnimator_t::animate()
      {
        if( !animSpr ) return;
        float angle = sin( float(millis())/500.0 )*180.0; // 1/2 round per second
        animSpr->clear();
        animSpr->pushImageRotateZoom(animSpr->width()/2, animSpr->height()/2, 7.5, 8, angle, 1, 1, 15, 16, sdUpdaterIcon15x16_raw);
        animSpr->pushSprite( SDU_GFX->width()/2-animSpr->width()/2, SDU_GFX->height()*.75-animSpr->height() );
      }

      inline void loaderAnimator_t::deinit()
      {
        if( animSpr ) {
          animSpr->deleteSprite(); // free framebuffer
          delete(animSpr);         // delete object
          animSpr = nullptr;       // reset pointer
          //log_d("[%d] Deinit animation sprite", ESP.getFreeHeap() );
        }
      }

      static void fillStyledRect( SplashPageElementStyle_t *style, int32_t x, int32_t y, uint16_t width, uint16_t height )
      {
        if( style->colorStart == style->colorEnd ) {
          SDU_GFX->fillRect( x, y, width, height, style->bgColor );
        } else {
          for( int i=y; i<y+height; i++ ) {
            SDU_GFX->drawGradientHLine( x, i, width, style->colorStart, style->colorEnd );
          }
        }
      }

      static void adjustFontSize( uint8_t *lineHeightBig, uint8_t *lineHeightSmall )
      {
        auto fontSize = SDU_GFX->getTextSizeX();
        SDU_GFX->setTextSize( fontSize*2.0 );
        *lineHeightBig = SDU_GFX->fontHeight();
        SDU_GFX->setTextSize( fontSize );
        *lineHeightSmall = SDU_GFX->fontHeight();
      }

      static void fillProgressBox( int32_t posX, int32_t posY, uint16_t offset, ProgressBarStyle_t *pgstyle )
      {
        for (uint8_t h = 0; h<pgstyle->height; h++) {
          SDU_GFX->drawGradientHLine( posX+1, posY+1 + h, offset, pgstyle->bgColor, pgstyle->textColor);
        }
        uint16_t barmod = pgstyle->width/12;
        if( barmod < 10 ) barmod = 10;
        for (uint16_t v = 1; v<offset; v++) {
          if(v%barmod == 0) {
            SDU_GFX->drawFastVLine( posX+1 + v, posY+1, pgstyle->height, pgstyle->bgColor);
          }
        }
      }


      static void resetScroll()
      {
        assert( SDU_GFX );
        uint16_t rst = 0;
        SDU_GFX->startWrite();
        SDU_GFX->writecommand(0x33);  // VSCRDEF Vertical scroll definition
        SDU_GFX->writedata(rst >> 8); // Top Fixed Area line count
        SDU_GFX->writedata(rst);
        SDU_GFX->writedata(rst >> 8); // Vertical Scrolling Area line count
        SDU_GFX->writedata(rst);
        SDU_GFX->writedata(rst >> 8); // Bottom Fixed Area line count
        SDU_GFX->writedata(rst);
        SDU_GFX->endWrite();

        SDU_GFX->startWrite();
        SDU_GFX->writecommand(0x37); // VSCRSADD Vertical scrolling pointer
        SDU_GFX->writedata(rst>>8);
        SDU_GFX->writedata(rst);
        SDU_GFX->endWrite();
      }



    #else // using TFT_eSPI based core (M5Core2.h, M5Stack.h, M5StickC.h).
      #define setFontInfo(x, i) {auto info=(fontInfo_t*)i;x->setTextFont(info->fontNumber);x->setTextSize(info->fontSize);}
      #define getTextFgColor(x) x->textcolor
      #define getTextBgColor(x) x->textbgcolor
      #define getTextDatum(x)   x->textdatum
      #define getTextSize(x)    x->textsize
      #define resetFont(x)      (void)0
      inline void loaderAnimator_t::init() {}
      inline void loaderAnimator_t::animate() {}
      inline void loaderAnimator_t::deinit() {}
      static void resetScroll() { }
      static void fillStyledRect( SplashPageElementStyle_t *style, int32_t x, int32_t y, uint16_t width, uint16_t height ) { SDU_GFX->fillRect( x, y, width, height, style->bgColor ); }
      static void adjustFontSize( uint8_t *lineHeightBig, uint8_t *lineHeightSmall ) { *lineHeightBig = 14; *lineHeightSmall = 8; }
      static void fillProgressBox( int32_t posX, int32_t posY, uint16_t offset, ProgressBarStyle_t *pgstyle )
      {
        SDU_GFX->fillRect( posX+1,        posY+1, offset,                  pgstyle->height, pgstyle->fillColor );
        SDU_GFX->fillRect( posX+1+offset, posY+1, pgstyle->width - offset, pgstyle->height, pgstyle->bgColor );
      }
    #endif

    static void freezeTextStyle() // onBeforeCb
    {
      resetScroll();
      if( SDUTextStyle.frozen ) {
        // log_v("can't freeze twice, thaw first !");
        return;
      }

      SDUTextStyle.textcolor   = getTextFgColor(SDU_GFX);
      SDUTextStyle.textbgcolor = getTextBgColor(SDU_GFX);
      SDUTextStyle.textdatum   = getTextDatum(SDU_GFX);
      SDUTextStyle.textsize    = getTextSize(SDU_GFX);
      SDUTextStyle.frozen = true;
      //log_d("Froze textStyle, size: %d, datum: %d, color: 0x%08x, bgcolor: 0x%08x", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
    }

    static void thawTextStyle()
    {
      SDU_GFX->setTextSize( SDUTextStyle.textsize);
      SDU_GFX->setTextDatum( SDUTextStyle.textdatum );
      SDU_GFX->setTextColor( SDUTextStyle.textcolor , SDUTextStyle.textbgcolor );
      resetFont(SDU_GFX);
      SDU_GFX->setCursor(0,0);
      SDU_GFX->fillScreen(TFT_BLACK);
      //log_d("Thawed textStyle, size: %d, datum: %d, color: 0x%08x, bgcolor: 0x%08x", SDUTextStyle.textsize, SDUTextStyle.textdatum, SDUTextStyle.textcolor, SDUTextStyle.textbgcolor );
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

    static void drawSDUSplashElement( const char* msg, int32_t x, int32_t y, SplashPageElementStyle_t *style )
    {
      setFontInfo( SDU_GFX, style->fontInfo );
      SDU_GFX->setTextDatum( style->textDatum );
      uint8_t lineHeight = SDU_GFX->fontHeight()*1.8;
      fillStyledRect( style, 0, y, SDU_GFX->width(), lineHeight );
      drawTextShadow( msg, x, y+lineHeight/2, style->textColor, TFT_DARKGREY );
    }

    static void drawSDUSplashPage( const char* msg )
    {
      int32_t centerX = SDU_GFX->width() >> 1;

      uint8_t lineHeightBig   = 14;
      uint8_t lineHeightSmall = 8;

      adjustFontSize( &lineHeightBig, &lineHeightSmall );

      uint8_t titleNamePosy   = 0;
      uint8_t appNamePosy     = lineHeightBig*1.8+lineHeightSmall;
      uint8_t authorNamePosY  = appNamePosy + lineHeightBig*1.8;
      uint8_t binFileNamePosY = authorNamePosY+lineHeightSmall*1.8;

      SDU_GFX->fillScreen(TFT_BLACK); // M5StickC does not have tft.clear()

      drawSDUSplashElement( msg, centerX, titleNamePosy, &SplashTitleStyle );

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

    static void drawSDUPushButton( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor, uint16_t shadowcolor )
    {
      Theme_t defaultTheme(&DefaultBtnStyle, &SplashTitleStyle, &SplashAppNameStyle, &SplashAuthorNameStyle, &SplashAppPathStyle, &ProgressStyle );
      bool use_default_theme = false;
      if( !SDUCfg.theme ) {
        use_default_theme = true;
        SDUCfg.theme = &defaultTheme;
      }
      const BtnStyles_t *bs = SDUCfg.theme->buttons; // ( SDUTheme.userBtnStyle == nullptr ) ? &DefaultBtnStyle : SDUTheme.userBtnStyle;
      uint32_t bx = SDUButtonsXOffset[position];
      uint32_t by = SDU_GFX->height() - bs->height - 2 - SDUButtonsYOffset[position];
      SDU_GFX->fillRoundRect( bx, by, bs->width, bs->height, 3, fillcolor );
      SDU_GFX->drawRoundRect( bx, by, bs->width, bs->height, 3, outlinecolor );
      SDU_GFX->setTextDatum( MC_DATUM );
      setFontInfo( SDU_GFX, bs->BtnFontInfo );
      drawTextShadow( label, bx+bs->width/2, by+bs->height/2, textcolor, shadowcolor );
      if( use_default_theme ) {
        SDUCfg.theme = nullptr;
      }
    }

    static void SDMenuProgressUI( int state, int size )
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
        fillProgressBox( posX, posY, offset, &ProgressStyle );
      } else {
        SDU_GFX->fillRect( posX+1,        posY+1, ProgressStyle.width,        ProgressStyle.height, ProgressStyle.bgColor );
      }
      if( ProgressStyle.clipText ) {
        String percentStr = " " + String( percent ) + "% ";
        setFontInfo( SDU_GFX, ProgressStyle.fontInfo );
        SDU_GFX->setTextDatum( ProgressStyle.textDatum );
        SDU_GFX->setTextColor( ProgressStyle.textColor, ProgressStyle.bgColor );
        SDU_GFX->drawString( percentStr, SDU_GFX->width() >> 1, posY+ProgressStyle.height+SDU_GFX->fontHeight() );
      }
    };

    static void DisplayUpdateUI( const String& label )
    {
      if (SDU_GFX->width() < SDU_GFX->height()) SDU_GFX->setRotation(SDU_GFX->getRotation() ^ 1);

      SDU_GFX->fillScreen( TFT_BLACK );
      SDU_GFX->setTextColor( TFT_WHITE, TFT_BLACK );
      SDU_GFX->setTextDatum( MC_DATUM );
      setFontInfo( SDU_GFX, &Font0Size2 );
      if( SDU_GFX->textWidth(label)>SDU_GFX->width() ) {
        setFontInfo( SDU_GFX, &Font0Size1 );
      }
      int posY = ((SDU_GFX->height()- ProgressStyle.height+2) >> 1) - 20;
      SDU_GFX->drawString( label, SDU_GFX->width()/2, posY );
    }

    static void DisplayErrorUI( const String& msg, unsigned long wait )
    {
      static uint8_t msgposy = 0;
      setFontInfo( SDU_GFX, &Font0Size2 );
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

};


