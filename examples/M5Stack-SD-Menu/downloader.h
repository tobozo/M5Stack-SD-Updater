
#include "certificates.h"
#include <WiFi.h>
#include <HTTPClient.h>
//#include <time.h> // https://github.com/PaulStoffregen/Time
#include <WiFi.h>
#include "mbedtls/md.h"

long timezone = 0; // UTC
byte daysavetime = 0; // UTC

HTTPClient http;

extern SDUpdater sdUpdater; // used for menu progress
extern M5SAM M5Menu;
extern uint16_t appsCount;
extern uint16_t MenuID;

#define DOWNLOADER_BIN "/Downloader.bin" // Fixme/Hack: a dummy file will be created so it appears in the menu as an app

bool done = false;
uint8_t progress = 0;
float progress_modulo = 0;
bool wifisetup = false;
const String API_URL = "https://phpsecu.re/m5stack/sd-updater/";
const String API_ENDPOINT = "catalog.json";
mbedtls_md_context_t ctx;
mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
byte shaResult[32];
static String shaResultStr = "f7ff9bcd52fee13ae7ebd6b4e3650a4d9d16f8f23cab370d5cdea291e5b6bba6";

void printProgress(uint16_t progress) {
  uint16_t x = tft.getCursorX();
  uint16_t y = tft.getCursorX();
  tft.setCursor(50, 220);
  tft.setTextColor(WHITE, BLACK);
  tft.print( OVERALL_PROGRESS_TITLE );
  tft.print(String(progress));
  tft.print("%");
  tft.setCursor(x, y);
}


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
    log_e("File %s does not exist, aborting\n", fileName);
    return;
  }
  File checkFile = fs.open( fileName );
  size_t fileSize = checkFile.size();
  size_t len = fileSize;

  if( !checkFile || fileSize==0 ) {
    log_e("Can't open %s file for reading, aborting\n", fileName);
    return;
  }

  tft.drawJpg( checksum_jpg, checksum_jpg_len, 10, 115, 22, 32 );
  
  uint8_t buff[4096] = { 0 };
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  size_t n;
  while ((n = checkFile.read(buff, sizeof(buff))) > 0) {
    mbedtls_md_update(&ctx, (const unsigned char *) buff, n);
    sdUpdater.M5SDMenuProgress(fileSize-len, fileSize);
    len -= n;
  }

  tft.fillRect( 10, 115, 22, 32, TFT_BLACK );

  checkFile.close();

  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  sha_sum_to_str();
}



void wget(String bin_url, String appName, const char* &ca) {
  //tft.setCursor(0,0);
  Serial.printf("Will download %s and save to SD as %s\n", bin_url.c_str(), appName.c_str());
  if( bin_url.startsWith("https://") ) {
    http.begin(bin_url, ca);
  } else {
    http.begin(bin_url);
  }
  int httpCode = http.GET();
  if(httpCode <= 0) {
    Serial.printf("%s", "[HTTP] GET... failed");
    http.end();
    return;
  }
  if(httpCode != HTTP_CODE_OK) {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str()); 
    http.end();
    return;
  }

  int len = http.getSize();
  if(len<=0) {
    Serial.printf("Failed to read %s, content is empty, aborting\n", bin_url.c_str());
    http.end();
    return;
  }
  int httpSize = len;
  uint8_t buff[2048] = { 0 };
  WiFiClient * stream = http.getStreamPtr();
  if( M5_FS.exists(appName) ) {
    M5_FS.remove(appName);
  }
  File myFile = M5_FS.open(appName, FILE_WRITE);
  if(!myFile) {
    Serial.printf("Failed to open %s for writing, aborting\n", appName.c_str());
    http.end();
    myFile.close();
    return;
  }
  unsigned long downwloadstart = millis();

  *shaResult = {0};
   
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);

  tft.drawJpg( checksum_jpg, checksum_jpg_len, 10, 115, 22, 32 );
  tft.drawJpg( download_jpg, download_jpg_len, 44, 115, 26, 32 );
  
  while(http.connected() && (len > 0 || len == -1)) {
    sdUpdater.M5SDMenuProgress(httpSize-len, httpSize);
    // get available data size
    size_t size = stream->available();
    if(size) {
      // read up to 128 byte
      int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
      // hashsum
      mbedtls_md_update(&ctx, (const unsigned char *) buff, c);
      // write it to SD
      myFile.write(buff, c);
      if(len > 0) {
        len -= c;
      }
    }
    delay(1);
  }
  myFile.close();

  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  tft.fillRect( 115, 10, 70, 32, TFT_BLACK );

  sha_sum_to_str();
  
  unsigned long dl_duration;
  float bytespermillis;
  dl_duration = ( millis() - downwloadstart );
  
  if( dl_duration > 0 ) {
    bytespermillis = httpSize / dl_duration;
    //float Kbpermillis = bytespermillis/1024; // kb per millis
    //float Kbpersecond = Kbpermillis*1000; // kb per second
    Serial.printf(" [OK][Downloaded %d KB at %d KB/s]\n", (httpSize/1024), (int)( bytespermillis / 1024.0 * 1000.0 ) );
  } else {
    Serial.printf("[OK] Copy done...");
  }
  http.end();
}


