#pragma once

#include <SD.h>
#include <ESP32-Chimera-Core.h> // use LGFX display autodetect
#include <ESP32-targz.h> // support gzipped applications

#define SDU_APP_NAME "Application Launcher"
#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater

#if defined(ARDUINO_M5STACK_ATOM_AND_TFCARD)

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

#else

  M5Display &tft( M5.Lcd );

#endif


static LGFX_Sprite sprite = LGFX_Sprite( &tft );
fs::SDFS &M5_FS(SD);

void progressBar( LGFX* tft, int x, int y, int w, int h, uint8_t val, uint16_t color = 0x09F1, uint16_t bgcolor = 0x0000 );
