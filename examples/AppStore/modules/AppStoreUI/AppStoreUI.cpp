#pragma once

#include "AppStoreUI.hpp"
#include "lgfx/utility/lgfx_qrcode.h"

using namespace MenuItems;
using namespace UIDraw;
using namespace UIDo;
using namespace UIUtils;

// destructor
AppStoreUI::~AppStoreUI()
{
  clearList();
}

// constructor
AppStoreUI::AppStoreUI( LGFX* _gfx, UITheme * theme )
{
  gfx   = _gfx;
  Theme = theme;
  Theme->setupCanvas( gfx );
  initSprites( gfx );
}

// some public getters
size_t            AppStoreUI::getListPages() {        return ListTotalPages; }
size_t            AppStoreUI::getListSize() {         return ListCount; }
uint16_t          AppStoreUI::getListID() {           return MenuID; }
uint16_t          AppStoreUI::getPageID() {           return PageID; }
const char*       AppStoreUI::getListTitle() {        return Menu->ActionLabels->title; }
const char*       AppStoreUI::getListItemTitle() {    return Menu->Actions[MenuID]->title; }
MenuActionLabels* AppStoreUI::getMenuActionLabels() { return Menu->ActionLabels; }
UITheme*          AppStoreUI::getTheme() {            return Theme; }
LGFX*             AppStoreUI::getGfx() {              return gfx; }


bool AppStoreUI::empty( const char* str )
{
  return str ? (str[0] == '\0') : true;
}


void AppStoreUI::clearList()
{
  PageID              = 0;
  MenuID              = 0;
  ListCount           = 0;
  ListTotalPages      = 0;
  ListLinesInLastPage = 0;
}


void AppStoreUI::setPageID( uint16_t idx )
{
  if( idx < ListCount/LinesPerPage ) {
    PageID = idx;
  }
}


void AppStoreUI::setListID( uint16_t idx )
{
  uint16_t was = PageID;
  if( idx < ListCount ) {
    MenuID = idx;
  } else {
    MenuID = ListCount -1;
  }
  PageID = MenuID / LinesPerPage;
  log_v("Setting list id as %d (was %d)", idx, was );
}


void AppStoreUI::nextList( bool renderAfter )
{
  if( MenuID < ( PageID * LinesPerPage + LinesInCurrentPage - 1 ) ) {
    MenuID++;
  } else {
    if( PageID<ListTotalPages - 1 ) {
      PageID++;
    } else {
      PageID = 0;
    }
    MenuID = PageID * LinesPerPage;
  }
  log_v("Next to %d/%d", PageID, MenuID);
  if( renderAfter ) showList();
}


void AppStoreUI::pageDown( bool renderAfter )
{
  if( PageID < ListTotalPages -1 ) {
    PageID++;
    MenuID = (PageID * LinesPerPage) -1;
    nextList( false );
  } else {
    PageID = 0;
    MenuID = 0;
  }
  log_v("Paging down to %d/%d", PageID, MenuID);
  if( renderAfter ) showList();
}


void AppStoreUI::pageUp( bool renderAfter )
{
  if( PageID > 0 ) {
    PageID--;
    MenuID -= LinesPerPage;
    log_v("Paging up to %d/%d", PageID, MenuID);
    if( renderAfter ) showList();
  }
}


void AppStoreUI::menuDown( bool renderAfter )
{
  int16_t lastId = MenuID;
  int16_t oldPageID = PageID;

  if( MenuID == ListCount-1 ) {
    setListID( 0 );
  } else {
    setListID( MenuID+1 );
  }

  if( PageID != oldPageID ) {
    if( renderAfter ) showList();
  } else {
    if( renderAfter ) updateList( lastId );
  }
}


void AppStoreUI::menuUp( bool renderAfter )
{
  int16_t oldPageID = PageID;
  int16_t lastId = MenuID;

  if( MenuID == 0 ) {
    // jump to end
    setListID( ListCount-1 );
  } else {
    setListID( MenuID-1 );
  }

  if( PageID != oldPageID ) {
    if( renderAfter ) showList();
  } else {
    if( renderAfter ) updateList( lastId );
  }
  log_v("Menu up to %d/%d", PageID, MenuID);

}


void AppStoreUI::setList( MenuGroup* menu )
{
  if( menu->actions_count == 0 ) {
    log_e("Cowardly refusing to insert an empty menu list for collection %s, aborting", menu->Title );
    return;
  }
  clearList();

  ListCount = menu->actions_count;

  if( ListCount>0 ) {
    if( ListCount > LinesPerPage ) {
      ListLinesInLastPage = ListCount % LinesPerPage;
      if( ListLinesInLastPage>0 ) {
        ListTotalPages = ( ListCount - ListLinesInLastPage ) / LinesPerPage;
        ListTotalPages++;
      } else {
        ListTotalPages = ListCount / LinesPerPage;
      }
    } else {
      ListTotalPages = 1;
    }
  }
  // if( Menu && menu->Parent != nullptr ) menu->Parent = Menu;
  Menu = menu;
  if( Menu->selectedindex > 0 ) {
    setListID( Menu->selectedindex );
  }
  log_v("Added list '%s' with %d elements", menu->Title, menu->actions_count );
}


void AppStoreUI::execBtn( uint8_t bnum )
{
  if( Menu->ActionLabels && Menu->ActionLabels->Buttons && Menu->ActionLabels->Buttons[bnum] && Menu->ActionLabels->Buttons[bnum]->onClick ) {
    log_v("Button #%d Inherited Callback for Item '%s' in menu '%s' (#%d / %d)", bnum, getListItemTitle(), getListTitle(), MenuID, Menu->actions_count );
    Menu->ActionLabels->Buttons[bnum]->onClick();
  } else {
    log_e("Button #%d MISSED Callback for Item #%d / %d", bnum, MenuID, Menu->actions_count );
  }
}


