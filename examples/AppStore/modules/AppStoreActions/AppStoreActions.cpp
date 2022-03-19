#pragma once

#include <ArduinoJson.h>
#include "AppStoreActions.hpp"
#include "../AppStoreUI/AppStoreUI.hpp"
#include "../Registry/Registry.hpp"
#include "../FSUtils/FSUtils.hpp"
#include "../Downloader/Downloader.hpp"

extern AppRegistry Registry;
extern LogWindow *Console;


namespace UIDo
{

  using namespace FSUtils;
  using namespace UILists;
  using namespace UIUtils;
  using namespace RegistryUtils;

  void BtnA()
  {
    // Action button
    UI->execBtnA();
  }

  void BtnB()
  {
    // Navigation button
    //UI->pageDown();
    UI->menuUp();
  }

  void BtnC()
  {
    // Navigation button
    UI->menuDown();
  }


  void checkSleepTimer()
  {
    if( lastpush + MsBeforeSleep < millis() ) { // go to sleep if nothing happens for a while
      if( brightness > 1 ) { // slowly dim the screen first
        brightness--;
        if( brightness %10 == 0 ) {
          Serial.print("(\".¬.\") ");
        }
        if( brightness %30 == 0 ) {
          Serial.print(" Yawn... ");
        }
        if( brightness %7 == 0 ) {
          Serial.println(" .zzZzzz. ");
        }
        UI->getGfx()->setBrightness( brightness );
        lastpush = millis() - (MsBeforeSleep - brightness*10); // exponential dimming effect
        return;
      }
      gotoSleep();
    }
  }


  void gotoSleep()
  {
    Serial.println( GOTOSLEEP_MESSAGE );
    #ifdef ARDUINO_M5STACK_Core2
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0); // gpio39 == touch INT
    #else
      #if defined HAS_POWER || defined HAS_IP5306
        // M5Fire / M5Stack / Odroid-Go
        M5.setWakeupButton( BUTTON_B_PIN );
        //M5.powerOFF();
      #endif
    #endif
    delay(100);
    M5.Lcd.fillScreen( UI->getTheme()->BgColor );
    M5.Lcd.sleep();
    M5.Lcd.waitDisplay();
    esp_deep_sleep_start();
  }


  void clearTLS()
  {
    // cleanup /cert/ and /.registry/ folders
    Console = new LogWindow();
    cleanDir( SD_CERT_PATH );
    cleanDir( appRegistryFolder.c_str() );
    delete Console;
    buildRootMenu();
  }


  void clearApps()
  {
    int resp = modalConfirm( MODAL_DELETEALL_TITLE, nullptr, MODAL_DELETEALL_MSG, MENUACTION_APPDELETE, MENU_BTN_CANCEL, MENU_BTN_NO );
    if( resp == HID_BTN_A ) {
      #if !defined HAS_RTC
        setTimeFromLastFSAccess(); // set the time before cleaning up the folder
      #endif
      Console = new LogWindow();
      cleanDir( CATALOG_DIR );
      // cleanDir( ROOT_DIR ); // <<< TODO: filter from app list
      // cleanDir( DIR_jpg ); // <<< TODO: filter from app list
      // cleanDir( DIR_json ); // <<< TODO: filter from app list
      cleanDir( "/tmp/" );
      delete Console;
    }
    buildRootMenu();
  }


  void setNtpServer()
  {
    uint8_t menuId = UI->getListID();
    if( menuId > 0 ) { // first element is "back to root menu"
      NTP::setServer( menuId-1 );
    }
    buildRootMenu();
  }


  void installApp( const char* appName )
  {
    // TODO: confirmation dialog
    if( !Downloader::downloadApp( String(appName) ) ) {
      log_e("Download of %s app failed");
    }
  }

  void installApp()
  {
    MenuActionLabels* tempLabels = UI->getMenuActionLabels();
    installApp( UI->getListItemTitle() );
    UI->setMenuActionLabels( tempLabels );
  }

  void deleteApp( const char* appName )
  {
    drawInfoWindow( String( "Deleting " + String(appName) ).c_str() );
    removeInstalledApp( String( appName ) );
  }

  void deleteApp()
  {
    deleteApp( UI->getListItemTitle() );
  }

  void removeHiddenApp()
  {
    // TODO: open appinfowindow + hide button
    String appName = String( UI->getListItemTitle() );
    drawInfoWindow( String( "Unhiding " + appName).c_str() );
    toggleHiddenApp( UI->getListItemTitle(), false );
  }

  void addHiddenApp()
  {
    // TODO: open appinfowindow + hide button
    String appName = String( UI->getListItemTitle() );
    drawInfoWindow( String( "Hiding " + appName).c_str() );
    toggleHiddenApp( UI->getListItemTitle(), true );
  }


  void downloadCatalog()
  {
    bool loop = true;
    do {
      loop = Downloader::downloadGzCatalog()
            ? false
            : modalConfirm( MODAL_DOWNLOADFAIL_TITLE, nullptr, MODAL_DOWNLOADFAIL_MSG, MENU_BTN_RETRY, MENU_BTN_CANCEL, MENU_BTN_NO ) == HID_BTN_A
      ;
    } while( loop == true );
    buildRootMenu();
  }


  void idle()
  {

  }

  void modal()
  {
    cycleAppAssets();
  }


  #if !defined FS_CAN_CREATE_PATH
    void doFSChecks()
    {
      #if !defined HAS_RTC
        setTimeFromLastFSAccess();
      #endif
      scanDataFolder(); // do SD health checks, create folders
    }
  #endif

};


