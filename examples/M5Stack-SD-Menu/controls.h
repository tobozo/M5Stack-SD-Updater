#ifndef __CONTROLS_H
#define __CONTROLS_H

/*
 * Mandatory and optional controls for the menu to be usable
 */
enum HIDSignal {
  HID_INERT     = 0, // when nothing happens
  HID_UP        = 1, // optional
  HID_DOWN      = 2,
  HID_SELECT    = 3,
  HID_PAGE_DOWN = 4,
  HID_PAGE_UP   = 5
};


HIDSignal getControls() {

  bool a = M5.BtnA.wasPressed();
  bool b = M5.BtnB.wasPressed() && !M5.BtnC.isPressed();
  bool c = M5.BtnC.wasPressed() && !M5.BtnB.isPressed();
  bool d = ( M5.BtnB.wasPressed() && M5.BtnC.isPressed() );
  bool e = ( M5.BtnB.isPressed() && M5.BtnC.wasPressed() );

  if( d || e ) return HID_PAGE_UP; // multiple push, suggested by https://github.com/mongonta0716
  if( b ) return HID_PAGE_DOWN;
  if( c ) return HID_DOWN;
  if( a ) return HID_SELECT;
  // TODO: implement Odroid-Go controls

  HIDSignal padValue = HID_INERT;

  return HID_INERT;
}


#endif
