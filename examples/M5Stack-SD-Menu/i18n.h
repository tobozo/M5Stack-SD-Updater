#ifndef __I18N_H
#define __I18N_H


#define WELCOME_MESSAGE F("Welcome to the M5Stack SD Menu Loader!")
#define INIT_MESSAGE F("M5Stack initializing...")
#define INSERTSD_MESSAGE F("Insert SD")
#define GOTOSLEEP_MESSAGE F("Will go to sleep")
#define MOVINGFILE_MESSAGE F("Moving ")
#define FILESIZE_UNITS F(" bytes")

#define MENU_TITLE F("SD CARD LOADER")
#define MENU_SUBTITLE F("Applications")
#define MENU_BTN_INFO F("INFO")
#define MENU_BTN_LOAD F("LOAD")
#define MENU_BTN_NEXT F(">")

#define ABOUT_THIS_MENU F("About This")

#define AUTHOR_PREFIX F("By ")
#define AUTHOR_SUFFIX F(" **")

#define DEBUG_DIRNAME F("Listing directory: %s\n")
#define DEBUG_DIROPEN_FAILED F("Failed to open directory")
#define DEBUG_NOTADIR F("Not a directory")
#define DEBUG_DIRLABEL F("  DIR : ")
#define DEBUG_IGNORED F("  IGNORED FILE: ")
#define DEBUG_FILELABEL F("  FILE: ")

#define DEBUG_SPIFFS_SCAN F("Scanning SPIFFS for binaries")
#define DEBUG_SPIFFS_MOUNTFAILED F("SPIFFS Mount Failed")
#define DEBUG_SPIFFS_WRITEFAILED F("- failed to open file for writing")
#define DEBUG_FILECOPY F("Starting File Copy for ")
#define DEBUG_FILECOPY_DONE F("Transfer finished")
#define DEBUG_WILL_RESTART F("Binary removed from SPIFFS, will now restart")
#define DEBUG_NOTHING_TODO F("No binary to transfer")
#define DEBUG_KEYPAD_NOTFOUND F("Keypad not installed")
#define DEBUG_KEYPAD_FOUND F("Keypad detected!")
#define DEBUG_JOYPAD_NOTFOUND F("No Joypad detected, disabling")
#define DEBUG_JOYPAD_FOUND F("Joypad detected!")



#endif
