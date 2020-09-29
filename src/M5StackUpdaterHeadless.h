#ifndef _M5UPDATER_HEADLESS_
#define _M5UPDATER_HEADLESS_

#include <Arduino.h>

#define SDU_LOAD_TPL   "Will Load menu binary : \n"
#define SDU_ROLLBACK_MSG "Will Roll back"

static int assertStartUpdateFromSerial( char* labelLoad,  char* labelSkip )
{
  if( Serial.available() ) {
    String out = Serial.readStringUntil('\n');
    if( out == "update" ) return 1;
    if( out == "rollback") return 0;
  }
  return -1;
}



static void checkSDUpdaterHeadless( fs::FS &fs, String fileName, unsigned long waitdelay, const int TfCardCsPin_ ) {
  Serial.printf("SDUpdater: you have %d milliseconds to send 'update' command", waitdelay);
  if( waitdelay == 0 ) {
    waitdelay = 100; // at lease give some time for the serial buffer to fill
  }
  auto msec = millis();
  do {
    if ( assertStartUpdateFromSerial( nullptr, nullptr ) == 1 ) {
      Serial.printf( SDU_LOAD_TPL, fileName.c_str() );
      updateFromFS( fs, fileName, TfCardCsPin_ );
      ESP.restart();
    }
  } while (millis() - msec < waitdelay);
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
