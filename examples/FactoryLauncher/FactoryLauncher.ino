#include <SD.h>
#include <SPIFFS.h>
#include <LittleFS.h>
#include <FFat.h>
#include <ESP32-targz.h>

#include <M5Unified.h>
#include <M5StackUpdater.h>

#define tft M5.Lcd


struct digest_t {
  String str{"0000000000000000000000000000000000000000000000000000000000000000"};
  const char* toString( uint8_t dig[32] )
  {
    str = "";
    char hex[3] = {0};
    for(int i=0;i<32;i++) {
      snprintf( hex, 3, "%02x", dig[i] );
      str += String(hex);
    }
    return str.c_str();
  }
};


void lsPart()
{
  esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
  log_w("Partition  Type   Subtype    Address   PartSize   ImgSize    Info    Digest");
  log_w("---------+------+---------+----------+----------+---------+--------+--------");
  while(pi != NULL) {
    const esp_partition_t* part = esp_partition_get(pi);
    esp_image_metadata_t meta = esp_image_metadata_t();
    digest_t digest;
    bool isFactory = part->type==ESP_PARTITION_TYPE_APP && part->subtype==ESP_PARTITION_SUBTYPE_APP_FACTORY;
    bool isOta = part->type==ESP_PARTITION_TYPE_APP && (part->subtype>=ESP_PARTITION_SUBTYPE_APP_OTA_MIN && part->subtype<ESP_PARTITION_SUBTYPE_APP_OTA_MAX);
    //bool isOta = (part->label[3]=='1' || part->label[3] == '0');
    String OTAName = "OTA";
    if( isOta || isFactory ) {
      meta  = SDUpdater::getSketchMeta( part );
      OTAName += String( part->subtype-ESP_PARTITION_SUBTYPE_APP_OTA_MIN );
    }

    int digest_total = 0;
    for( int i=0;i<sizeof(meta.image_digest);i++ ) {
      digest_total+=meta.image_digest[i];
    }

    if( isOta && digest_total > 0 ) {
      tft.println( digest.toString(meta.image_digest) );
    }

    log_w("%-8s   0x%02x      0x%02x   0x%06x   %8d  %8s %8s %8s",
      String( part->label ),
      part->type,
      part->subtype,
      part->address,
      part->size,
      meta.image_len>0 ? String(meta.image_len) : "n/a",
      isOta ? OTAName.c_str() : isFactory ? "Factory" : "n/a",
      digest_total>0 ? digest.toString(meta.image_digest) : ""
    );
    pi = esp_partition_next( pi );
  }
  esp_partition_iterator_release(pi);
}




void setup()
{

  M5.begin();

  lsPart();

  if( SDUpdater::saveSketchToFactory() ) {
    // sketch was just saved to factory partition, this is only triggered once
    SDUpdater::loadFactory(); // will restart on success
    log_e("Switching to factory app failed :-(");
  }


// Menu:
//   - Detect filesystems and pick a filesystem:
//    a) SD/littlefs/spiffs/fatfs -> fsLoader
//    b) factory partition -> factoryLoader

// fsLoader:
//   - list current binaries/gz
//   - sd-update from bin/gz
//   - pick destination partition

// factoryLoader:
//   - list current partitions
//   - load from partition
//   - dump partition to filesystem

}


void loop()
{

}
