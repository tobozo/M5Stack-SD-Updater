#pragma once

#include "../misc/types.h"

#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"


namespace SDUpdaterNS
{

  namespace ConfigManager
  {

  }


  namespace SDU_UI
  {

    static void SDMenuProgressUI( int state, int size );
    static void DisplayUpdateUI( const String& label );
    static void DisplayErrorUI( const String& msg, unsigned long wait );
    static void freezeTextStyle();
    static void thawTextStyle();
    static void drawSDUSplashPage( const char* msg );
    static void drawSDUPushButton( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor, uint16_t shadowcolor );
    static void fillStyledRect( SplashPageElementStyle_t *style, int32_t x, int32_t y, uint16_t width, uint16_t height );
    static void adjustFontSize( uint8_t *lineHeightBig, uint8_t *lineHeightSmall );
    static void drawTextShadow( const char* text, int32_t x, int32_t y, uint16_t textcolor, uint16_t shadowcolor );
    static void drawSDUSplashElement( const char* msg, int32_t x, int32_t y, SplashPageElementStyle_t *style );
    //void DisplayUpdateHeadless( const String& label );
    //void SDMenuProgressHeadless( int state, int size );


    static void inline DisplayUpdateHeadless( const String& label )
    {
      // TODO: draw some fancy serial output
    };

    static void inline SDMenuProgressHeadless( int state, int size )
    {
      static int Headless_Progress;
      int percent = ( state * 100 ) / size;
      if( percent == Headless_Progress ) {
        // don't render twice the same value
        return;
      }
      //Serial.printf("percent = %d\n", percent); // this is spammy
      Headless_Progress = percent;
      if ( percent >= 0 && percent < 101 ) {
        Serial.print( "." );
      } else {
        Serial.println();
      }
    };





  }

  namespace TriggerSource
  {
    // static int pushButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay=5000  );
    // //static int touchButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay=5000 );
    // static int serial( char* labelLoad,  char* labelSkip, char* labelSave, unsigned long waitdelay=5000  );

    static void triggerInitSerial(triggerMap_t* trigger) { }
    static bool triggerActionSerial(triggerMap_t* trigger, uint32_t msec )
    {
      if( Serial.available() ) {
        String out = Serial.readStringUntil('\n');
        if(      out == "update" ) { trigger->ret = 1; return true; }
        else if( out == "rollback") { trigger->ret = 0; return true; }
        else if( out == "skip" ) { trigger->ret = -1; return true; }
        else if( out == "save" ) { trigger->ret = 2; return true; }
        else Serial.printf("Ignored command: %s\n", out.c_str() );
      }
      return false;
    }
    static void triggerFinalizeSerial( triggerMap_t* trigger, int ret ) {  }


    static void triggerInitButton(triggerMap_t* trigger)
    {
      using namespace SDU_UI;
      log_d("Init button");
      if( trigger->waitdelay > 100 ) {
        log_d("Waitdelay: %d", trigger->waitdelay );
        if( SDUCfg.onBefore ) SDUCfg.onBefore();
        if( SDUCfg.onSplashPage ) SDUCfg.onSplashPage( BTN_HINT_MSG );
        if( SDUCfg.onButtonDraw ) {
          log_d("Drawing buttons");
          const BtnStyles_t btns;
          if( SDUCfg.Buttons[0].enabled) SDUCfg.onButtonDraw( trigger->labelLoad, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor, btns.Load.ShadowColor );
          if( SDUCfg.Buttons[1].enabled) SDUCfg.onButtonDraw( trigger->labelSkip, 1, btns.Skip.BorderColor, btns.Skip.FillColor, btns.Skip.TextColor, btns.Skip.ShadowColor );
          if( SDUCfg.binFileName != nullptr && SDUCfg.Buttons[2].enabled ) {
            SDUCfg.onButtonDraw( trigger->labelSave, 2, btns.Save.BorderColor, btns.Save.FillColor, btns.Save.TextColor, btns.Save.ShadowColor );
          }
        } else {
          log_d("No buttondraw!");
        }
      } else {
        log_d("No Waitdelay! (%d<100)", trigger->waitdelay );
      }
      if( SDUCfg.onProgress ) SDUCfg.onProgress( 100, 100 );

      #if defined SDU_Sprite
        loaderAnimator_t *loadingAnimation = new loaderAnimator_t();
        trigger->sharedptr = loadingAnimation;
        if( loadingAnimation ) {
          loadingAnimation->init();
        }
      #endif
    }

