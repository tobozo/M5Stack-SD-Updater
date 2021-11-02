#include "../menu.h"


void setup() {
  #if defined(_CHIMERA_CORE_)
    M5.begin(true, false, true, false, false); // bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable, bool ScreenShotEnable
  #else
    M5.begin(); // bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable, bool ScreenShotEnable
  #endif

  SDUCfg.setFS( &M5_FS );
  SDUCfg.setCSPin( TFCARD_CS_PIN );
  sdUpdater = new SDUpdater();

  //WiFi.onEvent(WiFiEvent); // helps debugging WiFi problems with the Serial console
  UISetup(); // UI init and check if a SD exists

  doFSChecks(); // replicate on SD and app1 partition, scan data folder, load registry
  doFSInventory(); // enumerate apps and render menu

}


void loop() {

  HIDMenuObserve();
  sleepTimer();

}

#if !defined ARDUINO
extern "C" {
  void loopTask(void*)
  {
    setup();
    for(;;) {
      loop();
    }
  }
  void app_main()
  {
    xTaskCreatePinnedToCore( loopTask, "loopTask", 8192, NULL, 1, NULL, 1 );
  }

}
#endif
