#define SDU_APP_NAME "Headless Example"
#define SDU_APP_PATH "/Headless_Example.bin"
#define SDU_NO_AUTODETECT // don't load gfx clutter (but implement my own action trigger)
#define SDU_NO_PRAGMAS    // don't print pragma messages when compiling
#define TFCARD_CS_PIN 4   // this is needed by SD.begin()
#include <SD.h>           // /!\ headless mode skips autodetect and may require to include the filesystem library **before** the M5StackUpdater library
#include <ESP32-targz.h>  // optional: https://github.com/tobozo/ESP32-targz
#include <M5StackUpdater.h>


static int myActionTrigger( char* labelLoad,  char* labelSkip, char* labelSave, unsigned long waitdelay )
{
  int64_t msec = millis();
  do {
    if( Serial.available() ) {
      String out = Serial.readStringUntil('\n');
      if(      out == "update" )  return  1; // load "/menu.bin"
      else if( out == "rollback") return  0; // rollback to other OTA partition
      else if( out == "skip" )    return -1; // do nothing
      else if( out == "save" ) return 2;
      else Serial.printf("Ignored command: %s\n", out.c_str() );
    }
  } while( msec > int64_t( millis() ) - int64_t( waitdelay ) );
  return -1;
}


void setup()
{
  Serial.begin( 115200 );
  Serial.println("Welcome to the SD-Updater Headless example!");
  Serial.println("Now waiting 30 seconds for user input in Serial console...");

  SDUCfg.setWaitForActionCb( myActionTrigger );

  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    30000,        // wait delay in milliseconds (default=0, e,g, 30000 will be forced to 30 seconds upon ESP.restart() )
    TFCARD_CS_PIN // (usually default=4 but your mileage may vary)
  );

  Serial.println("Starting application");
}

void loop()
{

}