void AppStoreUI::execBtnA()
{
  if( Menu && MenuID < Menu->actions_count ) {
    if( Menu->Actions[MenuID]->callbacks && Menu->Actions[MenuID]->callbacks->onSelect ) {
      log_v("ButtonA Callback for Item '%s' (#%d / %d)", getListItemTitle(), MenuID, Menu->actions_count );
      Menu->Actions[MenuID]->callbacks->onSelect();
    } else {
      execBtn( 0 );
    }
  } else {
    log_e("No menu or bad menu range( %d ) to exec from", MenuID );
  }
}


void AppStoreUI::execBtnB()
{
  execBtn( 1 );
}


void AppStoreUI::execBtnC()
{
  execBtn( 2 );
}


void AppStoreUI::idle()
{
  checkSleepTimer();
  if( Menu->Actions[MenuID]->callbacks && Menu->Actions[MenuID]->callbacks->onIdle ) {
    Menu->Actions[MenuID]->callbacks->onIdle();
  }
}

void AppStoreUI::modal()
{
  checkSleepTimer();
  if( Menu->Actions[MenuID]->callbacks && Menu->Actions[MenuID]->callbacks->onModal ) {
    Menu->Actions[MenuID]->callbacks->onModal();
  }
}


void AppStoreUI::buttonsClr()
{
  gfx->fillRect( Theme->WinPosX, Theme->ButtonsPosY, Theme->WinWidth, TITLEBAR_HEIGHT, Theme->BgColor );
}


void AppStoreUI::windowClr( uint32_t fillcolor )
{
  gfx->fillRect( Theme->WinPosX, Theme->WinPosY, Theme->WinWidth, Theme->WinHeight, fillcolor );
}


void AppStoreUI::windowClr()
{
  uint32_t begin = millis();

  if( !getGradientLine() ) {
    log_d("Failed to create sprite (%d x %d ), will fill with single color", Theme->WinWidth, 1 );
    gfx->fillScreen( Theme->MenuColor );
    return;
  }
  gfx->startWrite();
  gfx->pushImageRotateZoom( gfx->width()/2, gfx->height()/2, Theme->WinWidth/2, 0, 0.0, 1, Theme->WinHeight, GradienSprite->width(), GradienSprite->height(), (uint16_t*)GradienSprite->getBuffer() );
  GradienSprite->deleteSprite();
  cropRoundRect( gfx, Theme->WinPosX, Theme->WinPosY, Theme->WinWidth, Theme->WinHeight, 5, Theme->BgColor );
  gfx->endWrite();

  log_v("clear took %d ms", millis()-begin ); // avg 179ms
}


void AppStoreUI::drawListItem( uint16_t inIDX, uint16_t posY )
{
  if( inIDX==MenuID ) {
    drawTextShadow( gfx, Menu->Actions[inIDX]->title, Theme->LIOffsetX, Theme->LIOffsetY+(posY*Theme->LIHeight), Menu->Actions[inIDX]->textcolor, Theme->TextShadowColor, LIFont, TL_DATUM );
    drawCheckMark( gfx, 4, Theme->LIOffsetY+(posY*Theme->LIHeight)+3, Theme->arrowWidth, LIFontHeight/2, Menu->Actions[inIDX]->textcolor, Theme->TextShadowColor );
  } else {
    drawTextShadow( gfx, Menu->Actions[inIDX]->title, Theme->LIOffsetX, Theme->LIOffsetY+(posY*Theme->LIHeight), Menu->Actions[inIDX]->textcolor, Theme->TextShadowColor, LIFont, TL_DATUM );
  }
}


void AppStoreUI::updateList( uint16_t clearid )
{
  if( !getGradientLine() ) { // can't blit
    showList();
    return;
  }
  uint16_t prevPosY   = Theme->LIOffsetY+((clearid%LinesPerPage)*Theme->LIHeight)+3; // last arrow position
  uint16_t currPosY   = Theme->LIOffsetY+((MenuID%LinesPerPage)*Theme->LIHeight)+3;  // current arrow position
  uint16_t clipPosX   = 4;
  uint16_t clipWidth  = 1+Theme->arrowWidth;
  uint16_t clipHeight = 1+LIFontHeight/2;
  uint16_t *sprPtr    = (uint16_t*)GradienSprite->getBuffer();
  uint16_t *clipPtr   = &sprPtr[clipPosX];
  bool clear_asset    = false;
  // delete old checkmark
  gfx->pushImageRotateZoom( clipPosX+clipWidth/2, prevPosY+clipHeight/2, clipWidth/2, 0, 0.0, 1, clipHeight+1, clipWidth+1, GradienSprite->height()+1, clipPtr );
  // draw new checkmark
  drawCheckMark( gfx, 4, currPosY, Theme->arrowWidth, LIFontHeight/2, Menu->Actions[MenuID]->textcolor, Theme->TextShadowColor );

  GradienSprite->deleteSprite();

  callShowListHooks();
}


