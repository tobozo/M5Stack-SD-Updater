
#pragma once

#include <M5GFX.h>

class LGFX_8BIT_CVBS : public lgfx::LGFX_Device {
public:
  lgfx::Panel_CVBS _panel_instance;

  LGFX_8BIT_CVBS(void) {
    {                                       // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();  // 表示パネル設定用の構造体を取得します。

      // 出力解像度を設定;
      cfg.memory_width  = 360;  // 出力解像度 幅
      cfg.memory_height = 240;  // 出力解像度 高さ

      // 実際に利用する解像度を設定;
      cfg.panel_width  = 360 - 8;       // 実際に使用する幅   (memory_width と同値か小さい値を設定する)
      cfg.panel_height = 240 - 16;  // 実際に使用する高さ (memory_heightと同値か小さい値を設定する)

      // 表示位置オフセット量を設定;
      cfg.offset_x = 4;  // 表示位置を右にずらす量 (初期値 0)
      cfg.offset_y = 8;  // 表示位置を下にずらす量 (初期値 0)

      _panel_instance.config(cfg);

      // 通常は memory_width と panel_width に同じ値を指定し、 offset_x = 0 で使用します。;
      // 画面端の表示が画面外に隠れるのを防止したい場合は、 panel_width の値をmemory_widthより小さくし、offset_x で左右の位置調整をします。;
      // 例えば memory_width より panel_width を 32 小さい値に設定した場合、offset_x に 16 を設定することで左右位置が中央寄せになります。;
      // 上下方向 (memory_height , panel_height , offset_y ) についても同様に、必要に応じて調整してください。;
    }

    {
      auto cfg = _panel_instance.config_detail();

      // 出力信号の種類を設定;
      // cfg.signal_type = cfg.signal_type_t::NTSC;
      cfg.signal_type = cfg.signal_type_t::NTSC_J;
      // cfg.signal_type = cfg.signal_type_t::PAL;
      // cfg.signal_type = cfg.signal_type_t::PAL_M;
      // cfg.signal_type = cfg.signal_type_t::PAL_N;

      // 出力先のGPIO番号を設定;
      cfg.pin_dac = 26;  // DACを使用するため、 25 または 26 のみが選択できます;

      // PSRAMメモリ割当の設定;
      cfg.use_psram = 0;  // 0=PSRAM不使用 / 1=PSRAMとSRAMを半々使用 / 2=全部PSRAM使用;

      // 出力信号の振幅の強さを設定;
      cfg.output_level = 128;  // 初期値128
      // ※ GPIOに保護抵抗が付いている等の理由で信号が減衰する場合は数値を上げる。;
      // ※ M5StackCore2 はGPIOに保護抵抗が付いているため 200 を推奨。;

      // 彩度信号の振幅の強さを設定;
      cfg.chroma_level = 128;  // 初期値128
      // 数値を下げると彩度が下がり、0で白黒になります。数値を上げると彩度が上がります。;

      _panel_instance.config_detail(cfg);
    }

    setPanel(&_panel_instance);
  }

  template <typename T>
  void clearDisplay(T color = 0) { fillScreen(color); }

  void display() {}

  void progressBar(int x, int y, int w, int h, uint8_t val, uint16_t color = 0x09F1, uint16_t bgcolor = 0x0000)
  {
	drawRect(x, y, w, h, color);
	if (val > 100)
	  val = 100;
	if (val == 0)
	{
	  fillRect(x + 1, y + 1, w - 2, h - 2, bgcolor);
	}
	else
	{
	  int fillw = (w * (((float)val) / 100.0)) - 2;
	  fillRect(x + 1, y + 1, fillw - 2, h - 2, color);
	  fillRect(x + fillw + 1, y + 1, w - fillw - 2, h - 2, bgcolor);
	}
  }

  const uint32_t &textcolor = _text_style.fore_rgb888;
  const uint32_t &textbgcolor = _text_style.back_rgb888;
  const textdatum_t &textdatum = _text_style.datum;
  const float &textsize = _text_style.size_x;
};
