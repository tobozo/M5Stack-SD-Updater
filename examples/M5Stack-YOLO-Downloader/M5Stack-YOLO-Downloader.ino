/*
 * YOLO Downloader for M5Stack-SD-Updater 
 * Copyright (2018) tobozo
 * 
 * Requires: a valid WiFi connection (use an external app for that
 * or connect as WIFI_STA at least once from your M5Stack).
 * 
 * Downloads unsigned binaries using an HTTPS connection
 * from an untrusted source and saves the binaries onto the MicroSD
 * for later execution. It sounds cooler than having to copy the 
 * binaries manually onto the SD Card but from a security point of
 * view it's totally creepy.
 * 
 * You run this at your own risks!!
 * 
 * Only a size comparison is made to decide if an existing file needs 
 * to be overwritten or left alone.
 * 
 * Risks:
 *  - MITM attacks
 *  - DNS spoofing
 *  - ~~HTTP spoofing~~ mitigated by TLS, see MITM instead
 *  - Binary spoofing
 *  - Executing arbitrary binary in the M5Stack's context, leading to:
 *    - beaconing
 *    - being remotely updateable
 *    - exposing the registered WiFi connections
 *    - leaking of actual WiFi credentials
 *    
 * Mitigations:
 *  - run a sketch with WiFi.disconnect() to clear current credentials
 *  - create expendable, temporary WiFi credentials
 *  - isolate the traffic from your network
 *  - use a public WiFi
 *  - revoke WiFi credentials right after the download is done
 *  - burn this sketch after reading
 * 
 * ToDo's for security (will probably never happen)
 *  - Use HTTPS client to download binaries
 *  - Authenticate both client and server
 *  - Encrypt and sign Binaries
 *  - Check signature & Decrypt binaries on the fly using hardware AES
 *
 */
 
#include "certificates.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <M5Stack.h>
#include "M5StackUpdater.h"
#include "assets.h"

SDUpdater sdUpdater;
HTTPClient http;

const uint8_t extensionsCount = 4; // change this if you add / remove an extension
String allowedExtensions[extensionsCount] = {
    // do NOT remove jpg and json or the menu will crash !!!
    "jpg", "json", "mod", "mp3"
};

const String API_URL = "https://phpsecu.re/m5stack/sd-updater/";

bool done = false;
uint16_t appsCount = 0;
uint8_t progress = 0;
float progress_modulo = 0;




/*
void testTLS() {
  String TLS_URL = "https://github.com/espressif/arduino-esp32/blob/e3a5ae439bb94ae13bd970d9484d4665e1df4972/libraries/HTTPClient/examples/BasicHttpClient/BasicHttpClient.ino";
  http.begin(TLS_URL, github_ca);
  Serial.print("[HTTPS] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  
  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
  
      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
      }
  } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}*/



void printProgress(uint16_t progress) {
  uint16_t x = M5.Lcd.getCursorX();
  uint16_t y = M5.Lcd.getCursorX();
  M5.Lcd.setCursor(50, 220);
  M5.Lcd.print("Overall progress: ");
  M5.Lcd.print(String(progress));
  M5.Lcd.print("%");
  M5.Lcd.setCursor(x, y);
}


void wget (String bin_url, String appName, const char* &ca) {
  //M5.Lcd.setCursor(0,0);
  Serial.println("Will download " + bin_url + " and save to SD as " + appName);
  http.begin(bin_url, ca);
  
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
  File myFile = SD.open(appName, FILE_WRITE);
  if(!myFile) {
    Serial.println("Failed to open " + appName + " for writing, aborting");
    http.end();
    myFile.close();
    return;
  }
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
  Serial.println("Copy done...");
  http.end();
}