void AppStoreUI::showList()
{
  windowClr();
  uint16_t i, items = 0;
  gfx->startWrite();
  if( ListTotalPages > 1 ) {
    snprintf(paginationStr, 16, paginationTpl, PageID+1, ListTotalPages );
    drawTextShadow( gfx, paginationStr, Theme->LICaptionPosX, Theme->LICaptionPosY, Theme->TextColor, Theme->TextShadowColor, &Font2, TR_DATUM );
    snprintf(paginationStr, 16, totalCountTpl, ListCount );
    drawTextShadow( gfx, paginationStr, Theme->LICaptionPosX, gfx->height()-(Theme->LICaptionPosY-3), Theme->TextColor, Theme->TextShadowColor, &Font2, BR_DATUM );
  }
  if( (PageID + 1) == ListTotalPages ) { // in last page
    if( ListLinesInLastPage == 0 and ListCount >= LinesPerPage ) {
      LinesInCurrentPage = LinesPerPage;
      items = LinesPerPage;
    } else {
      if( ListTotalPages>1 ) {
        LinesInCurrentPage = ListLinesInLastPage;
        items = ListLinesInLastPage;
      } else {
        LinesInCurrentPage = ListCount;
        items = ListCount;
      }
    }
  } else { // in first page or paginaged
    LinesInCurrentPage = LinesPerPage;
    items = LinesPerPage;
  }
  for( i = 0; i<items; i++ ) {
    drawListItem( i+(PageID*LinesPerPage), i );
  }
  gfx->endWrite();
  callShowListHooks();
}


void AppStoreUI::callShowListHooks()
{
  if( Menu->Actions[MenuID]->callbacks && Menu->Actions[MenuID]->callbacks->onRender ) {
    log_v("Side Callback for MenuItem '%s' (item #%d)", Menu->Actions[MenuID]->title, MenuID );
    Menu->Actions[MenuID]->callbacks->onRender();
  }
  if( Menu->Actions[MenuID]->icon ) {
    log_v("Icon attached: %s", Menu->Actions[MenuID]->icon->path );
    RemoteAsset* asset = (RemoteAsset*)Menu->Actions[MenuID]->icon;
    drawAssetReveal( asset, gfx, Theme->assetPosX, Theme->assetPosY );
  }
}


void AppStoreUI::setMenuActionLabels( MenuActionLabels* _ActionLabels )
{
  log_v("Overwriting Menu ActionLabels '%s' with '%s'", Menu->ActionLabels->title, _ActionLabels->title );
  Menu->ActionLabels = _ActionLabels;
}


void AppStoreUI::drawButton( uint8_t bnum )
{
  if( Menu && Menu->ActionLabels && Menu->ActionLabels->Buttons && Menu->ActionLabels->Buttons[bnum] ) {
    ButtonAction* btn = Menu->ActionLabels->Buttons[bnum];
    ButtonSprite->pushSprite( buttonsXOffset[bnum], Theme->ButtonsPosY );
    if( btn->asset && M5_FS.exists( btn->asset->path ) ) {
      RemoteAsset *btnIcon = btn->asset;
      btnIcon->draw( gfx, buttonsXOffset[bnum]+BUTTON_HWIDTH - btnIcon->width/2, Theme->ButtonsPosY+BUTTON_HEIGHT/2 - btnIcon->height/2 );
    } else if( btn->title && !empty( btn->title ) ) {
      drawTextShadow( gfx, btn->title, buttonsXOffset[bnum]+BUTTON_HWIDTH, Theme->ButtonsPosY+BUTTON_HEIGHT/2, Theme->TextColor, Theme->TextShadowColor, ButtonFont, MC_DATUM );
    } else {
      // empty button wut ?
    }
  } else {
    gfx->fillRect( buttonsXOffset[bnum], Theme->ButtonsPosY, BUTTON_WIDTH, BUTTON_HEIGHT, Theme->BgColor );
  }
}


void AppStoreUI::drawButtons()
{
  createButtonMask();
  for( int i=0;i<BUTTONS_COUNT;i++ ) {
    drawButton(i);
  }
  clearButtonMask();
}


void AppStoreUI::drawMenu( bool clearWindow )
{
  // header background fill
  gfx->startWrite();
  if( !getGradientLine( true ) ) {
    gfx->fillRect( 0, 0, Theme->WinWidth, TITLEBAR_HEIGHT-2, Theme->MenuColor );
  } else {
    for( int i=0;i<TITLEBAR_HEIGHT-2; i++ ) {
      GradienSprite->pushSprite( 0, i );
    }
    GradienSprite->deleteSprite();
  }
  // header text
  drawTextShadow( gfx, Menu->Title, Theme->WinWidth/2, (TITLEBAR_HEIGHT-2)/2, Theme->TextColor, Theme->TextShadowColor, HeaderFont, MC_DATUM );
  cropRoundRect( gfx, 0, 0, Theme->WinWidth, TITLEBAR_HEIGHT-2, 3, Theme->BgColor );
  gfx->endWrite();
  // window
  if( clearWindow ) windowClr();
  // buttons
  drawButtons();
}





void UITheme::setupCanvas( LGFX* gfx )
{
  fontHeightFM9p7 = gfx->fontHeight(InfoWindowFont);
  LIFontHeight    = gfx->fontHeight(LIFont);
  fontHeight0     = gfx->fontHeight(&Font0);
  arrowWidth      = gfx->textWidth(">");
  ButtonsPosY     = gfx->height()-BUTTON_HEIGHT;
  WinHeight       = gfx->height()-TITLEBAR_HEIGHT*2;
  WinWidth        = gfx->width();
  WinPosY         = TITLEBAR_HEIGHT+1;
  WinPosX         = 0;
  WinMargin       = WINDOW_MARGINX;
  LIHeight        = LISTITEM_HEIGHT;
  LIOffsetY       = LISTITEM_OFFSETY; // pixels offset from top for list items
  LIOffsetX       = LISTITEM_OFFSETX; // pixels offset from left for list items
  LICaptionPosX   = WinWidth-WinMargin; // text cursor position-x for list caption
  LICaptionPosY   = LISTCAPTION_POSY; // text cursor position-Y for list caption
  assetPosY       = ASSET_POSY; // assets position
  assetPosX       = WinWidth-(assetWidth+WinMargin); // assets positionx
  pgW             = gfx->width()-LISTITEM_OFFSETX*2; // progress bar width
  pgX             = gfx->width()/2 - pgW/2; // pogress bar posx, based on width
  BgColor         = BG_COLOR;
  MenuColor       = MENU_COLOR;
  TextColor       = TEXT_COLOR;
  TextShadowColor = SHADOW_COLOR;
  gzProgressBar.setup(  gfx, pgX,   106, pgW,   10, GZ_PROGRESS_COLOR,  MenuColor, true ); // gzip
  tarProgressBar.setup( gfx, pgX,   152, pgW,   5,  TAR_PROGRESS_COLOR, MenuColor, true ); // tar
  dlProgressBar.setup(  gfx, pgX+1, 188, pgW-2, 4,  DL_PROGRESS_COLOR,  MenuColor, false ); // download/sha sum
  createGradients();
}


