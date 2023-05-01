
#include <SD.h>
#include <M5Unified.h>
//#define TFCARD_CS_PIN 4
#include <ESP32-targz.h> // optional: https://github.com/tobozo/ESP32-targz
#include <M5StackUpdater.h>

void setup(void)
{
  M5.begin();
  Serial.begin(115200);

  SDUCfg.setLabelMenu("< Menu");               // BtnA label: load menu.bin
  SDUCfg.setLabelSkip("Launch");               // BtnB label: skip the lobby countdown and run the app
  SDUCfg.setLabelSave("Save");                 // BtnC label: save the sketch to the SD
  SDUCfg.setAppName("M5Unified test");         // lobby screen label: application name
  SDUCfg.setBinFileName("/M5UnifiedTest.bin"); // if file path to bin is set for this app, it will be checked at boot and created if not exist

  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    5000,        // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // usually default=4 but your mileage may vary
  );

  M5.Display.print("M5Unified test");
}

void loop(void)
{
  // do your stuff
}

