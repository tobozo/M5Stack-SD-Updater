#!/bin/bash


if [ -f $SDAPP_FOLDER/install_$TRAVIS_BRANCH.sh ]; then
  source $SDAPP_FOLDER/install_$TRAVIS_BRANCH.sh
else
  echo "No install script for this branch"
fi