void UITheme::createGradients()
{
  uint8_t r = (MenuColor >> 16) & 0xff; // red
  uint8_t g = (MenuColor >> 8) & 0xff;  // green
  uint8_t b = MenuColor & 0xff;         // blue
  float sf = 0.2; // shade factor
  float tf = 0.2; // tint factor
  TintedMenuColor = ( uint8_t(r + (255 - r) * tf) << 16 ) + ( uint8_t(g + (255 - g) * tf) << 8 ) + ( uint8_t(b + (255 - b) * tf) ) ;
  ShadedMenuColor = ( uint8_t(r * (1 - sf)) << 16 ) + ( uint8_t(g * (1 - sf)) << 8 ) + ( uint8_t(b * (1 - sf)) ) ;
}


void ProgressBarTheme::clear()
{
  gfx->fillRect( x, y, w, caption?h+fontHeight0+2:h, bg );
}


void ProgressBarTheme::progress( uint8_t progress )
{
  uint16_t th = fontHeight0+2;
  uint16_t ty = y+h+2;
  drawProgressBar( gfx, x, y, w, h, progress, fg, bg );
  if( caption ) {
    String progressStr = "   " + String(progress)+"%   ";
    drawCaption( gfx, progressStr.c_str(), x, ty, w, th, &Font0, TC_DATUM, fg, bg );
  }
}


void ProgressBarTheme::setup(LGFX* _gfx, uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, uint32_t _fg, uint32_t _bg, bool _cap )
{
  gfx  = _gfx;
  x = _x;
  y = _y;
  w = _w;
  h = _h;
  fg = _fg;
  bg = _bg;
  caption = _cap;
}




namespace UIUtils
{

  using namespace MenuItems;
  using namespace AppRenderer;

  void initSprites( LGFX* gfx )
  {
    appInfo._gfx = gfx;

    GradienSprite = new LGFX_Sprite( gfx );
    GradienSprite->setColorDepth(16);

    ButtonSprite = new LGFX_Sprite( gfx );
    ButtonSprite->setColorDepth(16);

    MaskSprite = new LGFX_Sprite( gfx );
    MaskSprite->setColorDepth(16);

    AssetSprite = new LGFX_Sprite( gfx );
    AssetSprite->setColorDepth(16);

    ScrollSprite = new LGFX_Sprite( gfx );
    ScrollSprite->setColorDepth(1);

  }

  void drawProgressBar( LGFX* gfx, int x, int y, int w, int h, uint8_t val, uint32_t color, uint32_t bgcolor )
  {
    gfx->drawRect(x, y, w, h, color);
    if( val>100) val = 100;
    if( val==0 ) {
      gfx->fillRect(x + 1,         y + 1, w-2,       h - 2, bgcolor);
    } else {
      int fillw = (w * (((float)val) / 100.0)) -2;
      gfx->fillRect(x + 1,         y + 1, fillw-2,   h - 2, color);
      gfx->fillRect(x + fillw + 1, y + 1, w-fillw-2, h - 2, bgcolor);
    }
  }


  void gzProgressCallback( uint8_t progress )
  {
    static int8_t gzLibLastProgress = -1;
    if( gzLibLastProgress != progress ) {
      gzLibLastProgress = progress;
      UI->getTheme()->gzProgressBar.progress( progress );
    }
  }


  void tarProgressCallback( uint8_t progress )
  {
    static int8_t tarLibLastProgress = -1;
    if( tarLibLastProgress != progress ) {
      tarLibLastProgress = progress;
      UI->getTheme()->tarProgressBar.progress( progress );
    }
  }


  void tarStatusCallback( const char* name, size_t size, size_t total_unpacked )
  {
    log_d("[TAR] %-64s %8d bytes - %8d Total bytes", name, size, total_unpacked );
    size_t th_small = fontHeight0*1.3;
    size_t th_big   = fontHeightFM9p7*1.3;
    uint16_t posy = 162; // bottom text position (file size, overall size)
    static bool clean;
    UITheme* theme = UI->getTheme();
    LGFX* gfx = UI->getGfx();
    if( clean != true ) {
      clean = true;
      UI->windowClr( theme->MenuColor );
      drawInfoWindow( TAR_PROGRESS_TITLE );
    }
    // TODO: don't redraw OVERALL_PROGRESS_TITLE on every callback
    drawCaption( gfx, OVERALL_PROGRESS_TITLE, theme->gzProgressBar.x, theme->gzProgressBar.y - (th_big+2), theme->gzProgressBar.w, th_big, InfoWindowFont, MC_DATUM );

    drawCaption( gfx, name, WINDOW_MARGINX, theme->tarProgressBar.y - (th_small+2), gfx->width()-WINDOW_MARGINX*2, th_small, &Font0, MC_DATUM );
    drawCaption( gfx, String( formatBytes( size, formatBuffer ) ).c_str(), WINDOW_MARGINX, posy+th_big, gfx->width()/3-WINDOW_MARGINX, th_big, InfoWindowFont, TL_DATUM );
    const char* caption =  String( "Total: " + String( formatBytes( total_unpacked, formatBuffer ) ) ).c_str();
    drawCaption( gfx, caption, gfx->width()/3, posy+th_big, gfx->width()*2/3-WINDOW_MARGINX, th_big, InfoWindowFont, TR_DATUM );
  }


