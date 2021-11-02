#pragma once
#include <ESP32-Chimera-Core.h> // use LGFX display autodetect
M5Display &tft( M5.Lcd );
static LGFX_Sprite sprite = LGFX_Sprite( &tft );
