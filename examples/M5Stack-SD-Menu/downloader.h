
#include "certificates.h"
#include <WiFi.h>
#include <HTTPClient.h>
//#include <time.h> // https://github.com/PaulStoffregen/Time
#include <WiFi.h>

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


void wget (String bin_url, String appName, const char* &ca) {
  //tft.setCursor(0,0);
  Serial.println("Will download " + bin_url + " and save to SD as " + appName);
  if( bin_url.startsWith("https://") ) {
    http.begin(bin_url, ca);
  } else {
    http.begin(bin_url);
  }
  int httpCode = http.GET();
  if(httpCode <= 0) {
    Serial.println("[HTTP] GET... failed");
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
    Serial.println("Failed to read " + bin_url + " content is empty, aborting");
    http.end();
    return;
  }
  int httpSize = len;
  uint8_t buff[128] = { 0 };
  WiFiClient * stream = http.getStreamPtr();
  File myFile = M5_FS.open(appName, FILE_WRITE);
  if(!myFile) {
    Serial.println("Failed to open " + appName + " for writing, aborting");
    http.end();
    myFile.close();
    return;
  }
  unsigned long downwloadstart = millis();
  while(http.connected() && (len > 0 || len == -1)) {
    sdUpdater.M5SDMenuProgress(httpSize-len, httpSize);
    // get available data size
    size_t size = stream->available();
    if(size) {
      // read up to 128 byte
      int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
      // write it to SD
      myFile.write(buff, c);
      if(len > 0) {
        len -= c;
      }
    }
    delay(1);
  }
  myFile.close();
  unsigned long dl_duration;
  float bytespermillis;
  dl_duration = ( millis() - downwloadstart );
  
  if( dl_duration > 0 ) {
    bytespermillis = httpSize / dl_duration;
    //float Kbpermillis = bytespermillis/1024; // kb per millis
    //float Kbpersecond = Kbpermillis*1000; // kb per second
    Serial.printf(" [OK][Downloaded %d KB at %d KB/s]\n", (httpSize/1024), (int)( bytespermillis / 1024.0 * 1000.0 ) );
  } else {
    Serial.println("[OK] Copy done...");
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
    Serial.println("[HTTP] GET... failed");
    http.end();
    return;
  }
  if(httpCode != HTTP_CODE_OK) {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str()); 
    http.end();
  }
  String payload = http.getString();
  http.end();
  #if ARDUINOJSON_VERSION_MAJOR==6
    DynamicJsonDocument jsonBuffer(4096);
    DeserializationError error = deserializeJson(jsonBuffer, payload);
    if (error) {
      Serial.println(F("JSON Parsing failed!"));
      delay(10000);
      return;
    }
    JsonObject root = jsonBuffer.as<JsonObject>();
  #else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      Serial.println(F("JSON Parsing failed!"));
      delay(10000);
      return;
    }
  #endif
  uint16_t appsCount = root["apps_count"].as<uint16_t>();
  if(appsCount!=1) {
    Serial.println("appsCount misenumeration");
    return;
  }
  String base_url = root["base_url"].as<String>();
  tft.setTextColor( WHITE, BLACK );
  uint16_t assets_count = root["apps"][0]["json_meta"]["assets_count"].as<uint16_t>();
  for(uint16_t i=0;i<assets_count;i++) {
    tft.setCursor(0,i*20);
    String filePath     = root["apps"][0]["json_meta"]["assets"][i]["path"].as<String>();
    String fileName     = root["apps"][0]["json_meta"]["assets"][i]["name"].as<String>();
    uint32_t remoteTime = root["apps"][0]["json_meta"]["assets"][i]["created_at"].as<uint32_t>();
    size_t appSize      = root["apps"][0]["json_meta"]["assets"][i]["size"].as<size_t>();
    size_t mySize       = 0;
    
    if(M5_FS.exists(filePath + fileName)) {
      File myFile = M5_FS.open(filePath + fileName);
      mySize = myFile.size();
      time_t localTime = myFile.getLastWrite();
      myFile.close();

      if( remoteTime > localTime ) {
        // remote is fresher than local, update!
        Serial.printf("[%d > %d] ", remoteTime, localTime);
        Serial.print( WGET_UPDATING );
        tft.print( WGET_UPDATING );
      } else {
        // local is fresher than remote, skip!
        Serial.printf("[%d < %d]", remoteTime, localTime);
        Serial.println(String( WGET_SKIPPING ) + fileName);
        tft.print( WGET_SKIPPING );
        tft.println(fileName);
        continue;
      }
      /*
      if(mySize == appSize) {
        Serial.println(String( WGET_SKIPPING ) + fileName);
        tft.print( WGET_SKIPPING );
        tft.println(fileName);
        continue;
      } else {
        Serial.print( WGET_UPDATING );
        tft.print( WGET_UPDATING );
      }
      */
    } else {
      Serial.print( WGET_CREATING );
      tft.print( WGET_CREATING );
    }
    Serial.printf("%s ( L:%d / R:%d) \n", fileName.c_str(), mySize, appSize);
    tft.println( fileName );
    wget (base_url + filePath + fileName, filePath + fileName, phpsecure_ca);
    uint16_t myprogress = progress + (i* float(progress_modulo/assets_count));
    printProgress(myprogress);
  }

}



void syncAppRegistry(String API_URL, const char* ca) {
  http.setReuse(true);
  if ( API_URL.startsWith("https://") ) {
    http.begin(API_URL + API_ENDPOINT, ca);
  } else {
    http.begin(API_URL + API_ENDPOINT );
  }
  int httpCode = http.GET();
  if(httpCode <= 0) {
    http.end();
    Serial.printf("[HTTP] GET... failed");
    delay(10000);
    return;
  }
  Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  if(httpCode != HTTP_CODE_OK) {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str()); 
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
      Serial.println(F("JSON Parsing failed!"));
      delay(10000);
      return;
    }
    JsonObject root = jsonBuffer.as<JsonObject>();
  #else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      Serial.println(F("JSON Parsing failed!"));
      delay(10000);
      return;
    }
  #endif
  //Serial.println("Parsed JSON");
  appsCount = root["apps_count"].as<uint16_t>();
  if( appsCount==0 ) {
    Serial.println("No apps found, aborting");
    delay(10000);
    return;
  }
  progress_modulo = 100/appsCount;
  String base_url = root["base_url"].as<String>();
  Serial.println("Found " + String(appsCount) + " apps at " + base_url);
  for(uint16_t i=0;i<appsCount;i++) {
    String appName = root["apps"][i]["name"].as<String>();
    String appURL  = API_URL + appName + ".json";
    progress = float(i*progress_modulo);
    
    tft.clear();
    
    printProgress(progress);
    Serial.print(appName);
    Serial.print(" ");
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
    Serial.println( DEBUG_DIROPEN_FAILED );
    return;
  }
  if(!root.isDirectory()){
    Serial.println( DEBUG_NOTADIR );
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