namespace UIDraw
{

  using namespace MenuItems;
  using namespace UIUtils;
  using namespace Downloader;
  using namespace RegistryUtils;

  void drawDownloaderMenu( const char* title, const char* body )
  {
    UI->windowClr( UI->getTheme()->MenuColor );
    UI->setMenuActionLabels( &RefreshAppsActionButtons );
    UI->drawMenu( false );
    drawStatusBar();
    if( title != nullptr ) {
      drawInfoWindow( title, body );
    }
  }


  void drawList( bool renderButtons )
  {
    if( renderButtons ) {
      UI->drawMenu( false );
      drawStatusBar();
    }
    UI->showList();
  }


  void drawStatusBar()
  {
    UITheme* theme = UI->getTheme();
    LGFX* gfx = UI->getGfx();
    ForkIcon.draw( gfx, 4, (TITLEBAR_HEIGHT-2)/2-ForkIcon.height/2 );
    drawTextShadow(gfx, UI->channel_name, ForkIcon.width+8, (TITLEBAR_HEIGHT-2)/2, theme->TextColor, theme->TextShadowColor, &TomThumb, ML_DATUM );

    if( wifisetup ) {
      int8_t rssiminmaxed = min( (int8_t)-70, max( (int8_t)-50, (int8_t)WiFi.RSSI() ) );
      int16_t rssimapped = map( rssiminmaxed, -50, -70, 1, 5 );
      drawRSSIBar( 290, 4, rssimapped, theme->MenuColor, 2.0 );
    } else {
      SDUpdaterIcon.draw( gfx, 298, 6 );
    }
    if( ntpsetup ) {
      // TODO: draw some ntp icon
    }
  }


  void drawRegistryMenu()
  {

    String _mc = ""; // macro separator
    String modalMessage = _mc + CHANNEL_TOOL_TEXT + "\n"
                        + "\nCurrent: " + Registry.defaultChannel.name
                        + "\nAlternate: " + (Registry.defaultChannel.name == REGISTRY_MASTER ? Registry.unstableChannel.name : Registry.masterChannel.name)
    ;

    int resp = modalConfirm( CHANNEL_TOOL, ROOTACTION_SWITCH, modalMessage.c_str(), DOWNLOADER_MODAL_CHANGE, MENU_BTN_UPDATE, MENU_BTN_CANCEL );
    // choose between updating the JSON or changing the default channel
    switch( resp ) {
      case HID_BTN_A: // pick a channel
        resp = modalConfirm( CHANNEL_CHOOSER, CHANNEL_CHOOSER_PROMPT, CHANNEL_CHOOSER_TEXT, DOWNLOADER_MODAL_CHANGE, MENU_BTN_CANCEL, MENU_BTN_BACK );
        if( resp == HID_BTN_A ) {
          if( Registry.pref_default_channel == REGISTRY_MASTER ) {
            Registry.pref_default_channel = REGISTRY_UNSTABLE;
          } else {
            Registry.pref_default_channel = REGISTRY_MASTER;
          }
          registrySave( Registry, appRegistryFolder + PATH_SEPARATOR + appRegistryDefaultName );
          // TODO: download registry JSON
          //Serial.println("Will reload in 5 sec");
          delay(5000);
          ESP.restart();
        }
      break;
      case HID_BTN_B: // update channel
        resp = modalConfirm( CHANNEL_DOWNLOADER, CHANNEL_DOWNLOADER_PROMPT, CHANNEL_DOWNLOADER_TEXT, MENU_BTN_UPDATE, MENU_BTN_CANCEL, MENU_BTN_BACK );
        if( resp == HID_BTN_A ) {
          registryFetch( Registry, appRegistryFolder + PATH_SEPARATOR + appRegistryDefaultName );
        }
      break;
      default: // run wifi manager ?

      break;
    }
    drawList( true );
  }

};




