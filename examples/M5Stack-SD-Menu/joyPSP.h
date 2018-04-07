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


#define JOY_X 35 // analog pin
#define JOY_Y 36 // analog pin

bool joyPadDetected = false;

uint16_t joyX = analogRead(JOY_X);
uint16_t joyY = analogRead(JOY_Y);
extern unsigned long lastcheck;
extern uint16_t checkdelay;
extern uint16_t appsCount;


void initJoyPad() {
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  float totalx = 0;
  float totaly = 0;
  uint16_t i;
  // naive detection: make 1000 reads and hope they don't differ too much :p
  for(i=0;i<1000;i++) {
    totalx +=analogRead(JOY_X);
    totaly +=analogRead(JOY_Y);
  }
  totalx /=i;
  totaly /=i;

  if(totalx<10 && totaly<10) { 
    Serial.println(DEBUG_JOYPAD_NOTFOUND);
  } else {
    Serial.println(DEBUG_JOYPAD_FOUND);
    joyPadDetected = true;
  }  
}


HIDSignal getJoyPad() {
  if(joyPadDetected==false) return UI_INERT;

  unsigned long now = millis();
  long diff = now-lastcheck;
  if(diff>checkdelay) {
    lastcheck = now;
    joyX = analogRead(JOY_X);
    joyY = analogRead(JOY_Y);
  }
  if(joyX<=500) {
    return UI_UP;
  }
  if(joyX>=3500) {
    return UI_DOWN;
  }
  return UI_INERT;
}


#endif
