#!/bin/bash

curl -v -H "Authorization: token $GH_TOKEN" --retry 5 "https://api.github.com/repos/tobozo/M5Stack-SD-Updater/releases" | jq -r ".[] | select(.tag_name==\"$TRAVIS_TAG\")" | jq ".assets[] | select(.name=\"$ARCHIVE_ZIP\")" | jq "select(.browser_download_url | contains(\"untagged\") == true ) .browser_download_url"


# curl -v `curl -v -H "Authorization: token $GH_TOKEN" --retry 5 "https://api.github.com/repos/tobozo/M5Stack-SD-Updater/releases" | jq -r ".[] | select(.tag_name | contains(\"untagged\"))" | jq -r ".assets[]" | jq "select(.name==\"SD-Apps-Folder.zip\")" | jq -r "select(.browser_download_url | contains(\"untagged\") ) .browser_download_url"`
