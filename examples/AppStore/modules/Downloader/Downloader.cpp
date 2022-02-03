#pragma once

#include "Downloader.hpp"
#include "../AppStoreUI/AppStoreUI.hpp"
#include "../FSUtils/FSUtils.hpp"   // filesystem bin formats, functions, helpers
#include "../Registry/Registry.hpp" // registry this launcher is tied to
#include "../CertsManager/CertsManager.hpp"

//#define USE_SODIUM // as of 2.0.1-rc1 this produces a bigger binary (+4Kb) + occasional crashes
#define USE_MBEDTLS // old mbedtls still more stable and produces a smaller binary

#ifdef USE_SODIUM
  #include "sodium/crypto_hash_sha256.h"
  crypto_hash_sha256_state ctx;
  #define SHA_START() [](){}
  #define SHA_INIT crypto_hash_sha256_init
  #define SHA_UPDATE crypto_hash_sha256_update
  #define SHA_FINAL crypto_hash_sha256_final
#elif defined USE_MBEDTLS
  #define MBEDTLS_SHA256_ALT
  #define MBEDTLS_ERROR_C
  #include "mbedtls/sha256.h"
  mbedtls_sha256_context ctx;
  #define SHA_START() mbedtls_sha256_starts(&ctx,0)
  #define SHA_INIT mbedtls_sha256_init
  #define SHA_UPDATE mbedtls_sha256_update
  #define SHA_FINAL mbedtls_sha256_finish
#endif

// inherit progress bar from SD-Updater library
//#define M5SDMenuProgress SDUCfg.onProgress

namespace NTP
{

  const char* NVS_NAMESPACE = "NTP";
  const char* NVS_KEY       = "Server";
  uint32_t nvs_handle = 0;

  void setTimezone( float tz )
  {
    timezone = tz;
  }

  void setDst( bool set )
  {
    daysavetime = set ? 1 : 0;
  }

  void setServer( uint8_t id )
  {
    size_t servers_count = sizeof( Servers ) / sizeof( Server );

    if( id < servers_count ) {
      if( id != currentServer ) {
        currentServer = id;
        log_v("Setting NTP server to #%d ( %s / %s )", currentServer, Servers[currentServer].name, Servers[currentServer].addr );
        if (ESP_OK == nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle)) {
          if( ESP_OK == nvs_set_u8(nvs_handle, NVS_KEY, currentServer) ) {
            log_i("[NTP] saved to nvs::%s.%s=%d", NVS_NAMESPACE, NVS_KEY, currentServer);
          } else {
            log_e("[NTP] saving failed for nvs::%s.%s=%d", NVS_NAMESPACE, NVS_KEY, currentServer);
          }
          nvs_close(nvs_handle);
        } else {
          log_e("Can't open nvs::%s for writing", NVS_NAMESPACE );
        }
      }
      return;
    }
    log_e("Invalid NTP requested: #%d", id );
  }


  void loadPrefServer()
  {
    if (ESP_OK == nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle)) {
      uint8_t nvs_ntpserver = 0;
      if(ESP_OK == nvs_get_u8(nvs_handle, NVS_KEY, static_cast<uint8_t*>(&nvs_ntpserver)) ) {
        currentServer = nvs_ntpserver;
        log_i("[NTP] load from NVS: server=%d", nvs_ntpserver);
      } else {
        log_w("Can't access nvs::%s.%s", NVS_NAMESPACE, NVS_KEY );
      }
      nvs_close(nvs_handle);
    } else {
      log_e("Can't open nvs::%s for reading", NVS_NAMESPACE );
    }
  }


}


namespace Downloader
{

  using namespace MenuItems;
  using namespace UIDraw;
  using namespace UIUtils;
  using namespace RegistryUtils;
  using namespace FSUtils;

  void httpSetup()
  {
    http.setUserAgent( UserAgent );
    http.setConnectTimeout( 10000 ); // 10s timeout = 10000
    http.setReuse(true); // handle 301 redirects gracefully
  }


