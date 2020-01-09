/*
 *
 * M5Stack SD Menu
 * Project Page: https://github.com/tobozo/M5Stack-SD-Updater
 *
 * Copyright 2019 tobozo http://github.com/tobozo
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


extern SDUpdater sdUpdater; // used for menu progress


/*
 *
 * /!\ When set to true, files with those extensions
 * will be transferred to the SD Card if found on SPIFFS.
 * Directory is automatically created.
 *
 */
bool migrateSPIFFS = false;

const uint8_t extensionsCount = 6; // change this if you add / remove an extension
String allowedExtensions[extensionsCount] = {
    // do NOT remove jpg and json or the menu will crash !!!
    "jpg", "bmp", "json", "mod", "mp3", "cert"
};

const String appDataFolder = "/data"; // if an app needs spiffs data, it's stored here
const String launcherSignature = "Launcher.bin"; // app with name ending like this can overwrite menu.bin

const String appRegistryFolder = "/.registry";
const String appRegistryDefaultName = "default.json";


bool isBinFile( const char* fileName ) {
  return String( fileName ).endsWith( ".bin" ) || String( fileName ).endsWith( ".BIN" );
}


bool isValidAppName( const char* fileName ) {
  if(   String( fileName )!=MENU_BIN // ignore menu
     && ( isBinFile( fileName ) ) // ignore files not ending in ".bin"
     && !String( fileName ).startsWith( "/." ) ) { // ignore dotfiles (thanks to https://twitter.com/micutil)
    return true;
  }
  return false;
}


bool iFile_exists( fs::FS &fs, String &fname ) {
  if( fs.exists( fname.c_str() ) ) {
    return true;
  }
  String locasename = fname;
  String hicasename = fname;
  locasename.toLowerCase();
  hicasename.toUpperCase();
  if( fs.exists( locasename.c_str() ) ) {
    fname = locasename;
    return true;
  }
  if( fs.exists( hicasename.c_str() ) ) {
    fname = hicasename;
    return true;
  }
  return false;
}


/* Storing json meta file information r */
struct JSONMeta {
  int width; // app image width
  int height; // app image height
  String authorName = "";
  String projectURL = "";
  String credits = ""; // scroll this ?
  // TODO: add more interesting properties
};

/* filenames cache structure */
struct FileInfo {
  String fileName;  // path to the binary file
  String metaName;  // a json file with all meta info on the binary
  String iconName;  // a jpeg image representing the binary
  String faceName;  // a jpeg image representing the author
  uint32_t fileSize;
  String displayName() {
    String out = fileName.substring(1);
    out.replace( ".bin", "" );
    out.replace( ".BIN", "" );; // the binary name, without the file extension and the leading slash
    return out;
  }
  String shortName() {
    String out = displayName();
    if( out.startsWith("--") ) {
      out.replace("--", "");
    }
    return out;
  }
  bool hasIcon() {
    String currentIconFile = shortName();
    currentIconFile = "/jpg/" + shortName() + ".jpg";
    if( iFile_exists( M5_FS, currentIconFile ) ) {
      iconName = currentIconFile;
      return true;
    }
    log_w("[JSON]: no currentIconFile %s", currentIconFile.c_str() );
    return false;
  }
  bool hasFace() {
    String currentIconFile = shortName();
    currentIconFile = "/jpg/" + shortName() + "_gh.jpg";
    if( iFile_exists( M5_FS, currentIconFile ) ) {
      faceName = currentIconFile;
      return true;
    }
    if( hasIcon() ) {
      faceName = iconName;
      return true;
    }
    log_w("[JSON]: no currentIconFile %s", currentIconFile.c_str() );
    return false;
  }
  bool hasMeta() {
    String currentMetaFile = shortName();
    if( currentMetaFile.startsWith("/--") ) {
      currentMetaFile.replace("--", "");
    }
    currentMetaFile = "/json/" + shortName() + ".json";
    if( iFile_exists( M5_FS, currentMetaFile ) ) {
      metaName = currentMetaFile;
      return true;
    }
    log_w("[JSON]: no currentMetaFile %s", currentMetaFile.c_str() );
    return false;
  }
  bool hasData = false; // app requires a spiffs /data folder
  JSONMeta jsonMeta;
};


void getMeta( fs::FS &fs, String metaFileName, JSONMeta &jsonMeta ) {
  File file = fs.open( metaFileName );
  StaticJsonDocument<512> jsonMetaBuffer;
  DeserializationError error = deserializeJson( jsonMetaBuffer, file );
  if (error) return;
  JsonObject root = jsonMetaBuffer.as<JsonObject>();
  if ( !root.isNull() ) {
    jsonMeta.width  = root["width"];
    jsonMeta.height = root["height"];
    jsonMeta.authorName = root["authorName"].as<String>();
    jsonMeta.projectURL = root["projectURL"].as<String>();
    jsonMeta.credits    = root["credits"].as<String>();
  }
}


