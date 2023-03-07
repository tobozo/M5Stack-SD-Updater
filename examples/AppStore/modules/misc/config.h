#pragma once

#include "core.h"
#include <vector>
#include <M5StackUpdater.h>

#define SD_CERT_PATH "/cert" // Filesystem (SD) temporary path where certificates are stored. without trailing slash
#define ROOT_DIR    "/"
#define CATALOG_DIR "/catalog"
#define CATALOG_DIR_BKP "/catalog.old"
#define HIDDEN_APPS_FILE "/.hidden-apps.json"

#define REGISTRY_MASTER "master"
#define REGISTRY_UNSTABLE "unstable"

#define PATH_SEPARATOR "/"
#define EXT_bin        ".bin"
#define EXT_json       ".json"
#define EXT_jpg        ".jpg"
#define EXT_png        ".png"

#define DIR_jpg  "/jpg/"
#define DIR_png  "/png/"
#define DIR_json "/json/"

#define DEFAULT_REGISTRY_NAME "SDUpdater"
#define DEFAULT_REGISTRY_DESC "Tobozo's " PLATFORM_NAME " Application registry @ phpsecu.re"
#define DEFAULT_REGISTRY_URL "https://phpsecu.re/" DEFAULT_REGISTRY_BOARD "/registry/phpsecu.re.json" // should exist as "/.registry/default.json" on SD Card
#define DEFAULT_REGISTRY_CHANNEL "unstable" // "master" or "unstable"

#define DEFAULT_MASTER_DESC "Master channel at phpsecu.re/" DEFAULT_REGISTRY_BOARD " registry"
#define DEFAULT_MASTER_URL "https://phpsecu.re/" DEFAULT_REGISTRY_BOARD "/sd-updater/"
#define DEFAULT_MASTER_API_HOST "phpsecu.re"
#define DEFAULT_MASTER_API_PATH PATH_SEPARATOR DEFAULT_REGISTRY_BOARD
#define DEFAULT_MASTER_API_CERT_PATH "/cert/"
#define DEFAULT_MASTER_UPDATER_PATH "/sd-updater"
#define DEFAULT_MASTER_CATALOG_ENDPOINT "/catalog.json"

#define DEFAULT_UNSTABLE_DESC "Unstable channel at phpsecu.re/" DEFAULT_REGISTRY_BOARD " registry"
#define DEFAULT_UNSTABLE_URL "https://phpsecu.re/" DEFAULT_REGISTRY_BOARD "/sd-updater/unstable/"
#define DEFAULT_UNSTABLE_API_HOST "phpsecu.re"
#define DEFAULT_UNSTABLE_API_PATH PATH_SEPARATOR DEFAULT_REGISTRY_BOARD
#define DEFAULT_UNSTABLE_API_CERT_PATH "/cert/"
#define DEFAULT_UNSTABLE_UPDATER_PATH "/sd-updater/unstable"
#define DEFAULT_UNSTABLE_CATALOG_ENDPOINT "/catalog.json"

#define LIST_MAX_COUNT      96

#define MENU_TITLE_MAX_SIZE 24
#define BTN_TITLE_MAX_SIZE  6

#define LIST_MAX_LABEL_SIZE 36 // list labels will be trimmed
#define LINES_PER_PAGE      8

#if !defined BUTTON_HEIGHT
  #define BUTTON_HEIGHT   28
#endif

#if !defined BUTTON_WIDTH
  #define BUTTON_WIDTH  60
#endif

#if !defined BUTTON_HWIDTH
  #define BUTTON_HWIDTH BUTTON_WIDTH/2
#endif

#define TITLEBAR_HEIGHT  32
#define WINDOW_MARGINX   10
#define LISTITEM_OFFSETX 15
#define LISTITEM_OFFSETY 42
#define LISTITEM_HEIGHT  20
#define LISTCAPTION_POSY 38
#define ASSET_POSY       62

#define BUTTONS_COUNT 3


#define MAX_BRIGHTNESS 100

#define MS_BEFORE_SLEEP 600000 // 600000 = 10mn

const uint32_t MENU_COLOR     = 0x008000U; // must be dark (0x00) on two colors
const uint32_t BG_COLOR       = 0x000000U; // default bgcolor when nothing is drawn (e.g. behind the buttons)
const uint32_t TEXT_COLOR     = 0xffffffU; // text color, must have high contrast compared to MENU_COLOR
const uint32_t DIMMED_COLOR   = 0xaaaaaaU; // text color, must have high contrast compared to MENU_COLOR
const uint32_t LAUNCHER_COLOR = 0xcccc00U; // text color, must have high contrast compared to MENU_COLOR
const uint32_t SHADOW_COLOR   = 0x202020U; // shadow color, must be a shaded version of TEXT_COLOR

const uint32_t GZ_PROGRESS_COLOR   = 0x40bb40U; // uint32_t ProgressBarColor1 (gzip)
const uint32_t TAR_PROGRESS_COLOR  = 0xbbbb40U; // uint32_t ProgressBarColor2 (tar)
const uint32_t DL_PROGRESS_COLOR   = 0xff0000U; // uint32_t ProgressBarColor3 (download/sha_sum)

uint16_t buttonsXOffset[BUTTONS_COUNT] =
{
  31, 126, 221
};

#if defined USE_SCREENSHOT
  static bool ScreenShotEnable = true;
#else
  static bool ScreenShotEnable = false;
#endif


#undef FS_CAN_CREATE_PATH

#include "esp32-hal-log.h"

#if defined ESP_ARDUINO_VERSION_VAL
  #if __has_include("core_version.h") // for platformio
    #include "core_version.h"
  #endif

  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2,0,1) || ARDUINO_ESP32_GIT_VER == 0x15bbd0a1 || ARDUINO_ESP32_GIT_VER == 0xd218e58f || ARDUINO_ESP32_GIT_VER == 0xcaef4006
    // #pragma message "Filesystem can create subfolders on file creation"
    #define FS_CAN_CREATE_PATH
  #endif

  #if ARDUINO_ESP32_GIT_VER == 0x15bbd0a1
    //#pragma message "ESP32 Arduino 2.0.1 RC1 (0x15bbd0a1) is only partially supported"

  #elif ARDUINO_ESP32_GIT_VER == 0xd218e58f
    //#pragma message "ESP32 Arduino 2.0.1 (0xd218e58f) has OTA support broken!!"

  #elif ARDUINO_ESP32_GIT_VER == 0xcaef4006
    // readRAW() / writeRAW() / numSectors() / sectorSize() support
    //#pragma message "ESP32 Arduino 2.0.2 (0xcaef4006) has SD support broken!!"

  #else
    // unknown but probably 2.0.0
    //#pragma message "ESP32 Arduino 2.x.x (unknown)"
    #undef FS_CAN_CREATE_PATH
  #endif
#endif

