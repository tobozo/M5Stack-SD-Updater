#pragma once

#include "../misc/core.h"
#include "Assets.h"


class RemoteAsset
{
public:
  RemoteAsset( const char* p, uint32_t w, uint32_t h, const char* a ) : path(p), width(w), height(h), alt_text(a) { }
  const char* path; // jpg or png file
  uint32_t width;
  uint32_t height;
  const char* alt_text;
  void draw( LGFX* gfx, int32_t x=0, int32_t y=0, int32_t w=0, int32_t h=0 );
};


enum imageType_t
{
  IMG_JPG,
  IMG_PNG,
  IMG_RAW
};

class LocalAsset
{
public:
  LocalAsset( const unsigned char* d, size_t l, imageType_t t, uint32_t w, uint32_t h, const char* a ) : data(d), data_len(l), type(t), width(w), height(h), alt_text(a) { }
  const unsigned char* data; // MUST BE bytes array of png, jpg, or raw colors
  size_t data_len;
  imageType_t type;
  uint32_t width;
  uint32_t height;
  const char* alt_text;
  void draw( LGFX* gfx, int32_t x=0, int32_t y=0, int32_t w=0, int32_t h=0 );
};


namespace DummyAsset
{
  typedef void(*captionDrawer_t)( LGFX* gfx, const char* caption, int32_t x, int32_t y, uint32_t w, uint32_t h, const void* ffont, int datum, uint32_t fgcolor, uint32_t bgcolor );
  uint32_t fgcolor;
  uint32_t bgcolor;
  captionDrawer_t drawCaption = nullptr;
  LocalAsset *BrokenImage = nullptr;
  void setup( captionDrawer_t drawCb, LocalAsset *brokenImage, uint32_t _fgcolor, uint32_t _bgcolor );
  void drawAltText( LGFX* gfx, const char* caption, int32_t x, int32_t y, uint32_t w, uint32_t h );
};
