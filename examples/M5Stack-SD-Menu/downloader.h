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

#define MBEDTLS_ERROR_C
#include "certificates.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "mbedtls/md.h"

// registry this launcher is tied to
#include "registry.default.h"

long timezone = 0; // UTC
byte daysavetime = 1; // UTC + 1

HTTPClient http;

extern SDUpdater sdUpdater; // used for menu progress
extern M5SAM M5Menu;
extern uint16_t appsCount;
extern uint16_t MenuID;

#define DOWNLOADER_BIN "/--Downloader--.bin" // Fixme/Hack: a dummy file will be created so it appears in the menu as an app
#define DOWNLOADER_BIN_VIRTUAL "/Downloader.bin" // old bin name, will be renamed, kept for backwards compat
#define SD_CERT_PATH      "/cert/" // Filesystem (SD) temporary path where certificates are stored, needs a trailing slash !!
String UserAgent;

bool wifisetup = false;
bool ntpsetup  = false;
bool done      = false;
uint8_t progress = 0;
float progress_modulo = 0;

const String appRegistryFolder = "/.registry";
const String appRegistryDefaultName = "default.json";

const char* AppEndpointURLTpl = "/%s.json";
String SDUpdaterChannelNameStr    = "";
char AppEndpointURLStr[32];


mbedtls_md_context_t ctx;
mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
byte shaResult[32];
static String shaResultStr = "f7ff9bcd52fee13ae7ebd6b4e3650a4d9d16f8f23cab370d5cdea291e5b6bba6"; // cheap malloc: any string is good as long as it's 64 chars

int tlserrors = 0;
int jsonerrors = 0;
int downloadererrors = 0;
int updatedfiles = 0;
int newfiles = 0;
int checkedfiles = 0;

typedef struct {
  const char* host;
  const char* ca;
} TLSCert;

const char* nullHost;
const char* nullCa;

//TLSCert GithubCert = { "github.com", github_ca };
//TLSCert PHPSecureCert = { "phpsecu.re", phpsecu_re_ca };
TLSCert NULLCert = { nullHost, nullCa };

TLSCert TLSWallet[8] = {NULLCert, NULLCert, NULLCert, NULLCert, NULLCert, NULLCert, NULLCert, NULLCert };

bool wget( String bin_url, String appName );
int modalConfirm( String question, String title, String body, const char* labelA, const char* labelB, const char* labelC );
bool wifiSetupWorked();


struct URLParts {
  String url;
  String protocol;
  String host;
  String port;
  String auth;
  String uri;
};

URLParts parseURL( String url ) { // logic stolen from HTTPClient::beginInternal()
  URLParts urlParts;
  int index = url.indexOf(':');
  if(index < 0) {
    log_e("failed to parse protocol");
    return urlParts;
  }
  urlParts.url = String(url);
  urlParts.protocol = url.substring(0, index);
  url.remove(0, (index + 3)); // remove http:// or https://
  index = url.indexOf('/');
  String host = url.substring(0, index);
  url.remove(0, index); // remove host part
  index = host.indexOf('@'); // get Authorization
  if(index >= 0) { // auth info
    urlParts.auth = host.substring(0, index);
    host.remove(0, index + 1); // remove auth part including @
  }
  index = host.indexOf(':'); // get port
  if(index >= 0) {
    urlParts.host = host.substring(0, index); // hostname
    host.remove(0, (index + 1)); // remove hostname + :
    urlParts.port = host.toInt(); // get port
  } else {
    urlParts.host = host;
  }
  urlParts.uri = url;
  return urlParts;
}




typedef struct {
  String name;
  String description;
  String url;
  String api_host;
  String api_path;
  String api_cert_path;
  String updater_path;
  String catalog_endpoint;

  String api_cert_provider_url_http;
  String api_url_https;
  String api_url_http;

  void init() {
    api_cert_provider_url_http = "http://" + api_host + api_path + api_cert_path;
    api_url_https              = "https://" + api_host + api_path + updater_path;
    api_url_http               = "https://" + api_host + api_path + updater_path;
  }
  void print() {
    Serial.printf("\n\tname: %s\n\tdescription: %s\n\turl: %s\n\tapi_host: %s\n\tapi_path: %s\n\tapi_cert_path: %s\n\tupdater_path: %s\n\tcatalog_endpoint: %s\n\tapi_cert_provider_url_http: %s\n\tapi_url_https: %s\n\tapi_url_http: %s\n\n",
      name.c_str(),
      description.c_str(),
      url.c_str(),
      api_host.c_str(),
      api_path.c_str(),
      api_cert_path.c_str(),
      updater_path.c_str(),
      catalog_endpoint.c_str(),
      api_cert_provider_url_http.c_str(),
      api_url_https.c_str(),
      api_url_http.c_str()
    );
  }
} AppRegistryItem;

typedef struct {
  String name;
  String description;
  String url;
  String pref_default_channel; // local option for SDUpdater use only
  AppRegistryItem masterChannel;
  AppRegistryItem unstableChannel;
  AppRegistryItem defaultChannel;
  void init() {
    masterChannel.init();
    unstableChannel.init();
    if( pref_default_channel == "master" ) {
      defaultChannel = masterChannel;
    } else {
      defaultChannel = unstableChannel;
    }
    print();
  }
  void print() {
    Serial.println("Registry infos:");
    Serial.printf("\n\tname: %s\n\tdescription: %s\n\turl: %s\n\tpref_default_channel: %s\n\n",
      name.c_str(),
      description.c_str(),
      url.c_str(),
      pref_default_channel.c_str()
    );
    Serial.println("Master channel infos:");
    masterChannel.print();
    Serial.println("Unstable channel infos:");
    unstableChannel.print();
    Serial.println("Default channel infos:");
    defaultChannel.print();
  }
} AppRegistry;


AppRegistryItem defaultMasterChannel = {
  "master", // name
  DEFAULT_MASTER_DESC,
  DEFAULT_MASTER_URL,
  DEFAULT_MASTER_API_HOST,
  DEFAULT_MASTER_API_PATH,
  DEFAULT_MASTER_API_CERT_PATH,
  DEFAULT_MASTER_UPDATER_PATH,
  DEFAULT_MASTER_CATALOG_ENDPOINT
};

