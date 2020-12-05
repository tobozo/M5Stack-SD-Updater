#!/bin/bash

# REQUIRES: $IDE_VERSION, $M5_SD_BUILD_DIR

#date -u
#uname -a
#git fetch -t
#env | sort
#git log `git describe --tags --abbrev=0 HEAD^ --always`..HEAD --oneline



if [[ "$IDE_VERSION" != "" ]]; then

  /sbin/start-stop-daemon --start --quiet --pidfile /tmp/custom_xvfb_1.pid --make-pidfile --background --exec /usr/bin/Xvfb -- :1 -ac -screen 0 1280x1024x16
  sleep 3
  export DISPLAY=:1.0
  export JAVA_ARGS="-Djavax.jmdns.level=OFF"
  wget --quiet "http://downloads.arduino.cc/arduino-$IDE_VERSION-linux64.tar.xz" # &>/dev/null 2>&1
  if [ -f arduino-$IDE_VERSION-linux64.tar.xz ]; then
    echo "Downloaded arduino-$IDE_VERSION-linux64.tar.xz"
  else
    echo "Failed to download arduino-$IDE_VERSION-linux64.tar.xz"
    sleep 5
    exit 1
  fi
  tar xf arduino-$IDE_VERSION-linux64.tar.xz  &>/dev/null
  mv arduino-$IDE_VERSION ~/arduino-ide
  rm arduino-$IDE_VERSION-linux64.tar.xz
  export PATH=$PATH:~/arduino-ide
  mkdir -p $M5_SD_BUILD_DIR

else

  echo "NO IDE VERSION !!"
  sleep 5
  exit 1

fi
