#pragma once

#include <SD.h>
#include <ESP32-Chimera-Core.h> // use LGFX display autodetect

#define SDU_APP_NAME "Application Launcher"
#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater

M5Display &tft( M5.Lcd );
static LGFX_Sprite sprite = LGFX_Sprite( &tft );
fs::SDFS &M5_FS(SD);
