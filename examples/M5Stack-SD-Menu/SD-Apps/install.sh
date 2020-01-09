#!/bin/bash

gem install git.io

if [ -f $SDAPP_FOLDER/install_$TRAVIS_BRANCH.sh ]; then
  source $SDAPP_FOLDER/install_$TRAVIS_BRANCH.sh

  cd $TRAVIS_BUILD_DIR;
  mkdir -p ~/Arduino/libraries
  # link the project's folder into the libraries folder
  ln -s $PWD ~/Arduino/libraries/.

else
  echo "No install script for this branch"
fi

echo "Installing extra libraries"
source $SDAPP_FOLDER/get-deps.sh
