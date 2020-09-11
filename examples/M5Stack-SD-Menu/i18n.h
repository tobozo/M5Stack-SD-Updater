#ifndef __I18N_H
#define __I18N_H


#define WELCOME_MESSAGE F("Welcome to the " PLATFORM_NAME " SD Menu Loader!")
#define INIT_MESSAGE F( PLATFORM_NAME " SD Updater initializing...")
#define M5_SAM_MENU_SETTINGS "M5StackSam loaded with %d labels per page, max %d items\n"
#define SD_LOADING_MESSAGE F("Checking SD Card...")
#define INSERTSD_MESSAGE F("Insert SD")
#define GOTOSLEEP_MESSAGE F("Will go to sleep")
#define MOVINGFILE_MESSAGE F("Moving ")
#define FILESIZE_UNITS F(" bytes")

#define MENU_TITLE F( PLATFORM_NAME " SD LAUNCHER")
#define MENU_SUBTITLE F("Applications")
#define MENU_BTN_INFO F("SELECT")
#define MENU_BTN_SET F("SET")
#define MENU_BTN_LOAD F("LOAD")
#define MENU_BTN_LAUNCH F("LAUNCH")
#define MENU_BTN_UPDATE "UPDATE"
#define MENU_BTN_SOURCE "SOURCE"
#define MENU_BTN_BACK "BACK"
#define MENU_BTN_PAGE F(">>")
#define MENU_BTN_NEXT F(">")
#define MENU_BTN_WFT "UUH?"
#define MENU_BTN_YES "YES"
#define MENU_BTN_NO "NO"
#define MENU_BTN_CANCEL "CANCEL"

//#ifdef TFT_SDA_READ
#define MENU_SCREENSHOT "SNAP"
//#endif

#define ABOUT_THIS_MENU F("--About This Launcher--")

#define AUTHOR_PREFIX F("By ")
#define AUTHOR_SUFFIX F(" **")

#define APP_DOWNLOADER_MENUTITLE PLATFORM_NAME " Apps Downloader"

#define CHANNEL_TOOL "CHANNEL TOOL"
#define CHANNEL_TOOL_PROMPT "    Change or Update channel ?"
#define CHANNEL_TOOL_TEXT "    Do you want to change or update\n\n    your SD Card channel?\n\n\n    Please choose."

#define CHANNEL_CHOOSER "CHANNEL CHOOSER"
#define CHANNEL_CHOOSER_PROMPT "    Change channel ?"
#define CHANNEL_CHOOSER_TEXT "    You are about to change your SD Card channel.\n\n    Are you sure ?"

#define CHANNEL_DOWNLOADER "CHANNEL DOWNLOADER"
#define CHANNEL_DOWNLOADER_PROMPT "    Download channel ?"
#define CHANNEL_DOWNLOADER_TEXT "    You are about to overwrite your SD Card channel.\r\n    Are you sure ?"

#define DOWNLOADER_MODAL_NAME "Update binaries ?"
#define DOWNLOADER_MODAL_TITLE "This action will:"
#define DOWNLOADER_MODAL_BODY "  - Connect to WiFi\n\n  - Get app list from remote registry\n\n  - Download/overwrite files\n\n  - Restart this menu\n\n\n\n  THIS OPERATION IS INSECURE!!\n\n  YOU DO THIS AT YOUR OWN RISK!!"
#define DOWNLOADER_MODAL_ENDED "Synchronization complete"
#define DOWNLOADER_MODAL_TITLE_ERRORS_OCCURED "Some errors occured. "
#define DOWNLOADER_MODAL_BODY_ERRORS_OCCURED "  %d errors occured during the download\n\n  %d files were verified\n\n  %d files were updated\n\n  %d files were created\n\n\n\n  " PLATFORM_NAME " will reboot in 10s"
#define DOWNLOADER_MODAL_REBOOT "REBOOT"
#define DOWNLOADER_MODAL_RESTART "RESTART"

#define DOWNLOADER_MODAL_RETRY "RETRY"
#define DOWNLOADER_MODAL_CHANGE "CHANGE"
#define MENU_BTN_CANCELED "OPERATION CANCELED"






#define OVERALL_PROGRESS_TITLE "Overall progress: "
#define WGET_SKIPPING " [Checksum OK]"
#define WGET_UPDATING " [Outdated]"
#define WGET_CREATING " [New file]"
#define SYNC_FINISHED "Synch finished"
#define CLEANDIR_REMOVED "Removed %s\n"
#define DOWNLOAD_FAIL " [DOWNLOAD FAIL]"
#define SHASHUM_FAIL " [SHASUM FAIL]"
#define UPDATE_SUCCESS "UPDATE SUCCESS"

#define WIFI_MSG_WAITING "Waiting for WiFi to connect"
#define WIFI_MSG_CONNECTING "Establishing connection to WiFi.."
#define WIFI_MSG_TIMEOUT "Timed out, will try again"
#define WIFI_MSG_CONNECTED "Connected to wifi :-)"

#define NEW_TLS_CERTIFICATE_INSTALLED "    New TLS certificate installed"
#define MODAL_RESTART_REQUIRED "    This will require a restart.\r\n\r\n    Reboot now?"
#define MODAL_SAME_PLAYER_SHOOT_AGAIN "    Please reboot and try again.\r\n\r\n    Reboot now?"
#define MODAL_REGISTRY_UPDATED "    New Registry file has been updated"
#define MODAL_REGISTRY_DAMAGED "    New Registry file may be damaged"
#define MODAL_REBOOT_REGISTRY_UPDATED "    Please reboot and choose a channel.\r\n\r\n    Reboot now?"

#define DEBUG_DIRNAME "Listing directory: %s\n"
#define DEBUG_DIROPEN_FAILED F("Failed to open directory")
#define DEBUG_NOTADIR F("Not a directory")
#define DEBUG_DIRLABEL F("  DIR : ")
#define DEBUG_IGNORED F("  IGNORED FILE: ")
#define DEBUG_CLEANED F("  CLEANED FILE: ")
#define DEBUG_ABORTLISTING F("  ***Max files reached for M5StackSam Menu, please adjust M5SAM_LIST_MAX_COUNT for more (maximum is 255, sorry :-)")
#define DEBUG_FILELABEL F("  FILE: ")

#define DEBUG_SPIFFS_SCAN F("Scanning SPIFFS for binaries")
#define DEBUG_SPIFFS_MOUNTFAILED F("SPIFFS Mount Failed")
#define DEBUG_SPIFFS_WRITEFAILED F("- failed to open file for writing")
#define DEBUG_FILECOPY "Starting File Copy for "
#define DEBUG_FILECOPY_DONE F("Transfer finished")
#define DEBUG_WILL_RESTART F("Binary removed from SPIFFS, will now restart")
#define DEBUG_NOTHING_TODO F("No binary to transfer")
#define DEBUG_KEYPAD_NOTFOUND F("Keypad not installed")
#define DEBUG_KEYPAD_FOUND F("Keypad detected!")
#define DEBUG_JOYPAD_NOTFOUND F("No Joypad detected, disabling")
#define DEBUG_JOYPAD_FOUND F("Joypad detected!")

#define DEBUG_TIMESTAMP_GUESS "Menu.bin has %s time set (%04d-%02d-%02d %02d:%02d:%02d), will use %s date to set the clock\n"

#endif
