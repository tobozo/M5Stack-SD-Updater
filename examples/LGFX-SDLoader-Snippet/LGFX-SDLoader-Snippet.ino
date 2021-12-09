#define LGFX_AUTODETECT
#define LGFX_USE_V1
#include <SD.h>
#include <LovyanGFX.h>

// #include <M5GFX.h>
// #define LGFX M5GFX // just alias to LGFX for SD-Updater

#include "M5Stack_Buttons.h" // stolen from M5Stack Core
#define TFCARD_CS_PIN 4

#define LGFX_ONLY
#define SDU_APP_NAME "LGFX Loader Snippet"
#include <M5StackUpdater.h>

static LGFX tft;

static Button *BtnA;
static Button *BtnB;
static Button *BtnC;

void ButtonUpdate()
{
  BtnA->read();
  BtnB->read();
  BtnC->read();
}

static int myActionTrigger( char* labelLoad, char* labelSkip, char* labelSave, unsigned long waitdelay )
{
  if( waitdelay > 100 ) { // show button labels
    //SDUCfg.onBefore();
    SDUCfg.onSplashPage( "SD Updater Options" );
    BtnStyles btns; // use default theme from library
    SDUCfg.onButtonDraw( labelLoad, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor, btns.Load.ShadowColor );
    SDUCfg.onButtonDraw( labelSkip, 1, btns.Skip.BorderColor, btns.Skip.FillColor, btns.Skip.TextColor, btns.Skip.ShadowColor );
    SDUCfg.onButtonDraw( labelSave, 1, btns.Save.BorderColor, btns.Save.FillColor, btns.Save.TextColor, btns.Save.ShadowColor );
  }
  auto msec = millis();
  do {
    ButtonUpdate();
    if( BtnA->isPressed() ) return 1; // SD-Load menu (or rollback if avaiblable)
    if( BtnB->isPressed() ) return 0; // skip SD Loader screen
    if( BtnB->isPressed() ) return 2; // save sketch to FS
  } while (millis() - msec < waitdelay);
  return -1;
}

void setup()
{
  Serial.begin(115200);

  tft.init();

  BtnA = new Button(39, true, 10);
  BtnB = new Button(38, true, 10);
  BtnC = new Button(37, true, 10);
  ButtonUpdate();

  setSDUGfx( &tft ); // attach LGFX to SD-Updater
  // SDUCfg.setProgressCb  ( myProgress );       // void (*onProgress)( int state, int size )
  // SDUCfg.setMessageCb   ( myDrawMsg );        // void (*onMessage)( const String& label )
  // SDUCfg.setErrorCb     ( myErrorMsg );       // void (*onError)( const String& message, unsigned long delay )
  // SDUCfg.setBeforeCb    ( myBeforeCb );       // void (*onBefore)()
  // SDUCfg.setAfterCb     ( myAfterCb );        // void (*onAfter)()
  // SDUCfg.setSplashPageCb( myDrawSplashPage ); // void (*onSplashPage)( const char* msg )
  // SDUCfg.setButtonDrawCb( myDrawPushButton ); // void (*onButtonDraw)( const char* label, uint8_t position, uint16_t outlinecolor, uint16_t fillcolor, uint16_t textcolor )
  SDUCfg.setWaitForActionCb( myActionTrigger );  // int  (*onWaitForAction)( char* labelLoad, char* labelSkip, unsigned long waitdelay )


  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    2000,         // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // (usually default=4 but your mileage may vary)
  );
}



void loop()
{
  ButtonUpdate();

  if( BtnA->wasPressed() ) {
    Serial.println("BtnA pressed !");
  }
  if( BtnB->wasPressed() ) {
    Serial.println("BtnB pressed !");
  }
  if( BtnC->wasPressed() ) {
    Serial.println("BtnC pressed !");
  }

}