    static bool triggerActionButton(triggerMap_t* trigger, uint32_t msec )
    {
      using namespace SDU_UI;
      static uint32_t progress = 0, progressOld=1;
      SDUCfg.buttonsPoll();

      for( int i=0; i<3; i++ ) {
        if( SDUCfg.Buttons[i].changed() ) {
          log_v("SDUCfg.Buttons[%d] was triggered", i);
          trigger->ret = SDUCfg.Buttons[i].val;
          return true;
        }
      }
      if( SDUCfg.onProgress   ) {
        float barprogress = float(millis() - msec) / float(trigger->waitdelay);
        progress = 100- (100 * barprogress);
        if (progressOld != progress) {
          progressOld = progress;
          SDUCfg.onProgress( (uint8_t)progress, 100 );
        }
      }
      #if defined SDU_Sprite
        loaderAnimator_t *loadingAnimation = (loaderAnimator_t *)trigger->sharedptr;
        if( loadingAnimation ) {
          loadingAnimation->animate();
        }
      #endif
      return false;
    }




    static void triggerFinalizeButton( triggerMap_t* trigger, int ret )
    {
      using namespace SDU_UI;
      if( SDUCfg.onProgress ) SDUCfg.onProgress( 0, 100 );
      #if defined SDU_Sprite
        loaderAnimator_t *loadingAnimation = (loaderAnimator_t *)trigger->sharedptr;
        if( loadingAnimation ) {
          loadingAnimation->deinit();
          delete loadingAnimation;
        }
        trigger->sharedptr = nullptr;
      #endif
      if( ret > -1 ) { // wait for button release
        log_v("Waiting for Button #%d to be released", ret );
        while( SDUCfg.Buttons[ret].changed() ) {
          //if( SDUCfg.buttonsUpdate ) SDUCfg.buttonsUpdate();
          SDUCfg.buttonsPoll();
          vTaskDelay(10);
        }
      }
    }



    //static int actionTriggered( triggerMap_t *trigger  )
    static int actionTriggered( char* labelLoad,  char* labelSkip, char* labelSave, unsigned long waitdelay )
    {
      auto trigger = SDUCfg.triggers;

      if( !trigger ) {
        log_e("No triggers assigned, aborting");
        return -1;
      }

      trigger->waitdelay = waitdelay;

      if( trigger->waitdelay == 0 ) {
        log_i("waitdelay=0 -> skipping action trigger detection");
        return -1;
      }

      switch( trigger->source ) {
        case SDU_TRIGGER_SERIAL:      log_d("Listening to trigger source: Serial, delay=%dms", trigger->waitdelay); break;
        case SDU_TRIGGER_PUSHBUTTON:  log_d("Listening to trigger source: Push Button, delay=%dms", trigger->waitdelay); break;
        case SDU_TRIGGER_TOUCHBUTTON: log_d("Listening to trigger source: Touch Button, delay=%dms", trigger->waitdelay); break;
      }
      auto msec = millis();
      int ret = -1;
      if( trigger->init ) {
        log_d("Trigger init");
        trigger->init( trigger );
      }
      do {
        if( trigger->get( trigger, msec  ) ) {
          ret = trigger->ret;
          break;
        }
      } while (millis() - msec < trigger->waitdelay);
      if( trigger->finalize ) {
        log_d("Trigger finalize");
        trigger->finalize( trigger, ret );
      }
      return ret;
    }



    // headless method
    static inline int serial( char* labelLoad,  char* labelSkip, char* labelSave, unsigned long waitdelay )
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