  bool getGradientLine( bool invert )
  {
    uint32_t width = UI->getGfx()->width();
    if( !GradienSprite->createSprite( width, 1 ) ) {
      log_d("Failed to create sprite (%d x %d ), will fill with single color", width, 1 );
      return false;
    }
    UITheme* theme = UI->getTheme();
    GradienSprite->drawGradientHLine( 0, 0, width, invert?theme->TintedMenuColor:theme->ShadedMenuColor, invert?theme->ShadedMenuColor:theme->TintedMenuColor );
    return true;
  }


  void createButtonMask()
  {
    if( !ButtonSprite->createSprite( BUTTON_WIDTH, BUTTON_HEIGHT ) ) {
      ButtonSprite->setColorDepth( ButtonSprite->getColorDepth()/2 || 1 );
      log_d("Failed to create Button Sprite (%d x %d )", BUTTON_WIDTH, 1 );
      return;
    }
    UITheme* theme = UI->getTheme();
    // fill with gradient
    for( int i=0; i<BUTTON_HEIGHT; i++ ) {
      ButtonSprite->drawGradientHLine( 0, i, BUTTON_WIDTH, theme->TintedMenuColor, theme->ShadedMenuColor );
    }
    // make a button skin
    MaskSprite->createSprite( BUTTON_WIDTH, BUTTON_HEIGHT );
    // colors will be cut diagonally, prepare coord for triangles
    uint16_t x1 = 0              , y1 = 0;
    uint16_t x2 = 0              , y2 = BUTTON_HEIGHT-1;
    uint16_t x3 =  BUTTON_WIDTH-1, y3 = 0;
    uint16_t x4 =  BUTTON_WIDTH-1, y4 = BUTTON_HEIGHT-1;
    // apply both colors
    for( int i=-1; i<1; i++ ) {
      drawButtonMask( (LGFX*)MaskSprite, 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 3, 3, theme->BgColor, i?theme->ShadedMenuColor:theme->TintedMenuColor );
      MaskSprite->fillTriangle( x2, y2, i?x4:x1, i?y4:y1, x3, y3, theme->BgColor );
      MaskSprite->pushSprite( ButtonSprite, 0, 0, theme->BgColor );
    }
    MaskSprite->deleteSprite();
    // apply rounded borders
    cropRoundRect( (LGFX*)ButtonSprite, 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 5, theme->BgColor );
  }


  void drawButtonMask( LGFX* gfx, int32_t x, int32_t y, uint32_t w, uint32_t h, uint8_t radius, uint8_t thickness, uint32_t fillcolor, uint32_t bgcolor )
  {
    gfx->fillRect( x, y, w, h, fillcolor );
    for( int i=0; i<thickness;i++ ) {
      gfx->drawRoundRect( x+i, y+i, w-i*2, h-i*2, radius+i, bgcolor );
    }
  }


  void clearButtonMask()
  {
    ButtonSprite->deleteSprite();
  }


  void drawCheckMark( LGFX* gfx, uint16_t gx, uint16_t gy, uint16_t gw, uint16_t gh, uint32_t textcolor, uint32_t shadowcolor, int8_t dirx, int8_t diry )
  {
    gfx->fillTriangle( dirx+gx, diry+gy, dirx+gx, diry+gy+gh, dirx+gx+gw, diry+gy+gh/2, shadowcolor );
    gfx->fillTriangle( gx,      gy,      gx,      gy+gh,      gx+gw,      gy+gh/2,      textcolor );
  }


  void drawTextShadow( LGFX* gfx, const char*str, int32_t x, int32_t y, uint32_t textcolor, uint32_t shadowcolor, const lgfx::v1::IFont* font, textdatum_t datum, int8_t dirx, int8_t diry )
  {
    gfx->setTextDatum( datum );
    gfx->setFont( font );
    gfx->setTextColor( shadowcolor );
    gfx->drawString( str, dirx+x, diry+y );
    gfx->setTextColor( textcolor );
    gfx->drawString( str, x, y );
  }


  template<typename T> void cropRoundRect( T* gfx, int32_t x, int32_t y, uint32_t w, uint32_t h, uint8_t radius, uint32_t bgcolor )
  {
    if( cropSprite1bit == nullptr ) {
      cropSprite1bit = new LGFX_Sprite( gfx );
      cropSprite1bit->setColorDepth(1);
      cropSprite1bit->createSprite( 1, 1 );
    }
    if( w != cropSprite1bit->width() || h != cropSprite1bit->width() ) {
      cropSprite1bit->deleteSprite();
      if( ! cropSprite1bit->createSprite( w, h ) ) {
        log_e("Could not create sprite (%d x %x)", w, h);
        return;
      }
      log_v("Using 1BitMask@[%d:%d]->[%dx%d] radius(%d), color(0x%06x)", x, y, w, h, radius, bgcolor );
      cropSprite1bit->setPaletteColor( 0, 0x123456U );
      cropSprite1bit->setPaletteColor( 1, bgcolor );
      cropSprite1bit->fillSprite( 1 );
      cropSprite1bit->fillRoundRect( 0, 0, w, h, radius, 0 );
    }
    cropSprite1bit->pushSprite( gfx, x, y, 0 );
  }


