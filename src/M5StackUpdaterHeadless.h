#ifndef _M5UPDATER_HEADLESS_
#define _M5UPDATER_HEADLESS_

//#include <Arduino.h>

#define SDU_LOAD_TPL   "Will Load menu binary : %s\n"
#define SDU_ROLLBACK_MSG "Will Roll back"

__attribute__((unused))
static int assertStartUpdateFromSerial( char* labelLoad,  char* labelSkip, unsigned long waitdelay )
{
  auto msec = millis();
  do {
    if( Serial.available() ) {
      String out = Serial.readStringUntil('\n');
      if( out == "update" ) return 1;
      if( out == "rollback") return 0;
    }
  } while( msec > millis()-waitdelay );
  return -1;
}


__attribute__((unused))
static void checkSDUpdaterHeadless( fs::FS &fs, String fileName, unsigned long waitdelay, const int TfCardCsPin_ ) {
  Serial.printf("SDUpdater: you have %d milliseconds to send 'update' command", (int)waitdelay);
  if( waitdelay == 0 ) {
    waitdelay = 100; // at lease give some time for the serial buffer to fill
  }

  if ( SDU.onWaitForAction( nullptr, nullptr, waitdelay ) == 1 ) {
    Serial.printf( SDU_LOAD_TPL, fileName.c_str() );
    updateFromFS( fs, fileName, TfCardCsPin_ );
    ESP.restart();
  }

  Serial.print("Delay expired, no SD-Update will occur");
}

__attribute__((unused))
static void DisplayUpdateHeadless( const String& label ) {
  // TODO: draw some fancy serial output
};

__attribute__((unused))
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
