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

//

#pragma once

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "../misc/compile_time.h" // for app watermarking & user-agent customization
#include "../misc/config.h"

namespace NTP
{
  struct Server
  {
    const char* name;
    const char* addr;
  };

  // Timezone is using a float because Newfoundland, India, Iran, Afghanistan, Myanmar, Sri Lanka, the Marquesas,
  // as well as parts of Australia use half-hour deviations from standard time, also some nations such as Nepal
  // and some provinces such as the Chatham Islands of New Zealand, use quarter-hour deviations.
  float timezone = 0; // UTC
  uint8_t daysavetime = 1; // UTC + 1
  const char* defaultServer = "pool.ntp.org";
  uint8_t currentServer = 0;

  void setTimezone( float tz );
  void setDst( bool set );
  void setServer( uint8_t id );
  void loadPrefServer();

  const Server Servers[] =
  {
    { "Global",        "pool.ntp.org" },
    { "Africa",        "africa.pool.ntp.org" },
    { "Asia",          "asia.pool.ntp.org" },
    { "Europe",        "europe.pool.ntp.org" },
    { "North America", "north-america.pool.ntp.org" },
    { "Oceania",       "oceania.pool.ntp.org" },
    { "South America", "south-america.pool.ntp.org" },
  };

};


namespace Downloader
{

  HTTPClient http;
  // interesting http headers to watch for this module
  const char * headerKeys[] = {"location", "redirect", "Content-Type", "Content-Length", "Content-Disposition" };
  const size_t numberOfHeaders = 5;


  struct URLParts
  {
    String url;
    String protocol;
    String host;
    String port;
    String auth;
    String uri;
  };


  URLParts parseURL( String url );
  URLParts parseURL( const char* url );

  const String _ds = "-", _is = " ", _ts = ":"; // for iso datetime
  const String _sdp = "HTTPClient (SDU-", _sdc = "+Chimera-Core-", _sds = ", ", _sde = ")"; // for user agent

  // This sketch build date/time in iso format
  const String ISODateTime = __TIME_YEARS__+_ds+__TIME_MONTH__+_ds+__TIME_DAYS__+_is+__TIME_HOURS__+_ts+__TIME_MINUTES__+_ts+__TIME_SECONDS__;
  // A comprehensive user agent to provide some hardware/software identity to the remote registry
  const String UserAgent   = PLATFORM_NAME+_sdp+M5_SD_UPDATER_VERSION+_sdc+CHIMERA_CORE_VERSION+_sds+ISODateTime+_sde;

  String gzCatalogURL = "";
  String baseCatalogURL = "";

  // Tiny buffer shared by HTTP and sha256 sum
  size_t sizeOfTinyBuff = 512; // smaller is better because sha256 hashing happens between reads
  uint8_t *tinyBuff = nullptr;

  bool wifisetup = false;
  bool ntpsetup  = false;
  bool done      = false;

  uint8_t progress = 0;
  float progress_modulo = 0;

  uint8_t shaResult[32];
  static String shaResultStr = "****************************************************************"; // cheap malloc: any string is good as long as it's 64 chars

  int tlserrors = 0;
  int jsonerrors = 0;
  int downloadererrors = 0;
  int updatedfiles = 0;
  int newfiles = 0;
  int checkedfiles = 0;

  void httpSetup();
  bool tinyBuffInit();

  void sha_sum_to_str();
  void sha256_sum(const char* fileName);
  void sha256_sum( String fileName );

  bool wget( const char* url, const char* path );
  bool wget( String bin_url, String outputFile );
  bool wget( String bin_url, const char* outputFile );
  bool wget( const char* bin_url, String outputFile );

  WiFiClient *wgetptr( WiFiClientSecure *client, const char* url, const char *cert = nullptr );
  #if ARDUHAL_LOG_LEVEL < ARDUHAL_LOG_LEVEL_DEBUG
    void WiFiEvent(WiFiEvent_t event);
  #endif
  void disableWiFi();
  void enableWiFi();
  void enableNTP();
  bool wifiSetupWorked();

  void registryFetch( AppRegistry registry, String appRegistryLocalFile = "" );

  bool downloadGzCatalog();

  bool downloadApp( String appName );

};
