#pragma once

#include <ESP32-Chimera-Core.h> // https://github.com/tobozo/ESP32-Chimera-Core

#define SDU_APP_NAME   "M5Stack App Store" // app title for the sd-updater lobby screen
#define SDU_APP_PATH   "/AppStore.bin"     // app binary file name on the SD Card (also displayed on the sd-updater lobby screen)
#define SDU_APP_AUTHOR "@tobozo"           // app binary author name for the sd-updater lobby screen
#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater

#define DEST_FS_USES_SD      // -> This instance will be ecompressing to the SDCard
#include <ESP32-targz.h>     // https://github.com/tobozo/ESP32-targz

#include <nvs.h> // use to store some prefs

//#define USE_SCREENSHOT // keep this commented out unless you really need to take UI screenshots

// auto-select board
#if defined( ARDUINO_M5STACK_Core2 )
  #pragma message "M5Stack Core2 detected"
  #define PLATFORM_NAME "M5Core2"
  #define DEFAULT_REGISTRY_BOARD "m5core2"
#elif defined( ARDUINO_M5Stack_Core_ESP32 )
  #pragma message "M5Stack Classic detected"
  #define PLATFORM_NAME "M5Stack"
  #define DEFAULT_REGISTRY_BOARD "m5stack"
#elif defined( ARDUINO_M5STACK_FIRE )
  #pragma message "M5Stack Fire detected"
  #define PLATFORM_NAME "M5Fire"
  #define DEFAULT_REGISTRY_BOARD "m5fire"
#elif defined( ARDUINO_ODROID_ESP32 )
  #pragma message "Odroid Go detected"
  #define PLATFORM_NAME "Odroid-GO"
  #define DEFAULT_REGISTRY_BOARD "odroid"
#else
  #pragma message "Generic ESP32 detected"
  #define DEFAULT_REGISTRY_BOARD "esp32"
  #define PLATFORM_NAME "ESP32"
#endif

static M5Display &tft( M5.Lcd );
static fs::SDFS &M5_FS(SD);

static char formatBuffer[64];

static const char *formatBytes(long long bytes, char *str)
{
  const char *sizes[5] = { "B", "KB", "MB", "GB", "TB" };
  int i;
  double dblByte = bytes;
  for (i = 0; i < 5 && bytes >= 1024; i++, bytes /= 1024)
    dblByte = bytes / 1024.0;
  sprintf(str, "%.2f", dblByte);
  return strcat(strcat(str, " "), sizes[i]);
}

