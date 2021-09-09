#include <ESP32-Chimera-Core.h>
#define tft M5.Lcd

#define SDU_APP_PATH "/1_test_binary.bin" // path to file on the SD
#define SDU_APP_NAME "saveSketchToFS() Example" // title for SD-Updater UI

#include <M5StackUpdater.h>



void centerMessage( const char* message, uint16_t color )
{
  tft.clear();
  tft.setTextDatum( MC_DATUM );
  tft.setTextColor( color );
  tft.drawString( message, tft.width()/2, tft.height()/2 );
}


void setup()
{
  M5.begin();
  Serial.println("Welcome to the SD-Updater minimal example!");
  Serial.println("Now checking if a button was pushed during boot ...");
  // checkSDUpdater( SD 1);
  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    30000,         // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // (usually default=4 but your mileage may vary)
  );

  Serial.println("Nope, will run the sketch normally");
  tft.print("Sketch Copy Utility\n");
  tft.print("-------------------\n\n");
  tft.print("Press Button B to start\n\n");

}

void loop()
{

  M5.update();
  if( M5.BtnB.pressedFor( 1000 ) ) {
    tft.print("Please release Button B...\n\n");
    while( M5.BtnB.isPressed() ) { // wait for release
      M5.update();
      vTaskDelay(100);
    }
    tft.print("Will copy sketch to SD...\n\n");
    if( saveSketchToFS( SD, SDU_APP_PATH, TFCARD_CS_PIN ) ) {
      centerMessage( "Copy successful !\n", TFT_GREEN );
    } else {
      centerMessage( "Copy failed !\n", TFT_RED );
    }
  }

}

