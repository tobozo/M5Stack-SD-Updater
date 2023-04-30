
#include <ESP32-targz.h>

#if defined TEST_LGFX

  #include <SD.h> // WTF platformio lib_dep manager fails to resolve this ???
  #include "../../../LGFX-SDLoader-Snippet/LGFX-SDLoader-Snippet.ino"

#elif defined TEST_M5Core2

  #include "../../../M5Core2-SDLoader-Snippet/M5Core2-SDLoader-Snippet.ino"

#elif defined TEST_M5Stack || defined TEST_S3Box

  #include "../../../M5Stack-SDLoader-Snippet/M5Stack-SDLoader-Snippet.ino"

#elif defined TEST_M5StickC

  #include "../../../M5StickC-SPIFFS-Loader-Snippet/M5StickC-SPIFFS-Loader-Snippet.ino"

#elif defined TEST_M5Unified

  #include <SD.h> // WTF platformio lib_dep manager fails to resolve this ???
  #include "../../../M5Unified/M5Unified.ino"

#else

  #error "No device to test"

#endif

