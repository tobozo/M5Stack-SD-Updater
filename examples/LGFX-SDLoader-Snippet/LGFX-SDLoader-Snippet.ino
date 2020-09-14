#include <LGFX_TFT_eSPI.h>
#include <M5StackUpdater.h>

static TFT_eSPI lcd;
static TFT_eSprite canvas(&lcd);

void setup()
{
  Serial.begin(115200);

  lcd.init();

  checkSDUpdater();
}

void loop()
{



}
