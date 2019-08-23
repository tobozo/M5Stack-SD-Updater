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

static int SD_UI_Progress;
#ifdef M5STACK
#define tft M5.Lcd // syntax sugar, forward compat with other displays
#endif


// enable SPIFFS persistence by backuping/restoring to/from the SD
SDUpdater::SDUpdater( const String SPIFFS2SDFolder ) {
  if( SPIFFS2SDFolder!="" ) {
    SKETCH_NAME = SPIFFS2SDFolder;
    enableSPIFFS = true;
  }
}


void SDUpdater::displayUpdateUI( String label ) {
  tft.begin();
  tft.setBrightness( 100 );
  tft.fillScreen( BLACK );
  tft.setTextColor( WHITE );
  tft.setTextFont( 0 );
  tft.setTextSize( 2 );
  // attemtp to center the text
  int16_t xpos = ( tft.width() / 2) - ( tft.textWidth( label ) / 2 );
  if ( xpos < 0 ) {
    // try with smaller size
    tft.setTextSize(1);
    xpos = ( tft.width() / 2 ) - ( tft.textWidth( label ) / 2 );
    if( xpos < 0 ) {
      // give up
      xpos = 0 ;
    }
  }
  tft.setCursor( xpos, 10 );
  tft.print( label );
  tft.drawRect( 110, 130, 102, 20, WHITE );
  SD_UI_Progress = -1;
}


void SDUpdater::SDMenuProgress( int state, int size ) {
  int percent = ( state * 100 ) / size;
  if( percent == SD_UI_Progress ) {
    // don't render twice the same value
    return;
  }
  //Serial.printf("percent = %d\n", percent); // this is spammy
  SD_UI_Progress = percent;
  uint16_t x      = tft.getCursorX();
  uint16_t y      = tft.getCursorY();
  int textfont    = tft.textfont;
  int textsize    = tft.textsize;
  int textcolor   = tft.textcolor;
  int textbgcolor = tft.textbgcolor;
  
  if ( percent >= 0 && percent < 101 ) {
    tft.fillRect( 111, 131, percent, 18, GREEN );
    tft.fillRect( 111+percent, 131, 100-percent, 18, BLACK );
    Serial.print( "." );
  } else {
    tft.fillRect( 111, 131, 100, 18, BLACK );
    Serial.println();
  }
  String percentStr = String( percent ) + "% ";
  tft.setTextFont( 0 ); // Select font 0 which is the Adafruit font
  tft.setTextSize( 1 ); // smallish size but some filenames are very long
  tft.setTextColor( WHITE, BLACK );
  int16_t xpos = ( tft.width() / 2 ) - ( tft.textWidth( percentStr ) / 2 );
  if ( xpos < 0 ) xpos = 0 ;
  tft.setCursor( xpos, 155 );
  tft.print( percentStr ); // trailing space is important
  tft.setCursor( x, y );
  tft.setTextFont( textfont ); // Select font 0 which is the Adafruit font
  tft.setTextSize( textsize );
  tft.setTextColor( textcolor, textbgcolor );
}


