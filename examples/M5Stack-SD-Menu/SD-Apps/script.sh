#!/bin/bash

source $SDAPP_FOLDER/gen-menu.sh

if [ "$TRAVIS_BRANCH" != "master" ]; then
  # UNSTABLE
  export git_version_last="$(git describe --abbrev=0 --tags --always)"
  export git_version_next="v$(echo $git_version_last | awk -F . '{ printf "%d.%d.%d", $1,$2,$3 + 1}')"

  echo "Computed last version : $git_version_last"
  echo "Computed next version : $git_version_next"

  echo "Will download last binaries"
  #export LAST_SDAPP_FILE="SD-Apps-Folder.zip"
  #curl --retry 5 "https://api.github.com/repos/tobozo/M5Stack-SD-Updater/releases/latest?access_token=$GH_TOKEN" | jq -r ".assets[0].browser_download_url" | wget --output-document=$LAST_SDAPP_FILE -i -
  curl -H "Authorization: token $GH_TOKEN" --retry 5 "https://api.github.com/repos/tobozo/M5Stack-SD-Updater/releases" | jq -r ".[] | select(.tag_name==\"$git_version_last\")" | jq -r ".assets[] | select(.name==\"$ARCHIVE_ZIP\")  .browser_download_url" | wget --output-document=$ARCHIVE_ZIP -q -i -
  if [ -f $ARCHIVE_ZIP ]; then
     echo "$ARCHIVE_ZIP found"
     ls $ARCHIVE_ZIP -la
  else
    echo "ERROR: Could not find a valid $ARCHIVE_ZIP from latest releases, time to tune up jq queries ?"
    sleep 5
    exit 1
  fi

  unzip -d /tmp/$ARCHIVE_ZIP $ARCHIVE_ZIP
  cp -Ruf /tmp/$ARCHIVE_ZIP/* $M5_SD_BUILD_DIR/

  echo "Fetching precompiled projects"
  source $SDAPP_FOLDER/get-precompiled.sh
  ls $M5_SD_BUILD_DIR/ -la
  sleep 15 # give some time to the logs to come up

else
  # MASTER

#if [ ! -z "$TRAVIS_TAG" ]; then
#    # zip the package if tagged build
#    tools/build-release.sh -a$ESP32_GITHUB_TOKEN
#else
#    # run cmake and sketch tests
#    tools/check_cmakelists.sh && tools/build-tests.sh
#fi

  source $SDAPP_FOLDER/gen-apps.sh
fi
