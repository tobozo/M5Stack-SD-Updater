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
#include <Preferences.h>

#if !defined(TFCARD_CS_PIN) // override this from your sketch if the guess is wrong
  #if defined ARDUINO_LOLIN_D32_PRO || defined ARDUINO_M5STACK_Core2|| defined ARDUINO_M5STACK_CORE2 || defined ARDUINO_M5Stack_Core_ESP32 || defined ARDUINO_M5STACK_CORE_ESP32 || defined ARDUINO_M5STACK_FIRE
    #define TFCARD_CS_PIN  4
  #elif defined( ARDUINO_ESP32_WROVER_KIT ) || defined( ARDUINO_ODROID_ESP32 )
    #define TFCARD_CS_PIN 22
  #elif defined ARDUINO_TWATCH_BASE || defined ARDUINO_TWATCH_2020_V1 || defined ARDUINO_TWATCH_2020_V2 || defined(ARDUINO_TTGO_T1)
    #define TFCARD_CS_PIN 13
  #else
    #define TFCARD_CS_PIN SS
  #endif
#endif


#include "../ConfigManager/ConfigManager.hpp"

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
      void checkSDUpdaterHeadless( String fileName, unsigned long waitdelay );
      void checkSDUpdaterHeadless( fs::FS &fs, String fileName, unsigned long waitdelay );
      void checkSDUpdaterUI( String fileName, unsigned long waitdelay );
      void checkSDUpdaterUI( fs::FS &fs, String fileName, unsigned long waitdelay );
      // update methods
      void updateFromFS( const String& fileName );
      void updateFromFS( fs::FS &fs, const String& fileName = MENU_BIN );
      void updateFromStream( Stream &stream, size_t updateSize, const String& fileName );
      void doRollBack( const String& message = "" );
      // flash to SD binary replication
      bool compareFsPartition(const esp_partition_t* src1, fs::File* src2, size_t length);
      bool copyFsPartition(fs::File* dst, const esp_partition_t* src, size_t length);
      bool saveSketchToFS(fs::FS &fs, const char* binfilename = PROGMEM {MENU_BIN}, bool skipIfExists = false );
      // static methods
      static void updateNVS();
      static esp_image_metadata_t getSketchMeta( const esp_partition_t* source_partition );
      // fs::File->name() changed behaviour after esp32 sdk 2.x.x
      inline static const char* fs_file_path( fs::File *file )
      {
        #if defined ESP_IDF_VERSION_MAJOR && ESP_IDF_VERSION_MAJOR >= 4
          return file->path();
        #else
          return file->name();
        #endif
      }

      void _error( const String& errMsg, unsigned long waitdelay = 2000 );
      void _error( const char **errMsgs, uint8_t msgCount=1, unsigned long waitdelay=2000 );
      void _message( const String& label );
      config_sdu_t* cfg;
      //Stream &out;

    private:

      UpdateManagerInterface_t *UpdateIface = nullptr;
      const char* MenuBin = MENU_BIN;
      void performUpdate( Stream &updateSource, size_t updateSize, String fileName );
      void tryRollback( String fileName );

      #if defined _M5Core2_H_ || defined _M5CORES3_H_
        // Implicitely assume touch button support for TFT_eSpi based cores as per M5.begin() default behaviour
        const bool SDUHasTouch = true;
      #else
        const bool SDUHasTouch = false;
      #endif
      bool _fs_begun = false;
      bool _fsBegin( bool report_errors = true );
      bool _fsBegin( fs::FS &fs, bool report_errors = true );

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
    _fs_begun = _fsBegin( false );
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
    _fs_begun = _fsBegin( false );
  };


  inline bool SDUpdater::_fsBegin( bool report_errors )
  {
    if( cfg->fs != nullptr ) return _fsBegin( *cfg->fs, report_errors );
    if( !cfg->mounted ) _error( "No filesystem selected" ); // Note: rollback does not need filesystem
    return false;
  }


  inline bool SDUpdater::_fsBegin( fs::FS &fs, bool report_errors )
  {
    if( _fs_begun ) return true;
    if( cfg->fsChecker ) return cfg->fsChecker( this, *cfg->fs, report_errors );
    return false;
  }


}; // end namespace
