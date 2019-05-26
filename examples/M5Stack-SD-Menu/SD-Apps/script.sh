#!/bin/bash

cd $TRAVIS_BUILD_DIR;
arduino --pref "compiler.warning_level=none" --save-prefs   &>/dev/null
arduino --pref "build.warn_data_percentage=75" --save-prefs   &>/dev/null
arduino --pref "boardsmanager.additional.urls=https://dl.espressif.com/dl/package_esp32_index.json" --save-prefs   &>/dev/null
arduino --install-boards esp32:esp32 &>/dev/null
arduino --board $BOARD --save-prefs   &>/dev/null
source $SDAPP_FOLDER/gen-menu.sh


if [ "$TRAVIS_BRANCH" != "master" ]; then
  # only rebuild all when master is updated
  echo "Skipping rebuild, will download last binaries"
  export LAST_SDAPP_FILE="SD-Apps-Folder.zip"
  #curl --retry 5 "https://api.github.com/repos/tobozo/M5Stack-SD-Updater/releases/latest?access_token=$GH_TOKEN" | jq -r ".assets[0].browser_download_url" | wget --output-document=$LAST_SDAPP_FILE -i -
  curl -H "Authorization: token $GH_TOKEN" --retry 5 "https://api.github.com/repos/tobozo/M5Stack-SD-Updater/releases" | jq -r ".[] | select(.tag_name==\"unstable\")" | jq -r ".assets[] | select(.name==\"$ARCHIVE_ZIP\")  .browser_download_url" | wget --output-document=$ARCHIVE_ZIP -i -  &>/dev/null
  if [ -f $ARCHIVE_ZIP ]; then
     echo "$ARCHIVE_ZIP found"
  else
    echo "Could not find a valid $ARCHIVE_ZIP from latest releases, time to tune up jq queries ?"
    sleep 5
    exit 1
  fi
  unzip -d /tmp/$ARCHIVE_ZIP $ARCHIVE_ZIP
  cp -Ruf /tmp/$ARCHIVE_ZIP/* $M5_SD_BUILD_DIR/
  sleep 5
  exit 0
else
  source $SDAPP_FOLDER/gen-apps.sh
fi









echo "Fetching precompiled projects"
./get-precompiled.sh
ls $M5_SD_BUILD_DIR/ -la
sleep 15 # give some time to the logs to come up   