void getApp(String appURL, const char* &ca) {
  http.begin(appURL, ca);
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
    DynamicJsonDocument jsonBuffer;
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
  uint16_t assets_count = root["apps"][0]["json_meta"]["assets_count"].as<uint16_t>();
  for(uint16_t i=0;i<assets_count;i++) {
    M5.Lcd.setCursor(0,i*20);
    String filePath = root["apps"][0]["json_meta"]["assets"][i]["path"].as<String>();
    String fileName = root["apps"][0]["json_meta"]["assets"][i]["name"].as<String>();
    size_t appSize  = root["apps"][0]["json_meta"]["assets"][i]["size"].as<size_t>();
    
    if(SD.exists(filePath + fileName) && false) {
      File myFile = SD.open(filePath + fileName);
      size_t mySize = myFile.size();
      myFile.close();
      if(mySize == appSize) {
        Serial.println("Skipping " + fileName);
        M5.Lcd.print("Skipping ");
        M5.Lcd.println(fileName);
        continue;
      } else {
        Serial.print("Updating ");
        M5.Lcd.print("Updating ");
      }
    } else {
      Serial.print("Creating ");
      M5.Lcd.print("Creating ");
    }
    Serial.println(fileName);
    M5.Lcd.println(fileName);
    wget (base_url + filePath + fileName, filePath + fileName, phpsecure_ca);
    uint16_t myprogress = progress + (i* float(progress_modulo/assets_count));
    printProgress(myprogress);
  }

}



void syncAppRegistry(String API_URL, const char* ca) {

  wget("https://github.com/tobozo/M5Stack-SD-Updater/releases/download/v0.3.0/SD-Apps-Folder.zip", "/SD-Apps-Folder.zip", github_ca);

  http.setReuse(true);
  http.begin(API_URL + "catalog.json", ca);
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
    DynamicJsonDocument jsonBuffer;
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
  progress_modulo = 100/appsCount;
  String base_url = root["base_url"].as<String>();
  Serial.println("Found " + String(appsCount) + " apps at " + base_url);
  for(uint16_t i=0;i<appsCount;i++) {
    String appName = root["apps"][i]["name"].as<String>();
    String appURL  = API_URL + appName + ".json";
    progress = float(i*progress_modulo);
    
    M5.Lcd.clear();
    
    printProgress(progress);
    Serial.print(appName);
    Serial.print(" ");
    getApp( appURL, phpsecure_ca );
    
  }
  done = true;
  M5.Lcd.clear();
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("Synch finished");
  delay(1000);
  updateFromFS(SD);
  ESP.restart();
}




void setup() {
  unsigned long startup = millis();
  Serial.begin(115200);
  M5.begin();
  Wire.begin();
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  unsigned long lastcheck = millis();
  bool toggle = true;

  while(!SD.begin(TFCARD_CS_PIN)) {
    // TODO: make a more fancy animation
    unsigned long now = millis();
    toggle = !toggle;
    uint16_t color = toggle ? BLACK : WHITE;
    M5.Lcd.setCursor(10,100);
    M5.Lcd.setTextColor(color);
    M5.Lcd.print("INSERT SD");
    if(toggle) {
      M5.Lcd.drawJpg(disk01_jpg, 1775, 160, 100);
      delay(300);
    } else {
      M5.Lcd.drawJpg(disk00_jpg, 1775, 160, 100);
      delay(500);
    }
    // go to sleep after a minute, no need to hammer the SD Card reader
    if(lastcheck+60000<now) {
      Serial.println("Will go to sleep");
      M5.setWakeupButton(BUTTON_B_PIN);
      M5.powerOFF();
    }
  }

  for(uint8_t i=0;i<extensionsCount;i++) {
    String dir = "/" + allowedExtensions[i];
    if(!SD.exists(dir)) {
      SD.mkdir(dir);
    }
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(); // set SSID/PASS from another app (i.e. WiFi Manager) and reload this app

  M5.Lcd.clear();
  M5.Lcd.setCursor(0,0);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.println("Waiting for WiFi to connect");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
    if(startup + 30000 < millis()) {
      Serial.println("Been waiting too long for WiFi, will restart");
      M5.Lcd.println("Waiting too long, will restart");
      delay(1000);
      ESP.restart();
    }
  }
  M5.Lcd.setTextFont(2);
  M5.Lcd.println("connected to wifi");
  Serial.println("connected to wifi");
  if(SD.begin()) {
    Serial.println("SD detected");
    M5.Lcd.println("SD detected");
  }

  //testTLS();

  
  //http.setReuse(true);
}

void loop() {
  if(!done) {
    syncAppRegistry(API_URL, phpsecure_ca);
  }
}
