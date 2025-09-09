#pragma once

#include "../misc/assets.h"
#include "../misc/types.h"
#include "../UI/common.hpp"

namespace SDUpdaterNS
{

  namespace SDU_UI
  {

    struct TouchButtonWrapper
    {
      bool iconRendered = false;
      void handlePressed( SDU_TouchButton *btn, bool pressed, uint16_t x, uint16_t y);
      void handleJustPressed( SDU_TouchButton *btn, const char* label );
      bool justReleased( SDU_TouchButton *btn, bool pressed, const char* label );
      void pushIcon(const char* label);
    };

    inline void TouchButtonWrapper::handlePressed( SDU_TouchButton *btn, bool pressed, uint16_t x, uint16_t y)
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

    inline void TouchButtonWrapper::handleJustPressed( SDU_TouchButton *btn, const char* label )
    {
      if ( btn->justPressed() ) {
        btn->drawButton(true, label);
        pushIcon( label );
      }
    }

    inline bool TouchButtonWrapper::justReleased( SDU_TouchButton *btn, bool pressed, const char* label )
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

    inline void TouchButtonWrapper::pushIcon(const char* label)
    {
      if( strcmp( label, SDUCfg.labelMenu ) == 0 || strcmp( label, SDUCfg.labelRollback ) == 0 )
      {
        TouchStyles bs;
        auto IconSprite = SDU_Sprite( SDU_GFX );
        auto transcolor = SDU_GFX->color565( 0x01, 0x00, 0x80 );
        IconSprite.createSprite(15*bs.scale,16*bs.scale);
        IconSprite.fillSprite(transcolor);
        if( SDUCfg.rollBackToFactory ) {
          IconSprite.drawJpg(flashUpdaterIcon16x16_jpg, flashUpdaterIcon16x16_jpg_len, 0,0, 16*bs.scale, 16*bs.scale, 0, 0, bs.scale, bs.scale);
        } else {
          IconSprite.drawJpg(sdUpdaterIcon15x16_jpg, sdUpdaterIcon15x16_jpg_len, 0,0, 15*bs.scale, 16*bs.scale, 0, 0, bs.scale, bs.scale);
        }
        IconSprite.pushSprite( bs.icon_x, bs.icon_y, transcolor );
        IconSprite.deleteSprite();
      }
    }


    inline TouchStyles::~TouchStyles()
    {
      if( Load ) delete Load;
      if( Skip ) delete Skip;
      if( Save ) delete Save;
    }

    inline TouchStyles::TouchStyles()
    {
      auto screen_width = SDU_GFX->width(), screen_height = SDU_GFX->height();
      scale   = getTextSize(SDU_GFX);                    // proportional to screen size e.g. scale = 1 for 320x240
      padx    = 4*scale;                                 // buttons padding X
      pady    = 1*scale;                                 // buttons padding Y
      marginx = 2*scale;                                 // buttons margin X
      marginy = 2*scale;                                 // buttons margin Y
      x1      = marginx + screen_width/4;                // button 1 X position
      x2      = marginx+screen_width-screen_width/4;     // button 2 X position
      x3      = screen_width/2;                          // button 3 X position
      y       = screen_height/2;                         // buttons Y position
      w       = screen_width/2-marginx*2;                // buttons width
      h       = screen_height/5,                         // buttons height
      y1      = marginx*3+screen_height-h;               // button3 y position
      icon_x  = marginx*6;                               // icon (button 1) X position
      icon_y  = y-8*scale;                               // icon (button 1) Y position
      pgbar_x = screen_width/2+marginx*2+padx*2-1;       // progressbar X position
      pgbar_y = (y+h/2)+(marginy*2)-1;                   // progressbar Y position
      pgbar_w = w-(marginx*4)-(padx*4);                  // progressbar width
      btn_fsize = (screen_width<320?1:screen_width/320); // touch buttons font size
      Load = new BtnStyle_t( (uint16_t)TFT_ORANGE,                 SDU_GFX->color565( 0xaa, 0x00, 0x00), SDU_GFX->color565( 0xdd, 0xdd, 0xdd), (uint16_t)TFT_BLACK );
      Skip = new BtnStyle_t( SDU_GFX->color565( 0x11, 0x11, 0x11), SDU_GFX->color565( 0x33, 0x88, 0x33), SDU_GFX->color565( 0xee, 0xee, 0xee), (uint16_t)TFT_BLACK );
      Save = new BtnStyle_t( (uint16_t)TFT_ORANGE,                 (uint16_t)TFT_BLACK,                  (uint16_t)TFT_WHITE,                  (uint16_t)TFT_BLACK );
    }

  }



  namespace TriggerSource
  {


