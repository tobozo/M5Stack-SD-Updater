/*
 *
 * M5Stack SD Updater
 * Project Page: https://github.com/tobozo/M5Stack-SD-Updater
 *
 * Copyright 2018 tobozo http://github.com/tobozo
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files ("M5Stack SD Updater"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "M5StackUpdater.h"

#if defined( ARDUINO_M5Stick_C )

  #define SD_PLATFORM_NAME "M5StickC"

#else
  #if defined( ARDUINO_ODROID_ESP32 )
    //#pragma message ("Odroid-GO board detected")
    #define SD_PLATFORM_NAME "Odroid-GO"
  #elif defined( ARDUINO_M5Stack_Core_ESP32 )
    //#pragma message ("M5Stack Classic board detected")
    #define SD_PLATFORM_NAME "M5Stack"
  #elif defined( ARDUINO_M5STACK_FIRE )
    //#pragma message ("M5Stack Fire board detected")
    #define SD_PLATFORM_NAME "M5Stack-Fire"
  #elif defined( ARDUINO_M5STACK_Core2 )
    //#pragma message ("M5Stack Core2 board detected")
    #define SD_PLATFORM_NAME "M5StackCore2"
  #elif defined( ARDUINO_ESP32_DEV )
    //#pragma message ("WROVER KIT board detected")
    #define SD_PLATFORM_NAME "Wrover-Kit"
  #else
    //#pragma message ("Custom ESP32 board detected")
    // put your custom UI settings here
    #define SD_PLATFORM_NAME "ESP32"

  #endif
#endif


esp_image_metadata_t SDUpdater_Base::getSketchMeta( const esp_partition_t* source_partition ) {
  esp_image_metadata_t data;
  if ( !source_partition ) return data;
  const esp_partition_pos_t source_partition_pos  = {
     .offset = source_partition->address,
     .size = source_partition->size,
  };
  data.start_addr = source_partition_pos.offset;
  esp_image_verify( ESP_IMAGE_VERIFY, &source_partition_pos, &data );
  return data;//.image_len;
}



bool SDUpdater_Base::compareFsPartition(const esp_partition_t* src1, fs::File* src2, size_t length) {
  size_t lengthLeft = length;
  const size_t bufSize = SPI_FLASH_SEC_SIZE;
  std::unique_ptr<uint8_t[]> buf1(new uint8_t[bufSize]);
  std::unique_ptr<uint8_t[]> buf2(new uint8_t[bufSize]);
  uint32_t offset = 0;
  uint32_t progress = 0, progressOld = 1;
  size_t i;
  while( lengthLeft > 0) {
    size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
    if (!ESP.flashRead(src1->address + offset, reinterpret_cast<uint32_t*>(buf1.get()), (readBytes + 3) & ~3)
     || !src2->read(                           reinterpret_cast<uint8_t*>(buf2.get()), (readBytes + 3) & ~3)
    ) {
        return false;
    }
    for (i = 0; i < readBytes; ++i) if (buf1[i] != buf2[i]) return false;
    lengthLeft -= readBytes;
    offset += readBytes;
    if( cfg->onProgress ) {
      progress = 100 * offset / length;
      if (progressOld != progress) {
        progressOld = progress;
        cfg->onProgress( (uint8_t)progress, 100 );
       }
    }
  }
  return true;
}


bool SDUpdater_Base::copyFsPartition(File* dst, const esp_partition_t* src, size_t length) {
  //tft.fillRect( 110, 112, 100, 20, 0);
  size_t lengthLeft = length;
  const size_t bufSize = SPI_FLASH_SEC_SIZE;
  std::unique_ptr<uint8_t[]> buf(new uint8_t[bufSize]);
  uint32_t offset = 0;
  uint32_t progress = 0, progressOld = 1;
  while( lengthLeft > 0) {
    size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
    if (!ESP.flashRead(src->address + offset, reinterpret_cast<uint32_t*>(buf.get()), (readBytes + 3) & ~3)
    ) {
        return false;
    }
    if (dst) dst->write(buf.get(), (readBytes + 3) & ~3);
    lengthLeft -= readBytes;
    offset += readBytes;
    if( cfg->onProgress ) {
      progress = 100 * offset / length;
      if (progressOld != progress) {
        progressOld = progress;
        cfg->onProgress( (uint8_t)progress, 100 );
      }
    }
  }
  return true;
}


bool SDUpdater_Base::saveSketchToFS( fs::FS &fs, const char* binfilename ) {
  const esp_partition_t *running = esp_ota_get_running_partition();
  size_t sksize = ESP.getSketchSize();
  bool ret = false;
  fs::File dst = fs.open(binfilename, FILE_WRITE );
  if( cfg->onBefore) cfg->onBefore();
  if( cfg->onMessage ) cfg->onMessage( String("Overwriting ") + String(binfilename) );
  if (copyFsPartition( &dst, running, sksize)) {
    if( cfg->onMessage ) cfg->onMessage( String("Done ") + String(binfilename) );
    vTaskDelay(1000);
    ret = true;
  } else {
    if( cfg->onError ) cfg->onError("Copy failed", 2000);
  }
  dst.close();
  if( cfg->onAfter) cfg->onAfter();

  return ret;
}



// rollback helper, save menu.bin meta info in NVS
void SDUpdater_Base::updateNVS() {
  const esp_partition_t* update_partition = esp_ota_get_next_update_partition( NULL );
  esp_image_metadata_t nusketchMeta = getSketchMeta( update_partition );
  uint32_t nuSize = nusketchMeta.image_len;
  Serial.printf( "Updating menu.bin NVS size/digest after update: %d\n", nuSize );
  Preferences preferences;
  preferences.begin( "sd-menu", false );
  preferences.putInt( "menusize", nuSize );
  preferences.putBytes( "digest", nusketchMeta.image_digest, 32 );
  preferences.end();
}

// perform the actual update from a given stream
void SDUpdater_Base::performUpdate( Stream &updateSource, size_t updateSize, String fileName ) {
  if( cfg->onMessage ) cfg->onMessage( "LOADING " + fileName );
  if( cfg->onProgress ) Update.onProgress( cfg->onProgress );
  if (Update.begin( updateSize )) {
    size_t written = Update.writeStream( updateSource );
    if ( written == updateSize ) {
      Serial.println( "Written : " + String(written) + " successfully" );
    } else {
      Serial.println( "Written only : " + String(written) + "/" + String(updateSize) + ". Retry?" );
    }
    if ( Update.end() ) {
      Serial.println( "OTA done!" );
      if ( Update.isFinished() ) {
        if( strcmp( MENU_BIN, fileName.c_str() ) == 0 ) {
          // maintain NVS signature
          SDUpdater_Base::updateNVS();
        }
        Serial.println( "Update successfully completed. Rebooting." );
      } else {
        Serial.println( "Update not finished? Something went wrong!" );
      }
    } else {
      Serial.println( "Error Occurred. Error #: " + String( Update.getError() ) );
    }
  } else {
    Serial.println( "Not enough space to begin OTA" );
  }
}

// forced rollback (doesn't check NVS digest)
void SDUpdater_Base::doRollBack( const String& message ) {
  log_d("Wil check for rollback capability");
  if( !cfg->onMessage)   log_d("No message reporting");
  if( !cfg->onError )    log_d("No error reporting");
  if( !cfg->onProgress ) log_d("No progress reporting");

  if( Update.canRollBack() ) {
    if( cfg->onMessage) cfg->onMessage( message );
    for( uint8_t i=1; i<50; i++ ) {
      if( cfg->onProgress ) cfg->onProgress( i, 100 );
      vTaskDelay(10);
    }
    Update.rollBack();
    for( uint8_t i=50; i<=100; i++ ) {
      if( cfg->onProgress ) cfg->onProgress( i, 100 );
      vTaskDelay(10);
    }
    Serial.println( "Rollback done, restarting" );
    ESP.restart();
  } else {
    log_n("Cannot rollback: the other OTA partition doesn't seem to be populated or valid");
    if( cfg->onError ) cfg->onError("Cannot rollback", 2000);
  }
}



// if NVS has info about MENU_BIN flash size and digest, try rollback()
void SDUpdater_Base::tryRollback( String fileName ) {
  Preferences preferences;
  preferences.begin( "sd-menu" );
  uint32_t menuSize = preferences.getInt( "menusize", 0 );
  uint8_t image_digest[32];
  preferences.getBytes( "digest", image_digest, 32 );
  preferences.end();
  Serial.println( "Trying rollback" );

  if( menuSize == 0 ) {
    Serial.println( "Failed to get expected menu size from NVS ram, can't check if rollback is worth a try..." );
    return;
  }

  const esp_partition_t* update_partition = esp_ota_get_next_update_partition( NULL );
  esp_image_metadata_t sketchMeta = getSketchMeta( update_partition );
  uint32_t nuSize = sketchMeta.image_len;

  if( nuSize != menuSize ) {
    Serial.printf( "Cancelling rollback as flash sizes differ, update / current : %d / %d\n",  nuSize, menuSize );
    return;
  }

  Serial.println( "Sizes match! Checking digest..." );
  bool match = true;
  for( uint8_t i=0; i<32; i++ ) {
    if( image_digest[i]!=sketchMeta.image_digest[i] ) {
      Serial.println( "NO match for NVS digest :-(" );
      match = false;
      break;
    }
  }
  if( match ) {
    doRollBack( "HOT-LOADING " + fileName );
  }
}

// check given FS for valid menu.bin and perform update if available
void SDUpdater_Base::updateFromFS( fs::FS &fs, const String& fileName ) {
  #ifdef M5_SD_UPDATER_VERSION
    Serial.printf( "[" SD_PLATFORM_NAME "-SD-Updater] SD Updater version: %s\n", (char*)M5_SD_UPDATER_VERSION );
  #endif
  #ifdef M5_LIB_VERSION
    Serial.printf( "[" SD_PLATFORM_NAME "-SD-Updater] M5Stack Core version: %s\n", (char*)M5_LIB_VERSION );
  #endif
  Serial.printf( "[" SD_PLATFORM_NAME "-SD-Updater] Application was Compiled on %s %s\n", __DATE__, __TIME__ );

  #ifdef _SPIFFS_H_
    //#pragma message ("SPIFFS Support detected")
    log_i(" Checking for SPIFFS Support");
    if( &fs == &SPIFFS ) {
      if( !SPIFFS.begin() ){
        log_n( "SPIFFS MOUNT FAILED, ABORTING!!" );
        return;
      } else {
        log_i( "SPIFFS Successfully mounted");
        SPIFFS_MOUNTED = true;
      }
    }
  #endif
  #if SD_UPDATER_FS_TYPE==SDUPDATER_SD_FS
    //#pragma message ("SD Support detected")
    log_i(" Checking for SD Support");
    if( &fs == &SD ) {
      if( !SD.begin( cfg->TFCardCsPin /*_TFCardCsPin*/ /*4*/ ) ) {
        log_n( "SD MOUNT FAILED, ABORTING!!" );
        return;
      } else {
        log_i( "SD Successfully mounted");
      }
    }
  #endif
  #if SD_UPDATER_FS_TYPE==SDUPDATER_SD_MMC_FS
    //#pragma message ("SD_MMC Support detected")
    log_i(" Checking for SD_MMC Support");
    if( &fs == &SD_MMC ) {
      if( !SD_MMC.begin() ){
        log_n( "SD_MMC MOUNT FAILED, ABORTING!!" );
        return;
      } else {
        log_i( "SD_MMC Successfully mounted");
      }
    }
  #endif

  Serial.printf( "[" SD_PLATFORM_NAME "-SD-Updater] Will attempt to load binary %s \n", fileName.c_str() );
  // try rollback first, it's faster!
  if( strcmp( MENU_BIN, fileName.c_str() ) == 0 ) {
    tryRollback( fileName );
  }
  File updateBin = fs.open( fileName );
  if ( updateBin ) {
    if( updateBin.isDirectory() ){
      Serial.println( "Error, this is not a file" );
      updateBin.close();
      return;
    }
    size_t updateSize = updateBin.size();
    if ( updateSize > 0 ) {
      Serial.println( "Try to start update" );
      // disable WDT it as suggested by twitter.com/@lovyan03
      disableCore0WDT();
      performUpdate( updateBin, updateSize, fileName );
      enableCore0WDT();
    } else {
       Serial.println( "Error, file is empty" );
    }
    updateBin.close();
  } else {
    Serial.printf( "Could not load %s binary from sd root", fileName.c_str() );
  }
}


