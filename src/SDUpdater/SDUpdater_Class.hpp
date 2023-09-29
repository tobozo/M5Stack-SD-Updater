#pragma once

#include "../gitTagVersion.h"
#include <esp_partition.h> // required by getSketchMeta(), compareFsPartition() and copyFsPartition() methods
extern "C" {
  #include "esp_ota_ops.h"
  #include "esp_image_format.h"
}
// required to guess the reset reason
#if defined ESP_IDF_VERSION_MAJOR && ESP_IDF_VERSION_MAJOR >= 4
  #if defined CONFIG_IDF_TARGET_ESP32
    #include <esp32/rom/rtc.h>
  #elif defined CONFIG_IDF_TARGET_ESP32S2
    #include <esp32s2/rom/rtc.h>
  #elif defined CONFIG_IDF_TARGET_ESP32C3
    #include <esp32c3/rom/rtc.h>
  #elif defined CONFIG_IDF_TARGET_ESP32S3
    #include <rom/rtc.h>
  #else
    #warning "Target CONFIG_IDF_TARGET is unknown"
    #include <rom/rtc.h>
  #endif
#else
  #include <rom/rtc.h>
#endif

#include <FS.h>
// #include <Update.h>
// required to store the MENU_BIN hash
// #include <Preferences.h>

#include "../ConfigManager/ConfigManager.hpp"
#include "../PartitionManager/PartitionManager.hpp"


namespace SDUpdaterNS
{

  extern void updateFromFS( const String& fileName );
  // provide an imperative function to avoid breaking button-based (older) versions of the M5Stack SD Updater
  extern void updateFromFS( fs::FS &fs, const String& fileName = MENU_BIN, const int TfCardCsPin = TFCARD_CS_PIN );
  // copy compiled sketch from flash partition to filesystem binary file
  extern bool saveSketchToFS(fs::FS &fs, const char* binfilename = PROGMEM {MENU_BIN}, const int TfCardCsPin = TFCARD_CS_PIN );
  // provide a rollback function for custom usages
  extern void updateRollBack( String message );
  // provide a conditional function to cover more devices, including headless and touch
  extern void checkSDUpdater( fs::FS &fs, String fileName = MENU_BIN, unsigned long waitdelay = 0, const int TfCardCsPin_ = TFCARD_CS_PIN );

  using ConfigManager::config_sdu_t;
  using UpdateInterfaceNS::UpdateManagerInterface_t;

  #if !defined SDU_SERIAL
    #define SDU_SERIAL Serial
  #endif

  class SDUpdater
  {
    public:

      SDUpdater( config_sdu_t* _cfg );
      SDUpdater( const int TFCardCsPin_ = TFCARD_CS_PIN);

      // check methods
      void checkUpdaterHeadless( String fileName );
      void checkUpdaterHeadless( fs::FS &fs, String fileName );
      void checkUpdaterUI( String fileName );
      void checkUpdaterUI( fs::FS &fs, String fileName );
      // update methods
      void updateFromFS( const String& fileName );
      void updateFromFS( fs::FS &fs, const String& fileName = MENU_BIN );
      void updateFromStream( Stream &stream, size_t updateSize, const String& fileName );
      void doRollBack( const String& message = "" );

      static bool saveSketchToFS( SDUpdater* sdu, fs::FS &fs, const char* binfilename={MENU_BIN}, bool skipIfExists=false );
      inline bool saveSketchToFS( fs::FS &fs, const char* binfilename={MENU_BIN}, bool skipIfExists=false ) { return saveSketchToFS(this, fs, binfilename, skipIfExists ); }

      static bool saveSketchToFactory();
      //static bool migrateSketch();
      static void updateNVS();

      // fs::File->name() changed behaviour after esp32 sdk 2.x.x
      inline static const char* fs_file_path( fs::File *file )
      {
        #if defined ESP_IDF_VERSION_MAJOR && ESP_IDF_VERSION_MAJOR >= 4
          return file->path();
        #else
          return file->name();
        #endif
      }

      static void _error( const String& errMsg, unsigned long waitdelay = 2000 );
      static void _error( const char **errMsgs, uint8_t msgCount=1, unsigned long waitdelay=2000 );
      static void _message( const String& label );
      config_sdu_t* cfg;

    private:

      UpdateManagerInterface_t *UpdateIface = nullptr;
      const char* MenuBin = MENU_BIN;
      bool checkUpdaterCommon( String fileName );
      void performUpdate( Stream &updateSource, size_t updateSize, String fileName );
      void tryRollback( String fileName );

      #if defined _M5Core2_H_ || defined _M5CORES3_H_
        // Implicitely assume touch button support for TFT_eSpi based cores as per M5.begin() default behaviour
        const bool SDUHasTouch = true;
      #else
        const bool SDUHasTouch = false;
      #endif
      static bool _fsBegin( SDUpdater* sdu, bool report_errors = true );
      static bool _fsBegin( SDUpdater* sdu, fs::FS &fs, bool report_errors = true );

  };



  inline SDUpdater::SDUpdater( config_sdu_t* _cfg ) : cfg(_cfg)
  {
    if( !UpdateIface ) {
      UpdateIface = ConfigManager::GetUpdateInterface();
    }
    if( ConfigManager::SDUCfgLoader ) {
      log_v("Config manager loader called");
      ConfigManager::SDUCfgLoader();
    } else {
      cfg->setDefaults();
    }
    cfg->fs_begun = _fsBegin( this, false );
  };


  // legacy constructor
  inline SDUpdater::SDUpdater( const int TFCardCsPin_ )
  {
    if( !UpdateIface ) {
      UpdateIface = ConfigManager::GetUpdateInterface();
    }
    //log_d("SDUpdater base mode on CS pin(%d)", TFCardCsPin_ );
    SDUCfg.setCSPin( TFCardCsPin_ );
    cfg = &SDUCfg;
    if( ConfigManager::SDUCfgLoader ) {
      log_v("Config manager loader called");
      ConfigManager::SDUCfgLoader();
    } else {
      cfg->setDefaults();
    }
    cfg->fs_begun = _fsBegin( this, false );
  };


  inline bool SDUpdater::_fsBegin( SDUpdater* sdu, bool report_errors )
  {
    if( SDUCfg.fs != nullptr ) return _fsBegin( sdu, *SDUCfg.fs, report_errors );
    if( !SDUCfg.mounted && report_errors ) _error( "No filesystem selected" ); // Note: rollback does not need filesystem
    return false;
  }


  inline bool SDUpdater::_fsBegin( SDUpdater* sdu, fs::FS &fs, bool report_errors )
  {
    if( SDUCfg.fs_begun ) return true;
    if( SDUCfg.fsChecker ) return SDUCfg.fsChecker( sdu, *SDUCfg.fs, report_errors );
    return false;
  }


  inline bool SDUpdater::saveSketchToFactory()
  {
    if( !SDUCfg.triggers ) {
      SDUCfg.setDefaults();
    }
    return PartitionManager::flashFactory();
  }


  // inline bool SDUpdater::migrateSketch()
  // {
  //   if( !SDUCfg.binFileName ) return false; // need this in NVS
  //   if( !SDUCfg.triggers ) {
  //     SDUCfg.setDefaults();
  //   }
  //   return PartitionManager::migrateSketch( SDUCfg.binFileName );
  // }



}; // end namespace