    struct touchTriggerElements_t
    {
      touchTriggerElements_t() { };
      touchTriggerElements_t( SDU_TouchButton *_LoadBtn, SDU_TouchButton *_SkipBtn, SDU_TouchButton *_SaveBtn, SDU_UI::TouchButtonWrapper _tbWrapper, SDU_UI::TouchStyles _ts )
      : LoadBtn(_LoadBtn), SkipBtn(_SkipBtn), SaveBtn(_SaveBtn), tbWrapper(_tbWrapper), ts(_ts), ispressed(false), t_x(0), t_y(0) { }
      SDU_TouchButton *LoadBtn{nullptr};
      SDU_TouchButton *SkipBtn{nullptr};
      SDU_TouchButton *SaveBtn{nullptr};
      SDU_UI::TouchButtonWrapper tbWrapper;
      SDU_UI::TouchStyles ts;
      bool ispressed = false;
      uint16_t t_x{0};
      uint16_t t_y{0}; // To store the touch coordinates
    };



    static void triggerInitTouch(triggerMap_t* trigger)
    {
      touchTriggerElements_t *el = new touchTriggerElements_t();

      trigger->sharedptr = (void*)el;

      el->LoadBtn = new SDU_TouchButton();
      el->SkipBtn = new SDU_TouchButton();
      el->SaveBtn = new SDU_TouchButton();

      auto LoadBtn = el->LoadBtn;
      auto SkipBtn = el->SkipBtn;
      auto SaveBtn = el->SaveBtn;
      auto &tbWrapper = el->tbWrapper;
      auto &ts        = el->ts;
      //SDU_UI::TouchStyles ts;

      #if !defined HAS_LGFX
        if( SDUCfg.Buttons[0].enabled ) LoadBtn->setFont(nullptr);
        if( SDUCfg.Buttons[1].enabled ) SkipBtn->setFont(nullptr);
        if( SDUCfg.binFileName != nullptr && SDUCfg.Buttons[2].enabled ) {
          SaveBtn->setFont(nullptr);
        }
      #endif

      if( SDUCfg.Buttons[0].enabled ) {
        LoadBtn->initButton(
          SDU_GFX,
          ts.x1, ts.y,  ts.w, ts.h,
          ts.Load->BorderColor, ts.Load->FillColor, ts.Load->TextColor,
          (char*)trigger->labelLoad, ts.btn_fsize
        );
        LoadBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);
        LoadBtn->drawButton();
        LoadBtn->press(false);
      }

      if( SDUCfg.Buttons[1].enabled ) {
        SkipBtn->initButton(
          SDU_GFX,
          ts.x2, ts.y,  ts.w, ts.h,
          ts.Skip->BorderColor, ts.Skip->FillColor, ts.Skip->TextColor,
          (char*)trigger->labelSkip, ts.btn_fsize
        );
        SkipBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);
        SkipBtn->drawButton();
        SkipBtn->press(false);
      }