  URLParts parseURL( String url ) // logic stolen from HTTPClient::beginInternal()
  {
    URLParts urlParts;
    int index = url.indexOf(':');
    if(index < 0) {
      log_e("failed to parse protocol");
      return urlParts;
    }
    urlParts.url = "" + url;
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


  URLParts parseURL( const char* url )
  {
    return parseURL( String( url ) );
  }


  bool tinyBuffInit()
  {
    if( tinyBuff == nullptr ) {
      tinyBuff = (uint8_t *)heap_caps_malloc(sizeOfTinyBuff, MALLOC_CAP_8BIT);
      if( tinyBuff == NULL ) {
        return false;
      } else {
        log_v("Allocated %d bytes for wget buffer", sizeOfTinyBuff );
      }
    } else {
      log_v("Reusing wget buffer");
    }
    return true;
  }


  void sha_sum_to_str()
  {
    shaResultStr = "";
    char str[3];
    for(int i= 0; i< sizeof(shaResult); i++) {
      sprintf(str, "%02x", (int)shaResult[i]);
      shaResultStr += String( str );
    }
  }


  void sha256_sum(const char* fileName)
  {
    log_d("SHA256: checking file %s\n", fileName);
    File checkFile = M5_FS.open( fileName );
    size_t fileSize = checkFile.size();
    size_t len = fileSize;
    if( !checkFile || fileSize==0 ) {
      downloadererrors++;
      log_e("  [ERROR] Can't open %s file for reading, aborting\n", fileName);
      return;
    }
    //CheckSumIcon.draw( &tft, 288, 125 );

    tinyBuffInit();
    *shaResult = {0};

    SHA_INIT(&ctx);
    SHA_START();

    size_t n;
    while ((n = checkFile.read(tinyBuff, sizeOfTinyBuff)) > 0) {
      SHA_UPDATE(&ctx, (const unsigned char *) tinyBuff, n);
      if( fileSize/10 > sizeOfTinyBuff && fileSize != len ) {
        //shaProgressBar.draw( 100*len / fileSize );
      }
      len -= n;
      delay(1);
    }
    //shaProgressBar.clear();
    checkFile.close();

    SHA_FINAL(&ctx, shaResult);
    sha_sum_to_str();
  }


  void sha256_sum( String fileName )
  {
    return sha256_sum( fileName.c_str() );
  }



  bool wget( const char* url, const char* path )
  {
    WiFiClientSecure *client = new WiFiClientSecure;
    UITheme* theme = UI->getTheme();

    if( !tinyBuffInit() ) {
      log_e("Failed to allocate memory for download buffer, aborting");
      delete client;
      return false;
    }

    URLParts urlParts = parseURL( url );

    const char* cert = TLS::fetchCert( urlParts.host );
    if( cert == NULL ) client->setInsecure();
    else client->setCACert( cert );

    httpSetup();

    if( ! http.begin(*client, url ) ) {
      log_e("Can't open url %s", url );
      delete client;
      return false;
    }

    http.collectHeaders(headerKeys, numberOfHeaders);

    log_v("URL = %s", url);

    int httpCode = http.GET();

    // file found at server
    if (httpCode == HTTP_CODE_FOUND || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String newlocation = "";
      for(int i = 0; i< http.headers(); i++) {
        String headerContent = http.header(i);
        if( headerContent !="" ) {
          newlocation = headerContent;
          Serial.printf("%s: %s\n", headerKeys[i], headerContent.c_str());
        }
      }

      http.end();
      if( newlocation != "" ) {
        log_i("Found 302/301 location header: %s", newlocation.c_str() );
        delete client;
        return wget( newlocation.c_str(), path );
      } else {
        log_e("Empty redirect !!");
        delete client;
        return false;
      }
    }

    WiFiClient *stream = http.getStreamPtr();

    if( stream == nullptr ) {
      http.end();
      log_e("Connection failed!");
      delete client;
      return false;
    }

    #if defined FS_CAN_CREATE_PATH
      File outFile = M5_FS.open( path, FILE_WRITE, true );
    #else
      File outFile = M5_FS.open( path, FILE_WRITE );
    #endif

    if( ! outFile ) {
      log_e("Can't open %s file to save url %s", path, url );
      delete client;
      return false;
    }

    int len = http.getSize();
    int bytesLeftToDownload = len;
    int bytesDownloaded = 0;

    *tinyBuff = {0};
    *shaResult = {0};

    SHA_INIT( &ctx );
    SHA_START();

    //CheckSumIcon.draw( &tft, 288, 125 );
    //DownloadIcon.draw( &tft, 252, 125 );

    while(http.connected() && (len > 0 || len == -1)) {
      size_t size = stream->available();
      if(size) {
        // read up to 512 byte
        int c = stream->readBytes(tinyBuff, sizeOfTinyBuff);
        if( c > 0 ) {
          SHA_UPDATE(&ctx, (const unsigned char *)tinyBuff, c);
          outFile.write( tinyBuff, c );
          bytesLeftToDownload -= c;
          bytesDownloaded += c;
          //Serial.printf("%d bytes left\n", bytesLeftToDownload );
          float progress = (((float)bytesDownloaded / (float)len) * 100.00);
          theme->dlProgressBar.progress( progress );
        }
      }
      if( bytesLeftToDownload == 0 ) break;
    }

    SHA_FINAL(&ctx, shaResult);
    outFile.close();
    sha_sum_to_str();

    // clear progress bar
    theme->dlProgressBar.clear();
    delete client;
    return true;
  }


  // aliases
  bool wget( String bin_url, String outputFile )
  {
    return wget( bin_url.c_str(), outputFile.c_str() );
  }
  bool wget( String bin_url, const char* outputFile )
  {
    return wget( bin_url.c_str(), outputFile );
  }
  bool wget( const char* bin_url, String outputFile )
  {
    return wget( bin_url, outputFile.c_str() );
  }


  WiFiClient *wgetptr( WiFiClientSecure *client, const char* url, const char *cert )
  {
    if( cert == NULL ) client->setInsecure();
    else client->setCACert( cert );

    httpSetup();

    if( ! http.begin(*client, url ) ) {
      log_e("Can't open url %s", url );
      return nullptr;
    }

    http.collectHeaders(headerKeys, numberOfHeaders);
    int httpCode = http.GET();
    // file found at server
    if (httpCode == HTTP_CODE_FOUND || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String newlocation = "";
      String headerLocation = http.header("location");
      String headerRedirect = http.header("redirect");
      if( headerLocation !="" ) {
        newlocation = headerLocation;
        log_i("302 (location): %s => %s", url, headerLocation.c_str());
      } else if ( headerRedirect != "" ) {
        log_i("301 (redirect): %s => %s", url, headerLocation.c_str());
        newlocation = headerRedirect;
      }
      http.end();
      if( newlocation != "" ) {
        log_i("Found 302/301 location header: %s", newlocation.c_str() );
        // delete client;
        return wgetptr( client, newlocation.c_str(), cert );
      } else {
        log_e("Empty redirect !!");
        return nullptr;
      }
    }
    if( httpCode != 200 ) return nullptr;

    log_v("Got response with Content:Type: %s / Content-Length: %s", http.header("Content-Type").c_str(), http.header("Content-Length").c_str() );

    return http.getStreamPtr();
  }

  #if ARDUHAL_LOG_LEVEL < ARDUHAL_LOG_LEVEL_DEBUG
    void WiFiEvent(WiFiEvent_t event)
    {
      const char * arduino_event_names[] = {
          "WIFI_READY",
          "SCAN_DONE",
          "STA_START", "STA_STOP", "STA_CONNECTED", "STA_DISCONNECTED", "STA_AUTHMODE_CHANGE", "STA_GOT_IP", "STA_GOT_IP6", "STA_LOST_IP",
          "AP_START", "AP_STOP", "AP_STACONNECTED", "AP_STADISCONNECTED", "AP_STAIPASSIGNED", "AP_PROBEREQRECVED", "AP_GOT_IP6",
          "FTM_REPORT",
          "ETH_START", "ETH_STOP", "ETH_CONNECTED", "ETH_DISCONNECTED", "ETH_GOT_IP", "ETH_GOT_IP6",
          "WPS_ER_SUCCESS", "WPS_ER_FAILED", "WPS_ER_TIMEOUT", "WPS_ER_PIN", "WPS_ER_PBC_OVERLAP",
          "SC_SCAN_DONE", "SC_FOUND_CHANNEL", "SC_GOT_SSID_PSWD", "SC_SEND_ACK_DONE",
          "PROV_INIT", "PROV_DEINIT", "PROV_START", "PROV_END", "PROV_CRED_RECV", "PROV_CRED_FAIL", "PROV_CRED_SUCCESS"
      };
      log_i("[WiFi-event]: #%d (%s)", event, arduino_event_names[event]);
    }
  #endif

  void disableWiFi()
  {
    WiFi.mode(WIFI_OFF);
    delay(500);
    wifisetup = false;
  }


  void enableWiFi()
  {
    //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    WiFi.mode(WIFI_OFF);
    delay(500);
    WiFi.mode(WIFI_STA);
    String mac = WiFi.macAddress();
    Serial.println( mac ); // print mac address to serial so it can eventually be copy/pasted
    #if ARDUHAL_LOG_LEVEL < ARDUHAL_LOG_LEVEL_DEBUG
      // raise DEBUG messages to INFO level
      WiFi.onEvent(WiFiEvent);
    #endif
    WiFi.begin(); // set SSID/PASS from another app (i.e. WiFi Manager) and reload this app
    unsigned long startup = millis();

    const String nnnn = "\n\n\n\n";
    drawDownloaderMenu( "WiFi Setup", String( WIFI_MSG_WAITING+nnnn+mac ).c_str());
    size_t rssi = 0;

    while ( WiFi.status() != WL_CONNECTED ) {
      drawRSSIBar( 122, 100, rssi++, UI->getTheme()->MenuColor, 4.0 );
      delay(500);
      if(startup + 30000 < millis()) {
        drawInfoWindow( WIFI_TITLE_TIMEOUT, WIFI_MSG_TIMEOUT, 1000 );
        return;
      }
    }
    drawInfoWindow( WIFI_TITLE_CONNECTED, WIFI_MSG_CONNECTED );
    wifisetup = true;
  }


  void enableNTP()
  {
    drawDownloaderMenu(NTP_TITLE_SETUP, NTP_MSG_SETUP );
    LGFX* gfx = UI->getGfx();
    NtpIcon.draw( gfx, gfx->width()/2-NtpIcon.width/2, gfx->height()/2-NtpIcon.height/2 );
    configTime(NTP::timezone*3600, NTP::daysavetime*3600, NTP::Servers[NTP::currentServer].addr, NTP::Servers[3].addr, NTP::Servers[4].addr );
    struct tm tmstruct ;
    tmstruct.tm_year = 0;

    int max_attempts = 5;
    while( !ntpsetup && max_attempts-->0 ) {
      if( getLocalTime(&tmstruct, 5000) ) {
        // TODO: draw ntp-success icon
        Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct.tm_year)+1900,( tmstruct.tm_mon)+1, tmstruct.tm_mday,tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec);
        Serial.println("");
        ntpsetup = true;
      }
      delay(500);
    }

