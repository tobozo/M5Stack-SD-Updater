#pragma once

#include "../misc/i18n.h"
#include "../Assets/Assets.hpp"
#include "../AppStoreActions/AppStoreActions.hpp"
#include "../MenuUtils/MenuUtils.hpp"



namespace MenuItems
{

  using namespace UIDo;
  using namespace UIDraw;
  using namespace UILists;
  using namespace UIShow;

  // JPG assets (bytes array)
  LocalAsset DiskIcon              = { disk01_jpg, disk01_jpg_len, IMG_JPG,   0,   0, "Insert SD" };
  LocalAsset BrokenImage           = { broken_png, broken_png_len, IMG_PNG,  16,  18, "Broken asset" };

  // JPG assets (filesystem)
  RemoteAsset SDUpdaterIcon        = { "/catalog/jpg/sd-updater15x16.jpg" ,  15,  16, "SDUpdater" };
  RemoteAsset CautionModalIcon     = { "/catalog/jpg/caution.jpg"         ,  64,  46, "Warning" };

  // PNG assets (filesystem)
  RemoteAsset UnknownAppIcon       = { "/catalog/png/unknown-app.png"     , 120, 120, "Unknown App" };
  RemoteAsset CheckIcon            = { "/catalog/png/missing-meta.png"    ,  32,  32, "Meta" }; // used to overlay app icon
  RemoteAsset UpdateIcon           = { "/catalog/png/update-icon.png"     ,  32,  32, "Update" }; // used to overlay app icon
  RemoteAsset ForkIcon             = { "/catalog/png/fork.png"            ,  12,  16, "Channel" };
  RemoteAsset NtpIcon              = { "/catalog/png/ntp-connect.png"     ,  32,  32, "Syncing to NTP" };

  RemoteAsset SelectBtnIcon        = { "/catalog/png/select.png"          ,  16,  16, "Select" };
  RemoteAsset ArrowDownBtnIcon     = { "/catalog/png/arrowdown.png"       ,  16,  16, "Arrow down" };
  RemoteAsset ArrowUpBtnIcon       = { "/catalog/png/arrowup.png"         ,  16,  16, "Arrow up" };
  //RemoteAsset *defaultBtnIcons[3]  = { &SelectBtnIcon, &ArrowUpBtnIcon, &ArrowDownBtnIcon };

  RemoteAsset ManageAppsImage      = { "/catalog/png/manage-apps.png"     , 120, 120, "App Manager" };
  RemoteAsset RefreshCatalogImage  = { "/catalog/png/refresh-catalog.png" , 120, 120, "Catalog Updater" };
  RemoteAsset SwitchChannelImage   = { "/catalog/png/switch-channel.png"  , 120, 120, "Channel Switcher" };
  RemoteAsset ChangeNTPServerImage = { "/catalog/png/ntp.png"             , 120, 120, "Region Selector" };
  RemoteAsset ClearTLSImage        = { "/catalog/png/clear-TLS.png"       , 120, 120, "SD Cleaner" };
  RemoteAsset ClearAllImage        = { "/catalog/png/clear-ALL.png"       , 120, 120, "Certs Cleaner" };
  RemoteAsset SleepImage           = { "/catalog/png/sleep.png"           , 120, 120, "Turn Off" };
  RemoteAsset EditDeleteImage      = { "/catalog/png/edit-delete.png"     , 120, 120, "Edit/Delete" };
  RemoteAsset InstallAppsImage     = { "/catalog/png/add-apps.png"        , 120, 120, "Install/Hide" };
  RemoteAsset HideAppsImage        = { "/catalog/png/toggle.png"          , 120, 120, "Unhide" };
  RemoteAsset Cpanel1Image         = { "/catalog/png/cpanel2.png"         , 120, 120, "Settings" };
  RemoteAsset Cpanel2Image         = { "/catalog/png/cpanel.png"          , 120, 120, "Back to Settings" };


  // buttons
  ButtonAction defaultListSelect      = { MENU_BTN_INFO,         &BtnA,                &SelectBtnIcon };   // "SELECT" (exec selected item cb)
  ButtonAction defaultListPrevItem    = { MENU_BTN_PREV,         &BtnB,                &ArrowUpBtnIcon }; // ">>" (prev item)
  ButtonAction defaultListNextItem    = { MENU_BTN_NEXT,         &BtnC,                &ArrowDownBtnIcon }; // ">"  (next item)
  ButtonAction BackToBrowseAppsButton = { MENUACTION_BACKTOAPPS, &buildBrowseAppsMenu, nullptr };
  ButtonAction BackButton             = { MENU_BTN_BACK,         &BtnC,                nullptr };
  ButtonAction CancelButton           = { MENU_BTN_CANCEL,       &BtnA,                nullptr };
  ButtonAction GoButton               = { MENU_BTN_GO,           &BtnA,                nullptr };
  ButtonAction InstallButton          = { MENUACTION_APPINSTALL, &installApp,          nullptr };
  ButtonAction DeleteButton           = { MENUACTION_APPDELETE,  &deleteApp,           nullptr };
  ButtonAction HideButton             = { MENUACTION_APPHIDE,    &addHiddenApp,        nullptr };
  ButtonAction UnhideButton           = { MENUACTION_UNHIDE,     &removeHiddenApp,     nullptr };
  ButtonAction RefreshButton          = { MENUACTION_REFRESH,    &downloadCatalog,     nullptr };
  ButtonAction EmptyButton            = { "",                    [](){},               nullptr};