void getApp(String appURL, const char* &ca) {
  if( appURL.startsWith("https://") ) {
    http.begin(appURL, ca);
  } else {
    http.begin(appURL);
  }
  int httpCode = http.GET();
  if(httpCode <= 0) {
    log_e("%s", "[HTTP] GET... failed");
    http.end();
    return;
  }
  if(httpCode != HTTP_CODE_OK) {
    log_e("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str()); 
    http.end();
  }
  String payload = http.getString();
  http.end();
  #if ARDUINOJSON_VERSION_MAJOR==6
    DynamicJsonDocument jsonBuffer(2048);
    DeserializationError error = deserializeJson(jsonBuffer, payload);
    if (error) {
      log_e("%s", "JSON Parsing failed!");
      delay(10000);
      return;
    }
    JsonObject root = jsonBuffer.as<JsonObject>();
  #else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      log_e("%s", "JSON Parsing failed!");
      delay(10000);
      return;
    }
  #endif
  uint16_t appsCount = root["apps_count"].as<uint16_t>();
  if(appsCount!=1) {
    log_e("%s", "appsCount misenumeration");
    return;
  }
  String base_url = root["base_url"].as<String>();
  tft.setTextColor( WHITE, BLACK );
  String sha_sum;
  String filePath;
  String fileName;
  String finalName;
  String tempFileName;
  sha_sum.reserve(65);
  filePath.reserve(32);
  fileName.reserve(32);
  finalName.reserve(32);
  tempFileName.reserve(32);

  uint16_t assets_count = root["apps"][0]["json_meta"]["assets_count"].as<uint16_t>();
  for(uint16_t i=0;i<assets_count;i++) {
    tft.setCursor(0,i*20);
    filePath            = root["apps"][0]["json_meta"]["assets"][i]["path"].as<String>();
    fileName            = root["apps"][0]["json_meta"]["assets"][i]["name"].as<String>();
    uint32_t remoteTime = root["apps"][0]["json_meta"]["assets"][i]["created_at"].as<uint32_t>();
    sha_sum             = root["apps"][0]["json_meta"]["assets"][i]["sha256_sum"].as<String>();
    size_t appSize      = root["apps"][0]["json_meta"]["assets"][i]["size"].as<size_t>();
    size_t mySize       = 0;
    finalName = filePath + fileName;
    tempFileName = finalName + String(".tmp");

    if( sha_sum != "" ) {
      log_d("Got sha sum %s for file %s\n", sha_sum.c_str(), finalName.c_str() );
    }

    tft.print( fileName );
    Serial.print( fileName );
    
    if(M5_FS.exists( finalName )) {
      sha256_sum( M5_FS, finalName.c_str() );
      if( shaResultStr.equals( sha_sum ) ) {
        log_d("[checksums match]");
        Serial.print( WGET_SKIPPING );
        tft.println( WGET_SKIPPING );
        continue;
      }
      log_d("[checksums differ]");
      Serial.print( WGET_UPDATING );
      tft.println( WGET_UPDATING );
    } else {
      Serial.print( WGET_CREATING );
      tft.println( WGET_CREATING );
    }

    wget(base_url + filePath + fileName, tempFileName, phpsecure_ca);
    
    if( M5_FS.exists( tempFileName ) && shaResultStr.equals( sha_sum ) ) {
      if( M5_FS.exists( finalName ) ) {
        M5_FS.remove( finalName );
      }
      M5_FS.rename( tempFileName, finalName );
    } else {
      log_e("SHA256 Failed (json has given: %s, local has given: %s), removing downloaded file\n", String(sha_sum).c_str(), shaResultStr.c_str() );
      M5_FS.remove( tempFileName );
    }
    
    uint16_t myprogress = progress + (i* float(progress_modulo/assets_count));
    printProgress(myprogress);
  }

}



