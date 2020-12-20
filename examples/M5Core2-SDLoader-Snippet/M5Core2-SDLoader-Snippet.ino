#include <M5Core2.h>
#include <M5StackUpdater.h>


void setup()
{
  M5.begin();

  // checkSDUpdater();
  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    2000,         // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // (usually default=4 but your mileage may vary)
  );
}


void loop()
{

}
