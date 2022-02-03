#pragma once

#include "Console.hpp"

// Scrollable text window
class LogWindow
{
  public:
    LogWindow( LGFX* gfx=nullptr, int32_t _x=-1, int32_t _y=-1, uint32_t _w=0, uint32_t _h=0, uint32_t fgcolor=0x00ff00U, uint32_t bgcolor=0x000000U );
    ~LogWindow();
    void log( const char* str = nullptr );
    void clear();
    void render();
  private:
    int32_t x;
    int32_t y;
    uint32_t w;
    uint32_t h;
    uint32_t fgcolor;
    uint32_t bgcolor;
    LGFX_Sprite* sprite;
    uint16_t th; // text height
    uint16_t tw; // text width for "@" sign
    void checkScroll();
};