namespace AppRenderer
{
  using namespace UILists;
  using namespace FSUtils;
  //LGFX* gfx;

  void AppInfo::clear()
  {
    appNameStr = "";
    authorNameStr = NOT_IN_REGISTRY;
    projectURLStr = "";
    descriptionStr = "";
    creditsStr = "";
    type = REG_UNKNOWN;
    assets_folder = nullptr;
    assets.clear();
    binSize = packageSize = assetsCount = rawtime = 0;
    log_v("Cleared");
  }

  void AppInfo::parseAssets( JsonObject root )
  {
    if( assetsCount > 0 ) {
      String appImageShaSum = "";
      String authorImageShaSum = "";
      assets.clear();
      for (JsonVariant value : root["json_meta"]["assets"].as<JsonArray>() ) {
        String assetName      = value["name"].as<String>();//: "9axis_data_publisher.jpg",
        String assetPath      = value["path"].as<String>();//: "/jpg/",
        String assetSha256Sum = value["sha256_sum"].as<String>();
        size_t assetSize      = value["size"].as<size_t>();//: 7215,
        rawtime               = value["created_at"].as<time_t>();
        log_d("Collected rawtime: %d for file %s%s", rawtime, assetPath.c_str(), assetName.c_str() );
        String assetFullPath  = assetPath+assetName;
        packageSize += assetSize;
        if( isBinFile( assetFullPath.c_str() ) ) {
          if( M5_FS.exists( assetFullPath ) ) {
            type = REG_LOCAL;
          }
          binSize = assetSize;
          rawtime = rawtime;//: 1573146468,
        } else {
          if( assetName == appNameStr + EXT_jpg ) {
            // is app image
            appImageShaSum = assetSha256Sum;
          } else if ( assetName == appNameStr + "_gh" + EXT_jpg ) {
            // is author image
            authorImageShaSum = assetSha256Sum;
          }
        }
        assets.push_back({ assetFullPath, assetName, assetPath,  assetSha256Sum, assetSize, rawtime });
      }
      has_app_image = false;
      if( appImageShaSum != "" && authorImageShaSum != "" ) {
        if( appImageShaSum != authorImageShaSum ) {
          log_w("Author image and app image differ!");
          has_app_image = true;
        } else {
          log_w("Author image and app image are similar!");
        }
      } else {
        log_w("Author image and app image shasums are missing!");
      }

    }
  }

  void AppInfo::draw()
  {
    log_v("rendering");
    UI->windowClr();
    UITheme* theme = UI->getTheme();
    AppInfoPosY = 64;

    if( appInfo.appNameStr ) {
      drawTextShadow( _gfx, appInfo.appNameStr.c_str(), _gfx->width()/2, AppInfoPosY-20, theme->TextColor, theme->TextShadowColor, &FreeMono9pt7b, MC_DATUM );
    }

    if( authorNameStr ) {
      String authorString = authorNameStr;
      if( strcmp( NOT_IN_REGISTRY, authorString.c_str() ) !=0 ) {
        showAppImage( assets_folder, "_gh");
        authorString = "By: "+authorString;
      } else {
        UnknownAppIcon.draw( _gfx, theme->assetPosX, theme->assetPosY );
      }
      drawTextShadow( _gfx, authorString.c_str(), AppInfoPosX, AppInfoPosY, theme->TextColor, theme->TextShadowColor, &FreeMono9pt7b, ML_DATUM );
      AppInfoPosY += 21;
    } else { log_v("NO AUTHOR"); }
    if( assetsCount > 0 ) {
      String assetsString = "Assets: " + String( assetsCount );
      drawTextShadow( _gfx, assetsString.c_str(), AppInfoPosX, AppInfoPosY, theme->TextColor, theme->TextShadowColor, &FreeMono9pt7b, ML_DATUM );
      AppInfoPosY += 21;
    } else { log_v("NO ASSETS"); }
    if( binSize > 0 ) {
      String sizeString   = "Bin.: " + String(formatBytes( binSize, formatBuffer ));
      drawTextShadow( _gfx, sizeString.c_str(), AppInfoPosX, AppInfoPosY, theme->TextColor, theme->TextShadowColor, &FreeMono9pt7b, ML_DATUM );
      AppInfoPosY += 21;
    } else { log_v("NO BIN SIZE"); }
    if( packageSize > 0 ) {
      String totalSize    = "Tot.: " + String(formatBytes( packageSize, formatBuffer ));
      drawTextShadow( _gfx, totalSize.c_str(), AppInfoPosX, AppInfoPosY, theme->TextColor, theme->TextShadowColor, &FreeMono9pt7b, ML_DATUM );
      AppInfoPosY += 21;
    } else { log_v("NO PACK SIZE"); }
    if( rawtime > 0 ) {
      struct tm * timeinfo;
      //time (&rawtime);
      timeinfo = localtime (&rawtime);
      drawTextShadow( _gfx, "Creation date:", AppInfoPosX, AppInfoPosY, theme->TextColor, theme->TextShadowColor, &FreeMono9pt7b, ML_DATUM );
      AppInfoPosY += 21;

      strftime( formatBuffer, 64, "%F", timeinfo ); // to ISO date format
      String binDateStr = String( formatBuffer );
      drawTextShadow( _gfx, binDateStr.c_str(), AppInfoPosX+12, AppInfoPosY, theme->TextColor, theme->TextShadowColor, &FreeMono9pt7b, ML_DATUM );
      AppInfoPosY += 21;

      strftime( formatBuffer, 64, "%T", timeinfo ); // to ISO time format
      String binTimeStr = String( formatBuffer );
      drawTextShadow( _gfx, binTimeStr.c_str(), AppInfoPosX+12, AppInfoPosY, theme->TextColor, theme->TextShadowColor, &FreeMono9pt7b, ML_DATUM );
    } else { log_v("NO DATETIME"); }
    lastrender = millis();
  }

};