esp_image_metadata_t SDUpdater::getSketchMeta( const esp_partition_t* source_partition ) {
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

// rollback helper, save menu.bin meta info in NVS
void SDUpdater::updateNVS() {
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
void SDUpdater::performUpdate( Stream &updateSource, size_t updateSize, String fileName ) {
  displayUpdateUI( "LOADING " + fileName );
  Update.onProgress( SDMenuProgress );
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
          updateNVS();
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


// if NVS has info about MENU_BIN flash size and digest, try rollback()
void SDUpdater::tryRollback( String fileName ) {
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
    if( Update.canRollBack() )  {
      displayUpdateUI( "HOT-LOADING " + fileName );
      // animate something
      for( uint8_t i=1; i<50; i++ ) {
        SDMenuProgress( i, 100 );
      }
      Update.rollBack();
      for( uint8_t i=50; i<=100; i++ ) {
        SDMenuProgress( i, 100 );
      }
      Serial.println( "Rollback done, restarting" );
      ESP.restart();
    } else {
      Serial.println( "Sorry, looks like Updater.h doesn't want to rollback :-(" );
    }
  }
}

// check given FS for valid menu.bin and perform update if available
void SDUpdater::updateFromFS( fs::FS &fs, String fileName ) {
  #ifdef M5_SD_UPDATER_VERSION
    Serial.printf( "[M5Stack-SD-Updater] SD Updater version: %s\n", (char*)M5_SD_UPDATER_VERSION );
  #endif
  #ifdef M5_LIB_VERSION
    Serial.printf( "[M5Stack-SD-Updater] M5Stack Core version: %s\n", (char*)M5_LIB_VERSION );
  #endif
  Serial.printf( "[M5Stack-SD-Updater] Application was Compiled on %s %s\n", __DATE__, __TIME__ );
  
  if( enableSPIFFS ) {
    // firs backup SPIFFS to the SD
    copyDir( BACKUP_SPIFFS_TO_SD );
  }
  
  Serial.printf( "[M5Stack-SD-Updater] Will attempt to load binary %s \n", fileName.c_str() );
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

static void SDUpdater::getFactoryPartition() {
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




String SDUpdater::gnu_basename( String path ) {
  char *base = strrchr(path.c_str(), '/');
  return base ? String( base+1) : path;
}


void SDUpdater::copyFile( String sourceName, int dir ) {
  switch( dir ) {
    case BACKUP_SD_TO_SPIFFS:
      copyFile( sourceName, SD, dir );
    break;
    case BACKUP_SPIFFS_TO_SD:
      copyFile( sourceName, SPIFFS, dir );
    break;
  }
}


void SDUpdater::copyFile( String sourceName, fs::FS &sourceFS, int dir ) {
  File sourceFile = sourceFS.open( sourceName );
  if (! sourceFile ) {
    log_e("Unable to open source file for reading : %s", sourceName );
    return;
  }
  copyFile( sourceFile, dir );
  sourceFile.close();
}


void SDUpdater::copyFile( fs::File &sourceFile, int dir ) {
  String destName;
  String SDAppDataDir = String(DATA_DIR) + "/" + String( SKETCH_NAME );
  switch( dir ) {
    case BACKUP_SD_TO_SPIFFS:
      destName = sourceFile.name();
      destName.replace( SDAppDataDir, "" );
      log_d("BACKUP_SD_TO_SPIFFS source: %s, dest: %s", sourceFile.name(), destName.c_str() );
      if (SPIFFS.totalBytes() - SPIFFS.usedBytes() < sourceFile.size() ) {
        log_e("not enough space left to copy %s\n", sourceFile.name() );
        return;
      }
      copyFile( sourceFile, destName, SPIFFS );
    break;
    case BACKUP_SPIFFS_TO_SD:
      destName = SDAppDataDir + String( sourceFile.name() );
      log_d("BACKUP_SPIFFS_TO_SD source: %s, dest: %s", sourceFile.name(), destName.c_str() );
      copyFile( sourceFile, destName, SD );
    break;
  }
}


void SDUpdater::copyFile( String sourceName, fs::FS &sourceFS, String destName, fs::FS &destinationFS ) {
  File sourceFile = sourceFS.open( sourceName );
  if (! sourceFile ) {
    log_e("Unable to open source file for reading : %s", sourceName );
    return;
  }
  copyFile( sourceFile, destName, destinationFS );
  sourceFile.close();
}



#define BUFFER_SIZE 512

void SDUpdater::copyFile( fs::File &sourceFile, String destName, fs::FS &destinationFS ) {
  String sourceName = sourceFile.name();
  //displayUpdateUI( String( "MOVINGFILE_MESSAGE" ) + sourceName );
  size_t fileSize = sourceFile.size();

  String basename = gnu_basename( destName );
  String basepath = destName.substring( 0, destName.length()-(basename.length()+1) );
  if( !destinationFS.exists( basepath ) ) {
    log_d("destination path %s does not exist, will create", basepath.c_str() );
    makePathToFile( destName, destinationFS );
  }
  
  File destFile = destinationFS.open( destName, FILE_WRITE );
  if( !destFile ){ // for some reason this test is useless
    log_e( "Unable to open destination file for writing : %s", destName ) ;
    return;
  } else {
    log_d("Attempting to copy %s to %s", sourceName.c_str(), destName.c_str() );
    
    size_t bufferPos = 0;
    static uint8_t buf[BUFFER_SIZE];
    
    while (1) {
      if (bufferPos + BUFFER_SIZE >= fileSize) {
        // finalize buffer
        if (sourceFile.read(buf, fileSize - bufferPos) == -1) {
          break;
        }
        if (destFile.write(buf, fileSize - bufferPos) == -1) {
          break;
        }
        break;
      }
      bufferPos += BUFFER_SIZE;
      if (sourceFile.read(buf, BUFFER_SIZE) == -1) {
        break;
      }
      if (destFile.write(buf, BUFFER_SIZE) == -1) {
        break;
      }
      delay(1);
    }
    
    
    destFile.close();
    log_d( "File copy done :-)" );
  }
}


void SDUpdater::copyDir( int direction ) {
  String SDAppDataDir;

  if( !SPIFFS_MOUNTED ) {
    if( !SPIFFS.begin() ){
      log_e( "SPIFFS MOUNT FAILED, ABORTING!!" );
      return;
    } else {
      log_d( "SPIFFS Successfully mounted");
      SPIFFS_MOUNTED = true;
    }
  }
  
  switch( direction ) {
    case BACKUP_SD_TO_SPIFFS:
      SDAppDataDir = String(DATA_DIR) + "/" + String( SKETCH_NAME );
      copyDir( SDAppDataDir.c_str(), 4, direction );
    break;
    case BACKUP_SPIFFS_TO_SD:
      copyDir( "/", 4, direction );
    break;
  }
}


void SDUpdater::copyDir( const char * dirname, uint8_t levels, int direction ) {
  switch( direction ) {
    case BACKUP_SD_TO_SPIFFS:
      copyDir( SD, dirname, levels, direction );
    break;
    case BACKUP_SPIFFS_TO_SD:
      copyDir( SPIFFS, dirname, levels, direction );
    break;
  }  
}


void SDUpdater::copyDir(fs::FS &sourceFS, const char * dirname, uint8_t levels, int direction ) {
    log_d("Listing directory: %s\n", dirname);

    File root = sourceFS.open(dirname);
    if(!root){
        log_e("Failed to open %s directory", dirname);
        return;
    }
    if(!root.isDirectory()){
        log_e("%s is not a directory", dirname);
        return;
    }

    File sourceFile = root.openNextFile();
    while(sourceFile){
        if(sourceFile.isDirectory()){
            log_d("  DIR : %s", sourceFile.name());
            if(levels){
                copyDir(sourceFS, sourceFile.name(), levels -1, direction );
            }
        } else {
            // TODO : copy file
            log_d("  FILE: %32s - %8d bytes", sourceFile.name(), sourceFile.size());
            copyFile( sourceFile, direction );
        }
        sourceFile = root.openNextFile();
    }
}


void SDUpdater::makePathToFile( String destName, fs::FS destinationFS ) {
  String basename = gnu_basename( destName );
  String basepath = destName.substring( 0, destName.length()-(basename.length()+1) );
  if( basename == "/" || basename=="") {
    log_w("root dir or empty dir creation denied");
    return;
  }
  if( basename == destName ) {
    log_w("basename is equal to destName : %s", basename.c_str() );
    return;
  }
  if( !destinationFS.exists( basepath.c_str() ) ) {
    log_d("going recursive before creating %s folder", basepath.c_str());
    makePathToFile( basepath, destinationFS );
    log_w("creating %s folder after recursion", basepath.c_str());
    destinationFS.mkdir( basepath.c_str() );
    if( destinationFS.exists( basepath ) ) {
       // creation success
       log_d("folder creation success!");
     } else {
       log_d("folder creation failed!");
     }
  } else {
    log_d("destination path %s exists", basepath.c_str() );
  }
}


bool SDUpdater::SPIFFSFormat() {
  if( !SPIFFS.begin( true ) ){
    log_e( "SPIFFS Formatting FAILED!!" );
    return false;
  } else {
    SPIFFS.format();
    log_d( "SPIFFS Successfully formatted");
    SPIFFS_MOUNTED = true;
    return true;
  }
}


bool SDUpdater::SPIFFSisEmpty() {
  if( !SPIFFS_MOUNTED ) {
    if( !SPIFFS.begin() ) {
      log_e( "SPIFFS MOUNT FAILED, ABORTING!!" );
      return true;
    } else {
      log_d( "SPIFFS Successfully mounted");
      SPIFFS_MOUNTED = true;
    }
  }
  const char* dirname = "/";
  File root = SPIFFS.open(dirname);
  if(!root){
      log_e("Failed to open %s directory", dirname);
      return true;
  }
  if(!root.isDirectory()){
      log_e("%s is not a directory", dirname);
      return true;
  }
  File sourceFile = root.openNextFile();
  return sourceFile ? false : true;
}


#undef tft
