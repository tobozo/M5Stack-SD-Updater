#!/bin/bash

# REQUIRES: $IDE_VERSION, $M5_SD_BUILD_DIR

date -u
uname -a
git fetch -t
env | sort
git log `git describe --tags --abbrev=0 HEAD^ --always`..HEAD --oneline 

/sbin/start-stop-daemon --start --quiet --pidfile /tmp/custom_xvfb_1.pid --make-pidfile --background --exec /usr/bin/Xvfb -- :1 -ac -screen 0 1280x1024x16
sleep 3
export DISPLAY=:1.0
export JAVA_ARGS="-Djavax.jmdns.level=OFF"
wget http://downloads.arduino.cc/arduino-$IDE_VERSION-linux64.tar.xz
tar xf arduino-$IDE_VERSION-linux64.tar.xz
mv arduino-$IDE_VERSION ~/arduino-ide
rm arduino-$IDE_VERSION-linux64.tar.xz
export PATH=$PATH:~/arduino-ide
mkdir -p $M5_SD_BUILD_DIR
