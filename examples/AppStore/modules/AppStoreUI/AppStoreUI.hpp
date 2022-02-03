#pragma once

#include "../MenuItems/MenuItems.hpp"
#include "../MenuUtils/MenuUtils.hpp"
#include "../AppStoreActions/AppStoreActions.hpp"

static size_t fontHeight0;
static size_t LIFontHeight;
static size_t fontHeightFM9p7;

// Progressbar theme
struct ProgressBarTheme
{
  LGFX* gfx;
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
  uint32_t fg;
  uint32_t bg;
  bool caption;
  void progress( uint8_t progress );
  void setup(LGFX* _gfx, uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, uint32_t _fg, uint32_t _bg, bool _cap );
  void clear();
};


// UI Colors
struct UITheme
{
  uint32_t BgColor; // dark
  uint32_t MenuColor;       // light
  ProgressBarTheme gzProgressBar;
  ProgressBarTheme tarProgressBar;
  ProgressBarTheme dlProgressBar;
  uint32_t TextColor;       // light
  uint32_t TextShadowColor; // dark
  uint32_t TintedMenuColor; // generated
  uint32_t ShadedMenuColor; // generated

  uint16_t pgW; // Window width minus the margins ( gfx->width()-LISTITEM_OFFSETX*2)
  uint16_t pgX; // pogress bar posx, based on width

  uint16_t WinHeight;
  uint16_t WinWidth;
  uint16_t WinPosY;
  uint16_t WinPosX;
  uint16_t WinMargin;

  uint16_t ButtonsPosY;
  uint16_t arrowWidth;

  uint16_t LIOffsetY;
  uint16_t LIOffsetX;
  uint16_t LIHeight;
  uint16_t LICaptionPosX;
  uint16_t LICaptionPosY;

  // base dimensions for assets placeholder
  uint16_t assetPosX;
  uint16_t assetPosY;
  uint16_t assetWidth  = 120;
  uint16_t assetHeight = 120;

  // const lgfx::v1::IFont* ButtonFont     = _ButtonFont;
  // const lgfx::v1::IFont* HeaderFont     = _HeaderFont;
  // const lgfx::v1::IFont* LIFont         = _LIFont;
  // const lgfx::v1::IFont* InfoWindowFont = _InfoWindowFont;

  void setupCanvas( LGFX* gfx );
  void createGradients();

};


struct UIHScroll
{
  int16_t scrollPointer = 0; // pointer to the scrollText position
  unsigned long lastScrollRender = millis(); // timer for scrolling
  String lastScrollMessage; // last scrolling string state
  int16_t lastScrollOffset; // last scrolling string position
  bool scrollInited = false;
  uint16_t scrollTextColor1;// TFT_WHITE
  uint16_t scrollTextColor2;// = gfx->color565(0x40,0xaa,0x40);
  size_t scrollWidth1;
  size_t scrollWidth2;
  String scrollText;

  void scrollEnd();
  template<typename T> bool init( T* gfx, String _scrollText, uint32_t width, uint32_t height, size_t textSize, const void*font );
  template<typename T> void render( T* gfx, String text, uint32_t delay=15, size_t size=2, const void*font=nullptr, uint8_t x=0, uint8_t y=0, uint32_t width=0, uint32_t height=0 );
  template<typename T> void printText( T* gfx, String scrollText, float speed, size_t textWidth, uint16_t textColor, uint32_t delay, size_t textSize, uint32_t width, uint32_t height );
};



namespace UIUtils
{
  // buttons, masks, gradients, effects, etc
  LGFX_Sprite* captionSprite = nullptr;
  LGFX_Sprite* cropSprite1bit = nullptr;
  LGFX_Sprite* GradienSprite = nullptr;
  LGFX_Sprite* ButtonSprite = nullptr;
  LGFX_Sprite* MaskSprite = nullptr;
  LGFX_Sprite* AssetSprite = nullptr;
  LGFX_Sprite* ScrollSprite = nullptr;
  UITheme Theme;

