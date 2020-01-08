#include <M5Stack.h>
/*
// why was it even here ?
#if defined(WIRING) && WIRING >= 100
  #include <Wiring.h>
#elif defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif
*/
#ifndef M5StackSAM_h
#define M5StackSAM_h

#define M5SAM_MENU_TITLE_MAX_SIZE 24
#define M5SAM_BTN_TITLE_MAX_SIZE 6
#define M5SAM_MAX_SUBMENUS 8

#ifndef M5SAM_LIST_MAX_COUNT
#define M5SAM_LIST_MAX_COUNT 128
#endif

#define M5SAM_LIST_MAX_LABEL_SIZE 36 // list labels will be trimmed
#define M5SAM_LIST_PAGE_LABELS 6

volatile static uint8_t _keyboardIRQRcvd;
volatile static uint8_t _keyboardChar;

#ifdef ARDUINO_ODROID_ESP32
  #define BUTTONS_COUNT 4
#else
  #define BUTTONS_COUNT 3
#endif

class M5SAM {
  public:
    M5SAM();
    void up();
    void down();
    void execute();
    void windowClr();
    void setColorSchema(unsigned int inmenucolor, unsigned int inwindowcolor, unsigned int intextcolor);
    void drawAppMenu(String inmenuttl, String inbtnAttl, String inbtnBttl, String inbtnCttl);
    #ifdef ARDUINO_ODROID_ESP32
    void drawAppMenu(String inmenuttl, String inbtnAttl, String intSpeakerttl, String inbtnBttl, String inbtnCttl);
    #endif
    void GoToLevel(byte inlevel);
    unsigned int getrgb(byte inred, byte ingrn, byte inblue);
    void addMenuItem(byte levelID, const char *menu_title,const char *btnA_title,const char *btnB_title,const char *btnC_title, signed char goto_level, void(*function)());
    void show();
    void showList();
    void clearList();
    byte getListID();
    void setListID(byte idx);
    String getListString();
    void nextList( bool renderAfter = true );
    void addList(String inLabel);
    void setListCaption(String inCaption);
    String keyboardGetString();
    String lastBtnTittle[BUTTONS_COUNT];
    uint8_t listMaxLabelSize = M5SAM_LIST_MAX_LABEL_SIZE; // list labels will be trimmed
    uint8_t listPagination = M5SAM_LIST_PAGE_LABELS;
    uint8_t listPageLabelsOffset = 80; // initially 80, pixels offset from top screen for list items
    uint8_t listCaptionDatum = TC_DATUM; // initially TC_DATUM=top centered, TL_DATUM=top left (default), top/right/bottom/left
    uint16_t listCaptionXPos = 160; // initially M5.Lcd.width()/2, text cursor position-x for list caption
    uint16_t listCaptionYPos = 45; // initially 45, text cursor position-x for list caption
           
  private:
    String listCaption;
    void drawListItem(byte inIDX, byte postIDX);

    void btnRestore();
    void keyboardEnable();
    void keyboardDisable();
    static void keyboardIRQ();
    void drawMenu(String inmenuttl, String inbtnAttl, String inbtnBttl, String inbtnCttl, unsigned int inmenucolor, unsigned int inwindowcolor, unsigned int intxtcolor);
    #ifdef ARDUINO_ODROID_ESP32
    void drawMenu(String inmenuttl, String inbtnAttl, String intSpeakerttl, String inbtnBttl, String inbtnCttl, unsigned int inmenucolor, unsigned int inwindowcolor, unsigned int intxtcolor);
    #endif
    struct MenuCommandCallback {
      char title[M5SAM_MENU_TITLE_MAX_SIZE + 1];
      char btnAtitle[M5SAM_BTN_TITLE_MAX_SIZE + 1];
      char btnBtitle[M5SAM_BTN_TITLE_MAX_SIZE + 1];
      char btnCtitle[M5SAM_BTN_TITLE_MAX_SIZE + 1];
      signed char gotoLevel; 
      void (*function)();
    };
    String list_labels[M5SAM_LIST_MAX_COUNT];
    byte list_lastpagelines;
    byte list_count;
    byte list_pages;
    byte list_page;
    byte list_idx;
    byte list_lines;
    
    MenuCommandCallback *menuList[M5SAM_MAX_SUBMENUS];
    byte menuIDX;
    byte levelIDX;
    byte menuCount[M5SAM_MAX_SUBMENUS];
    unsigned int menucolor;
    unsigned int windowcolor;
    unsigned int menutextcolor;
};

#endif
