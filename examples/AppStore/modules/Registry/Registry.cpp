#pragma once

#include "Registry.hpp"
#include "../Downloader/Downloader.hpp"


void AppRegistry::init()
{
  masterChannel.init();
  unstableChannel.init();
  if( pref_default_channel == REGISTRY_MASTER ) {
    defaultChannel = masterChannel;
  } else {
    defaultChannel = unstableChannel;
  }
  print();
}


void AppRegistry::print()
{
  log_i("\nRegistry infos:\n\tname: %s\n\tdescription: %s\n\turl: %s\n\tpref_default_channel: %s",
    name.c_str(),
    description.c_str(),
    url.c_str(),
    pref_default_channel.c_str()
  );
  masterChannel.print();
  unstableChannel.print();
  defaultChannel.print();
}



void AppRegistryItem::init()
{
  api_cert_provider_url_http  = "http://" + api_host + api_path + api_cert_path;
  api_cert_provider_url_https = "https://" + api_host + api_path + api_cert_path;
  api_url_https               = "https://" + api_host + api_path + updater_path;
  api_url_http                = "http://" + api_host + api_path + updater_path;
}

void AppRegistryItem::print()
{
  log_i("\nChannel '%s' infos:\n\tdescription: %s\n\turl: %s\n\tapi_host: %s\n\tapi_path: %s\n\tapi_cert_path: %s\n\tupdater_path: %s\n\tcatalog_endpoint: %s\n\tapi_cert_provider_url_http: %s\n\tapi_url_https: %s\n\tapi_url_http: %s",
    name.c_str(),
    description.c_str(),
    url.c_str(),
    api_host.c_str(),
    api_path.c_str(),
    api_cert_path.c_str(),
    updater_path.c_str(),
    catalog_endpoint.c_str(),
    api_cert_provider_url_https.c_str(),
    api_url_https.c_str(),
    api_url_http.c_str()
  );
}



namespace RegistryUtils
{
  using namespace Downloader;


  void setJsonChannelItem( JsonObject json, AppRegistryItem reg )
  {
    json["name"]         = reg.name;
    json["description"]  = reg.description;
    json["url"]          = reg.url;
    json["api_host"]     = reg.api_host;
    json["api_path"]     = reg.api_path;
    json["cert_path"]    = reg.api_cert_path;
    json["updater_path"] = reg.updater_path;
    json["endpoint"]     = reg.catalog_endpoint;
  }


  void registrySave( AppRegistry registry, String appRegistryLocalFile )
  {
    URLParts urlParts = parseURL( registry.url );
    if( appRegistryLocalFile == "" ) {
      log_d("Will attempt to create/save %s", appRegistryLocalFile.c_str() );
      appRegistryLocalFile = appRegistryFolder + PATH_SEPARATOR + urlParts.host + EXT_json;
    }

    DynamicJsonDocument jsonRegistryBuffer(2048);
    if( jsonRegistryBuffer.capacity() == 0 ) {
      log_e("ArduinoJSON failed to allocate 2kb");
      return;
    }

    if( M5_FS.exists( appRegistryLocalFile ) ) {
      log_d("Removing %s before writing", appRegistryLocalFile.c_str());
      M5_FS.remove( appRegistryLocalFile );
    }
    // Open file for writing
    #if defined FS_CAN_CREATE_PATH
      File file = M5_FS.open( appRegistryLocalFile, FILE_WRITE, true );
    #else
      File file = M5_FS.open( appRegistryLocalFile, FILE_WRITE );
    #endif
    if (!file) {
      log_e("Failed to create file %s", appRegistryLocalFile.c_str());
      return;
    }

    JsonObject channels            = jsonRegistryBuffer.createNestedObject("channels");
    JsonObject masterChannelJson   = channels.createNestedObject(REGISTRY_MASTER);
    JsonObject unstableChannelJson = channels.createNestedObject(REGISTRY_UNSTABLE);

    setJsonChannelItem( masterChannelJson,   registry.masterChannel );
    setJsonChannelItem( unstableChannelJson, registry.unstableChannel );

    jsonRegistryBuffer["name"]                 = registry.name;
    jsonRegistryBuffer["description"]          = registry.description;
    jsonRegistryBuffer["url"]                  = registry.url;
    jsonRegistryBuffer["pref_default_channel"] = registry.pref_default_channel;

    log_i("Created json:");
    // serializeJsonPretty(jsonRegistryBuffer, Serial);

    if (serializeJson(jsonRegistryBuffer, file) == 0) {
      log_e( "Failed to write to file %s", appRegistryLocalFile.c_str() );
    } else {
      log_i ("Successfully created %s", appRegistryLocalFile.c_str() );
    }
    file.close();
  }


