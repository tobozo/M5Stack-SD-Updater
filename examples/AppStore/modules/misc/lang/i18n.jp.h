#define WELCOME_MESSAGE "Welcome to the " PLATFORM_NAME " App Store!"
#define INIT_MESSAGE  PLATFORM_NAME " App Store initializing..."
#define MENU_SETTINGS "AppStoreUI loaded with %d labels per page, max %d items\n"
#define GOTOSLEEP_MESSAGE "Will go to sleep"

#define MENU_BTN_INFO     "撰ぶ" // "SELECT"
#define MENU_BTN_UPDATE   "UPDATE"
#define MENU_BTN_BACK     "Back"
#define MENU_BTN_PREV     "<"
#define MENU_BTN_NEXT     ">"
#define MENU_BTN_YES      "YES"
#define MENU_BTN_NO       "NO"
#define MENU_BTN_GO       "GO"
#define MENU_BTN_CANCEL   "Cancel"
#define MENU_BTN_REBOOT   "Reboot"
#define MENU_BTN_RESTART  "Restart"
#define MENU_BTN_CONTINUE "Continue"
#define MENU_BTN_RETRY    "RETRY"


#define MENUTITLE_DEFAULT PLATFORM_NAME " アプリストア" // " App Store"
#define MENUTITLE_MYAPPS "My Applications"
#define MENUTITLE_APPSTORE "Applications Store"
#define MENUTITLE_TOGGLEAPPS "Unhide Apps"
#define MENUTITLE_DOWNLOADER "Apps Downloader"
#define MENUTITLE_MANAGEAPPS "アプリケーションの管理" // "Manage Applications"
#define MENUTITLE_MAINMENU   "Main Menu"

#define MENUACTION_BACKTOAPPS "Back to Browse Apps"
#define MENUACTION_APPINSTALL "Install"
#define MENUACTION_APPUPDATE  "Update"
#define MENUACTION_APPVERIFY  "Verify"
#define MENUACTION_VIEW       "View"
#define MENUACTION_APPHIDE    "Hide"
#define MENUACTION_APPDELETE  "Delete"
#define MENUACTION_BACK       "Back"
#define MENUACTION_UNHIDE     "Unhide"
#define MENUACTION_REFRESH    "Refresh"

#define MENUACTION_NTP        "NTP Server"
#define MENUACTION_MYAPPS     "*My Applications"
#define MENUACTION_APPTOGGLE  "Manage Hidden Apps"
#define MENUACTION_DOWNLOADER "*Apps Downloader"
#define MENUACTION_BROWSEAPP  "Browse App Store"
#define MENUACTION_DEL_APPS   "Manage My Apps"

#define ROOTACTION_DOWNLOAD  "Download Catalog"
#define ROOTACTION_REFRESH   "カタログを更新" // "Refresh Catalog"
#define ROOTACTION_GET       "Install Applications"

#define ROOTACTION_SWITCH    "チャンネルの切り替え" // "Switch Channel"
#define ROOTACTION_NTP_PICK  "NTPサーバーを変更する" // "Change NTP Server"
#define ROOTACTION_CLEAR_ALL "すべてクリア" // "Clear ALL"
#define ROOTACTION_CLEAR_TLS "TLSキャッシュをクリアする" // "Clear TLS cache"
#define ROOTACTION_SLEEP     "寝る" // "Sleep"

#define MODAL_DELETEALL_TITLE "DELETE EVERYTHING"
#define MODAL_DELETEALL_MSG "CAUTION! This will remove apps, assets, registries\nand databases, even those\noutside the scope of this\nApplication Manager!"

#define MODAL_DOWNLOADFAIL_TITLE "HTTP FAILED"
#define MODAL_DOWNLOADFAIL_MSG "An HTTP error occured\nwhile downloading\nthe registry\narchive.\nTry again?"

#define MODAL_WIFI_NOCONN_MSG "No connection"

#define DOWNLOADER_MODAL_CHANGE "CHANGE"

#define MODAL_CANCELED_TITLE "OPERATION CANCELED"
#define MODAL_SUCCESS_TITLE "OPERATION SUCCESSFUL"


#define CHANNEL_TOOL "CHANNEL TOOL"
#define CHANNEL_TOOL_TEXT "Do you want to change or\nupdate your SD Card\nchannel?"

#define CHANNEL_CHOOSER "CHANNEL CHOOSER"
#define CHANNEL_CHOOSER_PROMPT "Change channel ?"
#define CHANNEL_CHOOSER_TEXT "You are about to change\nyour SD Card channel.\n\n    Are you sure ?"

#define CHANNEL_DOWNLOADER "CHANNEL DOWNLOADER"
#define CHANNEL_DOWNLOADER_PROMPT "Download channel ?"
#define CHANNEL_DOWNLOADER_TEXT "You are about to overwrite\nyour SD Card channel.\n\n    Are you sure ?"

#define DOWNLOADER_MODAL_NAME "Update binaries ?"
#define DOWNLOADER_MODAL_TITLE "This action will:"
#define DOWNLOADER_MODAL_ENDED "Synchronization complete"
#define DOWNLOADER_MODAL_TITLE_ERRORS_OCCURED "Some errors occured. "

