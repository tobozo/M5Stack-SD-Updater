#include "SAM.h"

M5SAM::M5SAM()
//  : menuList(NULL),
//    menuIDX(0),
//    subMenuIDX(0),
//    menuCount(0)
{

  _keyboardIRQRcvd = LOW;

  levelIDX = 0;
  menuCount[levelIDX] = 0;
  menuIDX = 0;
  menucolor     = tft.color565( 0,0,128 );
  windowcolor   = tft.color565( 128,128,128 );
  menutextcolor = tft.color565( 255,255,255 );
  clearList();
}

void M5SAM::setListCaption(String inCaption){
  listCaption = inCaption;
}

void M5SAM::clearList(){
  list_count = 0;
  list_pages = 0;
  list_page = 0;
  list_lastpagelines = 0;
  list_idx = 0;
  for(byte x = 0; x<M5SAM_LIST_MAX_COUNT;x++){
    list_labels[x] = "";
  }
  listCaption = "";
}

void M5SAM::addList(String inStr){
  if(inStr.length()<=listMaxLabelSize and inStr.length()>0 and list_count < M5SAM_LIST_MAX_COUNT){
    list_labels[list_count] = inStr;
    list_count++;
  }
  if(list_count>0){
    if(list_count > listPagination){
      list_lastpagelines = list_count % listPagination;
      if(list_lastpagelines>0) {
        list_pages = (list_count - list_lastpagelines) / listPagination;
        list_pages++;
      }else{
        list_pages = list_count / listPagination;
      }
    }else{
      list_pages = 1;
    }
  }
}

byte M5SAM::getListID(){
  return list_idx;
}
void M5SAM::setListID(byte idx) {
  if(idx< list_page * listPagination + list_lines){
    list_idx = idx;
  }
  list_page = list_idx / listPagination;
}

String M5SAM::getListString(){
  return list_labels[list_idx];
}

void M5SAM::nextList( bool renderAfter ){
  if(list_idx< list_page * listPagination + list_lines - 1){
    list_idx++;
  }else{
    if(list_page<list_pages - 1){
      list_page++;
    }else{
      list_page = 0;
    }
    list_idx = list_page * listPagination;
  }
  if( renderAfter ) showList();
}

void M5SAM::drawListItem(byte inIDX, byte postIDX){
      if(inIDX==list_idx){
        M5.Lcd.drawString(list_labels[inIDX],15,listPageLabelsOffset+(postIDX*20),2);
        M5.Lcd.drawString(">",3,listPageLabelsOffset+(postIDX*20),2);
      }else{
        M5.Lcd.drawString(list_labels[inIDX],15,listPageLabelsOffset+(postIDX*20),2);
      }
}

void M5SAM::showList(){
    windowClr();
    byte labelid = 0;
    M5.Lcd.setTextDatum( listCaptionDatum );
    //M5.Lcd.drawCentreString(listCaption,M5.Lcd.width()/2,45,2);
    M5.Lcd.drawString(listCaption, listCaptionXPos, listCaptionYPos, 2);
    M5.Lcd.setTextDatum( TL_DATUM );
    if((list_page + 1) == list_pages){
      if(list_lastpagelines == 0 and list_count >= listPagination){
        list_lines = listPagination;
        for(byte i = 0;i<listPagination;i++){
          labelid = i+(list_page*listPagination);
          drawListItem(labelid,i);
        }
      }else{
        if(list_pages>1){
          list_lines = list_lastpagelines;
          for(byte i = 0;i<list_lastpagelines;i++){
            labelid = i+(list_page*listPagination);
            drawListItem(labelid,i);
          }
        }else{
          list_lines = list_count;
          for(byte i = 0;i<list_count;i++){
            labelid = i+(list_page*listPagination);
            drawListItem(labelid,i);
          }
        }
      }
    }else{
        list_lines = listPagination;
        for(byte i = 0;i<listPagination;i++){
            labelid = i+(list_page*listPagination);
            drawListItem(labelid,i);
        }
    }
}

void M5SAM::up(){
  if(menuIDX<menuCount[levelIDX]-1){
    menuIDX++;
    show();
  }
}

void M5SAM::down(){
  if(menuIDX>0){
    menuIDX--;
    show();
  }
}

void M5SAM::GoToLevel(byte inlevel){
  levelIDX = inlevel;
  menuIDX = 0;
  show();
}

