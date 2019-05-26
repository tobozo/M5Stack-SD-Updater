#!/bin/bash

gem install git.io
git submodule update --init --recursive
cd $SDAPP_FOLDER
# pull latest code from submodules
git submodule foreach --recursive git pull origin master
cd $TRAVIS_BUILD_DIR;
mkdir -p ~/Arduino/libraries
# link the project's folder into the libraries folder
ln -s $PWD ~/Arduino/libraries/.  
echo "Installing extra libraries"
cd $SDAPP_FOLDER
./get-deps.sh 
 
