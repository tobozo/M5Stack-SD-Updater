#pragma once

/*
 * Mandatory and optional controls for the menu to be usable
 */
enum HIDSignal
{
  HID_INERT      = 0, // when nothing happens
  HID_UP         = 1, // optional
  HID_DOWN       = 2,
  HID_SELECT     = 3,
  HID_PAGE_DOWN  = 4,
  HID_PAGE_UP    = 5,
  HID_SCREENSHOT = 6
};

#define HID_BTN_A HID_SELECT
#define HID_BTN_B HID_PAGE_DOWN
#define HID_BTN_C HID_DOWN

#define FAST_REPEAT_DELAY 50 // ms, push delay
#define SLOW_REPEAT_DELAY 500 // ms, must be higher than FAST_REPEAT_DELAY and smaller than LONG_DELAY_BEFORE_REPEAT
#define LONG_DELAY_BEFORE_REPEAT 1000 // ms, delay before slow repeat enables
unsigned long fastRepeatDelay = FAST_REPEAT_DELAY;
unsigned long beforeRepeatDelay = LONG_DELAY_BEFORE_REPEAT;


#if defined ARDUINO_M5STACK_Core2
  // enable M5Core2's haptic feedback !
  static bool isVibrating = false;

  static void vibrateTask( void * param )
  {
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

  static void HIDFeedback( int ms )
  {
    // xTaskCreatePinnedToCore( vibrateTask, "vibrateTask", 2048, (void*)&ms, 1, NULL , 1 );
  }

#else

  static void HIDFeedback( int ms ) { ;  }

#endif



HIDSignal HIDFeedback( HIDSignal signal, int ms = 50 )
{
  if( signal != HID_INERT ) {
    HIDFeedback( ms );
  }
  return signal;
}


HIDSignal getControls()
{
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
      default: if( command != '\n' ) { Serial.print("Ignoring serial input: ");Serial.println( String(command) ); }
    }
  }
  M5.update();

  // legacy buttons support
  bool a = M5.BtnA.wasPressed() || M5.BtnA.pressedFor( 500 );
  bool b = M5.BtnB.wasPressed() || M5.BtnB.pressedFor( 500 );
  bool c = M5.BtnC.wasPressed() || M5.BtnC.pressedFor( 500 );
  //bool d = ( M5.BtnB.wasPressed() && M5.BtnC.isPressed() );
  //bool e = ( M5.BtnB.isPressed() && M5.BtnC.wasPressed() );

  //if( d || e ) return HIDFeedback( HID_PAGE_UP ); // multiple push, suggested by https://github.com/mongonta0716
  if( b ) return HIDFeedback( HID_PAGE_DOWN );
  if( c ) return HIDFeedback( HID_DOWN );
  if( a ) return HIDFeedback( HID_SELECT );
  //HIDSignal padValue = HID_INERT;
  return HID_INERT;
}