namespace UIShow
{
  using namespace FSUtils;
  using namespace UILists;
  using namespace UIDo;
  using namespace MenuItems;
  using namespace AppRenderer;

  std::vector<onrender_cb_t> callbacks;


  void showAppQrCode()
  {
    UITheme* th=UI->getTheme();
    qrRender(UI->getGfx(), appInfo.projectURLStr, th->assetPosX, th->assetPosY, th->assetWidth, th->assetHeight );
  }


  void showAppAssetImage()
  {
    showAppImage( appInfo.assets_folder, "" );
  }


  void showAppAssetAuthor()
  {
    showAppImage( appInfo.assets_folder, "_gh" );
  }


  void showNTPImage()
  {
    const char* serverNameCstr = UI->getListItemTitle();
    String serverNameStr = String( serverNameCstr );
    String _ms = ""; // macro separator
    String iconPath = CATALOG_DIR + _ms + DIR_png + "NTP-" + serverNameStr + EXT_png;
    UITheme* theme = UI->getTheme();
    RemoteAsset appIcon = { iconPath.c_str(), theme->assetWidth, theme->assetHeight, serverNameCstr };
    drawAssetReveal( &appIcon, UI->getGfx(), theme->assetPosX, theme->assetPosY );
  }



  void updateCheckShowAppImage()
  {
    AppInfo tmpInfo;
    getAppInfo( &tmpInfo, JSON_LOCAL );
    getAppInfo( &appInfo, JSON_REMOTE );
    // "IN REGISTRY" => compare binary size with meta, propose sha256_sum check or update + delete
    showAppImage( "", "");
    UITheme* theme = UI->getTheme();
    LGFX* gfx = UI->getGfx();
    // add icon status as an overlay to the image
    if( appInfo.binSize != tmpInfo.binSize ) {
      UpdateIcon.draw( gfx, theme->assetPosX, theme->assetPosY );
      log_v("Bin sizes differ (local=%d, remote=%d)", tmpInfo.binSize, appInfo.binSize);
    } else {
      log_v("Bin sizes match (%d)", appInfo.binSize);
      CheckIcon.draw( gfx, theme->assetPosX, theme->assetPosY );
    }
  }

  void updateMetaShowAppImage()
  {
    // "BINARY INSTALLED" && "IN REGISTRY" => missing local meta ? => suggest copy meta + delete
    log_d("TODO: check if missing local meta");
    getAppInfo( &appInfo, JSON_REMOTE );
    showAppImage( CATALOG_DIR, "");
    UITheme* theme = UI->getTheme();
    UpdateIcon.draw( UI->getGfx(), theme->assetPosX, theme->assetPosY );
  }

  void showDeleteAppImage()
  {
    String _ms = ""; // for macro separation
    String imageLocal  =                     DIR_jpg + String(UI->getListItemTitle()) + EXT_jpg;
    String imageRemote = CATALOG_DIR + _ms + DIR_jpg + String(UI->getListItemTitle()) + EXT_jpg;
    getAppInfo( &appInfo, JSON_LOCAL );
    UITheme* theme = UI->getTheme();
    RemoteAsset appIcon = { nullptr, theme->assetWidth, theme->assetHeight, UI->getListItemTitle() };
    String imageRender = "";
    if( M5_FS.exists( imageLocal ) ) {
      appIcon.path = imageLocal.c_str();
      log_d("local image");
    } else if( M5_FS.exists( imageRemote ) ) {
      appIcon.path = imageRemote.c_str();
      log_d("remote image");
    } else {
      appIcon = UnknownAppIcon;
      log_d("default image");
    }
    drawAssetReveal( &appIcon, UI->getGfx(), theme->assetPosX, theme->assetPosY );
  }


