
#if defined( ARDUINO_M5Stack_Core_ESP32 ) || defined( ARDUINO_M5STACK_FIRE ) // M5Stack Classic/Fire
  #include <ESP32-Chimera-Core.h> // https://github.com/tobozo/ESP32-Chimera-Core
  //#include <M5Stack.h> // https://github.com/m5stack/M5Stack/
#elif defined( ARDUINO_M5STACK_Core2 ) // M5Stack Core2
  #include <ESP32-Chimera-Core.h> // https://github.com/tobozo/ESP32-Chimera-Core
  //#include <M5Core2.h> // https://github.com/m5stack/M5Core2/
#elif defined( ARDUINO_M5Stick_C ) // M5StickC
  //#include <ESP32-Chimera-Core.h> // https://github.com/tobozo/ESP32-Chimera-Core
  #include <M5StickC.h> // https://github.com/m5stack/M5StickC/
#else
  #include <ESP32-Chimera-Core.h> // use LGFX display autodetect
#endif

#define tft M5.Lcd // syntax sugar, forward compat with other displays (i.e GO.Lcd)
