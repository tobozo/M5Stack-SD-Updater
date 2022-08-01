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
#elif defined( ARDUINO_ODROID_ESP32 )
  #define SD_PLATFORM_NAME "Odroid-GO"
#elif defined( ARDUINO_M5Stack_Core_ESP32 )
  #define SD_PLATFORM_NAME "M5Stack"
#elif defined( ARDUINO_M5STACK_FIRE )
  #define SD_PLATFORM_NAME "M5Stack-Fire"
#elif defined( ARDUINO_M5STACK_Core2 )
  #define SD_PLATFORM_NAME "M5StackCore2"
#elif defined( ARDUINO_ESP32_WROVER_KIT )
  #define SD_PLATFORM_NAME "Wrover-Kit"
#elif defined( ARDUINO_TTGO_T1 )            // TTGO T1
  #define SD_PLATFORM_NAME "TTGO-T1"
#elif defined( ARDUINO_LOLIN_D32_PRO )      // LoLin D32 Pro
  #define SD_PLATFORM_NAME "LoLin D32 Pro"
#elif defined( ARDUINO_T_Watch )            // TWatch, all models
  #define SD_PLATFORM_NAME "TTGO TWatch"
#else
  //#pragma message ("Custom ESP32 board detected")
  #define SD_PLATFORM_NAME "ESP32"
#endif


void SDUpdater::_error( const char **errMsgs, uint8_t msgCount, unsigned long waitdelay )
{
  for( int i=0; i<msgCount; i++ ) {
    _error( String(errMsgs[i]), i<msgCount-1?0:waitdelay );
  }
}


void SDUpdater::_error( const String& errMsg, unsigned long waitdelay )
{
  Serial.print("[ERROR] ");
  Serial.println( errMsg );
  if( cfg->onError ) cfg->onError( errMsg, waitdelay );
}

void SDUpdater::_message( const String& msg )
{
   Serial.println( msg );
   if( cfg->onMessage ) cfg->onMessage( msg );
}


esp_image_metadata_t SDUpdater::getSketchMeta( const esp_partition_t* source_partition )
{
  esp_image_metadata_t data;
  if ( !source_partition ) {
    log_e("No source partition provided");
    return data;
  }
  const esp_partition_pos_t source_partition_pos  = {
     .offset = source_partition->address,
     .size = source_partition->size,
  };
  data.start_addr = source_partition_pos.offset;
  esp_err_t ret = esp_image_verify( ESP_IMAGE_VERIFY, &source_partition_pos, &data );
  // only verify OTA0 or OTA1
  if( source_partition->label[3] == '1' || source_partition->label[3] == '0' ) {
    if( ret != ESP_OK ) {
      log_e("Failed to verify image %s at addr %x", String( source_partition->label ), source_partition->address );
    } else {
      //log_w("Successfully verified image %s at addr %x", String( source_partition->label[3] ), source_partition->address );
    }
  }
  return data;//.image_len;
}

