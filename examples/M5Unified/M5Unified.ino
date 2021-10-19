#include <M5Unified.h>
#include <M5StackUpdater.h>

#include <esp_log.h>

void setup(void)
{
  M5.begin();

// M5.Power.setExtPower(true);  // If you need external port 5V output.
// M5.Imu.begin();              // If you need IMU
// SD.begin(4, SPI, 25000000);  // If you need SD card access
// Serial.begin(115200);        // If you need Serial

  M5.Display.setEpdMode(epd_mode_t::epd_fastest); /// For models with EPD: refresh control

  M5.Display.setBrightness(160); /// For models with LCD: backlight control (0~255)

  if (M5.Display.width() < M5.Display.height())
  { /// Landscape mode.
    M5.Display.setRotation(M5.Display.getRotation() ^ 1);
  }

  vTaskDelay(1);

  M5.update();

  SDUCfg.setLabelMenu("<< Menu");        // BtnA label: load menu.bin
  SDUCfg.setLabelSkip("Launch");         // BtnB label: skip the lobby countdown and run the app
  SDUCfg.setLabelSave("Save");           // BtnC label: save the sketch to the SD
  SDUCfg.setAppName( "M5Unified test" );     // lobby screen label: application name
  // SDUCfg.setBinFileName( "/M5Unified.bin" ); // if file path to bin is set for this app, it will be checked at boot and created if not exist

  /*
  if( M5.BtnA.wasPressed() ) {
    Serial.println("Will Load menu binary");
    updateFromFS( SD );
    ESP.restart();
  }
*/

  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    5000,         // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // (usually default=4 but your mileage may vary)
  );


}

void loop(void)
{


}

#if !defined ( ARDUINO )
extern "C" {
  void loopTask(void*)
  {
    setup();
    for (;;) {
      loop();
    }
    vTaskDelete(NULL);
  }

  void app_main()
  {
    xTaskCreatePinnedToCore(loopTask, "loopTask", 8192, NULL, 1, NULL, 1);
  }
}
#endif

