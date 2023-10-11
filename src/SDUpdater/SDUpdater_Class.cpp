#include "./SDUpdater_Class.hpp"

namespace SDUpdaterNS
{



  void SDUpdater::_error( const char **errMsgs, uint8_t msgCount, unsigned long waitdelay )
  {
    for( int i=0; i<msgCount; i++ ) {
      _error( String(errMsgs[i]), i<msgCount-1?0:waitdelay );
    }
  }


  void SDUpdater::_error( const String& errMsg, unsigned long waitdelay )
  {
    SDU_SERIAL.print("[ERROR] ");
    SDU_SERIAL.println( errMsg );
    if( SDUCfg.onError ) SDUCfg.onError( errMsg, waitdelay );
  }


  void SDUpdater::_message( const String& msg )
  {
    SDU_SERIAL.println( msg );
    if( SDUCfg.onMessage ) SDUCfg.onMessage( msg );
  }


  bool SDUpdater::saveSketchToFS( SDUpdater* sdu, fs::FS &fs, const char* binfilename, bool skipIfExists )
  {
    assert(sdu);
    // no rollback possible, start filesystem
    if( !_fsBegin( sdu, fs ) ) {
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
    if( SDUCfg.onBefore) SDUCfg.onBefore();
    if( SDUCfg.onProgress ) SDUCfg.onProgress( 0, 100 );
    const esp_partition_t *running = esp_ota_get_running_partition();
    size_t sksize = ESP.getSketchSize();
    bool ret = false;
    fs::File dst = fs.open(binfilename, FILE_WRITE );
    if( SDUCfg.onProgress ) SDUCfg.onProgress( 25, 100 );
    _message( String("Overwriting ") + String(binfilename) );

    if (Flash::copyPartition( &dst, running, sksize)) {
      if( SDUCfg.onProgress ) SDUCfg.onProgress( 75, 100 );
      _message( String("Done ") + String(binfilename) );
      vTaskDelay(1000);
      ret = true;
    } else {
      _error( "Copy failed" );
    }
    if( SDUCfg.onProgress ) SDUCfg.onProgress( 100, 100 );
    dst.close();
    if( SDUCfg.onAfter) SDUCfg.onAfter();

    return ret;
  }


  // rollback helper, save menu.bin meta info in NVS
  void SDUpdater::updateNVS()
  {
    SDU_SERIAL.printf( "Updating NVS preferences with current partition's size/digest" );
    NVS::saveMenuPrefs();
  }


  // perform the actual update from a given stream
  void SDUpdater::performUpdate( Stream &updateSource, size_t updateSize, String fileName )
  {
    assert(UpdateIface);
    UpdateIface->setBinName( fileName, &updateSource );

    _message( "LOADING " + fileName );
    log_d( "Binary size: %d bytes", updateSize );
    if( cfg->onProgress ) UpdateIface->onProgress( cfg->onProgress );
    if (UpdateIface->begin( updateSize )) {
      size_t written = UpdateIface->writeStream( updateSource, updateSize );
      if ( written == updateSize ) {
        SDU_SERIAL.println( "Written : " + String(written) + " successfully" );
      } else {
        SDU_SERIAL.println( "Written only : " + String(written) + "/" + String(updateSize) + ". Retry?" );
      }
      if ( UpdateIface->end() ) {
        SDU_SERIAL.println( "OTA done!" );
        if ( UpdateIface->isFinished() ) {
          if( strcmp( MenuBin, fileName.c_str() ) == 0 ) {
            // maintain NVS signature
            SDUpdater::updateNVS();
          }
          SDU_SERIAL.println( "Update successfully completed. Rebooting." );
        } else {
          SDU_SERIAL.println( "Update not finished? Something went wrong!" );
        }
      } else {
        SDU_SERIAL.println( "Update failed. Error #: " + String( UpdateIface->getError() ) );
      }
    } else {
      SDU_SERIAL.println( "Not enough space to begin OTA" );
    }
  }


  // forced rollback (doesn't check NVS digest)
  void SDUpdater::doRollBack( const String& message )
  {
    assert(UpdateIface);
    SDU_SERIAL.println( SDU_ROLLBACK_MSG );
    log_d("Wil check for rollback capability");
    if( !cfg->onMessage)   { log_d("No message reporting"); }
    //if( !cfg->onError )    log_d("No error reporting");
    if( !cfg->onProgress ) { log_d("No progress reporting"); }

    if( UpdateIface->canRollBack() ) {
      _message( message );
      for( uint8_t i=1; i<50; i++ ) {
        if( cfg->onProgress ) cfg->onProgress( i, 100 );
        vTaskDelay(10);
      }
      UpdateIface->rollBack();
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


  void SDUpdater::tryRollback( String fileName )
  {
    if( cfg->rollBackToFactory ) return; // SDU settings: factory supersedes rollback
    // if NVS has info about MENU_BIN flash size and digest, try rollback()
    uint32_t menuSize;// = preferences.getInt( "menusize", 0 );
    uint8_t image_digest[32];
    NVS::getMenuPrefs( &menuSize, image_digest );

    SDU_SERIAL.println( "Trying rollback" );

    if( menuSize == 0 ) {
      log_d( "Failed to get expected menu size from NVS ram, can't check if rollback is worth a try..." );
      return;
    }

    const esp_partition_t* update_partition = esp_ota_get_next_update_partition( NULL );
    if (!update_partition) {
      log_d( "Cancelling rollback as update partition is invalid" );
      return;
    }
    esp_image_metadata_t sketchMeta = Flash::getSketchMeta( update_partition );
    uint32_t nuSize = sketchMeta.image_len;

    if( nuSize != menuSize ) {
      log_d( "Cancelling rollback as flash sizes differ, update / current : %d / %d",  nuSize, menuSize );
      return;
    }

    SDU_SERIAL.println( "Sizes match! Checking digest..." );
    bool match = true;
    for( uint8_t i=0; i<32; i++ ) {
      if( image_digest[i]!=sketchMeta.image_digest[i] ) {
        SDU_SERIAL.println( "NO match for NVS digest :-(" );
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
      SDU_SERIAL.println( "Try to start update" );
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


  void SDUpdater::checkUpdaterHeadless( fs::FS &fs, String fileName )
  {
    cfg->setFS( &fs );
    checkUpdaterHeadless( fileName );
  }


  void SDUpdater::checkUpdaterUI( fs::FS &fs, String fileName )
  {
    cfg->setFS( &fs );
    checkUpdaterUI( fileName );
  }


  void SDUpdater::updateFromFS( const String& fileName )
  {
    SDU_SERIAL.printf( "[" SD_PLATFORM_NAME "-SD-Updater] SD Updater version: %s\n", (char*)M5_SD_UPDATER_VERSION );
    #ifdef M5_LIB_VERSION
      SDU_SERIAL.printf( "[" SD_PLATFORM_NAME "-SD-Updater] M5Stack Core version: %s\n", (char*)M5_LIB_VERSION );
    #endif
    SDU_SERIAL.printf( "[" SD_PLATFORM_NAME "-SD-Updater] Application was Compiled on %s %s\n", __DATE__, __TIME__ );
    SDU_SERIAL.printf( "[" SD_PLATFORM_NAME "-SD-Updater] Will attempt to load binary %s \n", fileName.c_str() );

    // try from flash, same as rollback but found from NVS partition table
    if( fileName!="" && Flash::hasFactoryApp() ) {
      auto part = NVS::findPartition( fileName.c_str() );
      if( part ) {
        Flash::bootPartition( part->ota_num );
      }
    }
    // try rollback
    if( strcmp( MenuBin, fileName.c_str() ) == 0 ) {
      if( cfg->use_rollback ) {
        tryRollback( fileName );
        log_e("Rollback failed, will try from filesystem");
      } else {
        log_d("Skipping rollback per config");
      }
    }
    // no bootPartition()/rollback() possible, can't go any further without filesystem
    if( cfg->fs == nullptr ) {
      const char *msg[] = {"No valid filesystem", "selected!"};
      _error( msg, 2 );
      return;
    }
    // start filesystem
    if( !_fsBegin(this) ) {
      const char* msg[] = {"No filesystem mounted.", "Can't load firmware."};
      _error( msg, 2 );
      return;
    }

    File updateBin = cfg->fs->open( fileName );
    if ( updateBin ) {
      updateFromStream( updateBin, updateBin.size(), fileName );
      updateBin.close();
    } else {
      const char* msg[] = {"Could not reach", fileName.c_str(), "Can't load firmware."};
      _error( msg, 3 );
    }
  }


  void SDUpdater::checkUpdaterHeadless( String fileName )
  {
    if( SDUCfg.waitdelay == 0 ) {
      SDUCfg.waitdelay = 100; // at least give some time for the serial buffer to fill
    }
    SDU_SERIAL.printf("SDUpdater: you have %d milliseconds to send 'update', 'rollback', 'skip' or 'save' command\n", (int)SDUCfg.waitdelay);

    if( !checkUpdaterCommon( fileName ) ) return;

    SDU_SERIAL.println("Delay expired, no SD-Update will occur");
  }


  void SDUpdater::checkUpdaterUI( String fileName )
  {
    bool draw = SDUHasTouch || (SDUCfg.waitdelay>100 && !SDUHasTouch); // note: draw forced if waitdelay>100

    if( SDUCfg.waitdelay <= 100 ) {
      draw = false;    // 100ms delay will just look like a blink, cancel UI draw
      SDUCfg.waitdelay = 100; // button press/touch on boot still needs "blind" detection, round up to 100ms for debounce
    }

    if( draw ) { // bring up the UI
      if( cfg->onBefore) cfg->onBefore();
      if( cfg->onSplashPage) cfg->onSplashPage( BTN_HINT_MSG );
    }

    checkUpdaterCommon( fileName );

    if( draw ) {
      // reset text styles to avoid messing with the overlayed application
      if( cfg->onAfter ) cfg->onAfter();
    }
  }


  // common logic to checkUpdaterUI() and checkUpdaterHeadless()
  bool SDUpdater::checkUpdaterCommon( String fileName )
  {
    bool hasFileName = (fileName!="");

    // if using factory: disable "Save FW" action button when running partition isn't the first partition
    SDUCfg.Buttons[2].enabled = cfg->rollBackToFactory
      ? esp_ota_get_running_partition()->subtype==ESP_PARTITION_SUBTYPE_APP_OTA_MIN
      : SDUCfg.Buttons[2].enabled
    ;

    if( cfg->onWaitForAction ) {
      int action = cfg->onWaitForAction( !hasFileName ? (char*)cfg->labelRollback : (char*)cfg->labelMenu,  (char*)cfg->labelSkip, (char*)cfg->labelSave, SDUCfg.waitdelay );

      if ( action == ConfigManager::SDU_BTNA_MENU ) { // all BtnA successful actions trigger a restart
        if( hasFileName ) {
          if( cfg->fs == nullptr ) {
            const char* msg[] = {"No valid filesystem", "selected!", "Cannot load", fileName.c_str()};
            _error( msg, 4 );
            return false;
          }
          SDU_SERIAL.printf( SDU_LOAD_TPL, fileName.c_str() );
          updateFromFS( fileName );
          ESP.restart();
        } else {
          if( cfg->rollBackToFactory ) Flash::loadFactory();
          doRollBack( SDU_ROLLBACK_MSG );
        }
        return false; // rollback failed
      } else if( cfg->binFileName != nullptr ) {
        if( cfg->fs != nullptr ) {
          log_v("Checking if %s needs saving", cfg->binFileName );
          saveSketchToFS( *cfg->fs,  cfg->binFileName, action != ConfigManager::SDU_BTNC_SAVE );
        } else if( cfg->rollBackToFactory ) {
          return PartitionManager::migrateSketch( cfg->binFileName );
        } else if( action == ConfigManager::SDU_BTNC_SAVE ) {
          const char* msg[] = {"No valid filesystem", "selected!", "Cannot save", fileName.c_str()};
          _error( msg, 4 );
          return false;
        }
      }
      return true;
    } else {
      _error( "Missing onWaitForAction!" );
      return false;
    }
  }


}; // end namespace