    static int pushButton( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay )
    {
      using namespace SDU_UI;
      auto msec = millis();
      if( waitdelay > 100 ) {
        if( SDUCfg.onBefore ) SDUCfg.onBefore();
        if( SDUCfg.onSplashPage ) SDUCfg.onSplashPage( BTN_HINT_MSG );
        if( SDUCfg.onButtonDraw ) {
          const BtnStyles_t btns;
          if( SDUCfg.Buttons[0].enabled) SDUCfg.onButtonDraw( labelLoad, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor, btns.Load.ShadowColor );
          if( SDUCfg.Buttons[1].enabled) SDUCfg.onButtonDraw( labelSkip, 1, btns.Skip.BorderColor, btns.Skip.FillColor, btns.Skip.TextColor, btns.Skip.ShadowColor );
          if( SDUCfg.binFileName != nullptr && SDUCfg.Buttons[2].enabled ) {
            SDUCfg.onButtonDraw( labelSave, 2, btns.Save.BorderColor, btns.Save.FillColor, btns.Save.TextColor, btns.Save.ShadowColor );
          }
        }
      }
      uint32_t progress = 0, progressOld = 1;
      if( SDUCfg.onProgress ) SDUCfg.onProgress( 100, 100 );

      #if defined SDU_Sprite
        loaderAnimator_t loadingAnimation;
        loadingAnimation.init();
      #endif

      int ret = -1;

      //if(!SDUCfg.buttonsUpdate) log_w("No button poller found in SDUCfg, does M5.update() run in another task ?");

      do {
        //if( SDUCfg.buttonsUpdate ) SDUCfg.buttonsUpdate();
        SDUCfg.buttonsPoll();

        for( int i=0; i<3; i++ ) {
          if( SDUCfg.Buttons[i].enabled && SDUCfg.Buttons[i].changed() ) {
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
        #if defined SDU_Sprite
          loadingAnimation.animate();
        #endif
      } while (millis() - msec < waitdelay);

      _endAssert:

      if( SDUCfg.onProgress ) SDUCfg.onProgress( 0, 100 );
      #if defined SDU_Sprite
        loadingAnimation.deinit();
      #endif
      if( ret > -1 ) { // wait for button release
        log_v("Waiting for Button #%d to be released", ret );
        while( SDUCfg.Buttons[ret].changed() ) {
          //if( SDUCfg.buttonsUpdate ) SDUCfg.buttonsUpdate();
          SDUCfg.buttonsPoll();
          vTaskDelay(10);
        }
      }
      return ret;
    }


    #if !defined SDU_HAS_TOUCH
      //struct triggerMap_t { };
      static void triggerInitTouch(triggerMap_t* trigger) { log_d("[NULLCB] Init"); }
      static bool triggerActionTouch(triggerMap_t* trigger, uint32_t msec ) { log_d("[NULLCB] trigger"); return false; }
      static void triggerFinalizeTouch( triggerMap_t* trigger, int ret ) { log_d("[NULLCB] Final"); }
    #endif

  };


  #if !defined SDU_USE_DISPLAY
    static inline void SDMenuProgressUI( int state, int size ) { log_d("[NULLCB] Progress: %d/%d", state, size); }
    static inline void DisplayUpdateUI( const String& label ) { log_d("[NULLCB] Update: %s", label.c_str()); }
    static inline void DisplayErrorUI( const String& msg, unsigned long wait ) { log_d("[NULLCB] Error: %s", msg.c_str()); delay(wait); }
    static inline void freezeTextStyle() { log_d("[NULLCB] Freezing"); }
    static inline void thawTextStyle() { log_d("[NULLCB] Thawing");}
    static inline void drawSDUSplashPage( const char* msg ) { log_d("[NULLCB] Splash Page: %s", msg ); }
    static inline void drawSDUPushButton( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor, uint16_t shadowcolor )
    {
      log_d("[NULLCB] PushButton '%s' [X:%dpx] [Outline:0x%04x] [Fill:0x%04x] [Text:0x%04x] [Shadow:0x%04x]", label, position, outlinecolor, fillcolor, textcolor, shadowcolor );
    }
  #endif



};
