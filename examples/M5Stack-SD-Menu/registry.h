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
#include "registry.default.h"

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
    log_i("\n\tname: %s\n\tdescription: %s\n\turl: %s\n\tapi_host: %s\n\tapi_path: %s\n\tapi_cert_path: %s\n\tupdater_path: %s\n\tcatalog_endpoint: %s\n\tapi_cert_provider_url_http: %s\n\tapi_url_https: %s\n\tapi_url_http: %s\n\n",
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
    log_i("%s", "Registry infos:");
    log_i("\n\tname: %s\n\tdescription: %s\n\turl: %s\n\tpref_default_channel: %s\n\n",
      name.c_str(),
      description.c_str(),
      url.c_str(),
      pref_default_channel.c_str()
    );
    log_i("%s", "Master channel infos:");
    masterChannel.print();
    log_i("%s", "Unstable channel infos:");
    unstableChannel.print();
    log_i("%s", "Default channel infos:");
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
