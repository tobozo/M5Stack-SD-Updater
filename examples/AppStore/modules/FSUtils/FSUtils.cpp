#pragma once

#include "FSUtils.hpp"

extern LogWindow *Console;

namespace FSUtils
{

  using namespace RegistryUtils;

  void setTimeFromLastFSAccess()
  {
    // Try to get a timestamp from filesystem in order to set system time
    // to a value closer to "now" than the defaults
    File root;
    if( M5_FS.exists( SDU_APP_PATH ) ) { // try self
      root = M5_FS.open( SDU_APP_PATH );
    } else if ( M5_FS.exists( MENU_BIN ) ) { // try /menu.bin
      root = M5_FS.open( MENU_BIN );
    } else { // try rootdir
      root = M5_FS.open( ROOT_DIR );
    }
    time_t lastWrite;
    String lastWriteSource;
    if( root ) {
      lastWrite = root.getLastWrite();
      lastWriteSource = root.name();
      root.close();
    } else {
      lastWrite = __TIME_UNIX__;
      lastWriteSource = "__TIME_UNIX__";
    }
    // RTC-less devices:
    // Set a pseudo realistic internal clock time when no NTP sync occured,
    // and before writing to the SD Card. Timestamps are still inacurate but
    // better than ESP32's [1980-01-01 00:00:00] default boot datetime.
    int epoch_time = __TIME_UNIX__; // this macro is populated at compilation time
    struct tm * tmstruct = localtime(&lastWrite);
    String timeSource;
    String timeStatus;

    // TODO: manually change this limit every century
    if( (tmstruct->tm_year)+1900 < 2000 || (tmstruct->tm_year)+1900 > 2100 ) {
      timeStatus = "an unreliable";
      timeSource = "sketch build time";
    } else {
      int tmptime = mktime(tmstruct); // epoch time ( seconds since 1st jan 1969 )
      if( tmptime > epoch_time ) {
        timeStatus = "a realistic";
        timeSource = lastWriteSource+" last write";
        epoch_time = tmptime;
      } else {
        timeStatus = "an obsolete";
        timeSource = "sketch build time";
      }
    }

    log_w(  DEBUG_TIMESTAMP_GUESS,
      lastWriteSource.c_str(),
      timeStatus.c_str(),
      (tmstruct->tm_year)+1900,
      ( tmstruct->tm_mon)+1,
      tmstruct->tm_mday,
      tmstruct->tm_hour,
      tmstruct->tm_min,
      tmstruct->tm_sec,
      timeSource.c_str()
    );

    timeval epoch = {epoch_time, 0};
    const timeval *tv = &epoch;
    settimeofday(tv, NULL);

    struct tm now;
    getLocalTime(&now,0);

    Serial.printf("[Hobo style] Clock set to %s source (%s): ", timeStatus.c_str(), timeSource.c_str());
    Serial.println(&now,"%B %d %Y %H:%M:%S (%A)");
  }


  bool getFileAttrs( const char* name, size_t *_size, time_t *_time )
  {
    fs::File file = M5_FS.open( name );
    if( !file ) {
      log_v("Can't reach file: %s", name );
      return false;
    }
    *_size = file.size();
    *_time = file.getLastWrite();
    log_v("Extracted size/time (%d/%d) for file %s", *_size, *_time, name );
    file.close();
    return true;
  }



  void countApps()
  {
    std::vector<String> files;
    getInstalledApps( files );
    appsCount = files.size();
  }


  bool getInstalledApps( std::vector<String> &files ) {
    files.clear();
    File root = M5_FS.open( ROOT_DIR );
    if( !root ) {
      log_e( "%s", DEBUG_DIROPEN_FAILED );
      return false;
    }
    if( !root.isDirectory() ) {
      log_e( "%s", DEBUG_NOTADIR );
      return false;
    }
    File file = root.openNextFile();
    if( !file ) {
      // empty root
      root.close();
      log_w( DEBUG_EMPTY_FS );
      //buildRootMenu();
      return false;
    }

    size_t files_count = 0;

    while( file ) {
      files_count++;
      String fPath = String( fs_file_path(&file ) );
      if( fPath == MENU_BIN || fPath == SDU_APP_PATH ) {
        file = root.openNextFile();
        continue; // ignore self and menu.bin
      }

      if( isValidAppName( fPath.c_str() ) ) {

        fPath = gnu_basename( fPath );
        fPath = fPath.substring( 0, fPath.length()-4 ); // remove extension

        files.push_back( fPath );
        log_v("Found app %s", fPath.c_str() );
      } else {
        log_v("Ignoring '%s' (not a valid appname)", fPath.c_str() );
      }
      file = root.openNextFile();
    }
    root.close();
    return true;
  }


