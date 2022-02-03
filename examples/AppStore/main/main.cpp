
#include "../modules/AppStoreMain/AppStoreMain.cpp"


void setup()
{
  AppStore::setup();
}


void loop()
{
  AppStore::loop();
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