/*

static void SDUpdater_Base::getFactoryPartition() {
  esp_partition_iterator_t pi = esp_partition_find( ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL );
  if(pi != NULL) {
    const esp_partition_t* factory = esp_partition_get(pi);
    esp_partition_iterator_release(pi);
    if(esp_ota_set_boot_partition(factory) == ESP_OK) {
      //esp_restart();
    }
  }
}

*/


void SDUpdater_Base::checkSDUpdaterHeadless( fs::FS &fs, String fileName, unsigned long waitdelay )
{
  if( waitdelay == 0 ) {
    waitdelay = 100; // at least give some time for the serial buffer to fill
  }
  Serial.printf("SDUpdater: you have %d milliseconds to send 'update', 'rollback' or 'skip' command\n", (int)waitdelay);

  if( cfg->onWaitForAction ) {
    if ( cfg->onWaitForAction( nullptr, nullptr, waitdelay ) == 1 ) {
      Serial.printf( SDU_LOAD_TPL, fileName.c_str() );
      updateFromFS( fs, fileName );
      ESP.restart();
    }
  } else {
    log_w("No cfg->onWaitForAction was defined!");
  }

  Serial.print("Delay expired, no SD-Update will occur");
}



void SDUpdater_Base::checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay )
{

    bool isRollBack = true;
    if( fileName != "" ) {
      isRollBack = false;
    }

    #if defined HAS_TOUCH || defined _M5Core2_H_ // default touch button support
      // TODO: see if "touch/press" detect on boot is possible (spoil : NO)
      // TODO: read signal from other (external?) buttons
      bool draw = true;
      //if( waitdelay == 0 ) return; // don't check for any touch/button signal if waitDelay = 0
    #else // default push buttons support
      bool draw = false;
      //if( waitdelay == 0 ) return; // don't check for any touch/button signal if waitDelay = 0
      if( waitdelay <= 100 ) {
        // no UI draw, but still attempt to detect "button is pressed on boot"
        // round up to 100ms for button debounce
        waitdelay = 100;
      } else {
        // only draw if waitdelay > 0
        draw = true;
      }
    #endif

    if( draw ) { // bring up the UI
      if( cfg->onBefore) cfg->onBefore();
      if( cfg->onSplashPage) cfg->onSplashPage( BTN_HINT_MSG );
    }

    if( cfg->onWaitForAction ) {
      if ( cfg->onWaitForAction( isRollBack ? (char*)ROLLBACK_LABEL : (char*)LAUNCHER_LABEL,  (char*)SKIP_LABEL, waitdelay ) == 1 ) {
        if( isRollBack == false ) {
          Serial.printf( SDU_LOAD_TPL, fileName.c_str() );
          updateFromFS( fs, fileName );
          ESP.restart();
        } else {
          Serial.println( SDU_ROLLBACK_MSG );
          doRollBack( SDU_ROLLBACK_MSG );
        }
      }
    } else {
      log_e("Where is cfg->onWaitForAction ??");
    }

    if( draw ) {
      // reset text styles to avoid messing with the overlayed application
      if( cfg->onAfter ) cfg->onAfter();
    }
}




