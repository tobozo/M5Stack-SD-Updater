#pragma once

#define ROLLBACK_LABEL   "Rollback" // reload app from the "other" OTA partition
#define LAUNCHER_LABEL   "Launcher" // load Launcher (typically menu.bin)
#define SKIP_LABEL       "Skip >>|" // resume normal operations (=no action taken)
#define SAVE_LABEL       "Save"     // copy sketch binary to FS
#define BTN_HINT_MSG     "SD-Updater Lobby"
#define SDU_LOAD_TPL     "Will Load menu binary : %s\n"
#define SDU_ROLLBACK_MSG "Will Roll back"

#if !defined SDU_APP_PATH
  #define SDU_APP_PATH nullptr
#endif
#if !defined SDU_APP_NAME
  #define SDU_APP_NAME nullptr
#endif
#if !defined SDU_APP_AUTHOR
  #define SDU_APP_AUTHOR nullptr
#endif

#ifndef MENU_BIN
  #define MENU_BIN "/menu.bin"
#endif


// Fancy names for detected boards
#if defined ARDUINO_M5Stick_C
  #define SD_PLATFORM_NAME "M5StickC"
#elif defined ARDUINO_ODROID_ESP32
  #define SD_PLATFORM_NAME "Odroid-GO"
#elif defined ARDUINO_M5Stack_Core_ESP32
  #define SD_PLATFORM_NAME "M5Stack"
#elif defined ARDUINO_M5STACK_FIRE
  #define SD_PLATFORM_NAME "M5Stack-Fire"
#elif defined ARDUINO_M5STACK_Core2
  #define SD_PLATFORM_NAME "M5StackCore2"
#elif defined ARDUINO_ESP32_WROVER_KIT
  #define SD_PLATFORM_NAME "Wrover-Kit"
#elif defined ARDUINO_TTGO_T1             // TTGO T1
  #define SD_PLATFORM_NAME "TTGO-T1"
#elif defined ARDUINO_LOLIN_D32_PRO       // LoLin D32 Pro
  #define SD_PLATFORM_NAME "LoLin D32 Pro"
#elif defined ARDUINO_T_Watch             // TWatch, all models
  #define SD_PLATFORM_NAME "TTGO TWatch"
#elif defined ARDUINO_M5STACK_ATOM_AND_TFCARD
  #define SD_PLATFORM_NAME "Atom"
#elif defined ARDUINO_ESP32_S3_BOX
  #define SD_PLATFORM_NAME "S3Box"
#else
  #define SD_PLATFORM_NAME "ESP32"
#endif
