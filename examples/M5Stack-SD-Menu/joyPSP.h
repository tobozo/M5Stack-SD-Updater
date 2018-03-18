#ifndef __JOYPSP_H
#define __JOYPSP_H

/*
 * JoyPSP Controls for M5Stack SD Menu
 * Currently only handles up/down actions
 * 
 * 4 Wires PSP JoyPad breakout
 * https://www.google.fr/search?q=psp+joypad+breakout
 * Pins 35 and 36 are appropriate for analog reading
 *  
 */


#define JOY_X 35
#define JOY_Y 36

bool joyPadDetected = false;

uint16_t joyX = analogRead(JOY_X);
uint16_t joyY = analogRead(JOY_Y);
extern uint16_t MenuID;
extern M5SAM M5Menu;
extern unsigned long lastcheck;
extern uint16_t checkdelay;
extern uint16_t appsCount;
extern bool inmenu;

extern void renderIcon(uint16_t MenuID);

void initJoyPad() {
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  float totalx = 0;
  float totaly = 0;
  uint16_t i;
  for(i=0;i<1000;i++) {
    totalx +=analogRead(JOY_X);
    totaly +=analogRead(JOY_Y);
  }
  totalx /=i;
  totaly /=i;

  if(totalx<10 && totaly<10) { 
    //Serial.print( totalx);
    //Serial.print("**--\t--**");
    //Serial.println( totaly);
    Serial.println("No Joypad detected, disabling");
  } else {
    joyPadDetected = true;
  }  
}


void handleJoyPad() {
  
  MenuID = M5Menu.getListID();

  if(joyPadDetected) {
  
    unsigned long now = millis();
    long diff = now-lastcheck;
    if(diff>checkdelay) {
      lastcheck = now;
      joyX = analogRead(JOY_X);
      joyY = analogRead(JOY_Y);
    }
    if(joyX<=500 /*&& joyX!=0*/) {
      if(MenuID+1<appsCount) {
        MenuID++;
      } else {
        MenuID = 0;
      }
      M5Menu.drawAppMenu(F("SD CARD LOADER"),F("INFO"),F("LOAD"),F(">"));
      M5Menu.setListID(MenuID);
      M5Menu.showList();
      renderIcon(MenuID);
      inmenu = false;
      joyX = 4096/2;
      lastcheck = millis();
    }
    if(joyX>=3500 /*&& joyX!=4095*/) {
      if(MenuID!=0) {
        MenuID--;
      } else {
        MenuID = appsCount-1;
      }
      M5Menu.drawAppMenu(F("SD CARD LOADER"),F("INFO"),F("LOAD"),F(">"));
      M5Menu.setListID(MenuID);
      M5Menu.showList();
      renderIcon(MenuID);
      inmenu = false;
      joyX = 4096/2;
      lastcheck = millis();
    }
    /*
    Serial.print( joyX );
    Serial.print( "\t" );
    Serial.print( joyY );
    Serial.println();
    */
  }
}




#endif
