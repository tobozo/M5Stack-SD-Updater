#define TFCARD_CS_PIN 4 // customize this


#include <M5Unified.h> // Note: don't mix M5Unified with LovyanGFX
//#include <M5Core2.h>
//#include <M5Stack.h>
//#define LGFX_AUTODETECT
//#include <LovyanGFX.h>


// M5StackUpdater library configuration

#define SDU_NO_PRAGMAS                   // don't spawn pragma messages during compilation
#define SDU_ENABLE_GZ                    // auto-load ESP32-targz (implicit if ESP32-targz.h is included before M5StackUpdater.h)
//#include <ESP32-targz.h>                 // optional gzipped firmware support, overriden by SDU_ENABLE_GZ -> https://github.com/tobozo/ESP32-targz
#define SDU_NO_AUTODETECT                // Disable SDUpdater autodetect: this prevents <SD.h> to be auto-selected, however it also disables board detection
#define USE_SDFATFS                      // Tell M5StackUpdater to load <SdFat.h> and wrap SdFat32 into fs::FS::SdFat32FSImpl
#define SDU_APP_NAME "SdFatUpdater test"
#define SDU_APP_PATH "/SdFatUpdater.bin"

#if __has_include(<LovyanGFX.h>)
  LGFX lcd;
  #define HAS_M5_API                       // Use M5 API (M5.BtnA, BtnB, BtnC...) for triggers
  #define SDU_USE_DISPLAY                  // Enable display (progress bar, lobby, b
  #define HAS_LGFX                         // Use LGFX Family display driver with zo
  #define SDU_Sprite LGFX_Sprite           // Inherit Sprite type from LGFX
  #define SDU_DISPLAY_TYPE LGFX*           // inherit display type from LGFX
  #define SDU_DISPLAY_OBJ_PTR &lcd         // alias display pointer from lcd object
#elif __has_include(<M5Unified.h>)
  #define HAS_M5_API                       // Use M5 API (M5.BtnA, BtnB, BtnC...) for triggers
  #define SDU_USE_DISPLAY                  // Enable display (progress bar, lobby, buttons)
  #define HAS_LGFX                         // Use LGFX Family display driver with zoom, rotate
  #define SDU_Sprite LGFX_Sprite           // Inherit Sprite type from M5GFX
  #define SDU_DISPLAY_TYPE M5GFX*          // inherit display type from M5GFX
  #define SDU_DISPLAY_OBJ_PTR &M5.Display  // alias display pointer from M5Unified
  #if defined ARDUINO_M5STACK_Core2
    #define SDU_TouchButton LGFX_Button    // inherit Buttons types from M5Unified
    #define SDU_HAS_TOUCH
  #endif
#elif __has_include(<M5Core2.h>) || __has_include(<M5Stack.h>)
  #define HAS_M5_API                       // Use M5 API (M5.BtnA, BtnB, BtnC...) for triggers
  #define SDU_USE_DISPLAY                  // Enable display (progress bar, lobby, buttons)
  #define SDU_Sprite TFT_eSprite           // Inherit TFT_eSprite type from M5 Core
  #define SDU_DISPLAY_TYPE M5Display*      // inherit TFT_eSpi type from M5 Core
  #define SDU_DISPLAY_OBJ_PTR &M5.Lcd      // alias display pointer from M5 Core
  #if defined ARDUINO_M5STACK_Core2 && __has_include(<M5Core2.h>)
    #define SDU_TouchButton TFT_eSPI_Button // inherit Buttons types from TFT_eSPI
    #define SDU_HAS_TOUCH
  #endif

#else

  #error "This example only supports the following cores: LovyanGFX.h, M5Unified.h, M5Stack.h or M5Core2.h"

#endif

//#include <SdFat.h> // not necessary when `USE_SDFATFS` is defined
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
  SDUCfg.setAppName( SDU_APP_NAME );      // lobby screen label: application name
  SDUCfg.setBinFileName( SDU_APP_PATH );  // if file path to bin is set for this app, it will be checked at boot and created if not exist

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