    if( !ntpsetup ) { // all attempts consumed
      // TODO: modal-confirm retry/restart-wifi/restart-esp
      drawInfoWindow( NTP_TITLE_FAIL, NTP_MSG_FAIL, 1500 );
    } else {
      drawInfoWindow( NTP_TITLE_SETUP, NTP_MSG_SETUP, 500 );
    }
  }


  bool wifiSetupWorked()
  {
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
      if( !ntpsetup ) {
        enableNTP();
      }
      drawDownloaderMenu();
    }
    return wifisetup;
  }



  void registryFetch( AppRegistry registry, String appRegistryLocalFile )
  {
    if( !wifiSetupWorked() ) {
      modalConfirm( MODAL_CANCELED_TITLE, MODAL_WIFI_NOCONN_MSG, MODAL_SAME_PLAYER_SHOOT_AGAIN, MENU_BTN_REBOOT, MENU_BTN_RESTART, MENU_BTN_CANCEL );
      ESP.restart();
    }
    URLParts urlParts = parseURL( registry.url );

    if( ! TLS::init( urlParts.host ) ) {
      log_e("Unable to init tls, aborting");
      return;
    }

    if( appRegistryLocalFile == "" ) {
      appRegistryLocalFile = appRegistryFolder + PATH_SEPARATOR + appRegistryDefaultName;
    } else {
      appRegistryLocalFile = appRegistryFolder + PATH_SEPARATOR + urlParts.host + ".json";
    }
    if( !wget( registry.url , appRegistryLocalFile ) ) {
      modalConfirm( MODAL_CANCELED_TITLE, MODAL_REGISTRY_DAMAGED, MODAL_SAME_PLAYER_SHOOT_AGAIN, MENU_BTN_REBOOT, MENU_BTN_RESTART, MENU_BTN_CANCEL );
    } else {
      String appRegistryDefaultFile = appRegistryFolder + PATH_SEPARATOR + appRegistryDefaultName;
      File sourceFile = M5_FS.open( appRegistryLocalFile );
      if( M5_FS.exists( appRegistryDefaultFile ) ) {
        M5_FS.remove( appRegistryDefaultFile );
      }
      #if defined FS_CAN_CREATE_PATH
        File destFile   = M5_FS.open( appRegistryDefaultFile, FILE_WRITE, true );
      #else
        File destFile   = M5_FS.open( appRegistryDefaultFile, FILE_WRITE );
      #endif
      static uint8_t buf[512];
      size_t packets = 0;
      while( (packets = sourceFile.read( buf, sizeof(buf))) > 0 ) {
        destFile.write( buf, packets );
      }
      destFile.close();
      sourceFile.close();
      modalConfirm( UPDATE_SUCCESS, MODAL_REGISTRY_UPDATED, MODAL_REBOOT_REGISTRY_UPDATED, MENU_BTN_REBOOT, MENU_BTN_RESTART, MENU_BTN_CANCEL );
    }
    ESP.restart();
  }



  bool downloadGzCatalog()
  {
    if( !wifiSetupWorked() ) {
      modalConfirm( MODAL_CANCELED_TITLE, MODAL_WIFI_NOCONN_MSG, MODAL_SAME_PLAYER_SHOOT_AGAIN, MENU_BTN_REBOOT, MENU_BTN_RESTART, MENU_BTN_CANCEL );
      ESP.restart();
    }

    Console = new LogWindow();

    bool has_backup = false;
    if( M5_FS.exists( CATALOG_DIR ) ) {
      if( M5_FS.exists( CATALOG_DIR_BKP ) ) {
        drawInfoWindow( DL_FSCLEANUP_TITLE, DL_FSCLEANUP_MSG, 500 );
        log_d("Removing previous backup");
        cleanDir( CATALOG_DIR_BKP );
      }
      log_i("Backing up registry");
      drawInfoWindow( DL_FSBACKUP_TITLE, DL_FSBACKUP_MSG, 500 );
      M5_FS.rename( CATALOG_DIR, CATALOG_DIR_BKP );
      has_backup = true;
    }

    TarGzUnpacker *TARGZUnpacker = new TarGzUnpacker();
    //TARGZUnpacker->haltOnError( true ); // stop on fail (manual restart/reset required)
    //TARGZUnpacker->setTarVerify( true ); // true = enables health checks but slows down the overall process
    TARGZUnpacker->setupFSCallbacks( targzTotalBytesFn, targzFreeBytesFn ); // prevent the partition from exploding, recommended
    TARGZUnpacker->setGzProgressCallback( gzProgressCallback /*BaseUnpacker::defaultProgressCallback*/ ); // targzNullProgressCallback or defaultProgressCallback
    TARGZUnpacker->setTarProgressCallback( tarProgressCallback /*BaseUnpacker::defaultProgressCallback*/ ); // prints the untarring progress for each individual file
    TARGZUnpacker->setTarStatusProgressCallback( tarStatusCallback /*BaseUnpacker::defaultTarStatusProgressCallback*/ ); // print the filenames as they're expanded
    //TARGZUnpacker->setPsram( true );

    gzCatalogURL = String( Registry.defaultChannel.api_url_https ) + "/catalog.tar.gz";

    URLParts urlParts = parseURL( gzCatalogURL );

    drawInfoWindow( DL_TLSFETCH_TITLE, DL_TLSFETCH_MSG, 500 );

    const char* cert = TLS::fetchCert( urlParts.host );

    if( cert == nullptr ) {
      if( ! TLS::init( urlParts.host ) ) {
        log_e("Unable to init TLS, aborting");
        drawInfoWindow( DL_TLSFAIL_TITLE, DL_TLSFAIL_MSG, 1000 );
        delete TARGZUnpacker;
        disableWiFi();
        return false;
      }
      cert = TLS::fetchCert( urlParts.host );
    }

    drawInfoWindow( DL_HTTPINIT_TITLE, DL_HTTPINIT_MSG );

    WiFiClientSecure *client = new WiFiClientSecure;
    Stream* streamptr = wgetptr( client, gzCatalogURL.c_str(), cert );
    bool ret = false;

    if( streamptr != nullptr ) {
      log_i("Fetching %s", gzCatalogURL.c_str() );
      drawInfoWindow( DL_AWAITING_TITLE, nullptr );
      // see if content-length was provided, and enable download progress
      String contentLengthStr = http.header("Content-Length");
      contentLengthStr.trim();
      int64_t streamSize = -1;
      if( contentLengthStr != "" ) {
        streamSize = atoi( contentLengthStr.c_str() );
      }
      // wait for data to happen, for some reason this is necessary
      unsigned long start = millis();
      unsigned long timeout = 10000; // wait 10 secs max
      float blah = -PI;

      while( !streamptr->available() && millis()-start < timeout ) {
        blah += .1;
        uint8_t r = abs(sin(blah))*128 + 128;
        uint8_t g = abs(cos(blah))*128 + 128;
        uint8_t b = abs(cos(blah))*256 - 128;
        //uint16_t color = tft.color565( r, g, b );
        uint32_t color = (r << 16) + (g << 8) + b;
        drawDownloadIcon( color );
        vTaskDelay(1);
      }

      drawDownloadIcon();

      if( !TARGZUnpacker->tarGzStreamExpander( streamptr, SD, CATALOG_DIR, streamSize ) ) {
        drawDownloadIcon( 0x800000U );
        log_e("tarGzStreamExpander failed with return code #%d", TARGZUnpacker->tarGzGetError() );
        drawInfoWindow( "Unpacking failed", "GZ archive could\nnot be unpacked.", 1000 );
        if( has_backup ) { // restore backup
          log_d("Restoring backup");
          drawInfoWindow( DL_FSBACKUP_TITLE, DL_FSRESTORE_MSG, 500 );
          M5_FS.rename( CATALOG_DIR_BKP, CATALOG_DIR );
        }
      } else {
        drawInfoWindow( DL_SUCCESS_TITLE, DL_SUCCESS_MSG, 500 );
        drawDownloadIcon( UI->getTheme()->MenuColor );
        ret = true; // success !
        if( has_backup ) {
          log_d("Removing old backup");
          drawInfoWindow( DL_FSCLEANUP_TITLE, DL_FSCLEANUP_MSG, 500 );
          cleanDir( CATALOG_DIR_BKP ); // no need to keep the backup
        }
      }
    } else {
      drawInfoWindow( DL_HTTPFAIL_TITLE, DL_FAIL_MSG, 1000 );
    }
    delete client;
    delete TARGZUnpacker;
    delete Console;
    disableWiFi();
    log_v(" Leaving registry fetcher with %d bytes free", ESP.getFreeHeap() );
    return ret;
  }


  bool downloadApp( String appName )
  {

    if( !wifisetup ) {
      if( !wifiSetupWorked() ) {
        modalConfirm( MODAL_CANCELED_TITLE, MODAL_WIFI_NOCONN_MSG, MODAL_SAME_PLAYER_SHOOT_AGAIN, MENU_BTN_REBOOT, MENU_BTN_RESTART, MENU_BTN_CANCEL );
        ESP.restart();
      }
    }

    drawInfoWindow( "Downloading", "..." );

    if( baseCatalogURL == "" ) {
      log_e("No base catalog url set, download catalog first ");
      return false;
    }

    String macroseparator = "";
    String jsonFile = CATALOG_DIR + macroseparator + DIR_json + appName + EXT_json;

    JsonObject root;
    DynamicJsonDocument jsonBuffer( 8192 );
    if( !getJson( jsonFile.c_str(), root, jsonBuffer ) ) {
      log_e("Failed to get json from %s", jsonFile.c_str() );
      return false;
    }

    progress_modulo = 100; // progress_modulo = 100/appsCount;
    size_t assets_count = root["json_meta"]["assets"].size();

    if( assets_count == 0 ) return false;
    int i=0;

    Console = new LogWindow();

    for (JsonVariant value : root["json_meta"]["assets"].as<JsonArray>() ) {
      String finalName = value["path"].as<String>() + value["name"].as<String>();
      String tempFileName = finalName + String(".tmp");
      Console->log( value["name"].as<const char*>() );
      if(M5_FS.exists( finalName.c_str() )) {
        // local file already exists, calculate sha shum and compare to registry
        sha256_sum( finalName );
        if( value["sha256_sum"].as<String>().equals(shaResultStr) ) {
          // doesn't need to be updated
          Console->log( WGET_SKIPPING );
          i++;
          continue; // no need to download
        }
        Console->log( WGET_UPDATING );
      } else {
        // file does not exist locally, see if it is present in the catalog folder
        if(M5_FS.exists( CATALOG_DIR+finalName )) {
          // file exists in the catalog folder as it should, no need to download => copy it locally
          if( copyFile( String( CATALOG_DIR+finalName ).c_str(), finalName.c_str() ) ) {
            Console->log( WGET_SKIPPING );
            i++;
            continue;
          }
        }
        Console->log( WGET_CREATING );
      }
      String appURL = baseCatalogURL + finalName;
      drawDownloadIcon();
      if( !wget( appURL, tempFileName ) ) { // uh-oh
        log_e("\n%s\n", "[ERROR] could not download %s to %s", appURL.c_str(), tempFileName.c_str() );
        drawDownloadIcon( 0x800000U );
        drawInfoWindow( DL_HTTPFAIL_TITLE, tempFileName.c_str(), 500 );
        i++;
        continue;
      }
      drawDownloadIcon( UI->getTheme()->MenuColor );
      if( value["sha256_sum"].as<String>().equals(shaResultStr) ) {
        if( M5_FS.exists( tempFileName ) ) {
          if( M5_FS.exists( finalName ) ) M5_FS.remove( finalName ); // remove existing as it'll be replaced
          M5_FS.rename( tempFileName, finalName );
        } else { // download failed, error was previously disclosed
          drawInfoWindow( DL_FSFAIL_TITLE, DOWNLOAD_FAIL, 500 );
        }
      } else {
        log_e("  [SHA256 SUM ERROR] Remote hash: %s, Local hash: %s ### keeping local file and removing temp file ###", value["sha256_sum"].as<const char*>(), shaResultStr.c_str() );
        drawInfoWindow( DL_SHAFAIL_TITLE, SHASHUM_FAIL, 500 );
        M5_FS.remove( tempFileName );
      }
      uint16_t myprogress = progress + (i* float(progress_modulo/assets_count));
      i++;
    }
    return true;
  }

};
