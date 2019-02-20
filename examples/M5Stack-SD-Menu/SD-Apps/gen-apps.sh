#!/bin/bash

function movebin {
 find /tmp -name \*.partitions.bin -exec rm {} \; #<-- you need that backslash before and space after the semicolon
 #find /tmp -name \*.ino.elf -exec rename 's/.ino.elf/.ino.bin/' {} \; # sometimes arduino produces ELF, sometimes it's BIN
 find /tmp -name \*.ino.bin -exec rename 's/.ino.bin/.bin/' {} \; # 
 find /tmp -name \*.bin -exec rename 's/(_for)?(_|-)?(m5)_?(stack)?(-|_)?//ig' {} \; #
 export DIRTY_BIN_FILE=`basename $( find /tmp -name \*.bin )`
 export BIN_FILE="${DIRTY_BIN_FILE^}"
 export DIRTY_FILE_BASENAME=${DIRTY_BIN_FILE%.bin}
 export FILE_BASENAME=${BIN_FILE%.bin}
 find /tmp -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/ \; #<-- you need that backslash before and space after the semicolon
 if [ "$BIN_FILE" != "$DIRTY_BIN_FILE" ]; then
   mv $M5_SD_BUILD_DIR/$DIRTY_BIN_FILE $M5_SD_BUILD_DIR/$BIN_FILE
   echo "[++++] UpperCasedFirst() $DIRTY_BIN_FILE to $BIN_FILE"
 fi
 echo $BIN_FILE
}

function injectupdater {
  export outfile=$PATH_TO_INO_FILE;
  echo "***** Injecting $1"
  awk '/#include <M5Stack.h>/{print;print "#include <M5StackUpdater.h>\nSDUpdater sdUpdater;";next}1' $outfile > tmp && mv tmp $outfile;
  awk '/M5.begin()/{print;print "  if(digitalRead(BUTTON_A_PIN) == 0) { sdUpdater.updateFromFS(SD); ESP.restart(); } ";next}1' $outfile > tmp && mv tmp $outfile;
  # the M5StackUpdater requires Wire.begin(), inject it if necessary
  egrep -R "Wire.begin()" || (awk '/M5.begin()/{print;print "  Wire.begin();";next}1' $outfile > tmp && mv tmp $outfile);
  # the display driver changed, get rid of default rotation in setup
  sed -i -e 's/M5.Lcd.setRotation(0);/\/\//g' $outfile
  # remove any hardcoded credentials so wifi auth can be done from another app (e.g. wifimanager)
  sed -i -e 's/WiFi.begin(ssid, password);/WiFi.begin();/g' $outfile
  sed -i -e 's/WiFi.begin(SSID, PASSWORD);/WiFi.begin();/g' $outfile
  echo "***** Injection successful"
}

function populatemeta {
  echo "***** Populating meta"
  export IMG_NAME=${FILE_BASENAME}_gh.jpg
  export REPO_URL=`git config remote.origin.url`
  export REPO_OWNER_URL=`echo ${REPO_URL%/*}`
  export REPO_USERNAME=$(echo "$REPO_URL" | cut -d "/" -f4)
  export AVATAR_URL=$REPO_OWNER_URL.png?size=200
  export JSONFILE="$M5_SD_BUILD_DIR/json/$FILE_BASENAME.json"
  export IMGFILE="$M5_SD_BUILD_DIR/jpg/$FILE_BASENAME.jpg"
  export AVATARFILE="$M5_SD_BUILD_DIR/jpg/${FILE_BASENAME}_gh.jpg"

  if [ -f $JSONFILE ]; then
    echo "JSON Meta file $JSONFILE exists, should check for contents or leave it be"
  else
    if [ -f "$M5_SD_BUILD_DIR/json/$DIRTY_FILE_BASENAME.json" ]; then
      echo "[++++] UpperCasedFirst() $DIRTY_FILE_BASENAME.json, renaming other meta components"
      mv $M5_SD_BUILD_DIR/json/$DIRTY_FILE_BASENAME.json $JSONFILE
      mv $M5_SD_BUILD_DIR/jpg/$DIRTY_FILE_BASENAME.jpg $IMGFILE &>/dev/null
      mv $M5_SD_BUILD_DIR/jpg/${DIRTY_FILE_BASENAME}_gh.jpg $AVATARFILE &>/dev/null
      sed -i -e "s/$DIRTY_FILE_BASENAME/$FILE_BASENAME/g" $JSONFILE &>/dev/null
    else
      echo "[++++] No $JSONFILE JSON Meta file found, creating from the ether"
      echo "{\"width\":110,\"height\":110, \"authorName\":\"@$REPO_USERNAME\", \"projectURL\": \"$REPO_URL\",\"credits\":\"$REPO_OWNER_URL\"}" > $JSONFILE
    fi
  fi
  cat $JSONFILE
  # no gist in URL is valid to retrieve the profile pic
  AVATAR_URL=`sed 's/gist.//g' <<< $AVATAR_URL`
  echo "**** Will download avatar from $AVATAR_URL and save it as $AVATARFILE"
  wget $AVATAR_URL --output-document=temp
  convert temp -resize 120x120 $AVATARFILE
  identify $AVATARFILE
  rm temp

  echo "***** Populating successful"
}

