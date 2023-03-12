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
#include <Update.h>
// required to store the MENU_BIN hash
#include <Preferences.h>

#if !defined(TFCARD_CS_PIN) // override this from your sketch if the guess is wrong
  #if defined( ARDUINO_LOLIN_D32_PRO ) || defined( ARDUINO_M5STACK_Core2  ) || defined( ARDUINO_M5Stack_Core_ESP32 ) || defined( ARDUINO_M5STACK_FIRE)
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

  // provide an imperative function to avoid breaking button-based (older) versions of the M5Stack SD Updater
  void updateFromFS( fs::FS &fs, const String& fileName = MENU_BIN, const int TfCardCsPin = TFCARD_CS_PIN );
  // copy compiled sketch from flash partition to filesystem binary file
  bool saveSketchToFS(fs::FS &fs, const char* binfilename = PROGMEM {MENU_BIN}, const int TfCardCsPin = TFCARD_CS_PIN );
  // provide a rollback function for custom usages
  void updateRollBack( String message );
  // provide a conditional function to cover more devices, including headless and touch
  void checkSDUpdater( fs::FS &fs, String fileName = MENU_BIN, unsigned long waitdelay = 0, const int TfCardCsPin_ = TFCARD_CS_PIN );

  using ConfigManager::config_sdu_t;
  //class SDUpdater;


  class SDUpdater
  {
    public:

      SDUpdater( config_sdu_t* _cfg );// : cfg(_cfg);
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
      bool copyFsPartition(File* dst, const esp_partition_t* src, size_t length);
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

    private:

      const char* MenuBin = MENU_BIN;
      void performUpdate( Stream &updateSource, size_t updateSize, String fileName );
      void tryRollback( String fileName );

      #if defined _M5Core2_H_ // enable additional touch button support
        const bool SDUHasTouch = true;
      #else
        const bool SDUHasTouch = false;
      #endif
      bool _fs_begun = false;
      bool _fsBegin( bool report_errors = true );
      bool _fsBegin( fs::FS &fs, bool report_errors = true );

  };


  namespace ConfigManager
  {
    extern void setup();
    extern bool hasFS( SDUpdater *sdu, fs::FS &fs, bool report_errors );
  }


  inline SDUpdater::SDUpdater( config_sdu_t* _cfg ) : cfg(_cfg)
  {
    if( ConfigManager::SDUCfgLoader ) {
      log_d("Config manager loader called");
      ConfigManager::SDUCfgLoader();
    } else {
      cfg->setDefaults();
      //ConfigManager::setup();
    }
    _fs_begun = _fsBegin( false );
    //if( cfg->fs == nullptr ) log_w("No filesystem selected in constructor!");
  };


  // legacy constructor
  inline SDUpdater::SDUpdater( const int TFCardCsPin_ )
  {
    //log_d("SDUpdater base mode on CS pin(%d)", TFCardCsPin_ );
    //SDUCfg.setSDU( this ); // attach this to SDUCfg.sdUpdater
    SDUCfg.setCSPin( TFCardCsPin_ );
    cfg = &SDUCfg;
    if( ConfigManager::SDUCfgLoader ) ConfigManager::SDUCfgLoader();
    else cfg->setDefaults();//ConfigManager::setup();
    _fs_begun = _fsBegin( false );
    //if( cfg->fs == nullptr ) log_w("No filesystem selected in constructor!");
  };


  inline bool SDUpdater::_fsBegin( bool report_errors )
  {
    if( cfg->fs != nullptr ) return _fsBegin( *cfg->fs, report_errors );
    _error( "No filesystem selected" );
    return false;
  }


  inline bool SDUpdater::_fsBegin( fs::FS &fs, bool report_errors )
  {
    if( _fs_begun ) return true;
    // bool mounted = false;
    // const char* msg[] = {nullptr, "ABORTING"};
    // #if defined _SPIFFS_H_
    //   log_d("Checking for SPIFFS Support");
    //   if( &fs == &SPIFFS ) {
    //     if( !SPIFFS.begin() ){
    //       msg[0] = "SPIFFS MOUNT FAILED";
    //       if( report_errors ) _error( msg, 2 );
    //       return false;
    //     } else { log_d("SPIFFS Successfully mounted"); }
    //     mounted = true;
    //   }
    // #endif
    // #if defined (_LITTLEFS_H_)
    //   log_d("Checking for LittleFS Support");
    //   if( &fs == &LittleFS ) {
    //     if( !LittleFS.begin() ){
    //       msg[0] = "LittleFS MOUNT FAILED";
    //       if( report_errors ) _error( msg, 2 );
    //       return false;
    //     } else { log_d("LittleFS Successfully mounted"); }
    //     mounted = true;
    //   }
    // #endif
    // #if defined (_SD_H_)
    //   log_d("[%d] Checking for SD Support (pin #%d)", ESP.getFreeHeap(), cfg->TFCardCsPin );
    //   if( &fs == &SD ) {
    //     // user provided specific pinout via build extra flags
    //     #if defined _CLK && defined _MISO && defined _MOSI
    //       SPI.begin(_CLK, _MISO, _MOSI, cfg->TFCardCsPin);
    //       SPI.setDataMode(SPI_MODE3);
    //       if (!SD.begin(cfg->TFCardCsPin, SPI, 80000000)) // 80MHz(MAX)
    //     #else
    //       if (!SD.begin(cfg->TFCardCsPin))
    //     #endif
    //     {
    //       msg[0] = String("SD MOUNT FAILED (pin #" + String(cfg->TFCardCsPin) + ")").c_str();
    //       if( report_errors ) _error( msg, 2 );
    //       return false;
    //     } else {
    //       log_d("[%d] SD Successfully mounted (pin #%d)", ESP.getFreeHeap(), cfg->TFCardCsPin );
    //     }
    //     mounted = true;
    //   }
    // #endif
    // #if defined (_SDMMC_H_)
    //   log_d(" Checking for SD_MMC Support");
    //   if( &fs == &SD_MMC ) {
    //     if( !SD_MMC.begin() ){
    //       msg[0] = "SD_MMC FAILED";
    //       if( report_errors ) _error( msg, 2 );
    //       return false;
    //     } else { log_d( "SD_MMC Successfully mounted"); }
    //     mounted = true;
    //   }
    // #endif
    // #if __has_include(<PSRamFS.h>) || defined _PSRAMFS_H_
    //   log_d(" Checking for PSRamFS Support");
    //   if( &fs == &PSRamFS ) {
    //     if( !PSRamFS.begin() ){
    //       msg[0] = "PSRamFS FAILED";
    //       if( report_errors ) _error( msg, 2 );
    //       return false;
    //     } else { log_d( "PSRamFS Successfully mounted"); }
    //     mounted = true;
    //   }
    // #endif
    // return mounted;
    return ConfigManager::hasFS( this, *cfg->fs, report_errors );
  }


}; // end namespace