  void showAppImage()
  {
    showAppImage( UI->getList()->assets_folder, "");
    if( strcmp( UI->getListItemTitle(), appInfo.appNameStr.c_str() ) != 0 ) {
      log_d("AppInfo expired (expecting:%s, got:%s)", UI->getListItemTitle(), appInfo.appNameStr.c_str() );
      getAppInfo( &appInfo, UI->getList()->assets_folder[0] == '0' ? JSON_LOCAL : JSON_REMOTE );
    } else {
      log_d("re-using existing appinfo");
    }
    appInfo.lastrender = millis();
  }


  void showAppImage( const char* prefix, const char* suffix )
  {
    const char* appNameStr      = UI->getListItemTitle();
    const char* menuNameStr     = UI->getListTitle();
    char appImageFile[255] = {0};
    memset( appImageFile, 0, 255 );
    snprintf( appImageFile, 255, "%s" DIR_jpg "%s%s" EXT_jpg, prefix, appNameStr, suffix );
    log_v("Extrapolated Icon Path: '%s' (%d bytes) for '%s' menu item", appImageFile, strlen(appImageFile), menuNameStr );
    UITheme* theme = UI->getTheme();
    RemoteAsset appIcon = { appImageFile, theme->assetWidth, theme->assetHeight, appNameStr };
    drawAssetReveal( &appIcon, UI->getGfx(), theme->assetPosX, theme->assetPosY );
    appInfo.lastrender = millis();
  }


  void getAppInfo( AppInfo *appInfo, AppJSONType jsonType )
  {
    appInfo->clear();
    appInfo->appNameStr    = String( UI->getListItemTitle() );
    appInfo->type          = isHiddenApp( appInfo->appNameStr ) ? REG_HIDDEN : M5_FS.exists( ROOT_DIR+appInfo->appNameStr+EXT_bin ) ? REG_LOCAL : REG_REMOTE;
    appInfo->assets_folder = UI->getList()->assets_folder;
    String assetsFolderStr = jsonType == JSON_LOCAL ? "" : CATALOG_DIR;
    String jsonFile        = assetsFolderStr + DIR_json + appInfo->appNameStr + EXT_json;
    String appImageFile    = assetsFolderStr + DIR_jpg + appInfo->appNameStr + EXT_jpg;
    String binFile         = ROOT_DIR + appInfo->appNameStr + EXT_bin;
    if( M5_FS.exists( binFile ) ) getFileAttrs( binFile.c_str(), &appInfo->binSize, &appInfo->rawtime );
    JsonObject root;
    DynamicJsonDocument jsonBuffer( 8192 );
    if( !getJson( jsonFile.c_str(), root, jsonBuffer ) ) {
      appInfo->type = NOREG;
      log_d("NOREG: '%s' has no JSON", jsonFile.c_str() );
      return;
    }
    if ( root.isNull() ) {
      appInfo->type = NOREG;
      log_e("No parsable JSON in %s file", jsonFile.c_str() );
      return;
    }

    size_t jsonMetaPropsCount;
    fs::File file;
    switch( jsonType ) {
      case JSON_LOCAL:
        if( M5_FS.exists( appImageFile ) ) appInfo->assetsCount++;
        if( M5_FS.exists( jsonFile ) ) appInfo->assetsCount++;
        appInfo->descriptionStr = root["description"].isNull() ? "" : root["description"].as<String>();
        appInfo->authorNameStr  = root["authorName"].isNull()  ? "" : root["authorName"].as<String>();
        appInfo->projectURLStr  = root["projectURL"].isNull()  ? "" : root["projectURL"].as<String>();
        appInfo->creditsStr     = root["credits"].isNull()     ? "" : root["credits"].as<String>();
        log_d("JSON_LOCAL: %s (%d bytes)", appInfo->appNameStr.c_str(), appInfo->binSize );
      break;
      case JSON_REMOTE:
        if( !root["name"].as<String>().equals( appInfo->appNameStr ) ) {
          log_e("AppName mismatch (expecting:'%s', got:'%s'", appInfo->appNameStr.c_str(), root["name"].as<String>().c_str() );
          return;
        }
        jsonMetaPropsCount      = root["json_meta"].size(); // only present on remote appinfo
        appInfo->binSize        = root["size"].as<size_t>();
        appInfo->descriptionStr = jsonMetaPropsCount > 0 ? root["json_meta"]["description"].isNull() ? "" : root["json_meta"]["description"].as<String>() : "";
        appInfo->assetsCount    = jsonMetaPropsCount > 0 ? root["json_meta"]["assets"].size() : 0;
        appInfo->authorNameStr  = jsonMetaPropsCount > 0 ? root["json_meta"]["authorName"].as<String>() : "";
        appInfo->projectURLStr  = jsonMetaPropsCount > 0 ? root["json_meta"]["projectURL"].as<String>() : "";
        appInfo->creditsStr     = jsonMetaPropsCount > 0 ? root["json_meta"]["credits"].as<String>() : "";
        if( appInfo->appNameStr == "" || appInfo->binSize <= 0 || jsonMetaPropsCount == 0 ) {
          log_e("Invalid AppInfo in JSON (path: %s)", jsonFile.c_str() );
          return;
        }
        appInfo->parseAssets( root );
        log_d("JSON_REMOTE: %s (%d bytes)", appInfo->appNameStr.c_str(), appInfo->binSize );
      break;
      default:
       log_e("INVALID TYPE");
      break;
    }

    cycleid = 0;
    cycleanimation = false;
    callbacks.clear();

    if( appInfo->projectURLStr !="" ) {
      callbacks.push_back( &showAppQrCode );
      cycleanimation = true;
    }

    callbacks.push_back( &showAppAssetAuthor );

    if( appInfo->has_app_image ) {
      callbacks.push_back( &showAppAssetImage );
      cycleanimation = true;
    }

  }


