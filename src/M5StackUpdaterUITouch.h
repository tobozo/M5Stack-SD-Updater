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
#if defined SDU_HAS_TOUCH
  #ifndef _SDU_TOUCH_UI_
    #define _SDU_TOUCH_UI_

    #if defined _M5Core2_H_
      // use TFT_eSPI_Touch emulation from M5Core2.h
      #define SDU_TouchButton TFT_eSPI_Button
    #else
      // use TouchButton/LGFX_Button from ESP32-Chimera-Core/LovyanGFX
      #define SDU_TouchButton TouchButton
    #endif

    struct TouchStyles
    {
      int padx    = 4,                                    // buttons padding X
          pady    = 5,                                    // buttons padding Y
          marginx = 4,                                    // buttons margin X
          marginy = 4,                                    // buttons margin Y
          x1      = marginx + SDU_GFX.width()/4,              // button 1 X position
          x2      = marginx+SDU_GFX.width()-SDU_GFX.width()/4,    // button 2 X position
          x3      = SDU_GFX.width()/2,                         // button 3 X position
          y       = SDU_GFX.height()/2.2,                       // buttons Y position
          w       = (SDU_GFX.width()/2)-(marginx*2),          // buttons width
          h       = SDU_GFX.height()/4,                       // buttons height
          y1      = marginx*3+SDU_GFX.height()-h,               // button3 y position
          icon_x  = marginx+12,                           // icon (button 1) X position
          icon_y  = y-8,                                  // icon (button 1) Y position
          pgbar_x = SDU_GFX.width()/2+(marginx*2)+(padx*2)-1, // progressbar X position
          pgbar_y = (y+h/2)+(marginy*2)-1,                // progressbar Y position
          pgbar_w = w-(marginx*4)-(padx*4),               // progressbar width
          btn_fsize = (SDU_GFX.width()>240?2:1)               // touch buttons font size
      ;
      BtnStyle Load = { TFT_ORANGE,                          SDU_GFX.color565( 0xaa, 0x00, 0x00), SDU_GFX.color565( 0xdd, 0xdd, 0xdd) };
      BtnStyle Skip = { SDU_GFX.color565( 0x11, 0x11, 0x11), SDU_GFX.color565( 0x33, 0x88, 0x33), SDU_GFX.color565( 0xee, 0xee, 0xee) };
      BtnStyle Save = { TFT_ORANGE, TFT_BLACK, TFT_WHITE };
    };

    struct TouchButtonWrapper
    {
      bool iconRendered = false;

      void handlePressed( SDU_TouchButton *btn, bool pressed, uint16_t x, uint16_t y)
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
        if( strcmp( label, SDUCfg.labelMenu ) == 0 || strcmp( label, SDUCfg.labelRollback ) == 0 )
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

    static int assertStartUpdateFromTouchButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay=5000 )
    {
      /* auto &tft = M5.Lcd; */
      if( waitdelay == 0 ) return -1;
      // chimera core any-touch support + buttons

      static SDU_TouchButton *LoadBtn = new SDU_TouchButton();
      static SDU_TouchButton *SkipBtn = new SDU_TouchButton();
      static SDU_TouchButton *SaveBtn = new SDU_TouchButton();

      TouchButtonWrapper tbWrapper;
      TouchStyles ts;

      #if !defined HAS_LGFX
        LoadBtn->setFont(nullptr);
        SkipBtn->setFont(nullptr);
        if( SDUCfg.binFileName != nullptr ) {
          SaveBtn->setFont(nullptr);
        }
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

      if( SDUCfg.binFileName != nullptr ) {
        SaveBtn->initButton(
          &SDU_GFX,
          ts.x3, ts.y1,  ts.w, ts.h,
          ts.Save.BorderColor, ts.Save.FillColor, ts.Save.TextColor,
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

      SDU_GFX.drawFastHLine( ts.pgbar_x, ts.pgbar_y, ts.pgbar_w-1, TFT_WHITE );

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
          uint16_t number = SDU_GFX.getTouch(&tp, 1);
          t_x = tp.x;
          t_y = tp.y;
          ispressed = number > 0;
        #else // M5Core2.h / TFT_eSPI_Button syntax
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

      #if defined _M5Core2_H_
        // clean handlers
        LoadBtn->delHandlers();
        SkipBtn->delHandlers();
        SaveBtn->delHandlers();
        delete LoadBtn;
        delete SkipBtn;
        delete SaveBtn;
      #endif

      return retval;
    }

  #endif // !defined _SDU_TOUCH_UI_
#endif // defined SDU_HAS_TOUCH