/*

static void SDUpdater::getFactoryPartition()
{
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



bool SDUpdater::compareFsPartition(const esp_partition_t* src1, fs::File* src2, size_t length)
{
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


bool SDUpdater::copyFsPartition(File* dst, const esp_partition_t* src, size_t length)
{
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
        vTaskDelay(10);
      }
    }
  }
  return true;
}


bool SDUpdater::saveSketchToFS( fs::FS &fs, const char* binfilename, bool skipIfExists )
{
  // no rollback possible, start filesystem
  if( !_fsBegin( fs ) ) {
    const char *msg[] = {"No Filesystem mounted.","Can't check firmware."};
    _error( msg, 2 );
    return false;
  }

  if( skipIfExists ) {
    if( fs.exists( binfilename ) ) {
      log_d("File %s exists, skipping overwrite", binfilename );
      //_message( String("\nChecked ") + String(binfilename) );
      return false;
    }
  }
  if( cfg->onBefore) cfg->onBefore();
  if( cfg->onProgress ) cfg->onProgress( 0, 100 );
  const esp_partition_t *running = esp_ota_get_running_partition();
  size_t sksize = ESP.getSketchSize();
  bool ret = false;
  fs::File dst = fs.open(binfilename, FILE_WRITE );
  if( cfg->onProgress ) cfg->onProgress( 25, 100 );
  _message( String("Overwriting ") + String(binfilename) );

  if (copyFsPartition( &dst, running, sksize)) {
    if( cfg->onProgress ) cfg->onProgress( 75, 100 );
    _message( String("Done ") + String(binfilename) );
    vTaskDelay(1000);
    ret = true;
  } else {
    _error( "Copy failed" );
  }
  if( cfg->onProgress ) cfg->onProgress( 100, 100 );
  dst.close();
  if( cfg->onAfter) cfg->onAfter();

  return ret;
}


// rollback helper, save menu.bin meta info in NVS
void SDUpdater::updateNVS()
{
  const esp_partition_t* update_partition = esp_ota_get_next_update_partition( NULL );
  if (!update_partition) {
    log_d( "Cancelling NVS Update as update partition is invalid" );
    return;
  }
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
void SDUpdater::performUpdate( Stream &updateSource, size_t updateSize, String fileName )
{
  _message( "LOADING " + fileName );
  log_d( "Binary size: %d bytes", updateSize );
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
        if( strcmp( MenuBin, fileName.c_str() ) == 0 ) {
          // maintain NVS signature
          SDUpdater::updateNVS();
        }
        Serial.println( "Update successfully completed. Rebooting." );
      } else {
        Serial.println( "Update not finished? Something went wrong!" );
      }
    } else {
      Serial.println( "Update failed. Error #: " + String( Update.getError() ) );
    }
  } else {
    Serial.println( "Not enough space to begin OTA" );
  }
}


// forced rollback (doesn't check NVS digest)
void SDUpdater::doRollBack( const String& message )
{
  log_d("Wil check for rollback capability");
  if( !cfg->onMessage)   { log_d("No message reporting"); }
  //if( !cfg->onError )    log_d("No error reporting");
  if( !cfg->onProgress ) { log_d("No progress reporting"); }

  if( Update.canRollBack() ) {
    _message( message );
    for( uint8_t i=1; i<50; i++ ) {
      if( cfg->onProgress ) cfg->onProgress( i, 100 );
      vTaskDelay(10);
    }
    Update.rollBack();
    for( uint8_t i=50; i<=100; i++ ) {
      if( cfg->onProgress ) cfg->onProgress( i, 100 );
      vTaskDelay(10);
    }
    _message( "Rollback done, restarting" );
    ESP.restart();
  } else {
    const char *msg[] = {"Cannot rollback", "The other OTA", "partition doesn't", "seem to be", "populated or valid"};
    _error( msg, 5 );
  }
}


// if NVS has info about MENU_BIN flash size and digest, try rollback()
void SDUpdater::tryRollback( String fileName )
{
  Preferences preferences;
  preferences.begin( "sd-menu" );
  uint32_t menuSize = preferences.getInt( "menusize", 0 );
  uint8_t image_digest[32];
  preferences.getBytes( "digest", image_digest, 32 );
  preferences.end();
  Serial.println( "Trying rollback" );

  if( menuSize == 0 ) {
    log_d( "Failed to get expected menu size from NVS ram, can't check if rollback is worth a try..." );
    return;
  }

  const esp_partition_t* update_partition = esp_ota_get_next_update_partition( NULL );
  if (!update_partition) {
    log_d( "Cancelling rollback as update partition is invalid" );
    return;
  }
  esp_image_metadata_t sketchMeta = getSketchMeta( update_partition );
  uint32_t nuSize = sketchMeta.image_len;

  if( nuSize != menuSize ) {
    log_d( "Cancelling rollback as flash sizes differ, update / current : %d / %d",  nuSize, menuSize );
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


// do perform update
void SDUpdater::updateFromStream( Stream &stream, size_t updateSize, const String& fileName )
{
  if ( updateSize > 0 ) {
    Serial.println( "Try to start update" );
    disableCore0WDT(); // disable WDT it as suggested by twitter.com/@lovyan03
    performUpdate( stream, updateSize, fileName );
    enableCore0WDT();
  } else {
    _error( "Stream is empty" );
  }
}


void SDUpdater::updateFromFS( fs::FS &fs, const String& fileName )
{
  cfg->setFS( &fs );
  updateFromFS( fileName );
}


void SDUpdater::checkSDUpdaterHeadless( fs::FS &fs, String fileName, unsigned long waitdelay )
{
  cfg->setFS( &fs );
  checkSDUpdaterHeadless( fileName, waitdelay );
}


void SDUpdater::checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay )
{
  cfg->setFS( &fs );
  checkSDUpdaterUI( fileName, waitdelay );
}


void SDUpdater::updateFromFS( const String& fileName )
{
  if( cfg->fs == nullptr ) {
    const char *msg[] = {"No valid filesystem", "selected!"};
    _error( msg, 2 );
    return;
  }
  Serial.printf( "[" SD_PLATFORM_NAME "-SD-Updater] SD Updater version: %s\n", (char*)M5_SD_UPDATER_VERSION );
  #ifdef M5_LIB_VERSION
    Serial.printf( "[" SD_PLATFORM_NAME "-SD-Updater] M5Stack Core version: %s\n", (char*)M5_LIB_VERSION );
  #endif
  Serial.printf( "[" SD_PLATFORM_NAME "-SD-Updater] Application was Compiled on %s %s\n", __DATE__, __TIME__ );
  Serial.printf( "[" SD_PLATFORM_NAME "-SD-Updater] Will attempt to load binary %s \n", fileName.c_str() );

  // try rollback first, it's faster!
  if( strcmp( MenuBin, fileName.c_str() ) == 0 ) {
    if( cfg->use_rollback ) {
      tryRollback( fileName );
      log_e("Rollback failed, will try from filesystem");
    } else {
      log_d("Skipping rollback per config");
    }
  }
  // no rollback possible, start filesystem
  if( !_fsBegin() ) {
    const char* msg[] = {"No filesystem mounted.", "Can't load firmware."};
    _error( msg, 2 );
    return;
  }

  File updateBin = cfg->fs->open( fileName );
  if ( updateBin ) {

    if( updateBin.isDirectory() ) {
      updateBin.close();
      _error( fileName + " is a directory" );
      return;
    }

    size_t updateSize = updateBin.size();

    updateFromStream( updateBin, updateSize, fileName );

    updateBin.close();

  } else {
    const char* msg[] = {"Could not reach", fileName.c_str(), "Can't load firmware."};
    _error( msg, 3 );
  }
}


// check given FS for valid menu.bin and perform update if available
void SDUpdater::checkSDUpdaterHeadless( String fileName, unsigned long waitdelay )
{
  if( waitdelay == 0 ) {
    waitdelay = 100; // at least give some time for the serial buffer to fill
  }
  Serial.printf("SDUpdater: you have %d milliseconds to send 'update', 'rollback', 'skip' or 'save' command\n", (int)waitdelay);

  if( cfg->onWaitForAction ) {
    int ret = cfg->onWaitForAction( nullptr, nullptr, nullptr, waitdelay );
    if ( ret == SDU_BTNA_MENU ) {
      Serial.printf( SDU_LOAD_TPL, fileName.c_str() );
      updateFromFS( fileName );
      ESP.restart();
    }
    if( cfg->binFileName != nullptr ) {
      log_d("Checking if %s needs saving", cfg->binFileName );
      saveSketchToFS( *cfg->fs,  cfg->binFileName, ret != SDU_BTNC_SAVE );
    }
  } else {
    _error( "Missing onWaitForAction!" );
  }

  Serial.println("Delay expired, no SD-Update will occur");
}


void SDUpdater::checkSDUpdaterUI( String fileName, unsigned long waitdelay )
{
  if( cfg->fs == nullptr ) {
    const char* msg[] = {"No valid filesystem", "selected!", "Cannot load", fileName.c_str()};
    _error( msg, 4 );
    return;
  }
  bool draw = SDUHasTouch;
  bool isRollBack = true;
  if( fileName != "" ) {
    isRollBack = false;
  }

  if( !draw ) { // default touch button support
    if( waitdelay <= 100 ) {
      // no UI draw, but still attempt to detect "button is pressed on boot"
      // round up to 100ms for button debounce
      waitdelay = 100;
    } else {
      // only force draw if waitdelay > 100
      draw = true;
    }
  }

  if( draw ) { // bring up the UI
    if( cfg->onBefore) cfg->onBefore();
    if( cfg->onSplashPage) cfg->onSplashPage( BTN_HINT_MSG );
  }

  if( cfg->onWaitForAction ) {
    [[maybe_unused]] unsigned long startwait = millis();
    int ret = cfg->onWaitForAction( isRollBack ? (char*)cfg->labelRollback : (char*)cfg->labelMenu,  (char*)cfg->labelSkip, (char*)cfg->labelSave, waitdelay );
    [[maybe_unused]] unsigned long actualwaitdelay = millis()-startwait;
    log_v("Action '%d' was triggered after %d ms (waidelay=%d)", ret, actualwaitdelay, waitdelay );

    if ( ret == SDU_BTNA_MENU ) {
      if( isRollBack == false ) {
        Serial.printf( SDU_LOAD_TPL, fileName.c_str() );
        updateFromFS( fileName );
        ESP.restart();
      } else {
        Serial.println( SDU_ROLLBACK_MSG );
        doRollBack( SDU_ROLLBACK_MSG );
      }
    }
    if( cfg->binFileName != nullptr ) {
      log_d("Checking if %s needs saving", cfg->binFileName );
      saveSketchToFS( *cfg->fs,  cfg->binFileName, ret != SDU_BTNC_SAVE );
    }
  } else {
    _error( "Missing onWaitForAction!" );
  }

  if( draw ) {
    // reset text styles to avoid messing with the overlayed application
    if( cfg->onAfter ) cfg->onAfter();
  }
}
