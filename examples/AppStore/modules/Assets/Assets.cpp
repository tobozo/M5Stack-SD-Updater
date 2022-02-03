#pragma once

#include "Assets.hpp"

void LocalAsset::draw( LGFX* gfx, int32_t x, int32_t y, int32_t w, int32_t h  )
{
  log_v("Image '%s'", alt_text);
  switch( type ) {
    case IMG_JPG: gfx->drawJpg( data, data_len, x, y, w>0?w:width, h>0?h:height ); break;
    case IMG_PNG: gfx->drawPng( data, data_len, x, y, w>0?w:width, h>0?h:height ); break;
    case IMG_RAW: gfx->pushImage( x, y, width, height, data ); break;
  }
}


void RemoteAsset::draw( LGFX* gfx, int32_t x, int32_t y, int32_t w, int32_t h )
{
  log_v("Image File: '%s'", path );
  fs::File iconFile = M5_FS.open( path  );
  if( !iconFile ) {
    log_v("File not found: %s, will draw caption", path );
    DummyAsset::drawAltText( gfx, alt_text, x, y, w>0?w:width, h>0?h:height );
    return;
  }

  if( String(path).endsWith(EXT_png) ) {
    gfx->drawPng( &iconFile, x, y, w>0?w:width, h>0?h:height );
  } else if( String(path).endsWith(EXT_jpg) ) {
    gfx->drawJpg( &iconFile, x, y, w>0?w:width, h>0?h:height );
  } else {
    log_w("Unsupported image format: %s, will draw caption", path );
    DummyAsset::drawAltText( gfx, alt_text, x, y, w>0?w:width, h>0?h:height );
  }
  iconFile.close();
}


namespace DummyAsset
{
  void drawAltText( LGFX* gfx, const char* caption, int32_t x, int32_t y, uint32_t w, uint32_t h )
  {
    if( drawCaption ) {
      drawCaption( gfx, caption, x, y, w, h, &Font0, MC_DATUM, fgcolor, bgcolor );
    }
    BrokenImage->draw( gfx, x, y );
  }

  void setup( captionDrawer_t drawCb, LocalAsset * brokenImage, uint32_t _fgcolor, uint32_t _bgcolor )
  {
    BrokenImage = brokenImage;
    drawCaption = drawCb;
    bgcolor = _bgcolor;
    fgcolor = _fgcolor;
  }

};
