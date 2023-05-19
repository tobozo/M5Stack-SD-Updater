
/*
#include <SPIFFS.h>
#include <LittleFS.h>
#include <SD.h>
#include <SD_MMC.h>
#include <FFat.h>
#include <SdFat.h>
#include <ESP32-targz.h>*/

#define SDU_NO_AUTODETECT                // Disable SDUpdater autodetect: this prevents <SD.h> to be auto-selected, however it also disables board detection


#define SDU_USE_SDFATFS                      // Tell M5StackUpdater to load <SdFat.h> and wrap SdFat32 into fs::FS::SdFat32FSImpl
#define SDU_USE_SD
#define SDU_USE_SD_MMC
#define SDU_USE_SPIFFS
#define SDU_USE_FFAT
#define SDU_USE_LITTLEFS
#define SDU_ENABLE_GZ

#include <M5StackUpdater.h>

SdFs sd;
auto SdFatSPIConfig = SdSpiConfig( TFCARD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(25) );

void setup()
{
  Serial.begin(115200);

  SDUpdater sdUpdater;

  bool has_littlefs = ConfigManager::hasFS( &sdUpdater, LittleFS );
  bool has_spiffs   = ConfigManager::hasFS( &sdUpdater, SPIFFS );
  bool has_sd       = ConfigManager::hasFS( &sdUpdater, SD );
  bool has_sd_mmc   = ConfigManager::hasFS( &sdUpdater, SD_MMC );
  bool has_ffat     = ConfigManager::hasFS( &sdUpdater, FFat );
  bool has_sdfat    = ConfigManager::hasFS( &sdUpdater, *ConfigManager::SDU_SdFatFsPtr );


  Serial.printf("Has littlefs:\t%s\nHas spiffs:\t%s\nHas sd:  \t%s\nHas sd_mmc:\t%s\nHas ffat:\t%s\nHas sdfat:\t%s\n",
    has_littlefs?"true":"false",
    has_spiffs  ?"true":"false",
    has_sd      ?"true":"false",
    has_sd_mmc  ?"true":"false",
    has_ffat    ?"true":"false",
    has_sdfat   ?"true":"false"
  );


}

void loop()
{

}
