#pragma once

#include "../misc/core.h"
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/

namespace UIDo
{
  unsigned long MsBeforeSleep  = MS_BEFORE_SLEEP;
  unsigned long lastcheck      = millis(); // timer check
  unsigned long lastpush       = millis(); // keypad/keyboard activity
  uint8_t brightness = MAX_BRIGHTNESS; // used for fadeout before sleep

  void gotoSleep();
  void clearTLS();
  void clearApps();
  void setRootMenu();
  void setMyAppsMenu();
  void setStoreMenu();
  void setNtpServer();
  void setTimezone( float tz );
  void setDst( bool set );
  void downloadCatalog();
  #if !defined FS_CAN_CREATE_PATH
    void doFSChecks();
  #endif
  void checkSleepTimer();
  void deleteApp();
  void deleteApp( const char* appName );
  void removeHiddenApp();
  void addHiddenApp();
  void installApp();
  void installApp( const char* appName );
  void downloadCatalog();
  void BtnA();
  void BtnB();
  void BtnC();
  void idle();
  void modal();
};

namespace UIDraw
{
  void drawBrowseAppsMenu();
  void drawStatusBar();
  void drawDownloaderMenu( const char* title = nullptr, const char* body = nullptr );
  void drawList( bool renderButtons = false );
  void drawRegistryMenu();
};

namespace UILists
{
  void buildBrowseAppsMenu();
  void buildMyAppsMenu();
  void buildStoreMenu();
  void buildRootMenu();
  void buildNtpMenu();
  void buildHiddenAppList();
};

enum AppJSONType
{
  JSON_LOCAL, // local json type, short version with authorName/width/height/projectURL/credits
  JSON_REMOTE // remote json type, long version with meta assets
};

enum AppType
{
  REG_UNKNOWN, // default when unscanned
  REG_LOCAL,   // in registry and installed
  REG_REMOTE,  // in registry but not installed
  REG_HIDDEN,  // in registry and hidden
  NOREG        // not in resistry but present
};

struct AppAsset
{
  String assetFullPath; //: "/jpg/9axis_data_publisher.jpg"
  String name;          //: "9axis_data_publisher.jpg"
  String path;          //: "/jpg/"
  String assetSha256Sum;
  size_t size;
  time_t created_at;
};

namespace AppRenderer
{
  uint16_t AppInfoPosY = 46;
  uint16_t AppInfoPosX = LISTITEM_OFFSETX;
  uint32_t cycleid = 0;
  uint32_t cbdelay = 5000;  // msec cycle per callback
  bool cycleanimation = false;

  struct AppInfo
  {
    LGFX* _gfx;
    String appNameStr;
    String authorNameStr = NOT_IN_REGISTRY;
    String projectURLStr;
    String creditsStr;
    String descriptionStr;
    AppType type;
    std::vector<AppAsset> assets;
    bool has_app_image;
    const char* assets_folder;
    size_t binSize;
    size_t packageSize;
    size_t assetsCount;
    time_t rawtime;
    void clear();
    void parseAssets( JsonObject root );
    void draw();
    uint32_t lastrender;
  };
  AppInfo appInfo;
};


namespace UIShow
{
  using namespace AppRenderer;

  void updateCheckShowAppImage();
  void updateMetaShowAppImage();
  void showDeleteAppImage();
  void handleModalAction( AppInfo * appInfo );

  void showAppInfo();
  void scrollAppInfo();
  void getAppInfo( AppInfo *appInfo, AppJSONType jsonType );
  void showNTPImage();
  void showAppImage();
  void showAppImage( const char* prefix, const char* suffix );
  void cycleAppAssets();
};
