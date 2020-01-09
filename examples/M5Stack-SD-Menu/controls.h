#ifndef __CONTROLS_H
#define __CONTROLS_H

/*
 * Mandatory and optional controls for the menu to be usable
 */
enum HIDSignal {
  HID_INERT      = 0, // when nothing happens
  HID_UP         = 1, // optional
  HID_DOWN       = 2,
  HID_SELECT     = 3,
  HID_PAGE_DOWN  = 4,
  HID_PAGE_UP    = 5,
  HID_SCREENSHOT = 6
};


#define FAST_REPEAT_DELAY 50 // ms, push delay
#define SLOW_REPEAT_DELAY 500 // ms, must be higher than FAST_REPEAT_DELAY and smaller than LONG_DELAY_BEFORE_REPEAT
#define LONG_DELAY_BEFORE_REPEAT 1000 // ms, delay before slow repeat enables
unsigned long fastRepeatDelay = FAST_REPEAT_DELAY;
unsigned long beforeRepeatDelay = LONG_DELAY_BEFORE_REPEAT;
#if defined(ARDUINO_ODROID_ESP32) && defined(_CHIMERA_CORE_)
  bool JOY_Y_pressed = false;
  bool JOY_X_pressed = false;
#endif

HIDSignal getControls() {
  // no buttons? no problemo! (c) Arnold S.
  if( Serial.available() ) {
    char command = Serial.read(); // read one char
    Serial.flush();
    switch(command) {
      case 'a':Serial.println("Sending HID_DOWN Signal");return HID_DOWN;
      case 'b':Serial.println("Sending HID_UP Signal");return HID_UP;
      case 'c':Serial.println("Sending HID_PAGE_DOWN Signal");return HID_PAGE_DOWN;
      case 'd':Serial.println("Sending HID_PAGE_UP Signal");return HID_PAGE_UP;
      case 'e':Serial.println("Sending HID_SCREENSHOT Signal");return HID_SCREENSHOT;
      case 'f':Serial.println("Sending HID_SELECT Signal");return HID_SELECT;
      default: Serial.print("Ignoring serial input: ");Serial.println( String(command) );
    }
  }

#if defined(ARDUINO_ODROID_ESP32) && defined(_CHIMERA_CORE_)

  if( M5.JOY_Y.pressedFor( fastRepeatDelay ) ) {
    uint8_t updown = M5.JOY_Y.isAxisPressed();
    if( JOY_Y_pressed == false || M5.JOY_Y.pressedFor( beforeRepeatDelay ) ) {
      beforeRepeatDelay += FAST_REPEAT_DELAY;
      JOY_Y_pressed = true;
      switch( updown ) {
        case 1: return HID_DOWN;
        case 2: return HID_UP;
        break;
      }
    }
    return HID_INERT;
  } else {
    if( JOY_Y_pressed == true && M5.JOY_Y.releasedFor( FAST_REPEAT_DELAY ) ) {
      // button released, reset timers
      fastRepeatDelay = FAST_REPEAT_DELAY;
      beforeRepeatDelay = LONG_DELAY_BEFORE_REPEAT;
      JOY_Y_pressed = false;
    }
  }

  if( M5.JOY_X.pressedFor( fastRepeatDelay ) ) {
    uint8_t leftright = M5.JOY_X.isAxisPressed();
    if( JOY_X_pressed == false || M5.JOY_X.pressedFor( beforeRepeatDelay ) ) {
      beforeRepeatDelay += FAST_REPEAT_DELAY;
      JOY_X_pressed = true;
      switch( leftright ) {
        case 1: return HID_PAGE_DOWN;
        case 2: return HID_PAGE_UP;
        break;
      }
    }
    return HID_INERT;
  } else {
    if( JOY_X_pressed == true && M5.JOY_X.releasedFor( FAST_REPEAT_DELAY ) ) {
      // button released, reset timers
      fastRepeatDelay = FAST_REPEAT_DELAY;
      beforeRepeatDelay = LONG_DELAY_BEFORE_REPEAT;
      JOY_X_pressed = false;
    }
  }

  bool a = M5.BtnMenu.wasPressed() || M5.BtnA.wasPressed(); // acts as "BntA" on M5Stack, leftmost button, alias of "(A)" gameboy button
  bool b = M5.BtnSelect.wasPressed() || M5.BtnB.wasPressed(); // acts as "BntB" on M5Stack, middle button, alias of "(B)" gameboy button
  bool c = M5.BtnStart.wasPressed(); // acts as "BntC" on M5Stack, rightmost button, no alias
  bool d = M5.BtnVolume.wasPressed();

  if( d ) return HID_SCREENSHOT;
  if( b ) return HID_PAGE_DOWN;
  if( c ) return HID_DOWN;
  if( a ) return HID_SELECT;

#else

  bool a = M5.BtnA.wasPressed();
  bool b = M5.BtnB.wasPressed() && !M5.BtnC.isPressed();
  bool c = M5.BtnC.wasPressed() && !M5.BtnB.isPressed();
  bool d = ( M5.BtnB.wasPressed() && M5.BtnC.isPressed() );
  bool e = ( M5.BtnB.isPressed() && M5.BtnC.wasPressed() );

  if( d || e ) return HID_PAGE_UP; // multiple push, suggested by https://github.com/mongonta0716
  if( b ) return HID_PAGE_DOWN;
  if( c ) return HID_DOWN;
  if( a ) return HID_SELECT;

#endif

  //HIDSignal padValue = HID_INERT;

  return HID_INERT;
}

#endif
