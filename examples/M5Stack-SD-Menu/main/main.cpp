#include "../menu.h"


void setup()
{

  #if defined __M5UNIFIED_HPP__
    M5.Log.setEnableColor(m5::log_target_serial, false);
  #endif

  #ifdef ARDUINO_M5STACK_FIRE
    spicommon_periph_free( VSPI_HOST ); // fix 2.0.4 psramInit mess
  #endif

  #if defined(ARDUINO_M5STACK_ATOM_AND_TFCARD)
    SDUCfg.setDisplay(&tft);
    tft.init();
  #else
    M5.begin(); // bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable, bool ScreenShotEnable
  #endif


  SDUCfg.setFS( &M5_FS );
  SDUCfg.setCSPin( TFCARD_CS_PIN );

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
    xTaskCreatePinnedToCore( loopTask, "loopTask", 16384, NULL, 1, NULL, 1 );
  }

}
#endif
