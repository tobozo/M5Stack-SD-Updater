#define LGFX_AUTODETECT
#define LGFX_USE_V1
#include <SD.h>
#include <LovyanGFX.h>

// #include <M5GFX.h>
// #define LGFX M5GFX // just alias to LGFX for SD-Updater

#include "M5Stack_Buttons.h" // stolen from M5Stack Core

#define LGFX_ONLY
#define SDU_APP_NAME "LGFX Loader Snippet"
#include <M5StackUpdater.h>

// #define TFCARD_CS_PIN 22
#define BUTTON_A_PIN 39
#define BUTTON_B_PIN 38
#define BUTTON_C_PIN 37


static LGFX tft;

static Button *BtnA;
static Button *BtnB;
static Button *BtnC;

static void ButtonUpdate()
{
  BtnA->read();
  BtnB->read();
  BtnC->read();
}


void setup()
{
  Serial.begin(115200);
  Serial.println("LGFX Example");

  tft.init();
  tft.fillRect( 10, 10, 100, 100, TFT_BLUE );
  delay(1000);

  BtnA = new Button(BUTTON_A_PIN, true, 10);
  BtnB = new Button(BUTTON_B_PIN, true, 10);
  BtnC = new Button(BUTTON_C_PIN, true, 10);
  ButtonUpdate();

  SDUCfg.setDisplay( &tft ); // attach LGFX to SD-Updater

  SDUCfg.setSDUBtnPoller( [](){ ButtonUpdate(); } );
  SDUCfg.setBtnAChecker( []() -> bool{ return BtnA->isPressed(); } );
  SDUCfg.setBtnBChecker( []() -> bool{ return BtnB->isPressed(); } );
  SDUCfg.setBtnCChecker( []() -> bool{ return BtnC->isPressed(); } );

  // SDUCfg.setProgressCb  ( myProgress );       // void (*onProgress)( int state, int size )
  // SDUCfg.setMessageCb   ( myDrawMsg );        // void (*onMessage)( const String& label )
  // SDUCfg.setErrorCb     ( myErrorMsg );       // void (*onError)( const String& message, unsigned long delay )
  // SDUCfg.setBeforeCb    ( myBeforeCb );       // void (*onBefore)()
  // SDUCfg.setAfterCb     ( myAfterCb );        // void (*onAfter)()
  // SDUCfg.setSplashPageCb( myDrawSplashPage ); // void (*onSplashPage)( const char* msg )
  // SDUCfg.setButtonDrawCb( myDrawPushButton ); // void (*onButtonDraw)( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor )
  // SDUCfg.setWaitForActionCb( myActionTrigger );  // int  (*onWaitForAction)( char* labelLoad, char* labelSkip, unsigned long waitdelay )

  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    10000,         // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // (usually default=4 but your mileage may vary)
  );


  tft.setTextSize(2);
  tft.println( SDU_APP_NAME );

}



void loop()
{
  ButtonUpdate();

  if( BtnA->wasPressed() ) {
    tft.println("BtnA pressed !");
  }
  if( BtnB->wasPressed() ) {
    tft.println("BtnB pressed !");
  }
  if( BtnC->wasPressed() ) {
    tft.println("BtnC pressed !");
  }

}
