#ifndef _M5UPDATER_HEADLESS_
#define _M5UPDATER_HEADLESS_

#include <Arduino.h>


static bool assertStartUpdateFromSerial()
{
  if( Serial.available() ) {
    String out = Serial.readStringUntil('\n');
    if( out == "update" ) return true;
  }
  return false;
}



static void checkSDUpdaterHeadless( fs::FS &fs, String fileName, unsigned long scanDelay ) {
  Serial.printf("SDUpdater: you have %d milliseconds to send 'update' command");
  auto msec = millis();
  do {
    if ( assertStartUpdateFromSerial() ) {
      Serial.println("Will Load menu binary");
      updateFromFS( fs, fileName );
      ESP.restart();
    }
  } while (millis() - msec < scanDelay);
  Serial.print("Delay expired, no SD-Update will occur");
}

static void DisplayUpdateHeadless( const String& label ) {
  // TODO: draw some fancy serial output
};

static void SDMenuProgressHeadless( int state, int size )
{
  static int SD_UI_Progress;
  int percent = ( state * 100 ) / size;
  if( percent == SD_UI_Progress ) {
    // don't render twice the same value
    return;
  }
  //Serial.printf("percent = %d\n", percent); // this is spammy
  SD_UI_Progress = percent;
  if ( percent >= 0 && percent < 101 ) {
    Serial.print( "." );
  } else {
    Serial.println();
  }
};





#endif // _M5UPDATER_HEADLESS_