  // all draw primitives
  void initSprites( LGFX*gfx );
  int modalConfirm( MenuActionLabels* labels, bool clearMenu = false );
  int modalConfirm(const char*question, const char*title, const char*body, const char*labelA=MENU_BTN_YES, const char*labelB=MENU_BTN_NO, const char*labelC=MENU_BTN_CANCEL);
  void drawProgressBar(LGFX*gfx, int x, int y, int w, int h, uint8_t val, uint32_t color=MENU_COLOR, uint32_t bgcolor=BG_COLOR );
  void drawCaption(LGFX*gfx, const char*caption, int32_t x, int32_t y, uint32_t w, uint32_t h, const void*font=nullptr, int dt=MC_DATUM, uint32_t fgcol=TEXT_COLOR, uint32_t bgcol=MENU_COLOR );
  void drawInfoWindow( const char* title = nullptr, const char* body = nullptr, unsigned long waitdelay = 0 );
  void drawTextShadow(LGFX*gfx, const char*str, int32_t x, int32_t y, uint32_t txtcol=TEXT_COLOR, uint32_t shcol=SHADOW_COLOR,
                      const lgfx::v1::IFont* font=&Font2, textdatum_t dt=MC_DATUM, int8_t dirx=1, int8_t diry=1 );
  void drawCheckMark( LGFX* _gfx, uint16_t gx, uint16_t gy, uint16_t gw, uint16_t gh, uint32_t textcolor=TEXT_COLOR, uint32_t shadowcolor=SHADOW_COLOR, int8_t dirx=1, int8_t diry=1 );
  uint32_t getHeatMapColor( int value, int minimum, int maximum, RGBColor *p, size_t psize  ); // like  map( value, min, max, COLOR_MIN, COLOR_MAX ) but with RGBColor Palette
  void drawDownloadIcon( uint32_t color=0x00ff00U, int16_t x=272, int16_t y=7, float size=2.0 );
  void drawRSSIBar( int16_t x, int16_t y, int16_t rssi, uint32_t bgcolor, float size=1.0 );
  void gzProgressCallback( uint8_t progress );
  void tarProgressCallback( uint8_t progress );
  void tarStatusCallback( const char* name, size_t size, size_t total_unpacked );
  bool getGradientLine( bool invert = false );
  void createButtonMask();
  void clearButtonMask();
  void drawButtonMask( LGFX*_gfx, int32_t x, int32_t y, uint32_t w, uint32_t h, uint8_t radius, uint8_t thickness, uint32_t fillcolor, uint32_t bgcolor );

  UIHScroll HeaderScroll;

  template<typename T> void cropRoundRect( T *gfx, int32_t x, int32_t y, uint32_t w, uint32_t h, uint8_t radius, uint32_t bgcolor );
  template<typename T> void drawAssetReveal( T *asset, LGFX*_gfx, int32_t posX, int32_t posY );

};


class AppStoreUI
{
  public:
    AppStoreUI( LGFX*_gfx, UITheme * theme = &UIUtils::Theme );
    ~AppStoreUI();
    void windowClr();
    void windowClr( uint32_t fillcolor );
    void buttonsClr();
    void drawMenu( bool clearWindow = true );
    void drawButtons();
    void drawButton( uint8_t bnum );
    void showList();
    void clearList();
    uint16_t getListID();
    uint16_t getPageID();
    size_t getListSize();
    size_t getListPages();
    void setListID( uint16_t idx );
    void setPageID( uint16_t idx );
    void nextList( bool renderAfter = true );
    void pageUp( bool renderAfter = true );
    void pageDown( bool renderAfter = true );
    void menuUp( bool renderAfter = true );
    void menuDown( bool renderAfter = true );
    void execBtn( uint8_t bnum );
    void execBtnA();
    void execBtnB();
    void execBtnC();
    void idle();
    void modal();

    void setList( MenuGroup *actions );
    MenuGroup* getList() { return Menu; }

    void setMenuActionLabels( MenuActionLabels *labels );
    MenuActionLabels* getMenuActionLabels();

    void setTheme( UITheme* theme );
    UITheme *getTheme();

    LGFX* getGfx();

    const char* getListItemTitle();
    const char* getListTitle();
    const char* channel_name;

  private:

    bool empty( const char* str );
    void drawListItem( uint16_t inIDX, uint16_t postIDX );
    void updateList( uint16_t clearid );
    void callShowListHooks();

    LGFX*      gfx = nullptr; // tft instance
    MenuGroup* Menu = nullptr;
    UITheme*   Theme = nullptr;

    const char* emptyString = "";
    char paginationStr[16];
    const char* paginationTpl = "%d/%d";
    const char* totalCountTpl = "Total: %d";

    uint16_t LinesPerPage = LINES_PER_PAGE;
    uint16_t ListLinesInLastPage;
    uint16_t ListCount;
    uint16_t ListTotalPages;
    int16_t PageID;
    int16_t MenuID;
    uint16_t LinesInCurrentPage;

    bool CyclePageList = false; // affects menuDown()/menuUp() behaviour when hitting end/bottom of page
};


extern AppStoreUI* UI;