  void handleModalAction( AppInfo * appInfo )
  {
    onselect_cb_t aftermodal = [](){};
    MenuActionLabels *confirmLabels = nullptr;
    int hidState = HID_BTN_C;

    switch( appInfo->type ) {
      case REG_HIDDEN:
        log_v("REG_HIDDEN");
        confirmLabels = &UnhideAppsActionButtons;
        aftermodal = &buildHiddenAppList;
      break;
      case NOREG:
        log_v("NOREG");
        confirmLabels = &DeleteAppsActionButtons;
        aftermodal = &buildMyAppsMenu;
      break;
      case REG_LOCAL:
        log_v("REG_LOCAL");
        confirmLabels = &DeleteAppsActionButtons;
        aftermodal = &buildMyAppsMenu;
      break;
      case REG_REMOTE:
        log_v("REG_REMOTE");
        confirmLabels = &InstallHideAppsActionButtons;
        aftermodal = &buildStoreMenu;
      break;
      case REG_UNKNOWN:
      default:
        log_e("REG_UNKNOWN, No valid AppInfo->type (#%d) provided for menu element '%s'", appInfo->type, UI->getListTitle() );
      break;
    }

    if( confirmLabels ) {
      confirmLabels->title = appInfo->appNameStr.c_str();
      hidState = modalConfirm( confirmLabels, true );
    }

    switch( hidState ) {
      case HID_BTN_A: confirmLabels->Buttons[0]->onClick(); aftermodal(); break;
      case HID_BTN_B: confirmLabels->Buttons[1]->onClick(); aftermodal(); break;
      case HID_BTN_C: UI->drawMenu( true ); UI->showList(); break;
    }
  }


  void showAppInfo()
  {
    appInfo.draw();
    handleModalAction( &appInfo );
  }


  void scrollAppInfo()
  {
    LGFX* gfx = UI->getGfx();
    String scrollStr = appInfo.creditsStr;
    if( appInfo.projectURLStr != "" ) scrollStr = scrollStr+ SCROLL_SEPARATOR + appInfo.projectURLStr;
    if( appInfo.descriptionStr != "" ) scrollStr = scrollStr+ SCROLL_SEPARATOR + appInfo.descriptionStr;
    scrollStr = scrollStr+SCROLL_SEPARATOR;
    HeaderScroll.render( gfx, scrollStr, 10, 2, nullptr, 0, 0, gfx->width(), TITLEBAR_HEIGHT );
  }


  void cycleAppAssets()
  {
    if( !cycleanimation ) return;
    uint32_t elapsed = millis()-appInfo.lastrender;
    if( elapsed > cbdelay ) {
      uint32_t cbid = cycleid%callbacks.size();
      callbacks[cbid]();
      appInfo.lastrender = millis();
      cycleid++;
    } else {
      if( elapsed > 0 ) {
        UITheme* th=UI->getTheme();
        float progress = float(float(elapsed)/float(cbdelay))*th->assetWidth;
        UI->getGfx()->fillRect( th->assetPosX, th->assetPosY+th->assetHeight-2, progress, 2, TFT_RED );
      }
      delay(10); // kill that buzz sound coming out of the speaker :D
    }
  }


};








