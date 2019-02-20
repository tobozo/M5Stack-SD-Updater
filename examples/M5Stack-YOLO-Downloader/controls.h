#ifndef __CONTROLS_H
#define __CONTROLS_H

/*
 * Mandatory and optional controls for the menu to be usable
 */
enum HIDSignal {
  UI_INERT = 0,
  UI_UP    = 1, // optional
  UI_DOWN  = 2,
  UI_GO    = 3,
  UI_LOAD  = 4
};

HIDSignal getControls() {

  if(M5.BtnB.wasPressed()) return UI_UP;
  if(M5.BtnC.wasPressed()) return UI_DOWN;
  if(M5.BtnA.wasPressed()) return UI_GO;

  return UI_INERT;
}




#endif
