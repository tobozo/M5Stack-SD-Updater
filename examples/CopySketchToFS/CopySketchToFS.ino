#include <ESP32-Chimera-Core.h>
#include <M5StackUpdater.h>



class mySDUpdater : public SDUpdater
{

  public:
    mySDUpdater( const int TFCardCsPin_ = TFCARD_CS_PIN ) : SDUpdater( TFCardCsPin_ ) {
      SDMenuProgress    = SDMenuProgressUI;
      displayUpdateUI   = DisplayUpdateUI;
    };

    void blah( int a )
    {
      log_e("bleh");
    }

};


void setup()
{

  M5.begin();

  Serial.println("Welcome to the SD-Updater minimal example!");
  Serial.println("Now checking if a button was pushed during boot ...");

  // checkSDUpdater();
  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    2000,         // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // (usually default=4 but your mileage may vary)
  );


  Serial.println("Nope, will run the sketch normally");

}

void loop()
{
  M5.update();
  if( M5.BtnB.pressedFor( 1000 ) ) {
    Serial.println("Will copy sketch from flash to filesystem");
    if( copyFsPartition( SD, "/1_test_binary.bin", TFCARD_CS_PIN ) ) {
      Serial.println("Copy successful !");
    } else {
      Serial.println("Copy failed !");
    }
  }
}