readonly ARDUINO_CI_SCRIPT_ARDUINO_OUTPUT_FILTER_REGEX='(^\[SocketListener\(travis-job-*|^  *[0-9][0-9]*: [0-9a-g][0-9a-g]*|^dns\[query,[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*:[0-9][0-9]*, length=[0-9][0-9]*, id=|^dns\[response,[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*:[0-9][0-9]*, length=[0-9][0-9]*, id=|^questions:$|\[DNSQuestion@|type: TYPE_IGNORE|^\.\]$|^\.\]\]$|^.\.\]$|^.\.\]\]$)'
readonly ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS=0
readonly ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS=1

cp -R $TRAVIS_BUILD_DIR/examples/M5Stack-SD-Menu/SD-Content/jpg $M5_SD_BUILD_DIR/
cp -R $TRAVIS_BUILD_DIR/examples/M5Stack-SD-Menu/SD-Content/json $M5_SD_BUILD_DIR/
cp -R $TRAVIS_BUILD_DIR/examples/M5Stack-SD-Menu/SD-Content/mp3 $M5_SD_BUILD_DIR/



for D in *; do
  if [ -d "${D}" ]; then
    echo "moving to ${D}";
    cd ${D};
    export hidecompilelogs=1
    # ls -la
    egrep -R M5StackUpdater && egrep -R updateFromFS && export m5enabled=1 || export m5enabled=0;
    if (( $m5enabled == 1 )); then

      case "$D" in

      'Pixel-Fun-M5Stack')
        echo "Renaming $D ino file"
        mv PixelFun.ino Pixel-Fun-M5Stack.ino
        sed -i -e 's/ILI9341/M5Display/g' Mover.cpp # https://github.com/neoxharsh/Pixel-Fun-M5Stack/issues/1
        #export hidecompilelogs=0
      ;;
      #*)
      #;;
      esac
      export PATH_TO_INO_FILE="$(find ${SDAPP_FOLDER}/${D} -type f -iname *.ino)";

    else

      case "$D" in
       'd_invader')
         echo "Should replace esp_deep_sleep => esp_sleep"
         # esp_deep_sleep => esp_sleep
       ;;
       'M5Stack_CrackScreen')
         echo "Fixing path to crack.jpg"
         sed -i 's/\/crack.jpg/\/jpg\/crack.jpg/g' M5Stack_CrackScreen.ino
         cp crack.jpg $M5_SD_BUILD_DIR/jpg/crack.jpg
       ;;

       'M5Stack-CrazyAsteroids')
         cat Crazy_Asteroid.ino EntrySection.ino ExitSection.ino printAsteroid.ino printSpaceShip.ino > out.blah
         rm *.ino
         mv out.blah M5Stack-CrazyAsteroids.ino
       ;;

       'M5Stack_Particle_demo')
         # this is an Arduino compatible Platformio project
         mv main.cpp M5Stack_Particle_demo.ino
         sed -i -e 's/define LCD_WIDTH 320/define LCD_WIDTH M5.Lcd.width() \/\//g' M5Stack_Particle_demo.ino
         sed -i -e 's/define LCD_HEIGHT 240/define LCD_HEIGHT M5.Lcd.height() \/\//g' M5Stack_Particle_demo.ino
       ;;

       'M5Stack_WebRadio_Avator')
         echo "Patching esp8266Audio with getLevel()"
         sed -i -e 's/bool SetOutputModeMono/int getLevel();\nbool  SetOutputModeMono/g' ~/Arduino/libraries/ESP8266Audio-master/src/AudioOutputI2S.h
         sed -i -e 's/include "AudioOutputI2S.h"/include "AudioOutputI2S.h"\n\n int aout_level = 0; int AudioOutputI2S::getLevel() { return aout_level; }/g' ~/Arduino/libraries/ESP8266Audio-master/src/AudioOutputI2S.cpp
         sed -i -e 's/int16_t l/aout_level = (int)sample[RIGHTCHANNEL];\nint16_t  l/g' ~/Arduino/libraries/ESP8266Audio-master/src/AudioOutputI2S.cpp
       ;;

       'M5Stack-WiFiScanner')
         # remove unnecessary include causing an error
         sed -i -e 's/#include <String.h>/\/\//g' M5Stack-WiFiScanner.ino
       ;;
       #'M5Stack-MegaChess')
       #  # fix rotation problem caused by display driver changes (now applied globally)
       #  sed -i -e 's/M5.Lcd.setRotation(0);/\/\//g' arduinomegachess_for_m5stack.ino
       #;;
