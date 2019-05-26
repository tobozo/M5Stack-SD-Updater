#!/bin/bash

cd $TRAVIS_BUILD_DIR;
arduino --pref "compiler.warning_level=none" --save-prefs   &>/dev/null
arduino --pref "build.warn_data_percentage=75" --save-prefs   &>/dev/null
arduino --pref "boardsmanager.additional.urls=https://dl.espressif.com/dl/package_esp32_index.json" --save-prefs   &>/dev/null
arduino --install-boards esp32:esp32 &>/dev/null
arduino --board $BOARD --save-prefs   &>/dev/null
source $SDAPP_FOLDER/gen-menu.sh
source $SDAPP_FOLDER/gen-apps.sh

echo "Fetching precompiled projects"
./get-precompiled.sh
ls $M5_SD_BUILD_DIR/ -la
sleep 15 # give some time to the logs to come up   