AppRegistryItem defaultUnstableChannel = {
  "unstable", // name
  DEFAULT_UNSTABLE_DESC,
  DEFAULT_UNSTABLE_URL,
  DEFAULT_UNSTABLE_API_HOST,
  DEFAULT_UNSTABLE_API_PATH,
  DEFAULT_UNSTABLE_API_CERT_PATH,
  DEFAULT_UNSTABLE_UPDATER_PATH,
  DEFAULT_UNSTABLE_CATALOG_ENDPOINT
};

AppRegistry defaultAppRegistry = {
  DEFAULT_REGISTRY_NAME,
  DEFAULT_REGISTRY_DESC,
  DEFAULT_REGISTRY_URL, // should exist as "default.json" on SD Card
  DEFAULT_REGISTRY_CHANNEL,
  defaultMasterChannel,
  defaultUnstableChannel
};

extern AppRegistry Registry;


void registrySave( AppRegistry registry, String appRegistryLocalFile = "" ) {
  URLParts urlParts = parseURL( registry.url );
  if( appRegistryLocalFile == "" ) {
    Serial.println("Will attempt to create/save " + appRegistryLocalFile );
    appRegistryLocalFile = String( appRegistryFolder + "/" + urlParts.host + ".json" );
  }
  if( M5_FS.exists( appRegistryLocalFile ) ) {
    Serial.println("Removing " + appRegistryLocalFile + " before writing");
    M5_FS.remove( appRegistryLocalFile );
  }
  // Open file for writing
  File file = M5_FS.open( appRegistryLocalFile, FILE_WRITE );
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  DynamicJsonDocument jsonBuffer(2048);

  JsonObject channels            = jsonBuffer.createNestedObject("channels");
  JsonObject masterChannelJson   = channels.createNestedObject("master");
  JsonObject unstableChannelJson = channels.createNestedObject("unstable");

  masterChannelJson["name"]         = registry.masterChannel.name;
  masterChannelJson["description"]  = registry.masterChannel.description;
  masterChannelJson["url"]          = registry.masterChannel.url;
  masterChannelJson["api_host"]     = registry.masterChannel.api_host;
  masterChannelJson["api_path"]     = registry.masterChannel.api_path;
  masterChannelJson["cert_path"]    = registry.masterChannel.api_cert_path;
  masterChannelJson["updater_path"] = registry.masterChannel.updater_path;
  masterChannelJson["endpoint"]     = registry.masterChannel.catalog_endpoint;

  unstableChannelJson["name"]         = registry.unstableChannel.name;
  unstableChannelJson["description"]  = registry.unstableChannel.description;
  unstableChannelJson["url"]          = registry.unstableChannel.url;
  unstableChannelJson["api_host"]     = registry.unstableChannel.api_host;
  unstableChannelJson["api_path"]     = registry.unstableChannel.api_path;
  unstableChannelJson["cert_path"]    = registry.unstableChannel.api_cert_path;
  unstableChannelJson["updater_path"] = registry.unstableChannel.updater_path;
  unstableChannelJson["endpoint"]     = registry.unstableChannel.catalog_endpoint;
  
  jsonBuffer["name"]                 = registry.name;
  jsonBuffer["description"]          = registry.description;
  jsonBuffer["url"]                  = registry.url;
  jsonBuffer["pref_default_channel"] = registry.pref_default_channel;

  Serial.println("Created json:");
  serializeJsonPretty(jsonBuffer, Serial);
  Serial.println();

  if (serializeJson(jsonBuffer, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  file.close();
  Serial.println("Successfully created " + appRegistryLocalFile);
}

void registryFetch( AppRegistry registry, String appRegistryLocalFile = "" ) {
  if( !wifiSetupWorked() ) {
    modalConfirm( DOWNLOADER_MODAL_CANCELED, "    No connexion available", MODAL_SAME_PLAYER_SHOOT_AGAIN, DOWNLOADER_MODAL_REBOOT, DOWNLOADER_MODAL_RESTART, DOWNLOADER_MODAL_WTF );
    ESP.restart();
  }
  URLParts urlParts = parseURL( registry.url );
  
  if( appRegistryLocalFile == "" ) {
    appRegistryLocalFile = appRegistryFolder + "/" + appRegistryDefaultName;
  } else {
    appRegistryLocalFile = appRegistryFolder + "/" + urlParts.host + ".json";
  }
  if( !wget( registry.url , appRegistryLocalFile ) ) {

    if( tlserrors > 0 ) {
      String certPath = String( SD_CERT_PATH ) + urlParts.host;
      String certURL = Registry.defaultChannel.api_cert_provider_url_http + urlParts.host;
      if( wget( certURL , certPath ) ) {
        modalConfirm( DOWNLOADER_MODAL_CANCELED, NEW_TLS_CERTIFICATE_INSTALLED, MODAL_RESTART_REQUIRED, DOWNLOADER_MODAL_REBOOT, DOWNLOADER_MODAL_RESTART, DOWNLOADER_MODAL_WTF );
        ESP.restart();
      }
    }

    modalConfirm( DOWNLOADER_MODAL_CANCELED, MODAL_REGISTRY_DAMAGED, MODAL_SAME_PLAYER_SHOOT_AGAIN, DOWNLOADER_MODAL_REBOOT, DOWNLOADER_MODAL_RESTART, DOWNLOADER_MODAL_WTF );
  } else {
    String appRegistryDefaultFile = appRegistryFolder + "/" + appRegistryDefaultName;
    File sourceFile = M5_FS.open( appRegistryLocalFile );
    if( M5_FS.exists( appRegistryDefaultFile ) ) {
      M5_FS.remove( appRegistryDefaultFile );
    }
    File destFile   = M5_FS.open( appRegistryDefaultFile, FILE_WRITE );
    static uint8_t buf[512];
    size_t packets = 0;
    Serial.println("Starting copy");
    while( (packets = sourceFile.read( buf, sizeof(buf))) > 0 ) {
      destFile.write( buf, packets );
      Serial.write( buf, packets );
    }
    Serial.println();
    destFile.close();
    sourceFile.close();
    modalConfirm( UPDATE_SUCCESS, MODAL_REGISTRY_UPDATED, MODAL_REBOOT_REGISTRY_UPDATED, DOWNLOADER_MODAL_REBOOT, DOWNLOADER_MODAL_RESTART, DOWNLOADER_MODAL_WTF );
  }
  ESP.restart();
}

AppRegistry registryInit( String appRegistryLocalFile = "" ) {
  if( appRegistryLocalFile == "" ) {
    //SDUpdaterChannelNameStr = String( DEFAULT_REGISTRY_CHANNEL );
    appRegistryLocalFile = appRegistryFolder + "/" + appRegistryDefaultName;
  } else {
    //SDUpdaterChannelNameStr = "master";
  }
  Serial.println("Opening channel file: " + appRegistryLocalFile);
  
  if( !M5_FS.exists( appRegistryLocalFile ) ) {
    // create file
    Serial.println("Registry file " + appRegistryLocalFile + " does not exist, creating from firmware defaults");
    registrySave( defaultAppRegistry, appRegistryFolder + "/" + appRegistryDefaultName );
    return defaultAppRegistry;
  }
  // load from file

  File file = M5_FS.open( appRegistryLocalFile );

  AppRegistryItem masterChannel;
  AppRegistryItem unstableChannel;

  StaticJsonDocument<2048> jsonBuffer;
  DeserializationError error = deserializeJson( jsonBuffer, file );
  if (error) {
    Serial.println("JSON Error while reading registry file " + appRegistryLocalFile );
    defaultAppRegistry.init();
    return defaultAppRegistry;
  }
  JsonObject root = jsonBuffer.as<JsonObject>();
  if ( root.isNull() ) {
    Serial.println("Registry file " + appRegistryLocalFile + " has empty JSON");
    defaultAppRegistry.init();
    return defaultAppRegistry;
  } else  {
    if( root["channels"]["master"]["name"].as<String>() ==""
     || root["channels"]["master"]["description"].as<String>() ==""
     || root["channels"]["master"]["url"].as<String>() ==""
     || root["channels"]["master"]["api_host"].as<String>() ==""
     || root["channels"]["master"]["api_path"].as<String>() ==""
     || root["channels"]["master"]["cert_path"].as<String>() ==""
     || root["channels"]["master"]["updater_path"].as<String>() ==""
     || root["channels"]["master"]["endpoint"].as<String>() =="" ) {
     // bad master item
     Serial.println("Bad master channel in JSON file");
     defaultAppRegistry.init();
     return defaultAppRegistry;
    } else {
      masterChannel = {
        "master",
        root["channels"]["master"]["description"].as<String>(),
        root["channels"]["master"]["url"].as<String>(),
        root["channels"]["master"]["api_host"].as<String>(),
        root["channels"]["master"]["api_path"].as<String>(),
        root["channels"]["master"]["cert_path"].as<String>(),
        root["channels"]["master"]["updater_path"].as<String>(),
        root["channels"]["master"]["endpoint"].as<String>()
      };
    }

    if( root["channels"]["unstable"]["name"].as<String>() ==""
     || root["channels"]["unstable"]["description"].as<String>() ==""
     || root["channels"]["unstable"]["url"].as<String>() ==""
     || root["channels"]["unstable"]["api_host"].as<String>() ==""
     || root["channels"]["unstable"]["api_path"].as<String>() ==""
     || root["channels"]["unstable"]["cert_path"].as<String>() ==""
     || root["channels"]["unstable"]["updater_path"].as<String>() ==""
     || root["channels"]["unstable"]["endpoint"].as<String>() =="" ) {
     // bad master item
     Serial.println("Bad unstable channel in JSON file");
     defaultAppRegistry.init();
     return defaultAppRegistry;
    } else {
      unstableChannel = {
        "unstable",
        root["channels"]["unstable"]["description"].as<String>(),
        root["channels"]["unstable"]["url"].as<String>(),
        root["channels"]["unstable"]["api_host"].as<String>(),
        root["channels"]["unstable"]["api_path"].as<String>(),
        root["channels"]["unstable"]["cert_path"].as<String>(),
        root["channels"]["unstable"]["updater_path"].as<String>(),
        root["channels"]["unstable"]["endpoint"].as<String>()
      };
    }

    if( root["name"].as<String>() == ""
     || root["description"].as<String>() == ""
     || root["url"].as<String>() == "" ) {
      Serial.println("Bad channel meta in JSON file");
      defaultAppRegistry.init();
      return defaultAppRegistry;
    } else {
      if( !root["pref_default_channel"].isNull() && root["pref_default_channel"].as<String>() != "" ) {
        // inherit from json
        SDUpdaterChannelNameStr = root["pref_default_channel"].as<String>();
      } else {
        // assign default
        SDUpdaterChannelNameStr = "master";
      }
      AppRegistry appRegistry = {
        root["name"].as<String>(),
        root["description"].as<String>(),
        root["url"].as<String>(),
        SDUpdaterChannelNameStr, // default channel
        masterChannel,
        unstableChannel
      };
      appRegistry.init();
      return appRegistry;
    }
  }

};




int modalConfirm( String question, String title, String body, const char* labelA=DOWNLOADER_MODAL_YES, const char* labelB=DOWNLOADER_MODAL_NO, const char* labelC=DOWNLOADER_MODAL_CANCEL ) {
  bool response = false;
  tft.clear();
  M5Menu.drawAppMenu( question, labelA, labelB, labelC);
  tft.setTextSize( 1 );
  tft.setTextColor( WHITE );
  tft.drawCentreString( title, 160, 50, 1 );
  tft.setCursor( 0, 72 );
  tft.print( body );
  
  tft.drawJpg(caution_jpg, caution_jpg_len, 224, 136, 64, 46 );
  HIDSignal hidState = HID_INERT;

  while( hidState==HID_INERT ) {
    delay( 100 );
    M5.update();
    hidState = getControls();
  }
  return hidState;
}


void printProgress(uint16_t progress) {
  uint16_t x = tft.getCursorX();
  uint16_t y = tft.getCursorX();
  tft.setCursor(10, 194);
  tft.setTextColor(WHITE, tft.color565(128,128,128));
  tft.print( OVERALL_PROGRESS_TITLE );
  tft.print(String(progress));
  tft.print("%");
  tft.setCursor(x, y);
}

void renderDownloadIcon(uint16_t color=GREEN, int16_t x=272, int16_t y=7, float size=2.0 ) {
  float halfsize = size/2;
  tft.fillTriangle(x,      y+2*size,   x+4*size, y+2*size,   x+2*size, y+5*size, color);
  tft.fillTriangle(x+size, y,          x+3*size, y,          x+2*size, y+5*size, color);
  tft.fillRect( x, -halfsize+y+6*size, 1+4*size, size, color);
}

void drawRSSIBar(int16_t x, int16_t y, int16_t rssi, uint16_t bgcolor, float size=1.0) {
  uint16_t barColors[4] = { bgcolor, bgcolor, bgcolor, bgcolor };
  switch(rssi%6) {
   case 5:
      barColors[0] = GREEN;
      barColors[1] = GREEN;
      barColors[2] = GREEN;
      barColors[3] = GREEN;
    break;
    case 4:  
      barColors[0] = GREEN;
      barColors[1] = GREEN;
      barColors[2] = GREEN;
    break;
    case 3:  
      barColors[0] = YELLOW;
      barColors[1] = YELLOW;
      barColors[2] = YELLOW;
    break;
    case 2:  
      barColors[0] = YELLOW;
      barColors[1] = YELLOW;
    break;
    case 1:  
      barColors[0] = RED;
    break;
    default:
    case 0:  
      barColors[0] = RED; // want: RAINBOW
    break;
  }
  tft.fillRect(x,          y + 4*size, 2*size, 4*size, barColors[0]);
  tft.fillRect(x + 3*size, y + 3*size, 2*size, 5*size, barColors[1]);
  tft.fillRect(x + 6*size, y + 2*size, 2*size, 6*size, barColors[2]);
  tft.fillRect(x + 9*size, y + 1*size, 2*size, 7*size, barColors[3]);
}


void drawSDUpdaterChannel() {
  tft.setTextColor(TFT_WHITE, tft.color565(0,0,128) );
  tft.drawJpg(bluefork_jpg, bluefork_jpg_len, 2, 8 );
  tft.drawString( Registry.defaultChannel.name, 16, 11 );
  tft.setTextColor(TFT_WHITE, tft.color565(128,128,128) );
}


void drawAppMenu() {
  M5Menu.windowClr();
  M5Menu.drawAppMenu( APP_DOWNLOADER_MENUTITLE, "", "", "");
  drawSDUpdaterChannel();
  if( wifisetup ) {
    drawRSSIBar( 290, 4, 5, tft.color565(0,0,128), 2.0 );
  }
  if( ntpsetup ) {
    // TODO: draw something
  }
}


void cleanDir( const char* dir) {
  tft.clear();
  tft.setCursor(0,0);
  tft.setTextColor(WHITE, BLACK);

  String dirToOpen = String( dir );

  if( dirToOpen.endsWith("/" ) ) {
    dirToOpen = dirToOpen.substring(0, dirToOpen.length()-1);   
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
  while(file){
    if(file.isDirectory()){
      //Serial.print("DEBUG_DIRLABEL");
      //Serial.println(file.name());
    } else {
      const char* fileName = file.name();
      file.close();
      M5_FS.remove( fileName );
      if( tft.getCursorY() > tft.height() ) {
        tft.clear();
        tft.setCursor(0,0);
      }
      Serial.printf( CLEANDIR_REMOVED, fileName );
      tft.printf( CLEANDIR_REMOVED, fileName );
    }
    file = root.openNextFile();
  }
  tft.clear();
  drawAppMenu();
}


/* this is to avoid using GOTO statements */
typedef struct {
  bool deleteclient = true;
  bool endhttp = false;
  bool dismiss( WiFiClientSecure *client, bool error = false ) {
    if( error ) {
      renderDownloadIcon( RED );
      downloadererrors++;
    }
    if( endhttp ) {
      log_d("[HEAP before http.end(): %d]", ESP.getFreeHeap() );
      if( http.connected() ) {
        http.end();
      }
      endhttp = false;
      log_d("[HEAP after http.end(): %d]", ESP.getFreeHeap() );
    }
    if( deleteclient ) {
      log_d("[Deleting client: %d]", ESP.getFreeHeap() );
      delete client;
      deleteclient = false;
      log_d("[Deleted client: %d]", ESP.getFreeHeap() );
    }
    return !error;
  }
} HTTPRouter;


void sha_sum_to_str() {
  shaResultStr = "";
  for(int i= 0; i< sizeof(shaResult); i++) {
    char str[3];
    sprintf(str, "%02x", (int)shaResult[i]);
    shaResultStr += String( str );
  }
}


void sha256_sum(fs::FS &fs, const char* fileName) {
  log_d("SHA256: checking file %s\n", fileName);
  if(! fs.exists( fileName ) ) {
    downloadererrors++;
    log_e("  [ERROR] File %s does not exist, aborting\n", fileName);
    return;
  }
  File checkFile = fs.open( fileName );
  size_t fileSize = checkFile.size();
  size_t len = fileSize;

  if( !checkFile || fileSize==0 ) {
    downloadererrors++;
    log_e("  [ERROR] Can't open %s file for reading, aborting\n", fileName);
    return;
  }
  tft.drawJpg( checksum_jpg, checksum_jpg_len, 10, 125, 22, 32 );
  uint8_t buff[4096] = { 0 };
  size_t sizeOfBuff = sizeof(buff);
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  size_t n;
  while ((n = checkFile.read(buff, sizeOfBuff)) > 0) {
    mbedtls_md_update(&ctx, (const unsigned char *) buff, n);
    if( fileSize/10 > sizeOfBuff && fileSize != len ) {
      sdUpdater.M5SDMenuProgress(fileSize-len, fileSize);
    }
    len -= n;
  }
  tft.fillRect( 10, 125, 22, 32, tft.color565(128,128,128) );
  checkFile.close();
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);
  sha_sum_to_str();
}



const char* updateWallet( String host, const char* ca) {
  int8_t idx = -1;
  uint8_t sizeOfWallet = sizeof( TLSWallet ) / sizeof( TLSWallet[0] );
  for(uint8_t i=0; i<sizeOfWallet; i++) {
    if( TLSWallet[i].host==NULL ) {
      if( idx == -1 ) {
        idx = i;
      }
      continue;
    }
    if( String( TLSWallet[i].host ) == host ) {
      log_d("[WALLET SKIP UPDATE] Wallet #%d exists ( %s )\n", i, TLSWallet[i].host );
      return TLSWallet[i].ca;
    }
  }
  if( idx > -1 ) {
    int hostlen = host.length() + 1;
    int certlen = String(ca).length() + 1;
    char *newhost = (char*)malloc( hostlen );
    char *newcert = (char*)malloc( certlen );
    memcpy( newhost, host.c_str(), hostlen );
    memcpy( newcert, ca, certlen);
    TLSWallet[idx] = { (const char*)newhost , (const char*)newcert };
    log_d("[WALLET UPDATE] Wallet #%d created ( %s )\n", idx, TLSWallet[idx].host );
    return TLSWallet[idx].ca;
  }
  return ca;
}


const char* fetchLocalCert( String host ) {
  String certPath = String( SD_CERT_PATH ) + host;
  File certFile = M5_FS.open( certPath );
  if(! certFile ) { // failed to open the cert file
    log_d("Failed to open the cert file %s", certPath.c_str() );
    return NULL;
  }
  String certStr = "";
  while( certFile.available() ) {
    certStr += certFile.readStringUntil('\n') + "\n";
  }
  certFile.close();
  log_v("\n%s\n", certStr.c_str() );
  const char* certChar = updateWallet( host, certStr.c_str() );
  return certChar;
}


const char* fetchWalletCert( String host ) {
  uint8_t sizeOfWallet = sizeof( TLSWallet ) / sizeof( TLSWallet[0] );
  log_d("Checking wallet (%d items)\n",  sizeOfWallet );
  for(uint8_t i=0; i<sizeOfWallet; i++) {
    if( TLSWallet[i].host==NULL ) continue;
    log_d("Wallet #%d ( %s ) : ", i, TLSWallet[i].host );
    if( String( TLSWallet[i].host ) == host ) {
      log_d(" [OK]");
      return TLSWallet[i].ca;
    } else {
      log_d(" [KO]");
    }
  }
  const char* nullcert = NULL;
  return nullcert;
}


const char* fetchCert( String host, bool checkWallet = true, bool checkFS = true ) {
  const char* nullcert = NULL;
  if( checkWallet ) {
    const char* walletCert = fetchWalletCert( host );
    if( walletCert != NULL ) {
      log_d("[FETCHED WALLET CERT] -> ");
      return walletCert;
    } else {
      //
    }
  }
  String certPath = String( SD_CERT_PATH ) + host;
  String certURL = Registry.defaultChannel.api_cert_provider_url_http + host;
  if(  !checkFS || !M5_FS.exists( certPath ) ) {
    //log_d("[FETCHING REMOTE CERT] -> ");
    //wget(certURL , certPath );
    return fetchLocalCert( host );
  } else {
    log_d("[FETCHING LOCAL CERT] -> ");
  }
  return fetchLocalCert( host );
}



bool syncConnect(WiFiClientSecure *client, HTTPRouter &router, URLParts urlParts) {
  if(!client) {
    return router.dismiss( client, true );
  }
  
  http.setConnectTimeout( 10000 ); // 10s timeout = 10000
  if( String( urlParts.protocol ) == "https" ) {
    const char* certdata = fetchCert( urlParts.host );
    if( certdata == NULL ) {
      tlserrors++;
      log_e(" [ERROR] An HTTPS URL was called (%s) but no certificate was provided", urlParts.url.c_str() );
      router.endhttp = false;
      return router.dismiss( client, true );
    }
    client->setCACert( certdata );
    client->setTimeout( 10 ); // in seconds
    router.endhttp = true;
    if ( ! http.begin(*client, urlParts.url ) ) {
      tlserrors++;
      log_e(" [ERROR] HTTPS failed");
      router.endhttp = false;
      return router.dismiss( client, true );
    }
  } else {
    log_d(" [INFO] An HTTP (NO TLS) URL was called (%s)", urlParts.url.c_str() );
    router.endhttp = true;
    http.begin( urlParts.url );
  }

  int httpCode = http.GET();
  if(httpCode <= 0) {
    log_e("\n[ERROR] HTTP GET %s failed\n", urlParts.url.c_str());
    return router.dismiss( client, true );
  }
  if(httpCode != HTTP_CODE_OK) {
    Serial.printf("\n[ERROR %d] HTTP GET %s failed: %s\n", httpCode, urlParts.url.c_str(), http.errorToString(httpCode).c_str());
    return router.dismiss( client, true );
  }
  return true;
}



bool wget( String bin_url, String appName ) {
  log_d("[HEAP Before: %d", ESP.getFreeHeap() );
  Serial.printf("#> wget %s --output-document=%s ", bin_url.c_str(), appName.c_str());
  renderDownloadIcon( GREEN );
  WiFiClientSecure *client = new WiFiClientSecure;
  int httpCode;

  HTTPRouter wgetRouter;
  URLParts urlParts = parseURL( bin_url );

  if( ! syncConnect(client, wgetRouter, urlParts) ) {
    return false;
  }

  int len = http.getSize();
  if(len<=0) {
    log_e("  [ERROR] %s has zero Content-Lenght, aborting\n", bin_url.c_str());
    return wgetRouter.dismiss( client, true );
  }
  int httpSize = len;
  uint8_t buff[2048] = { 0 };
  size_t sizeOfBuff = sizeof(buff);
  if( M5_FS.exists(appName) ) {
    M5_FS.remove(appName);
  }
  File myFile = M5_FS.open(appName, FILE_WRITE);
  if(!myFile) {
    myFile.close();
    log_e("  [ERROR] Failed to open %s for writing, aborting\n", appName.c_str());
    return wgetRouter.dismiss( client, true );
  }
  unsigned long downwloadstart = millis();
  WiFiClient *stream = http.getStreamPtr();
  *shaResult = {0};
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  tft.drawJpg( checksum_jpg, checksum_jpg_len, 10, 125, 22, 32 );
  tft.drawJpg( download_jpg, download_jpg_len, 44, 125, 26, 32 );
  Serial.print("[Download+SHA256 Sum -->][..");
  while(http.connected() && (len > 0 || len == -1)) {
    if( httpSize/10 > sizeOfBuff && httpSize!=len ) {
      sdUpdater.M5SDMenuProgress(httpSize-len, httpSize);
    }
    // get available data size
    size_t size = stream->available();
    if(size) {
      // read up to 128 byte
      int c = stream->readBytes(buff, ((size > sizeOfBuff) ? sizeOfBuff : size));
      // hashsum
      mbedtls_md_update(&ctx, (const unsigned char *) buff, c);
      // write it to SD
      myFile.write(buff, c);
      if(len > 0) {
        len -= c;
      }
    }
    //delay(1);
  }
  myFile.close();
  delete stream;
  Serial.print("]");
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);
  sha_sum_to_str();
  tft.fillRect( 10, 125, 70, 32, tft.color565(128,128,128) );
  renderDownloadIcon( tft.color565(128,128,128) );
  unsigned long dl_duration;
  float bytespermillis;
  dl_duration = ( millis() - downwloadstart );
  if( dl_duration > 0 ) {
    bytespermillis = httpSize / dl_duration;
    Serial.printf("  [OK][Downloaded %d KB at %d KB/s]\n", (httpSize/1024), (int)( bytespermillis / 1024.0 * 1000.0 ) );
  } else {
    Serial.printf("  [OK] Copy done...\n");
  }
  wgetRouter.endhttp = false;
  wgetRouter.deleteclient = false;
  return wgetRouter.dismiss( client, false );
}


static void countDownReboot( void * param ) {
  unsigned long wait = 10000;
  unsigned long startCountDown = millis();
  while( startCountDown + wait > millis() ) {
    delay( 100 );
  }
  ESP.restart();
  vTaskDelete( NULL );
}


void syncFinished( bool restart=true ) {
  M5Menu.windowClr();
  tft.setCursor(10,60);
  tft.setTextColor(WHITE, tft.color565(128,128,128));
  tft.println( SYNC_FINISHED );
  Serial.printf("\n\n## Download Finished  ##\n   Errors: %d\n\n", downloadererrors );
  xTaskCreatePinnedToCore( countDownReboot, "countDownReboot", 2048, NULL, 5, NULL, 0 );
  char modalBody[256];
  sprintf( modalBody, DOWNLOADER_MODAL_BODY_ERRORS_OCCURED, downloadererrors, checkedfiles, updatedfiles, newfiles);
  modalConfirm( DOWNLOADER_MODAL_ENDED, DOWNLOADER_MODAL_TITLE_ERRORS_OCCURED, modalBody, DOWNLOADER_MODAL_REBOOT, DOWNLOADER_MODAL_RETRY, MENU_BTN_BACK );
  if( restart ) {
    ESP.restart();
    delay(1000);
  }
}

void syncStart() {
  downloadererrors = 0;
  updatedfiles = 0;
  newfiles = 0;
  checkedfiles = 0;
  drawAppMenu();
  renderDownloadIcon( TFT_ORANGE, 140, 80, 10.0 );
  // TODO: add M5 model detection
  UserAgent = "ESP32HTTPClient (SDU-" + String(M5_SD_UPDATER_VERSION)+"-M5Core-"+String(M5_LIB_VERSION)+"-"+String(__DATE__)+"@"+String(__TIME__)+")";
  http.setUserAgent( UserAgent );
}


void printVerifyProgress( const char* msg, uint16_t textcolor, uint16_t bgcolor, uint16_t restorecolor) {
  Serial.println( msg );
  tft.setTextColor(textcolor, bgcolor);
  tft.print( msg );
  tft.setTextColor(restorecolor, bgcolor);
}


bool getApp( String appURL ) {
  renderDownloadIcon( GREEN );

  HTTPRouter getAppRouter;
  URLParts urlParts = parseURL( appURL );
  WiFiClientSecure *client = new WiFiClientSecure;

  if( ! syncConnect(client, getAppRouter, urlParts) ) {
    return false;
  }

  String payload = http.getString();
  getAppRouter.dismiss( client, false );
  renderDownloadIcon( tft.color565(128,128,128) );
  
  DynamicJsonDocument jsonBuffer(8192);
  DeserializationError error = deserializeJson(jsonBuffer, payload);
  if (error) {
    downloadererrors++;
    jsonerrors++;
    log_e("\n[ERROR] JSON Parsing failed on %s\n",  appURL.c_str());
    delay(10000);
    return false;
  }
  JsonObject root = jsonBuffer.as<JsonObject>();

  uint16_t appsCount = root["apps_count"].as<uint16_t>();
  if(appsCount!=1) {
    downloadererrors++;
    log_e("\n%s\n", "[ERROR] AppsCount misenumeration");
    return false;
  }

  String base_url = root["base_url"].as<String>();
  String sha_sum, filePath, fileName, finalName, tempFileName;
  sha_sum.reserve(65);
  filePath.reserve(32);
  fileName.reserve(32);
  finalName.reserve(32);
  tempFileName.reserve(32);

  uint16_t assets_count = root["apps"][0]["json_meta"]["assets_count"].as<uint16_t>();
  for(uint16_t i=0;i<assets_count;i++) {
    // TODO: properly verify/sanitize this + error handling
    filePath            = root["apps"][0]["json_meta"]["assets"][i]["path"].as<String>();
    fileName            = root["apps"][0]["json_meta"]["assets"][i]["name"].as<String>();
    uint32_t remoteTime = root["apps"][0]["json_meta"]["assets"][i]["created_at"].as<uint32_t>();
    sha_sum             = root["apps"][0]["json_meta"]["assets"][i]["sha256_sum"].as<String>();
    size_t appSize      = root["apps"][0]["json_meta"]["assets"][i]["size"].as<size_t>();
    finalName = filePath + fileName;
    tempFileName = finalName + String(".tmp");
    tft.setCursor(10, 54+i*10);
    tft.print( fileName );
    Serial.printf( "  [%s]", fileName.c_str() );
    if(M5_FS.exists( finalName )) {
      Serial.print("[SHA256 Sum ->][....");
      sha256_sum( M5_FS, finalName.c_str() );
      Serial.print("]");
      if( shaResultStr.equals( sha_sum ) ) {
        log_d("[checksums match]");
        checkedfiles++;
        printVerifyProgress( WGET_SKIPPING, GREEN, tft.color565(128,128,128), WHITE);
        continue;
      }
      log_d("[checksums differ]");
      printVerifyProgress( WGET_UPDATING, TFT_ORANGE, tft.color565(128,128,128), WHITE);
    } else {
      printVerifyProgress( WGET_CREATING, WHITE, tft.color565(128,128,128), WHITE);
    }
    {
      appURL = base_url + filePath + fileName;
      urlParts = parseURL( appURL );
      if( urlParts.protocol == "https" ) {
        const char* certdata = fetchCert( urlParts.host );
        wget( appURL, tempFileName );
      } else {
        const char* nullcert = (const char*)NULL;
        wget( appURL, tempFileName );
      }
    }
    if( shaResultStr.equals( sha_sum ) ) {
      checkedfiles++;
      if( M5_FS.exists( tempFileName ) ) {
        if( M5_FS.exists( finalName ) ) {
          M5_FS.remove( finalName );
          updatedfiles++;
        } else {
          newfiles++;
        }
        M5_FS.rename( tempFileName, finalName );
      } else {
        // download failed, error was previously disclosed
        printVerifyProgress( DOWNLOAD_FAIL, RED, tft.color565(128,128,128), WHITE);
      }
    } else {
      downloadererrors++;
      Serial.printf("  [SHA256 SUM ERROR] Remote hash: %s, Local hash: %s ### keeping local file and removing temp file ###\n", String(sha_sum).c_str(), shaResultStr.c_str() );
      printVerifyProgress( SHASHUM_FAIL, RED, tft.color565(128,128,128), WHITE);
      M5_FS.remove( tempFileName );
    }
    uint16_t myprogress = progress + (i* float(progress_modulo/assets_count));
    printProgress(myprogress);
  }
  return true;
}


bool syncAppRegistry(String BASE_URL/*, const char* ca*/) {
  syncStart();
  String appURL = BASE_URL + Registry.defaultChannel.catalog_endpoint;
  String payload = "";
  URLParts urlParts = parseURL( appURL );
  HTTPRouter syncAppRouter;

  WiFiClientSecure *client = new WiFiClientSecure;

  if( ! syncConnect(client, syncAppRouter, urlParts) ) {
    log_e( "Could not connect to registry at %s", appURL.c_str() );
    syncAppRouter.dismiss( client, false );
    if( tlserrors > 0 ) {
      cleanDir( SD_CERT_PATH ); // cleanup cached certs
      //URLParts urlParts = parseURL( appURL );
      String certPath = String( SD_CERT_PATH ) + urlParts.host;
      String certURL = Registry.defaultChannel.api_cert_provider_url_http + urlParts.host;
      if( wget( certURL , certPath ) ) {
        Serial.println("Certificate changed, the ESP will continue");
        modalConfirm( DOWNLOADER_MODAL_CANCELED, NEW_TLS_CERTIFICATE_INSTALLED, MODAL_RESTART_REQUIRED, DOWNLOADER_MODAL_REBOOT, DOWNLOADER_MODAL_RESTART, DOWNLOADER_MODAL_WTF );
        ESP.restart();
      }
    }
    return false;
  } else {
    payload = http.getString();
    syncAppRouter.dismiss( client, false );
  }

  renderDownloadIcon( TFT_GREEN, 140, 80, 10.0 );

  DynamicJsonDocument jsonBuffer(8192);
  DeserializationError error = deserializeJson(jsonBuffer, payload);
  if (error) {
    downloadererrors++;
    log_e("\n%s\n", "JSON Parsing failed!");
    return false;
  }
  JsonObject root = jsonBuffer.as<JsonObject>();

  appsCount = root["apps_count"].as<uint16_t>();
  if( appsCount==0 ) {
    downloadererrors++;
    log_e("%s", "No apps found, aborting");
    return false;
  }
  progress_modulo = 100/appsCount;
  String base_url = root["base_url"].as<String>();
  Serial.printf("\nFound %s apps at %s\n", String(appsCount).c_str(), base_url.c_str() );
  for(uint16_t i=0;i<appsCount;i++) {
    String appName = root["apps"][i]["name"].as<String>();
    //if( appName == "Downloader" ) continue;
    String appURL  = BASE_URL + "/" + appName + ".json";
    progress = float(i*progress_modulo);
    M5Menu.windowClr();
    printProgress(progress);
    Serial.printf("\n[%s] :\n", appName.c_str());
    tft.setTextColor( WHITE, tft.color565(128,128,128) );
    tft.setCursor(10, 36);
    tft.print( appName );
    getApp( appURL );
    delay(150);
  }

  syncFinished();

  return true;
}


void WiFiEvent(WiFiEvent_t event) {
  log_w("[WiFi-event] event: %d\n", event);

  switch (event) {
    case SYSTEM_EVENT_WIFI_READY:
        log_w("WiFi interface ready");
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        log_w("Completed scan for access points");
        break;
    case SYSTEM_EVENT_STA_START:
        log_w("WiFi client started");
        break;
    case SYSTEM_EVENT_STA_STOP:
        log_w("WiFi clients stopped");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        log_w("Connected to access point");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        log_w("Disconnected from WiFi access point");
        break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        log_w("Authentication mode of access point has changed");
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        log_w("Obtained IP address: %s", WiFi.localIP());
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        log_w("Lost IP address and IP address is reset to 0");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        log_w("WiFi Protected Setup (WPS): succeeded in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        log_w("WiFi Protected Setup (WPS): failed in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        log_w("WiFi Protected Setup (WPS): timeout in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
        log_w("WiFi Protected Setup (WPS): pin code in enrollee mode");
        break;
    case SYSTEM_EVENT_AP_START:
        log_w("WiFi access point started");
        break;
    case SYSTEM_EVENT_AP_STOP:
        log_w("WiFi access point  stopped");
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        log_w("Client connected");
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        log_w("Client disconnected");
        break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
        log_w("Assigned IP address to client");
        break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
        log_w("Received probe request");
        break;
    case SYSTEM_EVENT_GOT_IP6:
        log_w("IPv6 is preferred");
        break;
    case SYSTEM_EVENT_ETH_START:
        log_w("Ethernet started");
        break;
    case SYSTEM_EVENT_ETH_STOP:
        log_w("Ethernet stopped");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        log_w("Ethernet connected");
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        log_w("Ethernet disconnected");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        log_w("Ethernet obtained IP address");
        break;
    default: break;
  }
}



void enableWiFi() {
  WiFi.mode(WIFI_OFF);
  delay(500);
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());
  WiFi.begin(); // set SSID/PASS from another app (i.e. WiFi Manager) and reload this app
  unsigned long startup = millis();
  
  tft.clear();
  drawAppMenu();
  tft.setCursor(10, 50);
  tft.setTextColor(WHITE, tft.color565(128,128,128));
  tft.println(WIFI_MSG_WAITING);
  size_t rssi = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    drawRSSIBar( 122, 100, rssi++, tft.color565(128,128,128), 4.0 );
    delay(500);
    if(rssi%3==0) {
      Serial.println(WIFI_MSG_CONNECTING);
    }
    if(startup + 30000 < millis()) {
      Serial.println(WIFI_MSG_TIMEOUT);
      tft.println(WIFI_MSG_TIMEOUT);
      delay(1000);
      tft.clear();
      return;
    }
  }
  tft.println( WIFI_MSG_CONNECTED );
  Serial.println( WIFI_MSG_CONNECTED );
  wifisetup = true;
}


void enableNTP() {
  Serial.println("Contacting Time Server");
  tft.clear();
  drawAppMenu();
  tft.setCursor(10, 50);
  tft.setTextColor(WHITE, tft.color565(128,128,128));
  tft.println("Contacting NTP Server");
  tft.drawJpg( ntp_jpeg, ntp_jpeg_len, 144, 104, 32, 32 );
  configTime(timezone*3600, daysavetime*3600, "pool.ntp.org", "asia.pool.ntp.org", "europe.pool.ntp.org");
  struct tm tmstruct ;
  tmstruct.tm_year = 0;
  getLocalTime(&tmstruct, 5000);
  Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct.tm_year)+1900,( tmstruct.tm_mon)+1, tmstruct.tm_mday,tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec);
  Serial.println("");
  ntpsetup = true;
  // TODO: modal-confirm date
  delay(500);
  tft.clear();
}