void M5SAM::execute(){
  if(menuList[levelIDX][menuIDX].gotoLevel==-1){
    (*menuList[levelIDX][menuIDX].function)();
  }else{
    GoToLevel(menuList[levelIDX][menuIDX].gotoLevel);
  }
}

void M5SAM::addMenuItem(byte levelID, const char *menu_title,const char *btnA_title,const char *btnB_title,const char *btnC_title, signed char goto_level,  void(*function)())
{
  byte mCnt = menuCount[levelID];
  menuList[levelID] = (MenuCommandCallback *) realloc(menuList[levelID], (mCnt + 1) * sizeof(MenuCommandCallback));
  strncpy(menuList[levelID][mCnt].title, menu_title, M5SAM_MENU_TITLE_MAX_SIZE);
  strncpy(menuList[levelID][mCnt].btnAtitle, btnA_title, M5SAM_BTN_TITLE_MAX_SIZE);
  strncpy(menuList[levelID][mCnt].btnBtitle, btnB_title, M5SAM_BTN_TITLE_MAX_SIZE);
  strncpy(menuList[levelID][mCnt].btnCtitle, btnC_title, M5SAM_BTN_TITLE_MAX_SIZE);
  menuList[levelID][mCnt].gotoLevel = goto_level;
  menuList[levelID][mCnt].function = function;
  menuCount[levelID]++;
}

void M5SAM::show(){
  drawMenu(menuList[levelIDX][menuIDX].title, menuList[levelIDX][menuIDX].btnAtitle, menuList[levelIDX][menuIDX].btnBtitle, menuList[levelIDX][menuIDX].btnCtitle, menucolor, windowcolor, menutextcolor);
}

void M5SAM::windowClr(){
  M5.Lcd.fillRoundRect(0,32,M5.Lcd.width(),M5.Lcd.height()-32-32,3,windowcolor);
}

uint16_t M5SAM::getrgb(byte inred, byte ingrn, byte inblue){
  inred = map(inred,0,255,0,31);
  ingrn = map(ingrn,0,255,0,63);
  inblue = map(inblue,0,255,0,31);
  return inred << 11 | ingrn << 5 | inblue;
}

void M5SAM::drawAppMenu(String inmenuttl, String inbtnAttl, String inbtnBttl, String inbtnCttl){
  drawMenu(inmenuttl, inbtnAttl, inbtnBttl, inbtnCttl, menucolor, windowcolor, menutextcolor);
  M5.Lcd.setTextColor(menutextcolor,windowcolor);
}

void M5SAM::setColorSchema(uint16_t inmenucolor, uint16_t inwindowcolor, uint16_t intextcolor){
  menucolor = inmenucolor;
  windowcolor = inwindowcolor;
  menutextcolor = intextcolor;
}

String M5SAM::keyboardGetString(){
  String tmp_str = "";
  boolean tmp_klock = HIGH;
  keyboardEnable();
  M5.Lcd.fillRoundRect(0,M5.Lcd.height()-28,M5.Lcd.width(),28,3,windowcolor);
  M5.Lcd.drawString(">"+tmp_str,5,M5.Lcd.height()-28+6,2);
  while(tmp_klock==HIGH){
    if(_keyboardIRQRcvd==HIGH){
      if(_keyboardChar == 0x08){
        tmp_str = tmp_str.substring(0,tmp_str.length()-1);
      }else if(_keyboardChar == 0x0D){
        tmp_klock = LOW;
      }else{
        tmp_str = tmp_str + char(_keyboardChar);
      }
      M5.Lcd.fillRoundRect(0,M5.Lcd.height()-28,M5.Lcd.width(),28,3,windowcolor);
      M5.Lcd.drawString(">"+tmp_str,5,M5.Lcd.height()-28+6,2);
      _keyboardIRQRcvd = LOW;
    }
  }
  keyboardDisable();
  btnRestore();
  return tmp_str;
}


/*
void M5SAM::keyboardEnable(){
  pinMode(5, INPUT);
  attachInterrupt(digitalPinToInterrupt(5), keyboardIRQ, FALLING);
  while(!digitalRead(5)){
    Wire.requestFrom(0x08,1,true);
    Wire.read();
  }
}

void M5SAM::keyboardDisable(){
  detachInterrupt(5);
}

void M5SAM::keyboardIRQ(){
  while(!digitalRead(5)){
    Wire.requestFrom(0x08,1,true);
    _keyboardChar = Wire.read();
  }
  _keyboardIRQRcvd = HIGH;
}
*/

