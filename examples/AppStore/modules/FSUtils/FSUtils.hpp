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


#pragma once
#include <FS.h>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/
#include "../misc/compile_time.h" // for app watermarking & user-agent customization
#include "../misc/core.h"
#include "../misc/config.h"



namespace FSUtils

{

  uint16_t appsCount = 0;

  const uint8_t extensionsCount = 7; // change this if you add / remove an extension
  const String trashFolderPathStr = "/.trash";

  String allowedExtensions[extensionsCount] =
  {
    // do NOT remove jpg and json or the menu will crash !!!
    "jpg", "png", "bmp", "json", "mod", "mp3", "cert"
  };

  static std::vector<String> HiddenFiles;

  bool getInstalledApps( std::vector<String> &files );
  void removeInstalledApp( String appName );

  void setTimeFromLastFSAccess();
  void countApps();
  bool getFileAttrs( const char* name, size_t *_size, time_t *_time );
  bool isBinFile( const char* fileName );
  bool isLauncher( const char* binFileName );
  bool isJsonFile( const char* fileName );
  bool isValidAppName( const char* fileName );
  bool iFile_exists( fs::FS *fs, String &fname );
  bool isHiddenApp( String appName );
  void cleanDir( const char* dir );
  bool getJson( const char* path, JsonObject &root, DynamicJsonDocument &jsonBuffer );
  void getHiddenApps();
  void toggleHiddenApp( String appName, bool add = true );

  #if !defined FS_CAN_CREATE_PATH
    void scanDataFolder();
  #endif

  bool copyFile( const char* src, const char* dst );
  bool trashFile( String path );

  // inherit espressif32-2.x.x file->name() to file->path() migration handler from M5StackUpdater
  const char* (*fs_file_path)( fs::File *file ) = SDUpdater::fs_file_path;

  static String gnu_basename( String path )
  {
    char *base = strrchr(path.c_str(), '/');
    return base ? String( base+1) : path;
  }

};

