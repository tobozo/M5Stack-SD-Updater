#ifndef __CONTROLS_H
#define __CONTROLS_H

/*
 * Mandatory and optional controls for the menu to be usable
 */
enum HIDSignal {
  HID_INERT  = 0, // when nothing happens
  HID_UP     = 1, // optional
  HID_DOWN   = 2,
  HID_SELECT = 3,
  HID_PAGE   = 4
};


HIDSignal getControls() {

  if(M5.BtnB.wasPressed()) return HID_PAGE;
  if(M5.BtnC.wasPressed()) return HID_DOWN;
  if(M5.BtnA.wasPressed()) return HID_SELECT;
  // TODO: implement Odroid-Go controls

  HIDSignal padValue = HID_INERT;

  return HID_INERT;
}


#endif
