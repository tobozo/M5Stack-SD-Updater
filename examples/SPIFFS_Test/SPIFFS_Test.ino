#include <M5Stack.h>
#include <M5StackUpdater.h>

void setup() {

  M5.begin();
  Wire.begin();
  SDUpdater sdUpdater("MY_SDApp.bin"); // declaring app name enables SPIFFS auto-backup to SD

  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    sdUpdater.updateFromFS(SD); // will also backup the SPIFFS
    ESP.restart();
  } else {
    if ( sdUpdater.SPIFFSisEmpty() ) { // add other logic checks
      Serial.println("SPIFFS is empty!");
      sdUpdater.copyDir( SDUpdater::BACKUP_SD_TO_SPIFFS );
    } else {
      Serial.println("Nothing to update!");
    }
  }

  const char* filename = "/spiffs.tmp";

  M5.Lcd.setTextSize( 2 );
  M5.Lcd.println("SPIFFS FileReadWriteTest");
  M5.Lcd.println();
  SPIFFS.begin();
  File file = SPIFFS.open( filename );
  unsigned long filesize = file.size();
  if (filesize == 0) {
    file.close();
    M5.Lcd.println("Created File!!");
    file = SPIFFS.open(filename, FILE_WRITE);
    file.println("Test");
    file.println(0);
    file.close();
  } else {
    M5.Lcd.println("File Contents:");
    String tmpstr = file.readStringUntil('\n');
    M5.Lcd.println();
    M5.Lcd.println(tmpstr);
    String numstr = file.readStringUntil('\n');
    file.close();
    int num = numstr.toInt();
    num = num + 1;
    M5.Lcd.printf("number:%d\n", num);
    M5.Lcd.println();

    M5.Lcd.print("Incrementing ... ");
    file = SPIFFS.open(filename, FILE_WRITE);
    file.println("Test");
    file.println(num);
    file.close();
    M5.Lcd.println(" ...OK!");
  }
  M5.Lcd.println();
  M5.Lcd.println("Please reset to increment");


}

void loop() {
  // put your main code here, to run repeatedly:

}
