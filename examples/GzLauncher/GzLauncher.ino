#include <ESP32-Chimera-Core.h> // https://github.com/tobozo/ESP32-Chimera-Core
#define tft M5.Lcd

#define DEST_FS_USES_SD
#include <ESP32-targz.h>

#define SDU_APP_NAME "Application GzLauncher"
#define SDU_APP_PATH "/menu.bin" // path to file on the SD
#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater


SDUpdater *sdUpdater;


void byteCountSI(int64_t b, char* dest )
{
  const int64_t unit = 1000;
  const char* units = "kMGTPE";
  if( b < unit ) {
    sprintf(dest, "%d B", b);
  }
  int64_t div = unit;
  int64_t exp = 0;
  for (int64_t n = b / unit; n >= unit; n /= unit) {
    div *= unit;
    exp++;
  }
  sprintf(dest, "%.1f %cB", float(b)/float(div), units[exp]);
}

void byteCountIEC( int64_t b, char* dest )
{
  const int64_t unit = 1024;
  const char* units = "KMGTPE";
  if( b < unit ) {
    sprintf(dest, "%d B", b);
  }
  int64_t div = unit;
  int64_t exp = 0;
  for (int64_t n = b/unit; n >= unit; n /= unit) {
    div *= unit;
    exp++;
  }
  sprintf(dest, "%.1f %ciB", float(b)/float(div), units[exp]);
}


static void getPartitions()
{

  esp_partition_iterator_t pi;
  const esp_partition_t *partition;

  pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

  if (pi) {
    printf("%s - %s - %s - %s - %s - %s\r\n", "type", "subtype", "  address ", "  size  ", "  label ", "encrypted");
    do {
      partition = esp_partition_get(pi);
      char bytesStr[16] = {0};
      byteCountIEC( partition->size, bytesStr );
      printf(" %2d  -  %3d    - 0x%08x - %8s - %8s - %i\r\n", partition->type, partition->subtype, partition->address, bytesStr, partition->label, partition->encrypted);
    } while ((pi = esp_partition_next(pi)));
  }

  esp_partition_iterator_release(pi);

}

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

  Serial.printf("Welcome to %s\n", SDU_APP_NAME);

  getPartitions();

  /*
  SDUCfg.setCSPin( TFCARD_CS_PIN );
  SDUCfg.setFS( &SD );

  sdUpdater = new SDUpdater( &SDUCfg );
  */

}

void loop()
{
  /*
  M5.update();
  if( M5.BtnB.pressedFor( 1000 ) ) {
    tft.print("Please release Button B...\n\n");
    while( M5.BtnB.isPressed() ) { // wait for release
      M5.update();
      vTaskDelay(100);
    }
    tft.print("Will copy sketch to SD...\n\n");
    if( sdUpdater->saveSketchToFS( SD, SDU_APP_PATH ) ) {
      centerMessage( "Copy successful !\n", TFT_GREEN );
    } else {
      centerMessage( "Copy failed !\n", TFT_RED );
    }
  }
  */
}
