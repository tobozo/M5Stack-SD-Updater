#pragma once

//#include <FFat.h>
#include <SD.h>

#define ECC_NO_PRAGMAS // turn ESP32-Chimera-Core's pragma messages off
#define ECC_NO_SCREENSHOT // comment this out to take screenshots
#define ECC_NO_SPEAKER // comment this out to use audio
#define ECC_NO_NVSUTILS
#define ECC_NO_POWER
#define ECC_NO_MPU
#define ECC_NO_RTC
#include <ESP32-Chimera-Core.h> // use LGFX display autodetect

//#include <M5Unified.h>

#include <ESP32-targz.h> // optional: https://github.com/tobozo/ESP32-targz
#define SDU_NO_PRAGMAS // turn M5StackUpdater's pragma messages off
#define SDU_APP_NAME "Application Launcher"

#if defined(ARDUINO_M5STACK_ATOM_AND_TFCARD)

  #if defined _CLK && defined _MISO && defined _MOSI
    #if !defined SDU_SPI_MODE
      #define SDU_SPI_MODE SPI_MODE3
    #endif
    #if !defined SDU_SPI_FREQ
      #define SDU_SPI_FREQ 80000000
    #endif
    #define SDU_SD_BEGIN [](int csPin)->bool{ SPI.begin(_CLK, _MISO, _MOSI, csPin); SPI.setDataMode(SDU_SPI_MODE); return SD.begin(csPin, SPI, SDU_SPI_FREQ); }
  #endif

  class LGFX_8BIT_CVBS : public lgfx::LGFX_Device
  {
    public:
      lgfx::Panel_CVBS _panel_instance;
      LGFX_8BIT_CVBS(void)
      {
        {
          auto cfg = _panel_instance.config();
          cfg.memory_width  = 320;
          cfg.memory_height = 240;
          cfg.panel_width   = 320 - 8;
          cfg.panel_height  = 240 - 16;
          cfg.offset_x      = 4;
          cfg.offset_y      = 8;
          _panel_instance.config(cfg);
        }
        {
          auto cfg = _panel_instance.config_detail();
          cfg.signal_type  = cfg.signal_type_t::NTSC_J;
          cfg.pin_dac      = 26;
          cfg.use_psram    = 0;
          cfg.output_level = 128;
          cfg.chroma_level = 128;
          _panel_instance.config_detail(cfg);
        }
        setPanel(&_panel_instance);
      }
  };

  static LGFX_8BIT_CVBS tft;

#elif defined __M5UNIFIED_HPP__

  M5GFX &tft( M5.Lcd );

#else

  M5Display &tft( M5.Lcd );

#endif

#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater

static SDU_Sprite sprite = SDU_Sprite( &tft );
fs::SDFS &M5_FS(SD);

void progressBar( SDU_DISPLAY_TYPE tft, int x, int y, int w, int h, uint8_t val, uint16_t color = 0x09F1, uint16_t bgcolor = 0x0000 );
