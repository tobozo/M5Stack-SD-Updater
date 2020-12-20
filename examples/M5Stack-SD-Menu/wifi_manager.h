#include <ESPmDNS.h>
#include <WiFiClient.h>
#include "WebServer.h"


const IPAddress apIP(192, 168, 4, 1);
const char* apSSID = "M5STACK_SETUP";
boolean settingMode;
String ssidList;
String wifi_ssid;
String wifi_password;

// DNSServer dnsServer;
WebServer webServer(80);
Preferences preferences;

String makePage(String title, String contents);
String urlDecode(String input);
void setupMode();
void startWebServer();
boolean checkConnection();
boolean restoreConfig();



boolean restoreConfig() {
  wifi_ssid = preferences.getString("WIFI_SSID");
  wifi_password = preferences.getString("WIFI_PASSWD");
  if( wifi_ssid!="" && wifi_password!="" ) {
    Serial.print("WIFI-SSID: ");
    tft.print("WIFI-SSID: ");
    Serial.println(wifi_ssid);
    tft.println(wifi_ssid);
    Serial.print("WIFI-PASSWD: ");
    tft.print("WIFI-PASSWD: ");
    Serial.println(wifi_password);
    tft.println(wifi_password);
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    return true;
  } else {
    tft.print("No config to restore");
    WiFi.begin();
    return false;
  }
}

boolean checkConnection() {
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  tft.print("Waiting for Wi-Fi connection");
  while ( count < 30 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      tft.println();
      Serial.println("Connected!");
      tft.println("Connected!");
      return (true);
    }
    delay(500);
    Serial.print(".");
    tft.print(".");
    count++;
  }
  Serial.println("Timed out.");
  tft.println("Timed out.");
  return false;
}

void startWebServer() {
  if (settingMode) {
    Serial.print("Starting Web Server at ");
    tft.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());
    tft.println(WiFi.softAPIP());
    webServer.on("/settings", []() {
      String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
      s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
      s += ssidList;
      s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });
    webServer.on("/setap", []() {
      String ssid = urlDecode(webServer.arg("ssid"));
      Serial.print("SSID: ");
      tft.print("SSID: ");
      Serial.println(ssid);
      tft.println(ssid);
      String pass = urlDecode(webServer.arg("pass"));
      Serial.print("Password: ");
      tft.print("Password: ");
      Serial.println(pass);
      tft.println(pass);
      Serial.println("Writing SSID to EEPROM...");
      tft.println("Writing SSID to EEPROM...");

      // Store wifi config
      Serial.println("Writing Password to nvr...");
      tft.println("Writing Password to nvr...");
      preferences.putString("WIFI_SSID", ssid);
      preferences.putString("WIFI_PASSWD", pass);

      Serial.println("Write nvr done!");
      tft.println("Write nvr done!");
      String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
      s += ssid;
      s += "\" after the restart.";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
    });
    webServer.onNotFound([]() {
      String s = "<h1>AP mode</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("AP mode", s));
    });
  }
  else {
    Serial.print("Starting Web Server at ");
    tft.print("Starting Web Server at ");
    Serial.println(WiFi.localIP());
    tft.println(WiFi.localIP());
    webServer.on("/", []() {
      String s = "<h1>STA mode</h1><p><a href=\"/reset\">Reset Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });
    webServer.on("/reset", []() {
      // reset the wifi config
      preferences.remove("WIFI_SSID");
      preferences.remove("WIFI_PASSWD");
      String s = "<h1>Wi-Fi settings was reset.</h1><p>Please reset device.</p>";
      webServer.send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
    });
  }
  webServer.begin();
}

void setupMode() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  tft.println("");
  for (int i = 0; i < n; ++i) {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  WiFi.mode(WIFI_MODE_AP);
  // WiFi.softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
  // WiFi.softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0);
  // dnsServer.start(53, "*", apIP);
  startWebServer();
  Serial.print("Starting Access Point at \"");
  tft.print("Starting Access Point at \"");
  Serial.print(apSSID);
  tft.print(apSSID);
  Serial.println("\"");
  tft.println("\"");
}

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}



void wifiManagerSetup() {
  preferences.begin("wifi-config");
  tft.fillScreen( TFT_BLACK );

  delay(10);
  if (restoreConfig()) {
    if (checkConnection()) {
      settingMode = false;
      startWebServer();
      return;
    }
  }
  settingMode = true;
  setupMode();
}


void wifiManagerLoop() {
  do {
    webServer.handleClient();
  } while( settingMode == true );
}