void syncAppRegistry(String API_URL, const char* ca) {
  //http.setReuse(true);
  if ( API_URL.startsWith("https://") ) {
    http.begin(API_URL + API_ENDPOINT, ca);
  } else {
    http.begin(API_URL + API_ENDPOINT );
  }
  int httpCode = http.GET();
  if(httpCode <= 0) {
    http.end();
    log_e("%s", "[HTTP] GET... failed");
    delay(10000);
    return;
  }
  log_e("[HTTP] GET... code: %d\n", httpCode);
  if(httpCode != HTTP_CODE_OK) {
    log_e("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str()); 
    http.end();
    delay(10000);
    return;
  }
  
  String payload = http.getString();
  http.end();
  #if ARDUINOJSON_VERSION_MAJOR==6
    DynamicJsonDocument jsonBuffer(4096);
    DeserializationError error = deserializeJson(jsonBuffer, payload);
    if (error) {
      log_e("%s", "JSON Parsing failed!");
      delay(10000);
      return;
    }
    JsonObject root = jsonBuffer.as<JsonObject>();
  #else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      log_e("%s", "JSON Parsing failed!");
      delay(10000);
      return;
    }
  #endif
  //Serial.println("Parsed JSON");
  appsCount = root["apps_count"].as<uint16_t>();
  if( appsCount==0 ) {
    log_e("%s", "No apps found, aborting");
    delay(10000);
    return;
  }
  progress_modulo = 100/appsCount;
  String base_url = root["base_url"].as<String>();
  log_d("Found %s apps at %s\n", String(appsCount).c_str(), base_url.c_str() );
  for(uint16_t i=0;i<appsCount;i++) {
    String appName = root["apps"][i]["name"].as<String>();
    String appURL  = API_URL + appName + ".json";
    progress = float(i*progress_modulo);
    
    tft.clear();
    
    printProgress(progress);
    Serial.println();
    Serial.print(appName);
    Serial.print(": ");
    getApp( appURL, phpsecure_ca );
    
  }
  done = true;
  tft.clear();
  tft.setCursor(0,0);
  tft.setTextColor(WHITE, BLACK);
  tft.println( SYNC_FINISHED );
  delay(1000);
  ESP.restart();
}


void drawAppMenu() {
  M5Menu.windowClr();
  M5Menu.drawAppMenu( APP_DOWNLOADER_MENUTITLE, "", "", "");
}


void cleanDir() {
  tft.clear();
  tft.setCursor(0,0);
  tft.setTextColor(WHITE, BLACK);

  File root = M5_FS.open("/");
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
      if(String( fileName )!=MENU_BIN && String( fileName )!=String(DOWNLOADER_BIN) && String( fileName ).endsWith(".bin")) {
        file.close();
        M5_FS.remove( fileName );
        if( tft.getCursorY() > tft.height() ) {
          tft.clear();
          tft.setCursor(0,0);
        }
        Serial.printf( CLEANDIR_REMOVED, fileName );
        tft.printf( CLEANDIR_REMOVED, fileName );
      }
    }
    file = root.openNextFile();
  }
  tft.clear();
  drawAppMenu();
}


void enableWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(); // set SSID/PASS from another app (i.e. WiFi Manager) and reload this app
  unsigned long startup = millis();
  tft.clear();
  tft.setCursor(0,0);
  tft.setTextColor(WHITE, BLACK);
  tft.println(WIFI_MSG_WAITING);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(WIFI_MSG_CONNECTING);
    if(startup + 10000 < millis()) {
      Serial.println(WIFI_MSG_TIMEOUT);
      tft.println(WIFI_MSG_TIMEOUT);
      delay(1000);
      tft.clear();
      drawAppMenu();
      return;
    }
  }
  tft.println( WIFI_MSG_CONNECTED );
  Serial.println( WIFI_MSG_CONNECTED );
  wifisetup = true;
  delay(1000);
  tft.clear();
  drawAppMenu();
}


void enableNTP() {
  Serial.println("Contacting Time Server");
  configTime(3600*timezone, daysavetime*3600, "pool.ntp.org", "asia.pool.ntp.org", "europe.pool.ntp.org");
  struct tm tmstruct ;
  delay(2000);
  tmstruct.tm_year = 0;
  getLocalTime(&tmstruct, 5000);
  Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct.tm_year)+1900,( tmstruct.tm_mon)+1, tmstruct.tm_mday,tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec);
  Serial.println("");
}



void updateAll() {
  while( !wifisetup ) {
    enableWiFi();
  }
  if( wifisetup ) {
    enableNTP();
    while(!done) {
      syncAppRegistry(API_URL, phpsecure_ca);
    }
  }
}
