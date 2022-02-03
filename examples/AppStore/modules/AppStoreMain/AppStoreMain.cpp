/*
 *
 * M5Stack Application Store
 * Project Page: https://github.com/tobozo/M5Stack-SD-Updater
 *
 * Copyright 2021 tobozo http://github.com/tobozo
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

#pragma once

#include "AppStoreMain.hpp"
#include "../Console/Console.cpp"
#include "../Assets/Assets.cpp"
#include "../MenuItems/MenuItems.cpp"
#include "../MenuUtils/MenuUtils.cpp"
#include "../AppStoreUI/AppStoreUI.cpp"
#include "../AppStoreActions/AppStoreActions.cpp"
#include "../FSUtils/FSUtils.cpp"
#include "../Downloader/Downloader.cpp"
#include "../Registry/Registry.cpp"
#include "../CertsManager/CertsManager.cpp"

AppStoreUI *UI = nullptr;
LogWindow *Console = nullptr;
AppRegistry Registry;

namespace AppStore
{

  void setup()
  {
    M5.begin( true, true, true, false, ScreenShotEnable ); // bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable, bool ScreenShotEnable

    #if !defined FS_CAN_CREATE_PATH
      UIDo::doFSChecks();
    #endif

    UI = new AppStoreUI( &tft );

    #if !defined HAS_RTC
      FSUtils::setTimeFromLastFSAccess();
    #endif

    checkSDUpdater( M5_FS, MENU_BIN, 5000, TFCARD_CS_PIN );

    Registry = RegistryUtils::init(); // load registry profile
    UI->channel_name = Registry.defaultChannel.name.c_str();

    DummyAsset::setup( &UIUtils::drawCaption, &MenuItems::BrokenImage, UI->getTheme()->TextColor, UI->getTheme()->BgColor );

    TLS::wget = &Downloader::wget; // attach downloader to TLS
    TLS::modalConfirm = &UIUtils::modalConfirm;
    TLS::certProvider = Registry.defaultChannel.api_cert_provider_url_https; // set TLS cert provider
    NTP::loadPrefServer();

    BackToRootMenu.textcolor = DIMMED_COLOR;
    BackToManageApps.textcolor = DIMMED_COLOR;

    Serial.println( WELCOME_MESSAGE );
    Serial.println( INIT_MESSAGE );
    Serial.printf( MENU_SETTINGS, LINES_PER_PAGE, LIST_MAX_COUNT);
    Serial.printf("Has PSRam: %s\n", psramInit() ? "true" : "false");
    Serial.println("Build DateTime: "+ Downloader::ISODateTime );

    log_i("\nRAM SIZE:\t%s\nFREE RAM:\t%s\nMAX ALLOC:\t%s",
      String( formatBytes(ESP.getHeapSize(), formatBuffer) ).c_str(),
      String( formatBytes(ESP.getFreeHeap(), formatBuffer) ).c_str(),
      String( formatBytes(ESP.getMaxAllocHeap(), formatBuffer) ).c_str()
    );

    tft.setBrightness(100);
    lastcheck = millis();

    FSUtils::countApps();
    UILists::buildRootMenu();

  }


  void loop()
  {
    HIDSignal hidState = getControls();

    if( hidState!=HID_INERT && UIDo::brightness != MAX_BRIGHTNESS ) {
      // some activity occured, restore brightness
      Serial.println(".. !!! Waking up !!");
      UIDo::brightness = MAX_BRIGHTNESS;
      tft.setBrightness( UIDo::brightness );
    }

    UIDo::lastcheck = millis();

    switch( hidState ) {
      case HID_BTN_C:
        UIDo::BtnC();
      break;
      case HID_BTN_A:
        UI->getList()->selectedindex = UI->getListID();
        UIDo::BtnA();
      break;
      case HID_BTN_B:
        UI->getList()->selectedindex = UI->getListID();
        UIDo::BtnB();
      break;
      default:
      case HID_INERT:
        UIDo::lastcheck = UIDo::lastpush;
      break;
      case HID_SCREENSHOT:
        #if defined USE_SCREENSHOT
          M5.ScreenShot->snap( "screenshot" );
        #endif
      break;
    }
    UIDo::lastpush = UIDo::lastcheck;
    UI->idle();
  }

};