      if( SDUCfg.binFileName != nullptr && SDUCfg.Buttons[2].enabled ) {
        SaveBtn->initButton(
          SDU_GFX,
          ts.x3, ts.y1,  ts.w, ts.h,
          ts.Save->BorderColor, ts.Save->FillColor, ts.Save->TextColor,
          (char*)trigger->labelSave, ts.btn_fsize
        );
        SaveBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);
        SaveBtn->drawButton();
        SaveBtn->press(false);
      }

      SDU_GFX->drawFastHLine( ts.pgbar_x, ts.pgbar_y, ts.pgbar_w-1, TFT_WHITE );
    }

    static bool triggerActionTouch(triggerMap_t* trigger, uint32_t msec )
    {
      auto el = (touchTriggerElements_t*)trigger->sharedptr;
      if(!el) {
        log_e("No trigger elements for this action, aborting!");
        return true;
      }

      auto LoadBtn    = el->LoadBtn;
      auto SkipBtn    = el->SkipBtn;
      auto SaveBtn    = el->SaveBtn;
      auto &tbWrapper = el->tbWrapper;
      auto &ts        = el->ts;
      auto &ispressed = el->ispressed;
      auto &t_x       = el->t_x;
      auto &t_y       = el->t_y;

      if( tbWrapper.iconRendered == false ) {
        tbWrapper.pushIcon( trigger->labelLoad );
        tbWrapper.iconRendered = true;
      }

      if( SDUCfg.Buttons[0].enabled ) tbWrapper.handlePressed( LoadBtn, ispressed, t_x, t_y );
      if( SDUCfg.Buttons[1].enabled ) tbWrapper.handlePressed( SkipBtn, ispressed, t_x, t_y );
      if( SDUCfg.binFileName != nullptr && SDUCfg.Buttons[2].enabled )  tbWrapper.handlePressed( SaveBtn, ispressed, t_x, t_y );

      if( SDUCfg.Buttons[0].enabled ) tbWrapper.handleJustPressed( LoadBtn, trigger->labelLoad );
      if( SDUCfg.Buttons[1].enabled ) tbWrapper.handleJustPressed( SkipBtn, trigger->labelSkip );
      if( SDUCfg.binFileName != nullptr && SDUCfg.Buttons[2].enabled )  tbWrapper.handleJustPressed( SaveBtn, trigger->labelSave );

      if( SDUCfg.Buttons[0].enabled && tbWrapper.justReleased( LoadBtn, ispressed, trigger->labelLoad ) ) {
        trigger->ret = 1;
        log_d("LoadBTN Pressed");
        return true;
      }
      if( SDUCfg.Buttons[1].enabled ) if( tbWrapper.justReleased( SkipBtn, ispressed, trigger->labelSkip ) ) {
        trigger->ret = 0;
        log_d("SkipBTN Pressed");
        return true;
      }
      if( SDUCfg.binFileName != nullptr && SDUCfg.Buttons[2].enabled && tbWrapper.justReleased( SaveBtn, ispressed, trigger->labelSave ) ) {
        trigger->ret = 2;
        log_d("SaveBtn Pressed");
        return true;
      }

      #if defined HAS_LGFX
        lgfx::touch_point_t tp;
        uint16_t number = SDU_GFX->getTouch(&tp, 1);
        t_x = tp.x;
        t_y = tp.y;
        ispressed = number > 0;
      #else // M5Core2.h / TFT_eSPI_Button syntax
        #if __has_include(<M5Touch.h>)
          TouchPoint_t pos = M5.Touch.getPressPoint();
          t_x = pos.x;
          t_y = pos.y;
          ispressed = pos.x != -1;
          if( ispressed ) log_d("touch pressed x:%d, y:%d", t_x, t_y );
        #else
          ispressed = SDU_GFX->getTouch(&t_x, &t_y);
        #endif

      #endif

      float barprogress = float(millis() - msec) / float(trigger->waitdelay);
      int linewidth = float(ts.pgbar_w) * barprogress;
      if( linewidth > 0 ) {
        int linepos = ts.pgbar_w - ( linewidth +1 );
        uint16_t grayscale = 255 - (192*barprogress);
        SDU_GFX->drawFastHLine( ts.pgbar_x,         ts.pgbar_y, ts.pgbar_w-linewidth-1, SDU_GFX->color565( grayscale, grayscale, grayscale ) );
        SDU_GFX->drawFastHLine( ts.pgbar_x+linepos, ts.pgbar_y, 1,                      TFT_BLACK );
      }
      return false;
    }

    static void triggerFinalizeTouch( triggerMap_t* trigger, int ret )
    {

      auto el = (touchTriggerElements_t*)trigger->sharedptr;
      if(!el) {
        log_e("No trigger elements to delete!");
        return;
      }

      auto LoadBtn    = el->LoadBtn;
      auto SkipBtn    = el->SkipBtn;
      auto SaveBtn    = el->SaveBtn;

      #if ! defined HAS_LGFX // defined _M5Core2_H_ || defined _M5CORES3_H_
        // clear TFT_eSpi button handlers
        if( SDUCfg.Buttons[0].enabled ) LoadBtn->delHandlers();
        if( SDUCfg.Buttons[1].enabled ) SkipBtn->delHandlers();
        if( SDUCfg.binFileName != nullptr && SDUCfg.Buttons[2].enabled ) SaveBtn->delHandlers();
      #endif
      delete LoadBtn;
      delete SkipBtn;
      delete SaveBtn;
      delete el;
      trigger->sharedptr = nullptr;
    }


    // static int touchButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay )
    // {
    //   /* auto &tft = M5.Lcd; */
    //   if( waitdelay == 0 ) return -1;
    //   // touch support + buttons
    //   SDU_TouchButton *LoadBtn = new SDU_TouchButton();
    //   SDU_TouchButton *SkipBtn = new SDU_TouchButton();
    //   SDU_TouchButton *SaveBtn = new SDU_TouchButton();
    //
    //   SDU_UI::TouchButtonWrapper tbWrapper;
    //   SDU_UI::TouchStyles ts;
    //
    //   #if !defined HAS_LGFX
    //     LoadBtn->setFont(nullptr);
    //     SkipBtn->setFont(nullptr);
    //     if( SDUCfg.binFileName != nullptr ) {
    //       SaveBtn->setFont(nullptr);
    //     }
    //   #endif
    //
    //   LoadBtn->initButton(
    //     SDU_GFX,
    //     ts.x1, ts.y,  ts.w, ts.h,
    //     ts.Load->BorderColor, ts.Load->FillColor, ts.Load->TextColor,
    //     labelLoad, ts.btn_fsize
    //   );
    //   SkipBtn->initButton(
    //     SDU_GFX,
    //     ts.x2, ts.y,  ts.w, ts.h,
    //     ts.Skip->BorderColor, ts.Skip->FillColor, ts.Skip->TextColor,
    //     labelSkip, ts.btn_fsize
    //   );
    //
    //   if( SDUCfg.binFileName != nullptr ) {
    //     SaveBtn->initButton(
    //       SDU_GFX,
    //       ts.x3, ts.y1,  ts.w, ts.h,
    //       ts.Save->BorderColor, ts.Save->FillColor, ts.Save->TextColor,
    //       labelSave, ts.btn_fsize
    //     );
    //     SaveBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);
    //     SaveBtn->drawButton();
    //     SaveBtn->press(false);
    //   }
    //
    //   LoadBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);
    //   SkipBtn->setLabelDatum(ts.padx, ts.pady, MC_DATUM);
    //
    //   LoadBtn->drawButton();
    //   SkipBtn->drawButton();
    //
    //   LoadBtn->press(false);
    //   SkipBtn->press(false);
    //
    //   uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
    //   bool ispressed = false;
    //   int retval = -1; // return status
    //
    //   SDU_GFX->drawFastHLine( ts.pgbar_x, ts.pgbar_y, ts.pgbar_w-1, TFT_WHITE );
    //
    //   auto msectouch = millis();
    //   do {
    //
    //     if( tbWrapper.iconRendered == false ) {
    //       tbWrapper.pushIcon( labelLoad );
    //       tbWrapper.iconRendered = true;
    //     }
    //     tbWrapper.handlePressed( LoadBtn, ispressed, t_x, t_y );
    //     tbWrapper.handlePressed( SkipBtn, ispressed, t_x, t_y );
    //     if( SDUCfg.binFileName != nullptr ) {
    //       tbWrapper.handlePressed( SaveBtn, ispressed, t_x, t_y );
    //     }
    //     tbWrapper.handleJustPressed( LoadBtn, labelLoad );
    //     tbWrapper.handleJustPressed( SkipBtn, labelSkip );
    //     if( SDUCfg.binFileName != nullptr ) {
    //       tbWrapper.handleJustPressed( SaveBtn, labelSave );
    //     }
    //
    //     if( tbWrapper.justReleased( LoadBtn, ispressed, labelLoad ) ) {
    //       retval = 1;
    //       log_v("LoadBTN Pressed at [%d:%d]!", t_x, t_y);
    //       break;
    //     }
    //     if( tbWrapper.justReleased( SkipBtn, ispressed, labelSkip ) ) {
    //       retval = 0;
    //       log_v("SkipBTN Pressed at [%d:%d]!", t_x, t_y);
    //       break;
    //     }
    //     if( SDUCfg.binFileName != nullptr ) {
    //       if( tbWrapper.justReleased( SaveBtn, ispressed, labelSave ) ) {
    //         retval = 2;
    //         log_v("SaveBtn Pressed at [%d:%d]!", t_x, t_y);
    //         break;
    //       }
    //     }
    //
    //     #if defined HAS_LGFX
    //       lgfx::touch_point_t tp;
    //       uint16_t number = SDU_GFX->getTouch(&tp, 1);
    //       t_x = tp.x;
    //       t_y = tp.y;
    //       ispressed = number > 0;
    //     #else // M5Core2.h / TFT_eSPI_Button syntax
    //       ispressed = SDU_GFX->getTouch(&t_x, &t_y);
    //     #endif
    //
    //     float barprogress = float(millis() - msectouch) / float(waitdelay);
    //     int linewidth = float(ts.pgbar_w) * barprogress;
    //     if( linewidth > 0 ) {
    //       int linepos = ts.pgbar_w - ( linewidth +1 );
    //       uint16_t grayscale = 255 - (192*barprogress);
    //       SDU_GFX->drawFastHLine( ts.pgbar_x,         ts.pgbar_y, ts.pgbar_w-linewidth-1, SDU_GFX->color565( grayscale, grayscale, grayscale ) );
    //       SDU_GFX->drawFastHLine( ts.pgbar_x+linepos, ts.pgbar_y, 1,                      TFT_BLACK );
    //     }
    //
    //   } while (millis() - msectouch < waitdelay);
    //
    //   #if defined _M5Core2_H_
    //     // clean handlers
    //     LoadBtn->delHandlers();
    //     SkipBtn->delHandlers();
    //     SaveBtn->delHandlers();
    //   #endif
    //
    //   delete LoadBtn;
    //   delete SkipBtn;
    //   delete SaveBtn;
    //
    //   return retval;
    // }

  }; // end TriggerSource namespace

};

