#define TFCARD_CS_PIN 4 // customize this

#include <M5Unified.h>                   // /!\ When using SdFat, always include LGFX/M5GFX *before* M5StackUpdater.h to prevent problems with macros

#define SDU_NO_AUTODETECT                // Disable SDUpdater autodetect: this prevents <SD.h> to be auto-selected, however it also disables board detection
#define USE_SDFATFS                      // Tell M5StackUpdater to load <SdFat.h> and wrap SdFat32 into fs::FS::SdFat32FSImpl

#define HAS_M5_API                       // Use M5 API (M5.BtnA, BtnB, BtnC...) for triggers
#define SDU_USE_DISPLAY                  // Enable display (progress bar, lobby, buttons)
#define HAS_LGFX                         // Use LGFX Family display driver with zoom, rotate
#define SDU_Sprite LGFX_Sprite           // Inherit Sprite type from M5GFX
#define SDU_DISPLAY_TYPE M5GFX*          // inherit display type from M5GFX
#define SDU_DISPLAY_OBJ_PTR &M5.Display  // alias display pointer from M5Unified
#define SDU_TouchButton LGFX_Button      // inherit Buttons types from M5Unified
#if !defined SDU_HAS_TOUCH && defined ARDUINO_M5STACK_Core2
  #define SDU_HAS_TOUCH
#endif


#include <M5StackUpdater.h>

SdFs sd;
auto SdFatSPIConfig = SdSpiConfig( TFCARD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(25) );


void setup()
{
  M5.begin();

  SDUCfg.use_rollback = false;                 // Disable rollbadk (loading the menu may be slower, but no false positives during tests)
  SDUCfg.setLabelMenu("< Menu");               // BtnA label: load menu.bin
  SDUCfg.setLabelSkip("Launch");               // BtnB label: skip the lobby countdown and run the app
  SDUCfg.setLabelSave("Save");                 // BtnC label: save the sketch to the SD
  SDUCfg.setAppName("SdFatUpdater test");      // lobby screen label: application name
  SDUCfg.setBinFileName("/SdFatUpdater.bin");  // if file path to bin is set for this app, it will be checked at boot and created if not exist

  checkSDUpdater(
    sd,             // filesystem (must be type SdFat32)
    MENU_BIN,       // path to binary (default=/menu.bin, empty string=rollback only)
    5000,           // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    &SdFatSPIConfig // usually default=4 but your mileage may vary
  );
}

void loop()
{
  // do your stuff
}