namespace UILists
{
  using namespace MenuItems;
  using namespace FSUtils;
  using namespace UIDo;
  using namespace UIShow;
  using namespace Downloader;

  void buildNtpMenu()
  {
    size_t before = ESP.getFreeHeap();
    NtpMenuGroup.clear();
    NtpMenuGroup.push( &BackToRootMenu );
    size_t servers_count = sizeof( NTP::Servers ) / sizeof( NTP::Server );

    for( int i=0; i<servers_count; i++ ) {
      log_v("Adding action menu %d : %s", i, NTP::Servers[i].name );
      NtpMenuGroup.push( NTP::Servers[i].name, &NtpItemCallbacks );
    }
    NtpMenuGroup.selectedindex = NTP::currentServer+1;
    UI->setList( &NtpMenuGroup );
    UIDraw::drawList( true ); // render the menu
    log_v("Servers count: %d (bytes free: before=%d, after=%d)", UI->getListSize(), before, ESP.getFreeHeap() );
  }


  void buildRootMenu()
  {
    size_t before = ESP.getFreeHeap();
    countApps();
    String jsonFile = CATALOG_DIR + Registry.defaultChannel.catalog_endpoint /*"/catalog.json"*/;

    bool has_catalog = M5_FS.exists( jsonFile );
    bool has_certs   = M5_FS.exists( SD_CERT_PATH );

    log_v("Root menu: has_catalog:%s, has_certs:%s", has_catalog?"true":"false", has_certs?"true":"false" );

    RootMenuGroup.clear();

    if( has_catalog ) {
      RootActionRefresh.setTitle( ROOTACTION_REFRESH );
    } else {
      RootActionRefresh.setTitle( ROOTACTION_DOWNLOAD );
    }

    RootActionBrowse.setTitle( MENUTITLE_MANAGEAPPS );

    if( has_catalog || appsCount > 0 ) {
      RootMenuGroup.push( &RootActionBrowse );
    }
    RootMenuGroup.push( &RootActionRefresh );
    RootMenuGroup.push( &RootActionSwitch );
    RootMenuGroup.push( &RootActionNtp );
    if( has_certs ) {
      RootMenuGroup.push( &RootActionClearTls );
    }
    if( has_catalog ) {
      RootMenuGroup.push( &RootActionClearAll );
    }
    RootMenuGroup.push( &RootActionSleep );

    UI->setList( &RootMenuGroup );
    UI->setMenuActionLabels( &RootListActionButtons );
    UIDraw::drawList( true ); // render the menu
    log_v("Rootmenu items count: %d (bytes free: before=%d, after=%d)", UI->getListSize(), before, ESP.getFreeHeap() );
  }


  void buildBrowseAppsMenu()
  {
    countApps();
    ManageAppsGroup.clear();
    ManageAppsGroup.push( &BackToRootMenu );
    String jsonFile = CATALOG_DIR + Registry.defaultChannel.catalog_endpoint /*"/catalog.json"*/;
    if( M5_FS.exists( jsonFile ) ) {
      ManageAppsGroup.push( &BrowseAppStore ); // [Browse/Install/Hide] Available Apps
    } else { // no catalog available
      ManageAppsGroup.push( &RootActionRefresh ); // Download Catalog
    }
    if( appsCount > 0 ) {
      ManageAppsGroup.push( &ManageMyApps );   // [Browse/Delete] Installed Apps
    }
    if( M5_FS.exists( HIDDEN_APPS_FILE ) ) {
      ManageAppsGroup.push( &ManageAppStore ); // [Unhide] Available Apps
    }
    UI->setList( &ManageAppsGroup );
    UI->setMenuActionLabels( &MyAppsActionButtons );
    UIDraw::drawList( true ); // render the menu
  }



  void buildHiddenAppList()
  {
    getHiddenApps();
    if( HiddenFiles.size() == 0 ) {
      return;
    }
    HiddenAppsMenuGroup.clear();
    HiddenAppsMenuGroup.push( &BackToManageApps );

    for( int i=0; i<HiddenFiles.size(); i++ ) {
      HiddenAppsMenuGroup.push( HiddenFiles[i].c_str(), &HiddenAppsCallbacks, nullptr, isLauncher(HiddenFiles[i].c_str())?LAUNCHER_COLOR:TEXT_COLOR );
    }
    UI->setList( &HiddenAppsMenuGroup );
    UIDraw::drawList( true ); // render the menu
  }



