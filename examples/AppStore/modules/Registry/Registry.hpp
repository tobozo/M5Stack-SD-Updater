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

// load the registry information this launcher is attached to
#pragma once

#include <ArduinoJson.h>
#include "../misc/config.h"


// registry
class AppRegistryItem
{
  public:
    String name;
    String description;
    String url;
    String api_host;
    String api_path;
    String api_cert_path;
    String updater_path;
    String catalog_endpoint;
    String api_cert_provider_url_http;
    String api_cert_provider_url_https;
    String api_url_https;
    String api_url_http;
    void init() ;
    void print();
};

class AppRegistry
{
  public:
    String name;
    String description;
    String url;
    String pref_default_channel; // local option for SDUpdater use only
    AppRegistryItem masterChannel;
    AppRegistryItem unstableChannel;
    AppRegistryItem defaultChannel;
    void init();
    void print();
};


namespace RegistryUtils
{
  const String appRegistryFolder = "/.registry";
  const String appRegistryDefaultName = "default.json";
  void setJsonChannelItem( JsonObject json, AppRegistryItem reg );
  void registrySave( AppRegistry registry, String appRegistryLocalFile = "" );
  bool isValidRegistryChannel( JsonVariant json );
  AppRegistryItem getJsonChannel( JsonVariant jsonChannels, const char* channel );
  AppRegistry init( String appRegistryLocalFile = "" );

  AppRegistryItem defaultMasterChannel = {
    REGISTRY_MASTER, // "master"
    DEFAULT_MASTER_DESC,
    DEFAULT_MASTER_URL,
    DEFAULT_MASTER_API_HOST,
    DEFAULT_MASTER_API_PATH,
    DEFAULT_MASTER_API_CERT_PATH,
    DEFAULT_MASTER_UPDATER_PATH,
    DEFAULT_MASTER_CATALOG_ENDPOINT
  };

  AppRegistryItem defaultUnstableChannel = {
    REGISTRY_UNSTABLE, // "unstable"
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
    DEFAULT_REGISTRY_URL,
    DEFAULT_REGISTRY_CHANNEL,
    defaultMasterChannel,
    defaultUnstableChannel
  };

};

