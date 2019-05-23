#!/bin/bash

curl -v -H "Authorization: token $GH_TOKEN" --retry 5 "https://api.github.com/repos/tobozo/M5Stack-SD-Updater/releases" | jq -r ".[].draft"