#      'M5Stack-Pacman-JoyPSP')
#      ;;
#      'M5Stack-SpaceShooter')
#      ;;
#      'M5Stack-ESP32-Oscilloscope')
#      ;;
#      'M5Stack-NyanCat')
#      ;;
#      'M5Stack-Rickroll')
#      ;;
      'M5Stack_NyanCat_Ext')
        echo "Renaming file to prevent namespace collision"
        mv M5Stack_NyanCat.ino M5Stack_NyanCat_Ext.ino
        wget https://github.com/jimpo/nyancat/raw/master/nyancat.mp3 --output-document=$M5_SD_BUILD_DIR/mp3/NyanCat.mp3
        sed -i 's/\/NyanCat.mp3/\/mp3\/NyanCat.mp3/g' M5Stack_NyanCat_Ext.ino
      ;;

      'M5Stack_lifegame')
        echo "Renaming file to .ino"
        mv M5Stack_lifegame M5Stack_lifegame.ino
      ;;
      'M5Stack-Tetris')
         echo "Renaming Tetris to M5Stack-Tetris + changing path to bg image"
         mv Tetris.ino M5Stack-Tetris.ino
         sed -i 's/\/tetris.jpg/\/jpg\/tetris_bg.jpg/g' M5Stack-Tetris.ino
         cp tetris.jpg $M5_SD_BUILD_DIR/jpg/tetris_bg.jpg
      ;;
#      'SpaceDefense-m5stack')
#      ;;

      'M5Stack_FlappyBird_game')
        # put real comments prevent syntax error
        sed -i -e 's/#By Ponticelli Domenico/\/\/By Ponticelli Domenico/g' M5Stack_FlappyBird.ino
      ;;
#      'M5Stack-PacketMonitor')
#      ;;
#      'M5Stack_Sokoban')
#      ;;
      'M5Stack-Thermal-Camera')
         echo "Renaming to M5Stack-Thermal-Camera.ino"
         mv thermal_cam_interpolate.ino M5Stack-Thermal-Camera.ino
      ;;
      'mp3-player-m5stack')
        echo "Changing mp3 path in sketch"
        # TODO: fix this
        sed -i 's/createTrackList("\/")/createTrackList("\/mp3\/")/g' mp3-player-m5stack.ino
      ;;
      esac
      export PATH_TO_INO_FILE="$(find ${SDAPP_FOLDER}/${D} -type f -iname *.ino)";
      injectupdater # $PATH_TO_INO_FILE
    fi

    echo "**** Compiling ${PATH_TO_INO_FILE}";

    #arduino --preserve-temp-files --verify --board $BOARD $PATH_TO_INO_FILE >> $SDAPP_FOLDER/out.log && movebin && populatemeta
    set +o errexit
    # shellcheck disable=SC2086
    # eval \"arduino --preserve-temp-files --verify --board $BOARD $PATH_TO_INO_FILE\" &>/dev/null | tr --complement --delete '[:print:]\n\t' | tr --squeeze-repeats '\n' | grep --extended-regexp --invert-match "$ARDUINO_CI_SCRIPT_ARDUINO_OUTPUT_FILTER_REGEX"
    if (( $hidecompilelogs == 0 )); then
      arduino --preserve-temp-files --verbose-build --verify --board $BOARD $PATH_TO_INO_FILE
    else
      arduino --preserve-temp-files --verbose-build --verify --board $BOARD $PATH_TO_INO_FILE &>/dev/null
    fi
    # local -r arduinoPreferenceSettingExitStatus="${PIPESTATUS[0]}"
    export arduinoPreferenceSettingExitStatus="${PIPESTATUS[0]}"
    set -o errexit
    #arduino --preserve-temp-files --verify --board $BOARD $PATH_TO_INO_FILE | tr --complement --delete '[:print:]\n\t' | tr --squeeze-repeats '\n' | grep --extended-regexp --invert-match "$ARDUINO_CI_SCRIPT_ARDUINO_OUTPUT_FILTER_REGEX"
    #  local -r arduinoInstallPackageExitStatus="${PIPESTATUS[0]}"
    #if [[ "$arduinoPreferenceSettingExitStatus" != "$ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS" ]]; then
      movebin && populatemeta
    #else
    #  echo "**** Bad exit status"
    #fi
    ls $M5_SD_BUILD_DIR -la;
    cd ..
  fi
done

ls $M5_SD_BUILD_DIR/jpg -la;
ls $M5_SD_BUILD_DIR/json -la;

# egrep -R M5StackUpdater $SDAPP_FOLDER/* 
# egrep -R updateFromFS $SDAPP_FOLDER/*

