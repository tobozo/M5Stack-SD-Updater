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

#if defined ARDUINO_M5Stack_Core_ESP32 || defined ARDUINO_M5STACK_FIRE
  #define CAN_I_HAZ_M5FACES
#endif


#if defined ARDUINO_M5STACK_Core2
  // enable M5Core2's haptic feedback !
  static bool isVibrating = false;

  static void vibrateTask( void * param ) {
    if( !isVibrating ) {
      isVibrating = true;
      int ms = *((int*)param); // dafuq
      M5.Axp.SetLDOEnable( 3,1 );
      delay( ms );
      M5.Axp.SetLDOEnable( 3,0 );
      isVibrating = false;
    }
    vTaskDelete( NULL );
  }

  static void HIDFeedback( int ms ) {
    xTaskCreatePinnedToCore( vibrateTask, "vibrateTask", 2048, (void*)&ms, 16, NULL , 1 );
  }

#else

  static void HIDFeedback( int ms ) { ;  }

#endif

static bool M5FacesEnabled = false;

#if defined CAN_I_HAZ_M5FACES
  #if defined(_CHIMERA_CORE_)
    #include "drivers/M5Stack/M5Faces.h"
  #else
    #include <M5Faces.h>
  #endif
  #define GAMEBOY_KEY_NONE        0x00
  #define GAMEBOY_KEY_RELEASED    0xFF
  #define GAMEBOY_KEY_START       0x7F
  #define GAMEBOY_KEY_SELECT      0xBF
  #define GAMEBOY_KEY_A           0xEF
  #define GAMEBOY_KEY_B           0xDF
  #define GAMEBOY_KEY_UP          0xFE
  #define GAMEBOY_KEY_DOWN        0xFD
  #define GAMEBOY_KEY_LEFT        0xFB
  #define GAMEBOY_KEY_RIGHT       0xF7

  M5Faces Faces;

  uint8_t M5FacesLastReleasedKey = 0x00;
  unsigned long M5FacesLastI2CQuery = millis();
  unsigned long M5FacesI2CQueryDelay = 50; // some debounce
  static bool M5FacesPressed = false;

  void IRAM_ATTR M5FacesIsr() {
    M5FacesPressed = true;
  }

  HIDSignal M5FacesOnKeyPushed( HIDSignal signal ) {
    log_d("Key %d pushed", signal );
    M5FacesLastReleasedKey = GAMEBOY_KEY_NONE;
    return signal;
  }

  HIDSignal extKey() {
    if( ! M5FacesPressed ) {
      // interrupt wasn't called
      return HID_INERT;
    }
    M5FacesPressed = false;
    uint8_t keypadState = Faces.getch();
    // look for "released" signal
    if( keypadState == GAMEBOY_KEY_RELEASED ) {
      keypadState = M5FacesLastReleasedKey;
    } else {
      M5FacesLastReleasedKey = keypadState;
      keypadState = GAMEBOY_KEY_NONE;
    }
    switch( keypadState ) {
      case GAMEBOY_KEY_UP :
        return M5FacesOnKeyPushed( HID_UP );
      break;
      case GAMEBOY_KEY_DOWN :
        return M5FacesOnKeyPushed( HID_DOWN );
      break;
      case GAMEBOY_KEY_SELECT :
        return M5FacesOnKeyPushed( HID_SELECT );
      break;
      case GAMEBOY_KEY_RIGHT :
      case GAMEBOY_KEY_LEFT :
      case GAMEBOY_KEY_START :
      case GAMEBOY_KEY_A :
      case GAMEBOY_KEY_B :
        return M5FacesOnKeyPushed( HID_INERT );
      break;
      default:
        return HID_INERT;
    }
  }

#else

  HIDSignal extKey() { return HID_INERT; }

#endif


void HIDInit() {
  #if defined CAN_I_HAZ_M5FACES
    Wire.begin(SDA, SCL);
    M5FacesEnabled = Faces.canControlFaces();
    if( M5FacesEnabled ) {
      // set the interrupt
      attachInterrupt(5, M5FacesIsr, FALLING); // 5 = KEYBOARD_INT from M5Faces.cpp
      Serial.println("M5Faces enabled and listening");
    }
  #endif
}



HIDSignal HIDFeedback( HIDSignal signal, int ms = 100 ) {
  if( signal != HID_INERT ) {
    HIDFeedback( ms );
  }
  return signal;
}


HIDSignal getControls() {
  // no buttons? no problemo! (c) Arnold S.
  if( Serial.available() ) {
    char command = Serial.read(); // read one char
    Serial.flush();
    switch(command) {
      case 'a':Serial.println("Sending HID_DOWN Signal");      return HIDFeedback( HID_DOWN );
      case 'b':Serial.println("Sending HID_UP Signal");        return HIDFeedback( HID_UP );
      case 'c':Serial.println("Sending HID_PAGE_DOWN Signal"); return HIDFeedback( HID_PAGE_DOWN );
      case 'd':Serial.println("Sending HID_PAGE_UP Signal");   return HIDFeedback( HID_PAGE_UP );
      case 'e':Serial.println("Sending HID_SCREENSHOT Signal");return HIDFeedback( HID_SCREENSHOT );
      case 'f':Serial.println("Sending HID_SELECT Signal");    return HIDFeedback( HID_SELECT );
      default: Serial.print("Ignoring serial input: ");Serial.println( String(command) );
    }
  }

#if defined(ARDUINO_ODROID_ESP32) && defined(_CHIMERA_CORE_)
  // Odroid-Go buttons support ** with repeat delay **
  if( M5.JOY_Y.pressedFor( fastRepeatDelay ) ) {
    uint8_t updown = M5.JOY_Y.isAxisPressed();
    if( JOY_Y_pressed == false || M5.JOY_Y.pressedFor( beforeRepeatDelay ) ) {
      beforeRepeatDelay += FAST_REPEAT_DELAY;
      JOY_Y_pressed = true;
      switch( updown ) {
        case 1: return HIDFeedback( HID_DOWN );
        case 2: return HIDFeedback( HID_UP );
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
        case 1: return HIDFeedback( HID_PAGE_DOWN );
        case 2: return HIDFeedback( HID_PAGE_UP );
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

  if( d ) return HIDFeedback( HID_SCREENSHOT );
  if( b ) return HIDFeedback( HID_PAGE_DOWN );
  if( c ) return HIDFeedback( HID_DOWN );
  if( a ) return HIDFeedback( HID_SELECT );

#else

  // M5Faces support
  if( M5FacesEnabled ) {
    HIDSignal M5FacesSignal = extKey();
    if( M5FacesSignal != HID_INERT ) {
      return HIDFeedback( M5FacesSignal );
    }
  }

  // legacy buttons support
  bool a = M5.BtnA.wasPressed();
  bool b = M5.BtnB.wasPressed() && !M5.BtnC.isPressed();
  bool c = M5.BtnC.wasPressed() && !M5.BtnB.isPressed();
  bool d = ( M5.BtnB.wasPressed() && M5.BtnC.isPressed() );
  bool e = ( M5.BtnB.isPressed() && M5.BtnC.wasPressed() );

  if( d || e ) return HIDFeedback( HID_PAGE_UP ); // multiple push, suggested by https://github.com/mongonta0716
  if( b ) return HIDFeedback( HID_PAGE_DOWN );
  if( c ) return HIDFeedback( HID_DOWN );
  if( a ) return HIDFeedback( HID_SELECT );

#endif

  //HIDSignal padValue = HID_INERT;

  return HID_INERT;
}

#endif
