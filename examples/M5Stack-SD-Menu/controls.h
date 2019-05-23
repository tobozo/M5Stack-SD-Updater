#ifndef __CONTROLS_H
#define __CONTROLS_H

/*
 * Mandatory and optional controls for the menu to be usable
 */
enum HIDSignal {
  UI_INERT = 0,
  UI_UP    = 1, // optional
  UI_DOWN  = 2,
  UI_SELECT  = 3,
  UI_PAGE  = 4
};

/* PSP JoyPad control plugin is provided as an example for custom controls.  */
//#define USE_PSP_JOY
/* M5Stack Faces Gameboy control plugin */
//#define USE_FACES_GAMEBOY
#ifdef ARDUINO_ODROID_ESP32
  // #include "odroidGoControls.h"
#endif

#ifdef USE_PSP_JOY
  #include "joyPSP.h"
#endif
#ifdef USE_FACES_GAMEBOY
  #include "joyFacesGameboy.h"
#endif

HIDSignal getControls() {

  if(M5.BtnB.wasPressed()) return UI_PAGE;
  if(M5.BtnC.wasPressed()) return UI_DOWN;
  if(M5.BtnA.wasPressed()) return UI_SELECT;

  HIDSignal padValue = UI_INERT;
  
  #ifdef USE_PSP_JOY
    padValue = getJoyPad();
    if(padValue!=UI_INERT) return padValue;
  #endif
  #ifdef USE_FACES_GAMEBOY
    padValue = getKeypad();
    if(padValue!=UI_INERT) return padValue;
  #endif

  return UI_INERT;
}




#endif