  void removeInstalledApp( String appName )
  {
    log_d("Deleting App '%s' + its assets", appName.c_str() );
    // TODO: confirmation dialog
    trashFile( ROOT_DIR + appName + EXT_bin );
    trashFile( DIR_jpg + appName + EXT_jpg );
    trashFile( DIR_jpg + appName + "_gh" + EXT_jpg );
    // TODO: read json first and delete assets
    trashFile( DIR_json + appName + EXT_json );
  }


  bool isBinFile( const char* fileName )
  {
    String fName = String( fileName );
    fName.toLowerCase();
    return fName.endsWith( EXT_bin );
  }


  bool isLauncher( const char* binFileName )
  {
    String fName = String( binFileName );
    fName.toLowerCase();
    return fName.indexOf( "launcher" )>=0 || fName.indexOf( "menu" )>=0;
  }


  bool isJsonFile( const char* fileName )
  {
    String fName = String( fileName );
    fName.toLowerCase();
    return fName.endsWith( EXT_json );
  }


  bool isValidAppName( const char* fileName )
  {
    if( ( isBinFile( fileName ) ) // ignore files not ending in ".bin"
      && !String( fileName ).startsWith( "/." ) ) { // ignore dotfiles (thanks to https://twitter.com/micutil)
      return true;
    }
    return false;
  }


  bool iFile_exists( fs::FS *fs, String &fname )
  {
    if( fs->exists( fname.c_str() ) ) {
      return true;
    }
    String locasename = fname;
    String hicasename = fname;
    locasename.toLowerCase();
    hicasename.toUpperCase();
    if( fs->exists( locasename.c_str() ) ) {
      fname = locasename;
      return true;
    }
    if( fs->exists( hicasename.c_str() ) ) {
      fname = hicasename;
      return true;
    }
    return false;
  }


  void cleanDir( const char* dir )
  {
    String dirToOpen = String( dir );
    bool selfdeletable = false;

    if( Console ) Console->clear();

    // trim last slash if any, except for rootdir
    if( dirToOpen != ROOT_DIR ) {
      selfdeletable = true;
      if( dirToOpen.endsWith( PATH_SEPARATOR ) ) {
        dirToOpen = dirToOpen.substring(0, dirToOpen.length()-1);
      }
    }

    File root = M5_FS.open( dirToOpen );
    if(!root){
      log_e("%s",  DEBUG_DIROPEN_FAILED );
      return;
    }
    if(!root.isDirectory()){
      log_e("%s",  DEBUG_NOTADIR );
      return;
    }

    File file = root.openNextFile();
    while( file ) {
      const char* path = fs_file_path(&file);
      if(file.isDirectory()){
        // go recursive
        cleanDir( path );
        M5_FS.rmdir( path );
        Serial.printf( CLEANDIR_REMOVED, path );
        file = root.openNextFile();
        continue;
      }
      Serial.printf( CLEANDIR_REMOVED, path );
      if( Console ) Console->log( path );
      M5_FS.remove( path );
      file = root.openNextFile();
    }
    root.close();
    if( selfdeletable ) {
      if( M5_FS.exists( dirToOpen ) ) {
        M5_FS.rmdir( dirToOpen );
        Serial.printf( CLEANDIR_REMOVED, dir );
      }
    }
    if( Console ) Console->clear();
  }



  bool copyFile( const char* src, const char* dst )
  {
    fs::File sourceFile = M5_FS.open(src);
    if( !sourceFile ) {
      log_e("Can't open source file %s, aborting", src);
      return false;
    }
    #if !defined FS_CAN_CREATE_PATH
      // WARNING: not creating traversing folders unless sdk version >= 2.0.0
      // TODO: mkdir -p dirname( dst )
      fs::File destFile = M5_FS.open(dst, FILE_WRITE);
    #else
      fs::File destFile = M5_FS.open(dst, FILE_WRITE, true);
    #endif
    if( !destFile ) {
      log_e("Can't open dest file %s, aborting", dst);
      sourceFile.close();
      return false;
    }
    while( sourceFile.available() ) destFile.write( sourceFile.read() );
    destFile.close();
    return true;
  }


  bool trashFile( String path )
  {
    if( !M5_FS.exists( path ) ) {
      log_e("Can't trash unexistent file %s", path.c_str() );
      return false;
    }

    String newPath = trashFolderPathStr + PATH_SEPARATOR + gnu_basename( path );

    if( M5_FS.exists( newPath ) ) M5_FS.remove( newPath );
    else if( !M5_FS.exists( trashFolderPathStr ) ) M5_FS.mkdir( trashFolderPathStr );

    if( ! M5_FS.rename( path.c_str(), newPath.c_str() ) ) {
      log_e("Can't trash '%s', renaming to '%s' failed :(", path.c_str(), newPath.c_str() );
      return false;
    }
    return true;
  }




