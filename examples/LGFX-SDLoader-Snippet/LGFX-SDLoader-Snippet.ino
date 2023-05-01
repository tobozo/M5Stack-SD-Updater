// M5Stack classic buttons/SD pinout
#define SDU_BUTTON_A_PIN 39
#define SDU_BUTTON_B_PIN 38
#define SDU_BUTTON_C_PIN 37
#define TFCARD_CS_PIN 4

// usual SD+TFT stack
#define LGFX_AUTODETECT
#define LGFX_USE_V1
#include <SD.h>
#include <LovyanGFX.h>

// the display object
static LGFX tft;

// #include <M5GFX.h>
// #define LGFX M5GFX // just alias to LGFX for SD-Updater

// A GPIO button library mocking M5.BtnX Buttons API, for the sakes of this demo
#include "M5Stack_Buttons.h" // stolen from M5Stack Core
//#define USE_TOUCH_BUTTONS // uncomment this use Touch UI

#define SDU_NO_AUTODETECT           // Disable autodetect (only works with <M5xxx.h> and <Chimera> cores)
#define SDU_USE_DISPLAY             // Enable display functionalities (lobby, buttons, progress loader)
#define HAS_LGFX                    // Display UI will use LGFX API (without this it will be tft_eSPI API)
#define SDU_TouchButton LGFX_Button // Set button renderer
#define SDU_Sprite LGFX_Sprite      // Set sprite type
#define SDU_DISPLAY_TYPE LGFX*      // Set display driver type
#define SDU_DISPLAY_OBJ_PTR &tft    // Set display driver pointer

#if defined ARDUINO_M5STACK_Core2 || defined USE_TOUCH_BUTTONS
  #define SDU_HAS_TOUCH             // Enable touch buttons
  #define SDU_TRIGGER_SOURCE_DEFAULT TriggerSource::SDU_TRIGGER_TOUCHBUTTON // Attach Touch buttons as trigger source
#else
   // Use any type of push button
  #define SDU_TRIGGER_SOURCE_DEFAULT TriggerSource::SDU_TRIGGER_PUSHBUTTON // Attach push buttons as trigger source
#endif

// #define _CLK  1
// #define _MISO 2
// #define _MOSI 3

#define SDU_APP_NAME "LGFX Loader Snippet"
#include <ESP32-targz.h> // optional: https://github.com/tobozo/ESP32-targz
#include <M5StackUpdater.h>



static SDUButton *BtnA = new SDUButton(SDU_BUTTON_A_PIN, true, 10);
static SDUButton *BtnB = new SDUButton(SDU_BUTTON_B_PIN, true, 10);
static SDUButton *BtnC = new SDUButton(SDU_BUTTON_C_PIN, true, 10);

static void ButtonUpdate()
{
  if( BtnA ) BtnA->read();
  if( BtnB ) BtnB->read();
  if( BtnC ) BtnC->read();
}


void setup()
{
  Serial.begin(115200);
  Serial.println("LGFX Example");

  tft.init();

  // SDUCfg.setDisplay( &tft ); // attach LGFX to SD-Updater, only mandatory if SDU_DISPLAY_OBJ_PTR isn't defined earlier

  if( tft.touch() ) {

    log_d("Display has touch");

    SDUCfg.useBuiltinTouchButton();

  } else {

    log_d("Display has buttons");

    //BtnA = new SDUButton(SDU_BUTTON_A_PIN, true, 10);
    //BtnB = new SDUButton(SDU_BUTTON_B_PIN, true, 10);
    //BtnC = new SDUButton(SDU_BUTTON_C_PIN, true, 10);

    ButtonUpdate();

    SDUCfg.setBtnPoller( [](){ ButtonUpdate(); } );
    SDUCfg.setBtnA( []() -> bool{ return BtnA->isPressed(); } );
    SDUCfg.setBtnB( []() -> bool{ return BtnB->isPressed(); } );
    SDUCfg.setBtnC( []() -> bool{ return BtnC->isPressed(); } );

  }

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

  if( ! tft.touch() ) {
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

}
