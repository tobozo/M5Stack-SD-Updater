#include <SD.h>

//#define LGFX_AUTODETECT
//#define LGFX_USE_V1
//#include <LovyanGFX.h>

#include <M5GFX.h>
#define LGFX M5GFX // just alias to LGFX for SD-Updater

#include "M5Stack_Buttons.h" // stolen from M5Stack Core
#define TFCARD_CS_PIN 4

#define LGFX_ONLY
#include <M5StackUpdater.h>

static LGFX tft;
//static LGFX_Sprite canvas(&tft);

static Button *BtnA;
static Button *BtnB;
static Button *BtnC;

void ButtonUpdate()
{
  BtnA->read();
  BtnB->read();
  BtnC->read();
}

static int assertStartUpdateFromButton( char* labelLoad, char* labelSkip, unsigned long waitdelay )
{
  if( waitdelay > 100 ) {
    freezeTextStyle();
    drawSDUMessage();
    BtnStyles btns;
    drawSDUPushButton( labelLoad, 0, btns.Load.BorderColor, btns.Load.FillColor, btns.Load.TextColor );
    drawSDUPushButton( labelSkip, 1, btns.Skip.BorderColor, btns.Skip.FillColor, btns.Skip.TextColor );
  }
  auto msec = millis();
  do {
    ButtonUpdate();
    if( BtnA->isPressed() ) return 1;
    if( BtnB->isPressed() ) return 0;
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
  setAssertTrigger( assertStartUpdateFromButton ); // attach custom button support
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