  void drawDownloadIcon( uint32_t color, int16_t x, int16_t y, float size )
  {
    float halfsize = size/2;
    LGFX* gfx = UI->getGfx();
    gfx->fillTriangle( x,      y+2*size, x+4*size, y+2*size, x+2*size, y+5*size, color );
    gfx->fillTriangle( x+size, y,        x+3*size, y,        x+2*size, y+5*size, color );
    gfx->fillRect( x, -halfsize+y+6*size, 1+4*size, size, color );
  }

  void drawRSSIBar( int16_t x, int16_t y, int16_t rssi, uint32_t bgcolor, float size )
  {
    RGBColor heatMapColors[4] = { {0xff, 0, 0}, {0xff, 0xa5, 0x00}, {0xff, 0xff, 0}, {0, 0xff, 0} }; // # [RED, ORANGE, YELLOW, GREEN ]
    uint32_t heatColor32 = getHeatMapColor( rssi%6, 0, 5, heatMapColors, sizeof(heatMapColors)/sizeof(RGBColor) );
    uint8_t bars = 0;
    uint32_t barColors[4] = { bgcolor, bgcolor, bgcolor, bgcolor };
    LGFX* gfx = UI->getGfx();
    switch(rssi%6) {
    case 5: bars = 4; break;
      case 4: bars = 3; break;
      case 3: bars = 3; break;
      case 2: bars = 2; break;
      case 1: bars = 1; break;
      default:
      case 0: bars = 1; break;
    }
    for( int i=0; i<bars; i++ ) {
      barColors[i] = heatColor32;
    }
    for( int i=0; i<4; i++ ) {
      gfx->fillRect(x + (i*3*size),  y + (4-i)*size, 2*size, (4+i)*size, barColors[i]);
    }
  }


  void drawCaption( LGFX* gfx, const char* caption, int32_t x, int32_t y, uint32_t w, uint32_t h, const void* font, int datum, uint32_t fgcolor, uint32_t bgcolor )
  {
    if( captionSprite == nullptr ) {
      captionSprite = new LGFX_Sprite( gfx );
      captionSprite->setColorDepth(1);
    }
    captionSprite->createSprite( w, h );
    captionSprite->setPaletteColor(0, bgcolor );
    captionSprite->setPaletteColor(1, fgcolor );
    captionSprite->setFont((lgfx::v1::IFont*)font );
    captionSprite->setTextDatum( TL_DATUM ); // we manage our own datum here
    captionSprite->setTextWrap(false);
    int32_t tx, ty;
    uint16_t tw = captionSprite->textWidth( caption );
    uint16_t th = captionSprite->fontHeight();
    bool wrap = true;
    switch( datum ) {
      case TL_DATUM: tx=0;        ty=0;        wrap = true;  break;
      case TC_DATUM: tx=w/2-tw/2; ty=0;        wrap = false; break;
      case TR_DATUM: tx=w-tw-1;   ty=0;        wrap = true;  break;
      case ML_DATUM: tx=0;        ty=h/2-th/2; wrap = true;  break;
      case MC_DATUM: tx=w/2-tw/2; ty=h/2-th/2; wrap = false; break;
      case MR_DATUM: tx=w-tw-1;   ty=h/2-th/2; wrap = true;  break;
      case BL_DATUM: tx=0;        ty=h-1;      wrap = true;  break;
      case BC_DATUM: tx=w/2-tw/2; ty=h-1;      wrap = false; break;
      case BR_DATUM: tx=w-tw-1;   ty=h-1;      wrap = true;  break;
    }
    captionSprite->setTextWrap(wrap);
    captionSprite->setCursor( tx, ty );
    captionSprite->println( caption );
    captionSprite->pushSprite( gfx, x, y );
    captionSprite->deleteSprite();
  }


  void drawInfoWindow( const char* title, const char* body, unsigned long waitdelay )
  {
    UITheme* theme = UI->getTheme();
    LGFX* gfx = UI->getGfx();
    UI->windowClr( theme->MenuColor );
    if( title ) {
      drawCaption( gfx, title, LISTITEM_OFFSETX, LISTITEM_OFFSETY, gfx->width()-LISTITEM_OFFSETX*2, TITLEBAR_HEIGHT, &FreeMonoBold12pt7b, MC_DATUM, theme->TextColor, theme->MenuColor );
    }
    if( body ) {
      drawCaption( gfx, body, LISTITEM_OFFSETX, 80, gfx->width()-LISTITEM_OFFSETX*2, TITLEBAR_HEIGHT*3, InfoWindowFont, TL_DATUM );
    }
    if( waitdelay > 0 ) {
      delay( waitdelay );
    }
  }


  int modalConfirm( MenuActionLabels* labels, bool clearAfter )
  {
    // backup UI labels
    MenuActionLabels* tempLabels = UI->getMenuActionLabels();
    const char* tempTitle = UI->getList()->Title;
    // set temporary labels and redraw
    UI->setMenuActionLabels( labels );
    UI->getList()->Title = labels->title;
    UI->drawMenu( false );
    // wait for button input
    int hidState = HID_INERT;
    while( hidState == HID_INERT ) {
      hidState = getControls();
      UI->modal();
    }
    // deinit scroll if necessary
    if( HeaderScroll.scrollInited ) HeaderScroll.scrollEnd();
    // restore labels to lask known values and redraw UI
    UI->setMenuActionLabels( tempLabels );
    UI->getList()->Title = tempTitle;
    if( clearAfter ) {
      UI->drawMenu( true );
      drawStatusBar();
    }
    return hidState;
  }