#define OVERALL_PROGRESS_TITLE "Overall progress"
#define TAR_PROGRESS_TITLE  "Downloading Registry"
#define NOT_IN_REGISTRY "NOT IN REGISTRY"

#define WGET_SKIPPING " [Checksum OK]"
#define WGET_UPDATING " [Outdated]"
#define WGET_CREATING " [New file]"
#define SYNC_FINISHED "Synch finished"
#define CLEANDIR_REMOVED "Removed %s\n"
#define DOWNLOAD_FAIL " [DOWNLOAD FAIL]"
#define SHASHUM_FAIL " [SHASUM FAIL]"
#define UPDATE_SUCCESS "UPDATE SUCCESS"

#define WIFI_MSG_WAITING "Enabling WiFi..."
#define WIFI_MSG_CONNECTING "Connecting WiFi.."
#define WIFI_TITLE_TIMEOUT "WiFi Timeout"
#define WIFI_MSG_TIMEOUT "Timed out, will try again"
#define WIFI_TITLE_CONNECTED "WiFi OK"
#define WIFI_MSG_CONNECTED "Connected to wifi :-)"

#define NTP_TITLE_SETUP "NTP Setup"
#define NTP_MSG_SETUP "Contacting NTP Server"
#define NTP_TITLE_FAIL "NTP Down"
#define NTP_MSG_FAIL "Can't enable NTP"


#define DL_FSCLEANUP_TITLE "Cleanup"
#define DL_FSCLEANUP_MSG "Removing previous\nbackup"
#define DL_FSBACKUP_TITLE "Backup"
#define DL_FSBACKUP_MSG "Backing up registry"
#define DL_FSRESTORE_MSG "Restoring backup"

#define DL_TLSFETCH_TITLE "TLS"
#define DL_TLSFETCH_MSG "Fetching TLS Cert"

#define DL_TLSFAIL_TITLE "TLS Error"
#define DL_TLSFAIL_MSG "Could not init TLS"

#define DL_HTTPINIT_TITLE "HTTP Init"
#define DL_HTTPINIT_MSG "Contacting catalog\nendpoint"

#define DL_HTTPFAIL_TITLE "HTTP Error"
#define DL_FSFAIL_TITLE "Filesystem Error"
#define DL_SHAFAIL_TITLE "SHA256 Error"

#define DL_AWAITING_TITLE "Awaiting response"

#define DL_SUCCESS_TITLE "Success"
#define DL_SUCCESS_MSG "Registry fetched!"
#define DL_FAIL_MSG "Registry could not\nbe reached."


#define WGET_MSG_FAIL "Please check the remote server"
#define FS_MSG_FAIL "Please check the filesystem"

#define MODAL_TLSCERT_INSTALLFAILED_MSG "Certificate fetching\nOK but TLS Install\nfailed"
#define MODAL_TLSCERT_FETCHINGFAILED_MSG "Unable to wget()\ncertificate"
#define NEW_TLS_CERTIFICATE_TITLE "New TLS certificate installed"
#define NEW_TLS_CERTIFICATE_TEXT "A new certificate\nwas fetched and\ninstalled successfully."
#define MODAL_RESTART_REQUIRED "Restarting is\noptional.\n\nReboot anyway?"
#define MODAL_SAME_PLAYER_SHOOT_AGAIN "Please try again.\n\nReboot now?"
#define MODAL_REGISTRY_UPDATED "New Registry file\nhas been updated"
#define MODAL_REGISTRY_DAMAGED "New Registry file\nmay be damaged"
#define MODAL_REBOOT_REGISTRY_UPDATED "Please reboot and\nchoose a channel.\n\nReboot now?"


#define DEBUG_DIROPEN_FAILED "Failed to open directory"
#define DEBUG_EMPTY_FS "Empty SD Card, falling back to root menu"
#define DEBUG_NOTADIR "Not a directory"
#define DEBUG_DIRLABEL "  DIR : "
#define DEBUG_IGNORED "  IGNORED FILE: "
#define DEBUG_CLEANED "  CLEANED FILE: "
#define DEBUG_ABORTLISTING "  ***Max files reached for Menu, please adjust LIST_MAX_COUNT for more (maximum is 255, sorry :-)"
#define DEBUG_FILELABEL "  FILE: "

#define DEBUG_FILECOPY "Starting File Copy for "
#define DEBUG_FILECOPY_DONE "Transfer finished"
#define DEBUG_WILL_RESTART "Binary removed from SPIFFS, will now restart"
#define DEBUG_NOTHING_TODO "No binary to transfer"
#define DEBUG_KEYPAD_NOTFOUND "Keypad not installed"
#define DEBUG_KEYPAD_FOUND "Keypad detected!"
#define DEBUG_JOYPAD_NOTFOUND "No Joypad detected, disabling"
#define DEBUG_JOYPAD_FOUND "Joypad detected!"

#define DEBUG_TIMESTAMP_GUESS "%s has %s time set (%04d-%02d-%02d %02d:%02d:%02d), will use %s date to set the clock"

