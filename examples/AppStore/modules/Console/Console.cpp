#pragma once

#include "Console.hpp"
#include "../misc/config.h"
#include "../misc/i18n.h"

// Scrollable text window
LogWindow::LogWindow( LGFX* gfx, int32_t _x, int32_t _y, uint32_t _w, uint32_t _h,  uint32_t _fgcolor, uint32_t _bgcolor )
{
  x = _x;
  y = _y;
  w = _w;
  h = _h;
  fgcolor = _fgcolor;
  bgcolor = _bgcolor;

  if( gfx==nullptr ) gfx = &tft;
  if( w == 0 )       w = gfx->width()-LISTITEM_OFFSETX*2;
  if( h == 0 )       h = TITLEBAR_HEIGHT*3.5;
  if( x == -1 )      x = gfx->width()/2 - w/2;
  if( y == -1 )      y = 75;

  sprite = new LGFX_Sprite( gfx );
  sprite->setColorDepth(1);
  sprite->createSprite( w, h );
  sprite->setPaletteColor(0, bgcolor );
  sprite->setPaletteColor(1, fgcolor );
  sprite->setTextDatum( TL_DATUM );
  sprite->setTextWrap( false, true );
  sprite->setFont( &Font0 );
  th = sprite->fontHeight();
  tw = sprite->textWidth("@");
  sprite->setCursor( tw, th );
}

LogWindow::~LogWindow()
{
  sprite->deleteSprite();
}

void LogWindow::log( const char* str )
{
  if( str ) sprite->println( str );
  else sprite->println();
  render();
}

void LogWindow::clear()
{
  sprite->fillSprite( bgcolor );
  sprite->setCursor( tw, th );
  render();
}

void LogWindow::render()
{
  checkScroll();
  sprite->fillRect( 0, 0, sprite->width()-(tw+1), th, bgcolor );
  sprite->fillRect( sprite->width()-(tw+1), 0, tw, sprite->height(), bgcolor );
  sprite->pushSprite( x, y );
}

void LogWindow::checkScroll()
{
  int32_t posy = sprite->getCursorY();
  if( posy + th > sprite->height() ) {
    sprite->scroll( 0, -th );
    posy -= th;
  }
  sprite->setCursor( tw, posy );
}