  int modalConfirm( const char* question, const char* title, const char* body, const char* labelA, const char* labelB, const char* labelC )
  {
    ButtonAction btnA = { labelA, nullptr };
    ButtonAction btnB = { labelB, nullptr };
    ButtonAction btnC = { labelC, nullptr };
    ButtonAction *buttons[BUTTONS_COUNT] = { &btnA, &btnB, &btnC };
    MenuActionLabels ActionLabels = { question, buttons };
    LGFX* gfx = UI->getGfx();
    drawInfoWindow( title?title:UI->getListItemTitle(), body );
    CautionModalIcon.draw( gfx, gfx->width()-CautionModalIcon.width-LISTITEM_OFFSETX, gfx->height()-CautionModalIcon.height-LISTITEM_OFFSETY );
    HIDSignal hidState = HID_INERT;
    return modalConfirm( &ActionLabels );
  }

  // like map( value, min, max, COLOR_MIN, COLOR_MAX ) but with RGBColor Palette
  uint32_t getHeatMapColor( int value, int minimum, int maximum, RGBColor *p, size_t psize  )
  {
    // They float, they all float ​🤡
    float f = float(value-minimum) / float(maximum-minimum) * float(psize-1); // convert to palette index
    int i = int(f/1); // closest color index in palette
    float dst = f - float(i); // distance to next color in palette
    if( dst < std::numeric_limits<float>::epsilon() ) { // exact color match
      return ( p[i].r << 16 ) + ( p[i].g << 8 ) + p[i].b;
    } else { // apply distance to matched color
      return (uint8_t( p[i].r + dst * float( p[i+1].r - p[i].r ) ) << 16) +(uint8_t( p[i].g + dst * float( p[i+1].g - p[i].g ) ) << 8)+ +(uint8_t( p[i].b + dst * float( p[i+1].b - p[i].b ) ) ) ;
    }
  }


  /* give up on redundancy and ECC to produce less and bigger squares */
  uint8_t getLowestQRVersionFromString( String text, uint8_t ecc )
  {
    #define QR_MAX_VERSION 9
    if(ecc>QR_MAX_VERSION) return QR_MAX_VERSION; // fail fast
    uint16_t len = text.length();
    uint8_t QRMaxLenByECCLevel[4][QR_MAX_VERSION] = {
      // https://www.qrcode.com/en/about/version.html
      // Handling version 1-9 only since there's no point with M5Stack's 320x240 display (next version is at 271)
      { 17, 32, 53, 78, 106, 134, 154, 192, 230 }, // L
      { 14, 26, 45, 62, 84,  106, 122, 152, 180 }, // M
      { 11, 20, 32, 46, 60,  74,  86,  108, 130 }, // Q
      { 7,  14, 24, 34, 44,  58,  64,  84,  98  }  // H
    };
    for( uint8_t i=0; i<QR_MAX_VERSION; i++ ) {
      if( len <= QRMaxLenByECCLevel[ecc][i] ) {
        log_d("string len=%d bytes, fits in version %d / ecc %d (max=%d)", len, i+1, ecc, QRMaxLenByECCLevel[ecc][i] );
        return i+1;
      }
    }
    log_e("String length exceeds output parameters");
    return QR_MAX_VERSION;
  }


  void qrRender( LGFX* gfx, String text, int posX, int posY, uint32_t width, uint32_t height )
  {
    if( text.length()==0 ) {
      log_d("Cowardly refusing to qRender an empty string");
      return; // empty string for some reason
    }

    // see https://github.com/Kongduino/M5_QR_Code/blob/master/M5_QRCode_Test.ino
    // Create the QR code
    QRCode qrcode;

    uint8_t ecc = 0; // QR on TFT can do with minimal ECC
    uint8_t version = getLowestQRVersionFromString( text, ecc );

    uint8_t qrcodeData[lgfx_qrcode_getBufferSize( version )];
    lgfx_qrcode_initText( &qrcode, qrcodeData, version, ecc, text.c_str() );

    uint32_t gridSize  = (qrcode.size + 4);   // margin: 2 dots
    uint32_t dotWidth  = width / gridSize;    // rounded
    uint32_t dotHeight = height / gridSize;   // rounded
    uint32_t realWidth  = dotWidth*gridSize;  // recalculated
    uint32_t realHeight = dotHeight*gridSize; // recalculated
    if( realWidth > width || realHeight > height ) {
      log_e("Can't fit QR with gridsize(%d),dotSize(%d) =>[%dx%d] => [%dx%d]", gridSize, dotWidth, realWidth, realHeight, width, height );
      return;
    } else {
      log_d("Rendering QR Code '%s' (%d bytes) on version #%d on [%dx%d] => [%dx%d] grid", text.c_str(), text.length(), version, qrcode.size, qrcode.size, realWidth, realHeight );
    }

    uint8_t marginX = (width  - qrcode.size*dotWidth)/2;
    uint8_t marginY = (height - qrcode.size*dotHeight)/2;

    gfx->fillRect( posX, posY, width, height, TFT_WHITE );

    for ( uint8_t y = 0; y < qrcode.size; y++ ) {
      // Each horizontal module
      for ( uint8_t x = 0; x < qrcode.size; x++ ) {
        bool q = lgfx_qrcode_getModule( &qrcode, x, y );
        if (q) {
          gfx->fillRect( x*dotWidth +posX+marginX, y*dotHeight +posY+marginY, dotWidth, dotHeight, TFT_BLACK );
        }
      }
    }
  }