  bool isValidRegistryChannel( JsonVariant json )
  {
    return( json["name"].as<String>() !=""
        && json["description"].as<String>() !=""
        && json["url"].as<String>().startsWith("http")
        && json["api_host"].as<String>() !=""
        && json["api_path"].as<String>() !=""
        && json["cert_path"].as<String>() !=""
        && json["updater_path"].as<String>() !=""
        && json["endpoint"].as<String>() !=""
    );
  }


  AppRegistryItem getJsonChannel( JsonVariant jsonChannels, const char* channel )
  {
    return {
      String(channel),
      jsonChannels[channel]["description"].as<String>(),
      jsonChannels[channel]["url"].as<String>(),
      jsonChannels[channel]["api_host"].as<String>(),
      jsonChannels[channel]["api_path"].as<String>(),
      jsonChannels[channel]["cert_path"].as<String>(),
      jsonChannels[channel]["updater_path"].as<String>(),
      jsonChannels[channel]["endpoint"].as<String>()
    };
  }


  AppRegistry init( String appRegistryLocalFile )
  {
    if( appRegistryLocalFile == "" ) {
      appRegistryLocalFile = appRegistryFolder + PATH_SEPARATOR + appRegistryDefaultName;
    }
    log_i("Opening channel file: %s", appRegistryLocalFile.c_str());

    if( !M5_FS.exists( appRegistryLocalFile ) ) {
      // create file from registry default template, return template
      log_i("Registry file %s does not exist, creating from firmware defaults", appRegistryLocalFile.c_str() );
      registrySave( defaultAppRegistry, appRegistryFolder + PATH_SEPARATOR + appRegistryDefaultName );
      defaultAppRegistry.init();
      return defaultAppRegistry;
    }

    // load registry profiles from file
    File file = M5_FS.open( appRegistryLocalFile );

    DynamicJsonDocument jsonRegistryBuffer(2048);
    DeserializationError error = deserializeJson( jsonRegistryBuffer, file );
    file.close();
    if (error) {
      log_e("JSON Error while reading registry file %s", appRegistryLocalFile.c_str() );
      defaultAppRegistry.init();
      return defaultAppRegistry;
    }

    JsonObject root = jsonRegistryBuffer.as<JsonObject>();
    if ( root.isNull() ) {
      log_w("Registry file %s has empty JSON", appRegistryLocalFile.c_str() );
      defaultAppRegistry.init();
      return defaultAppRegistry;
    }

    if( !isValidRegistryChannel( root["channels"][REGISTRY_MASTER] ) ) {
      // bad master item
      log_w("%s", "Bad master channel in JSON file");
      defaultAppRegistry.init();
      return defaultAppRegistry;
    }

    if( !isValidRegistryChannel( root["channels"][REGISTRY_UNSTABLE] ) ) {
      // bad master item
      log_w("%s", "Bad unstable channel in JSON file");
      defaultAppRegistry.init();
      return defaultAppRegistry;
    }

    if( root["name"].as<String>() == ""
      || root["description"].as<String>() == ""
      || root["url"].as<String>() == "" ) {
      log_w("%s", "Bad channel meta in JSON file");
      defaultAppRegistry.init();
      return defaultAppRegistry;
    }

    String SDUpdaterChannelNameStr    = "";
    if( !root["pref_default_channel"].isNull() && root["pref_default_channel"].as<String>() != "" ) {
      // inherit from json
      SDUpdaterChannelNameStr = root["pref_default_channel"].as<String>();
    } else {
      // assign default
      SDUpdaterChannelNameStr = REGISTRY_MASTER;
    }

    AppRegistry appRegistry = {
      root["name"].as<String>(),
      root["description"].as<String>(),
      root["url"].as<String>(),
      SDUpdaterChannelNameStr, // default channel
      getJsonChannel( root["channels"], REGISTRY_MASTER ),
      getJsonChannel( root["channels"], REGISTRY_UNSTABLE )
    };
    appRegistry.init();
    return appRegistry;
  }

};