  bool getJson( const char* path, JsonObject &root, DynamicJsonDocument &jsonBuffer )
  {
    if( jsonBuffer.capacity() == 0 ) {
      log_e("JSON Buffer allocation failed" );
      return false;
    }
    if( ! M5_FS.exists( path ) ) {
      log_v("JSON File %s does not exist", path );
      return false;
    }
    fs::File file = M5_FS.open( path );
    if( !file ) {
      log_e("JSON File %s can't be opened");
      return false;
    }
    log_v("Opened JSON File %s for reading (%d bytes)", path, file.size() );
    DeserializationError error = deserializeJson( jsonBuffer, file );
    if (error) {
      log_e("JSON deserialization error #%d in %s", error, path );
      file.close();
      return false;
    }
    root = jsonBuffer.as<JsonObject>();
    file.close();
    return true;
  }


  void getHiddenApps()
  {
    //String jsonFile = "/.hidden-apps.json";
    if( !M5_FS.exists( HIDDEN_APPS_FILE ) ) return;
    HiddenFiles.clear();
    JsonObject root;
    DynamicJsonDocument jsonBuffer( 8192 );
    if( !getJson( HIDDEN_APPS_FILE , root, jsonBuffer ) ) {
      log_v("No hidden apps (file %s not created)", HIDDEN_APPS_FILE  );
      return;
    }
    if ( root.isNull() ) {
      log_e("No parsable JSON in %s file", HIDDEN_APPS_FILE  );
      return;
    }
    if( root["apps"].size() <= 0 ) {
      log_e("No apps in catalog");
      return;
    }
    for( int i=0; i<root["apps"].size(); i++ ) {
      HiddenFiles.push_back( root["apps"][i].as<String>() );
    }
    std::sort( HiddenFiles.begin(), HiddenFiles.end() );
  }


  void toggleHiddenApp( String appName, bool add )
  {
    getHiddenApps();

    if( add ) {
      if ( std::find(HiddenFiles.begin(), HiddenFiles.end(), appName) != HiddenFiles.end() ) {
        log_w("App %s is already hidden!", appName.c_str() );
        return;
      }
      HiddenFiles.push_back( appName );
    }

    std::sort( HiddenFiles.begin(), HiddenFiles.end() );

    DynamicJsonDocument doc(8192);
    if( doc.capacity() == 0 ) {
      log_e("ArduinoJSON failed to allocate 2kb");
      return;
    }

    // Open file for writing
    #if defined FS_CAN_CREATE_PATH
      File file = M5_FS.open( HIDDEN_APPS_FILE , FILE_WRITE, true );
    #else
      File file = M5_FS.open( HIDDEN_APPS_FILE , FILE_WRITE );
    #endif
    if (!file) {
      log_e("Failed to open file %s", HIDDEN_APPS_FILE );
      return;
    }

    JsonObject root = doc.to<JsonObject>();
    JsonArray array = root.createNestedArray("apps");

    for( int i=0; i<HiddenFiles.size(); i++ ) {
      if( HiddenFiles[i] != appName ) {
        array.add( HiddenFiles[i] );
      }
    }
    if( add ) {
      array.add( appName );
    }

    if( array.size() > 0 ) {
      log_i("Created json:");
      serializeJsonPretty(doc, Serial);
      if (serializeJson(doc, file) == 0) {
        log_e( "Failed to write to file %s", HIDDEN_APPS_FILE  );
      } else {
        log_i ("Successfully created %s", HIDDEN_APPS_FILE  );
      }
      file.close();
    } else {
      HiddenFiles.clear();
      file.close();
      M5_FS.remove( HIDDEN_APPS_FILE );
    }

  }


  bool isHiddenApp( String appName )
  {
    if( HiddenFiles.size() == 0 ) getHiddenApps();
    if( HiddenFiles.size() == 0 ) return false;
    return std::find( HiddenFiles.begin(), HiddenFiles.end(), appName) != HiddenFiles.end();
  }


  #if !defined FS_CAN_CREATE_PATH
    void scanDataFolder()
    {
      // check if mandatory folders exists and create if necessary
      if( !M5_FS.exists( appRegistryFolder ) ) {
        M5_FS.mkdir( appRegistryFolder );
      }
      for( uint8_t i=0; i<extensionsCount; i++ ) {
        String dir = ROOT_DIR + allowedExtensions[i];
        if( !M5_FS.exists( dir ) ) {
          M5_FS.mkdir( dir );
        }
      }
    }
  #endif

};