void getFileInfo( FileInfo &fileInfo, fs::FS &fs, File &file, const char* binext=".bin" ) {
  String BINEXT = binext;
  BINEXT.toUpperCase();
  String fileName   = file.name();
  uint32_t fileSize = file.size();
  time_t lastWrite = file.getLastWrite();
  struct tm * tmstruct = localtime(&lastWrite);
  char fileDate[64] = "1980-01-01 00:07:20";
  sprintf(fileDate, "%04d-%02d-%02d %02d:%02d:%02d",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
  if( (tmstruct->tm_year)+1900 < 2000 ) {
    // time is not set
  }
  Serial.println( "[" + String(fileDate) + "]" + String( DEBUG_FILELABEL ) + fileName );
  fileInfo.fileName = fileName;

  fileInfo.fileSize = fileSize;
  if( fileName.startsWith("/--") ) {
    fileName.replace("--", "");
  }

  if( fileInfo.hasIcon() && !fileInfo.hasFace() ) {
    fileInfo.faceName = fileInfo.iconName;
  }

  String currentDataFolder = appDataFolder + fileName;
  currentDataFolder.replace( binext, "" );
  currentDataFolder.replace( BINEXT, "" );
  if( fs.exists( currentDataFolder.c_str() ) ) {
    fileInfo.hasData = true; // TODO: actually use this feature
  }

  if( fileInfo.hasMeta() == true ) {
    getMeta( fs, fileInfo.metaName, fileInfo.jsonMeta );
  }
}


/*
 *  Scan SPIFFS for binaries and move them onto the SD Card
 *  TODO: create an app manager for the SD Card
 */
void scanDataFolder() {
  // check if mandatory folders exists and create if necessary
  if( !M5_FS.exists( appDataFolder ) ) {
    M5_FS.mkdir( appDataFolder );
  }
  if( !M5_FS.exists( appRegistryFolder ) ) {
    M5_FS.mkdir( appRegistryFolder );
  }
  for( uint8_t i=0; i<extensionsCount; i++ ) {
    String dir = "/" + allowedExtensions[i];
    if( !M5_FS.exists( dir ) ) {
      M5_FS.mkdir( dir );
    }
  }
  if( !migrateSPIFFS ) {
    return;
  }
  log_i( "%s", DEBUG_SPIFFS_SCAN );
  if( !SPIFFS.begin() ){
    log_e( "%s", DEBUG_SPIFFS_MOUNTFAILED );
  } else {
    File root = SPIFFS.open( "/" );
    if( !root ){
      log_e( "%s", DEBUG_DIROPEN_FAILED );
    } else {
      if( !root.isDirectory() ){
        log_i( "%s", DEBUG_NOTADIR );
      } else {
        File file = root.openNextFile();
        log_i( "%s", file.name() );
        String fileName = file.name();
        String destName = "";
        if( isBinFile( fileName.c_str() ) ) {
          destName = fileName;
        }
        // move allowed file types to their own folders
        for( uint8_t i=0; i<extensionsCount; i++)  {
          String ext = "." + allowedExtensions[i];
          if( fileName.endsWith( ext ) ) {
            destName = "/" + allowedExtensions[i] + fileName;
          }
        }
        if( destName!="" ) {
          sdUpdater.displayUpdateUI( String( MOVINGFILE_MESSAGE ) + fileName );
          size_t fileSize = file.size();
          File destFile = M5_FS.open( destName, FILE_WRITE );
          if( !destFile ){
            log_e( "%s", DEBUG_SPIFFS_WRITEFAILED) ;
          } else {
            static uint8_t buf[512];
            size_t packets = 0;
            log_i( "%s%s", DEBUG_FILECOPY, fileName.c_str() );

            while( file.read( buf, 512) ) {
              destFile.write( buf, 512 );
              packets++;
              sdUpdater.SDMenuProgress( (packets*512)-511, fileSize );
            }
            destFile.close();
            Serial.println();
            log_i( "%s", DEBUG_FILECOPY_DONE );
            SPIFFS.remove( fileName );
            log_i( "%s", DEBUG_WILL_RESTART );
            delay( 500 );
            ESP.restart();
          }
        } else {
          log_i( "%s", DEBUG_NOTHING_TODO );
        } // aa
      } // aaaaa
    } // aaaaaaaaa
  } // aaaaaaaaaaaaah!
} // nooooooooooooooes!!


bool replaceItem( fs::FS &fs, String SourceName, String  DestName) {
  if( !fs.exists( SourceName ) ) {
    log_e("Source file %s does not exists !\n", SourceName.c_str() );
    return false;
  }
  fs.remove( DestName );
  fs::File source = fs.open( SourceName );
  if( !source ) {
    log_e("Failed to open source file %s\n", SourceName.c_str() );
    return false;
  }
  fs::File dest = fs.open( DestName, FILE_WRITE );
  if( !dest ) {
    log_e("Failed to open dest file %s\n", DestName.c_str() );
    return false;
  }
  uint8_t buf[4096]; // 4K buffer should be enough to fast-copy the file
  uint8_t dot = 0;
  size_t fileSize = source.size();
  size_t n;
  while ((n = source.read(buf, sizeof(buf))) > 0) {
    Serial.print(".");
    if(dot++%64==0) {
      Serial.println();
      sdUpdater.SDMenuProgress( (dot*4096)-4095, fileSize );
    }
    dest.write(buf, n);
  }
  dest.close();
  source.close();
  return true;
}


bool replaceLauncher( fs::FS &fs, FileInfo &info) {
  if(!replaceItem( fs, info.fileName, String(MENU_BIN) ) ) {
    return false;
  }
  if( info.hasIcon() ) {
    replaceItem( fs, info.iconName, "/jpg/" MENU_BIN ".jpg" );
  }
  if( info.hasFace() ) {
    replaceItem( fs, info.faceName, "/jpg/" MENU_BIN "_gh.jpg" );
  }
  if( info.hasMeta() ) {
    replaceItem( fs, info.metaName, "/json/" MENU_BIN ".json" );
  }
  return true;
}


FileInfo fileInfo[M5SAM_LIST_MAX_COUNT];