  void buildMyAppsMenu()
  {
    countApps();
    if( appsCount == 0 ) {
      log_v("No apps on SD Card, falling back to root menu");
      buildRootMenu();
    }

    std::vector<String> files;
    getInstalledApps( files );

    if( files.size() <= 0 ) {
      log_v("No apps found, falling back to root menu");
      buildRootMenu();
      return;
    }

    std::sort( files.begin(), files.end() );

    MyAppsMenuGroup.clear();

    for( int i=0; i<files.size(); i++ ) {
      if( i%7==0 ) MyAppsMenuGroup.push( &BackToManageApps );
      String _ms = ""; // for macro separation
      String jsonLocalFile  =                     DIR_json + String( files[i].c_str() ) +  EXT_json;
      String jsonRemoteFile = CATALOG_DIR + _ms + DIR_json + String( files[i].c_str() ) +  EXT_json;
      if( M5_FS.exists( jsonLocalFile ) ) { // "IN REGISTRY" => compare size with meta, propose sha256_sum check or update + delete
        log_v("[#%d] mb=updateCheckShowAppImage(%s)", i, files[i].c_str() );
        MyAppsMenuGroup.push( files[i].c_str(), &UpdateCheckCallbacks, nullptr, isLauncher(files[i].c_str())?LAUNCHER_COLOR:TEXT_COLOR );
      } else if( M5_FS.exists( jsonRemoteFile ) ) { // "BINARY INSTALLED" && "IN REGISTRY" => missing local meta ? => suggest copy meta + delete
        log_v("[#%d] mb=updateMetaShowAppImage(%s)", i, files[i].c_str() );
        MyAppsMenuGroup.push( files[i].c_str(), &UpdateMetaCallbacks, nullptr, isLauncher(files[i].c_str())?LAUNCHER_COLOR:TEXT_COLOR );
      } else { // "BINARY INSTALLED" && "NOT IN REGISTRY" => only suggest delete
        log_v("[#%d] mb=showDeleteAppImage(%s)", i, files[i].c_str() );
        MyAppsMenuGroup.push( files[i].c_str(), &DeleteAppCallbacks, nullptr, isLauncher(files[i].c_str())?LAUNCHER_COLOR:TEXT_COLOR );
      }
    }

    UI->setList( &MyAppsMenuGroup );
    // TODO: focus selected item
    UIDraw::drawList( true ); // render the menu
    log_v("Added %d items to menu", MyAppsMenuGroup.actions_count );
  }



  void buildStoreMenu()
  {
    size_t before = ESP.getFreeHeap();

    getHiddenApps(); // Fill 'HiddenFiles' vector

    String jsonFile = CATALOG_DIR + Registry.defaultChannel.catalog_endpoint; // "/catalog.json"

    JsonObject root;
    DynamicJsonDocument jsonBuffer( 8192 );
    if( !getJson( jsonFile.c_str(), root, jsonBuffer ) ) {
      log_e("Failed to get json from %s", jsonFile.c_str() );
      return;
    }
    if ( root.isNull() ) {
      log_e("No parsable JSON in %s file", jsonFile.c_str() );
      return;
    }
    if( root["apps"].size() <= 0 ) {
      log_e("No apps in catalog");
      return;
    }
    if( root["apps_count"].as<size_t>() == 0 ) { // TODO: root["generated_at"]
      log_e("Empty catalog");
      return;
    }
    if( root["base_url"].as<String>() == "" ) {
      log_e("No base url");
      return;
    }
    if( root["gz_url"].as<String>() == "" ) {
      log_e("No gz url");
      return;
    }

    baseCatalogURL = root["base_url"].as<String>();
    gzCatalogURL   = root["gz_url"].as<String>();

    AppStoreMenuGroup.clear();
    std::vector<String> files;

    for( JsonVariant appEntry : root["apps"].as<JsonArray>() ) {
      String fName = appEntry["name"].as<String>();
      fName.trim();
      if( fName != "" && !isHiddenApp(fName) && !M5_FS.exists( ROOT_DIR + fName + EXT_bin ) )
        files.push_back( fName ); // don't list installed or hidden apps
    }

    std::sort( files.begin(), files.end() );

    for( int i=0; i<files.size(); i++ ) {
      if( i%7==0 ) AppStoreMenuGroup.push( &BackToManageApps );
      log_v("Adding action menu %d : %s", i, files[i].c_str() );
      AppStoreMenuGroup.push( files[i].c_str(), &AppStoreCallbacks, nullptr, isLauncher(files[i].c_str())?LAUNCHER_COLOR:TEXT_COLOR );
    }
    files.clear();

    UI->setList( &AppStoreMenuGroup );
    UIDraw::drawList( true ); // render the menu
    log_v("Added %d apps (bytes free: before=%d, after=%d)", AppStoreMenuGroup.actions_count-1, before, ESP.getFreeHeap() );
  }

};