  // buttons-sets
  ButtonAction *defaultListButtons[3]       = { &defaultListSelect, &defaultListPrevItem, &defaultListNextItem }; // "select", "next page", "next item"
  ButtonAction *emptyButtons[3]             = { &EmptyButton,       &EmptyButton,         &EmptyButton };
  ButtonAction *manageAppsButtons[3]        = { &GoButton,          &defaultListPrevItem, &defaultListNextItem }; // "go", "next page", "next item"
  ButtonAction *myAppsButtons[3]            = { &DeleteButton,      &EmptyButton,         &BackButton }; // "delete", [empty], "back"
  ButtonAction *InstallHideActionButtons[3] = { &InstallButton,     &HideButton,          &BackButton };
  ButtonAction *unhideAppsButtons[3]        = { &UnhideButton,      &EmptyButton,         &BackButton };

  // buttons-sets/title for static lists
  MenuActionLabels RootListActionButtons    = { MENUTITLE_DEFAULT, defaultListButtons };
  MenuActionLabels ManageAppsActionButtons  = { MENUTITLE_MANAGEAPPS, manageAppsButtons };
  MenuActionLabels RefreshAppsActionButtons = { MENUTITLE_DOWNLOADER, emptyButtons };

  // buttons-sets/title for dynamic lists
  MenuActionLabels MyAppsActionButtons      = { MENUTITLE_MYAPPS, defaultListButtons };
  MenuActionLabels AppStoreActionButtons    = { MENUTITLE_APPSTORE, defaultListButtons };
  MenuActionLabels HiddenAppsActionButtons  = { MENUTITLE_TOGGLEAPPS, defaultListButtons };
  MenuActionLabels NTPServersActionButtons  = { MENUACTION_NTP, defaultListButtons };

  // buttons-sets/title for dynamic list items
  MenuActionLabels InstallHideAppsActionButtons = { MENUTITLE_DOWNLOADER, InstallHideActionButtons };
  MenuActionLabels UnhideAppsActionButtons      = { MENUTITLE_TOGGLEAPPS, unhideAppsButtons };
  MenuActionLabels DeleteAppsActionButtons      = { MENUTITLE_MYAPPS,     myAppsButtons };