  void fillAssetSprite( LGFX_Sprite* dst, int32_t offsetX, int32_t offsetY=0 )
  {
    if( !getGradientLine() ) { // can't blit
      uint16_t transcolor = 0x1234;
      dst->fillSprite( UI->getTheme()->MenuColor );
      return;
    }
    uint16_t *sprPtr = (uint16_t*)GradienSprite->getBuffer();
    uint16_t *clipPtr = &sprPtr[offsetX];
    int32_t width = dst->width(), height = dst->height();
    dst->pushImageRotateZoom( width/2, height/2, width/2, 0, 0.0, 1, height, width, 1, clipPtr );
    GradienSprite->deleteSprite();
  }


  template<typename T> void drawAssetReveal( T *asset, LGFX*_gfx, int32_t posX, int32_t posY )
  {
    if( ! AssetSprite->createSprite( UI->getTheme()->assetWidth, UI->getTheme()->assetHeight ) ) {
      log_w("Can't create %dx%d sprite, drawing raw", UI->getTheme()->assetWidth, UI->getTheme()->assetHeight );
      asset->draw( _gfx, posX, posY );
      return;
    }
    fillAssetSprite( AssetSprite, posX );
    asset->draw( (LGFX*)AssetSprite, 0, 0 );
    uint16_t *sprPtr = (uint16_t*)AssetSprite->getBuffer();
    for( int y=0; y<asset->height*2; y++ ) {
      int scanY = y%2==0? y/4 : (-1+asset->height-y/4);
      _gfx->pushImageRotateZoom( posX+asset->width/2, posY+scanY, asset->width/2, 0, 0.0, 1, 1, asset->width, 1, &sprPtr[scanY*asset->width] );
      delayMicroseconds(300);
    }
    AssetSprite->deleteSprite();
  }

};




template<typename T> bool UIHScroll::init( T* gfx, String _scrollText, uint32_t width, uint32_t height, size_t textSize, const void*font )
{
  if( scrollInited ) return true;
  if( _scrollText=="" ) {
    log_e("No text to scroll");
    return false;
  }
  scrollText = _scrollText;
  ScrollSprite->setColorDepth( 16 );
  if( width==0 || height==0 || !ScrollSprite->createSprite( width, height ) ) {
    log_e("Can't create scrollsprite");
    return false;
  }
  if( !getGradientLine() ) {
    log_d("Failed to create gradient sprite (%d x %d )", width, 1 );
    return false;
  }

  ScrollSprite->setTextWrap( false ); // lazy way to solve a wrap bug
  ScrollSprite->setFont((lgfx::v1::IFont*)font );
  ScrollSprite->setTextSize( textSize ); // setup text size before it's measured
  ScrollSprite->setTextDatum( ML_DATUM );

  scrollTextColor1 = TFT_WHITE;
  scrollTextColor2 = ScrollSprite->color565(0x40,0xaa,0x40);

  if( !scrollText.endsWith( " " )) {
    scrollText += SCROLL_SEPARATOR; // append a space since scrolling text *will* repeat
  }
  while( ScrollSprite->textWidth( scrollText ) < width ) {
    scrollText += scrollText; // grow text to desired width
  }

  scrollWidth1 = ScrollSprite->textWidth( scrollText );
  ScrollSprite->setTextSize( textSize+1 ); // setup text size before it's measured
  scrollWidth2 = ScrollSprite->textWidth( scrollText );

  log_w("Inited scroll to [%dx%d] virtual [%d / %d]", width, height, scrollWidth1, scrollWidth2 );

  scrollInited = true;
  return true;
}


template<typename T> void UIHScroll::render( T* gfx, String _scrollText, uint32_t delay, size_t textSize, const void*font, uint8_t x, uint8_t y, uint32_t width, uint32_t height )
{
  if( millis() - lastScrollRender < delay ) return; // debouncing
  if( !init( gfx, _scrollText, width, height, textSize, font ) ) return; // first call
  if( scrollText=="" ) {
    log_e("No text to scroll");
    return;
  }
  // push background
  ScrollSprite->pushImageRotateZoom( ScrollSprite->width()/2, ScrollSprite->height()/2, width/2, 0, 0.0, 1, height, GradienSprite->width(), 1, (uint16_t*)GradienSprite->getBuffer() );
  // draw text to scroll
  printText( ScrollSprite, scrollText, 0.75, scrollWidth2, scrollTextColor2, delay, textSize+1, width, height );
  printText( ScrollSprite, scrollText, 1.5,   scrollWidth1, scrollTextColor1, delay, textSize, width, height );
  cropRoundRect( ScrollSprite, 0, 0, width, height, 5, 0x000000U );
  ScrollSprite->pushSprite( gfx, x, y );
  lastScrollRender = millis();
}

template<typename T> void UIHScroll::printText( T* gfx, String scrollText, float speed, size_t textWidth, uint16_t textColor, uint32_t delay, size_t textSize, uint32_t width, uint32_t height )
{
  gfx->setTextSize( textSize ); // setup text size before it's measured
  int32_t tick = int(millis()/(delay*speed)) % textWidth*2;
  gfx->setTextColor( textColor );
  if( -tick > -textWidth ) {
    gfx->drawString( scrollText, -tick, height/2 );
  }
  gfx->drawString( scrollText, textWidth - tick, height/2 );
  if( textWidth*2 - tick < width ) {
    gfx->drawString( scrollText, textWidth*2 - tick, height/2 );
  }
}


void UIHScroll::scrollEnd()
{
  GradienSprite->deleteSprite();
  ScrollSprite->deleteSprite();
  scrollInited = false;
  log_e("Deinited scroll");
}