#ifdef ARDUINO_ODROID_ESP32

  #define BUTTON_WIDTH 60
  #define BUTTON_HWIDTH BUTTON_WIDTH/2 // 30
  #define BUTTON_HEIGHT 28
  uint16_t buttonsXOffset[4] = {
    1, 72, 188, 260
  };

  void M5SAM::drawMenu(String inmenuttl, String inbtnAttl, String intSpeakerttl, String inbtnBttl, String inbtnCttl, uint16_t inmenucolor, uint16_t inwindowcolor, uint16_t intxtcolor) {
    lastBtnTittle[1] = intSpeakerttl;
    drawMenu(inmenuttl, inbtnAttl, inbtnBttl, inbtnCttl,  inmenucolor, inwindowcolor, intxtcolor);
  }
  void M5SAM::drawAppMenu(String inmenuttl, String inbtnAttl, String intSpeakerttl, String inbtnBttl, String inbtnCttl){
    drawMenu(inmenuttl, inbtnAttl, intSpeakerttl, inbtnBttl, inbtnCttl, menucolor, windowcolor, menutextcolor);
    M5.Lcd.setTextColor(menutextcolor,windowcolor);
  }

#else

  #define BUTTON_WIDTH 60
  #define BUTTON_HWIDTH BUTTON_WIDTH/2 // 30
  #define BUTTON_HEIGHT 28
  uint16_t buttonsXOffset[3] = {
    31, 126, 221
  };

#endif

void M5SAM::btnRestore(){
  M5.Lcd.setTextColor(menutextcolor);
  M5.Lcd.fillRoundRect(0,M5.Lcd.height()-BUTTON_HEIGHT,M5.Lcd.width(),BUTTON_HEIGHT,3,0x00);
  for( byte i=0; i<BUTTONS_COUNT; i++ ) {
    M5.Lcd.fillRoundRect(buttonsXOffset[i],M5.Lcd.height()-BUTTON_HEIGHT,BUTTON_WIDTH,BUTTON_HEIGHT,3,menucolor);
    if( lastBtnTittle[i] != "" ) {
      M5.Lcd.drawCentreString( lastBtnTittle[i], buttonsXOffset[i]+BUTTON_HWIDTH, M5.Lcd.height()-BUTTON_HEIGHT+6, 2);

    }
  }
  M5.Lcd.setTextColor(menutextcolor,windowcolor);
}

void M5SAM::drawMenu(String inmenuttl, String inbtnAttl, String inbtnBttl, String inbtnCttl, uint16_t inmenucolor, uint16_t inwindowcolor, uint16_t intxtcolor) {
  #ifdef ARDUINO_ODROID_ESP32
    lastBtnTittle[0] = inbtnAttl;
    //lastBtnTittle[1] = "";
    lastBtnTittle[2] = inbtnBttl;
    lastBtnTittle[3] = inbtnCttl;
  #else
    lastBtnTittle[0] = inbtnAttl;
    lastBtnTittle[1] = inbtnBttl;
    lastBtnTittle[2] = inbtnCttl;
  #endif
  for( byte i=0; i<BUTTONS_COUNT; i++ ) {
    M5.Lcd.fillRoundRect(buttonsXOffset[i],M5.Lcd.height()-BUTTON_HEIGHT,BUTTON_WIDTH,BUTTON_HEIGHT,3,inmenucolor);
  }
  M5.Lcd.fillRoundRect(0,0,M5.Lcd.width(),BUTTON_HEIGHT,3,inmenucolor);
  M5.Lcd.fillRoundRect(0,32,M5.Lcd.width(),M5.Lcd.height()-32-32,3,inwindowcolor);

  M5.Lcd.setTextColor(intxtcolor);
  M5.Lcd.drawCentreString(inmenuttl,M5.Lcd.width()/2,6,2);
  for( byte i=0; i<BUTTONS_COUNT; i++ ) {
    if( lastBtnTittle[i] != "" ) {
      M5.Lcd.drawCentreString(lastBtnTittle[i],buttonsXOffset[i]+BUTTON_HWIDTH,M5.Lcd.height()-BUTTON_HEIGHT+6,2);
    }
  }
}