bool wifiSetupWorked() {
  int16_t maxAttempts = 5;
  while( !wifisetup ) {
    enableWiFi();
    maxAttempts--;
    if( maxAttempts < 0 ) {
      WiFi.mode(WIFI_OFF);
      break;
    }
  }
  if( wifisetup ) {
    enableNTP();
    drawAppMenu();
  }
  return wifisetup;
}



void updateOne(const char* appName) {
  syncStart();
  uint16_t oldAppsCount = appsCount;
  appsCount = 1;
  if( wifiSetupWorked() ) {
    sprintf(AppEndpointURLStr, AppEndpointURLTpl, appName);
    Serial.printf("Will update app : %s\n", AppEndpointURLStr);
    String appURL  = Registry.defaultChannel.api_url_https + String(AppEndpointURLStr);
    M5Menu.windowClr();
    Serial.printf("\n[%s] :\n", AppEndpointURLStr);
    tft.setTextColor( WHITE, tft.color565(128,128,128) );
    tft.setCursor(10, 36);
    tft.print( AppEndpointURLStr );
    if( ! getApp( appURL ) ) { // no cert, invalid cert, invalid TLS host or JSON parsin failed ?
      if( tlserrors > 0 ) {
        cleanDir( SD_CERT_PATH ); // cleanup cached certs
        URLParts urlParts = parseURL( appURL );
        String certPath = String( SD_CERT_PATH ) + urlParts.host;
        String certURL = Registry.defaultChannel.api_cert_provider_url_http + urlParts.host;
        if( wget( certURL , certPath ) ) {
          modalConfirm( DOWNLOADER_MODAL_CANCELED, NEW_TLS_CERTIFICATE_INSTALLED, MODAL_RESTART_REQUIRED,  DOWNLOADER_MODAL_REBOOT, DOWNLOADER_MODAL_RESTART, DOWNLOADER_MODAL_WTF );
          ESP.restart();
          //getApp( appURL );
        } else {
          // failed
          log_e("Failed to negotiate certificate for appURL %s\n", appURL.c_str() );
        }
      } else {
        log_e( "Failed to get app %s, probably a JSON error ?", appName );
      }
    }
    delay(300);
    M5Menu.windowClr();
    tft.setCursor(10,60);
    tft.setTextColor(WHITE, tft.color565(128,128,128));
    tft.println( SYNC_FINISHED );
    Serial.printf("\n\n## Download Finished  ##\n   Errors: %d\n\n", downloadererrors );
    WiFi.mode( WIFI_OFF );
    wifisetup = false;
    syncFinished(false); // throw a modal
  } else {
    // tried all attempts and gave up
  }
  appsCount = oldAppsCount;
}


void updateAll() {
  if( wifiSetupWorked() ) {
    while(!done && downloadererrors==0 ) {
      syncAppRegistry( Registry.defaultChannel.api_url_https );
      if( downloadererrors > 0 ) {
        syncFinished();
        break;
      }
    }
  } else {
    // tried all attempts and gave up
  }
}