  // callbacks for menu events:                     select         /      render      /             idle     /       modal
  MenuItemCallBacks BackToRootMenuCallbacks     = { &buildRootMenu,       nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks BackToManageAppsCallbacks   = { &buildBrowseAppsMenu, nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks RootActionRefreshCallbacks  = { &downloadCatalog,     nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks RootActionBrowseCallbacks   = { &buildBrowseAppsMenu, nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks ManageMyAppsCallbacks       = { &buildMyAppsMenu,     nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks BrowseAppStoreCallbacks     = { &buildStoreMenu,      nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks ManageAppStoreCallbacks     = { &buildHiddenAppList,  nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks RootActionSwitchCallbacks   = { &drawRegistryMenu,    nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks RootActionNtpCallbacks      = { &buildNtpMenu,        nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks RootActionClearAllCallbacks = { &clearApps,           nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks RootActionClearTlsCallbacks = { &clearTLS,            nullptr,                  nullptr,         nullptr };
  MenuItemCallBacks RootActionSleepCallbacks    = { &gotoSleep,           nullptr,                  nullptr,         nullptr };
  // callbacks for menuitem events:                 select         /      render      /             idle     /       modal
  MenuItemCallBacks UpdateCheckCallbacks        = { &showAppInfo,         &updateCheckShowAppImage, &cycleAppAssets, &scrollAppInfo };
  MenuItemCallBacks UpdateMetaCallbacks         = { &showAppInfo,         &updateMetaShowAppImage,  &cycleAppAssets, &scrollAppInfo };
  MenuItemCallBacks DeleteAppCallbacks          = { &showAppInfo,         &showDeleteAppImage,      &cycleAppAssets, &scrollAppInfo };
  MenuItemCallBacks AppStoreCallbacks           = { &showAppInfo,         &showAppImage,            &cycleAppAssets, &scrollAppInfo };
  MenuItemCallBacks HiddenAppsCallbacks         = { &showAppInfo,         &showAppImage,            &cycleAppAssets, &scrollAppInfo };
  MenuItemCallBacks NtpItemCallbacks            = { &setNtpServer,        &showNTPImage,            nullptr,         nullptr };



  MenuAction BackToRootMenu   = MenuAction( MENUACTION_BACK " to " MENUTITLE_MAINMENU,   &BackToRootMenuCallbacks, &ManageAppsImage );
  MenuAction BackToManageApps = MenuAction( MENUACTION_BACK " to " MENUTITLE_MANAGEAPPS, &BackToManageAppsCallbacks, &ManageAppsImage );

  /*    Main Menu level 0           */
  /*  */MenuGroup RootMenuGroup = MenuGroup( &RootListActionButtons );
  /*  |    MenuItem 1 level 0       */  // Apps Management action: Refresh Catalog
  /*  +--*/MenuAction RootActionRefresh = MenuAction( ROOTACTION_REFRESH, &RootActionRefreshCallbacks, &RefreshCatalogImage );
  /*  |    MenuItem 2 level 0       */  // General Action: Open Apps Management Menu
  /*  +--*/MenuAction RootActionBrowse = MenuAction( MENUTITLE_MANAGEAPPS, &RootActionBrowseCallbacks, &ManageAppsImage );
  /*  |    MenuItem 3 level 0       */  // Apps Management Menu
  /*  +--*/MenuGroup ManageAppsGroup = MenuGroup( &ManageAppsActionButtons );
  /*  |  |    MenuItem 9 level 1    */  // Apps Management: SDCard applications list
  /*  |  +--*/MenuGroup MyAppsMenuGroup = MenuGroup( &MyAppsActionButtons );
  /*  |  |  |    MenuItem 13 level 2 */ // SDCard applications list actions: delete/update
  /*  |  |  +--*/MenuAction ManageMyApps = MenuAction( MENUACTION_DEL_APPS, &ManageMyAppsCallbacks, &EditDeleteImage );
  /*  |  |    MenuItem 10 level 1    */ // Apps Management: Installable applications list
  /*  |  +--*/MenuGroup AppStoreMenuGroup = MenuGroup( &AppStoreActionButtons, CATALOG_DIR );
  /*  |  |  |    MenuItem 14 level 2 */ // Installable applications list actions: install/hide
  /*  |  |  +--*/MenuAction BrowseAppStore = MenuAction( MENUACTION_BROWSEAPP, &BrowseAppStoreCallbacks, &InstallAppsImage );
  /*  |  |    MenuItem 11 level 1   */  // Apps Management: Hidden applications list
  /*  |  +--*/MenuGroup HiddenAppsMenuGroup = MenuGroup( &HiddenAppsActionButtons, CATALOG_DIR );
  /*  |     |    MenuItem 15 level 2 */ // Hidden applications list actions: unhide
  /*  |     +--*/MenuAction ManageAppStore = MenuAction( MENUTITLE_TOGGLEAPPS, &ManageAppStoreCallbacks, &HideAppsImage );
  /*  |    MenuItem 4 level 0       */  // Registry switcher
  /*  +--*/MenuAction RootActionSwitch = MenuAction( ROOTACTION_SWITCH, &RootActionSwitchCallbacks, &SwitchChannelImage );
  /*  |    MenuItem 5 level 0       */  // NTP Management: NTP Servers list
  /*  +--*/MenuAction RootActionNtp = MenuAction( ROOTACTION_NTP_PICK, &RootActionNtpCallbacks, &ChangeNTPServerImage );
  /*  |  |    MenuItem 12 level 1   */  // NTP Servers list actions: Pick server
  /*  |  +--*/MenuGroup NtpMenuGroup = MenuGroup( &NTPServersActionButtons );
  /*  |    MenuItem 6 level 0       */  // Full filesystem wipe
  /*  +--*/MenuAction RootActionClearAll = MenuAction( ROOTACTION_CLEAR_ALL, &RootActionClearAllCallbacks, &ClearAllImage );
  /*  |    MenuItem 7 level 0       */  // TLS wipe
  /*  +--*/MenuAction RootActionClearTls = MenuAction( ROOTACTION_CLEAR_TLS, &RootActionClearTlsCallbacks, &ClearTLSImage );
  /*  |    MenuItem 8 level 0       */  // Sleep mode
  /*  +--*/MenuAction RootActionSleep = MenuAction( ROOTACTION_SLEEP, &RootActionSleepCallbacks, &SleepImage );


};
