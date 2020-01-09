#!/bin/bash

my_dir="$(dirname "$0")"
source $my_dir/functions.sh

readonly ARDUINO_CI_SCRIPT_ARDUINO_OUTPUT_FILTER_REGEX='(^\[SocketListener\(travis-job-*|^  *[0-9][0-9]*: [0-9a-g][0-9a-g]*|^dns\[query,[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*:[0-9][0-9]*, length=[0-9][0-9]*, id=|^dns\[response,[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*:[0-9][0-9]*, length=[0-9][0-9]*, id=|^questions:$|\[DNSQuestion@|type: TYPE_IGNORE|^\.\]$|^\.\]\]$|^.\.\]$|^.\.\]\]$)'
readonly ARDUINO_CI_SCRIPT_SUCCESS_EXIT_STATUS=0
readonly ARDUINO_CI_SCRIPT_FAILURE_EXIT_STATUS=1

cp -R $TRAVIS_BUILD_DIR/examples/M5Stack-SD-Menu/SD-Content/jpg $M5_SD_BUILD_DIR/
cp -R $TRAVIS_BUILD_DIR/examples/M5Stack-SD-Menu/SD-Content/json $M5_SD_BUILD_DIR/
cp -R $TRAVIS_BUILD_DIR/examples/M5Stack-SD-Menu/SD-Content/mp3 $M5_SD_BUILD_DIR/

export hidecompilelogs=1


for D in *; do
  if [ -d "${D}" ]; then
    echo "moving to ${D}";
    cd ${D};
    # ls -la
    egrep -R M5StackUpdater && egrep -R updateFromFS && export m5enabled=1 || export m5enabled=0;
    if (( $m5enabled == 1 )); then

      case "$D" in

        'Pixel-Fun-M5Stack')
          echo "Renaming $D ino file"
          mv PixelFun.ino Pixel-Fun-M5Stack.ino
          sed -i -e 's/ILI9341/M5Display/g' Mover.cpp # https://github.com/neoxharsh/Pixel-Fun-M5Stack/issues/1
          #export hidecompilelogs=0
          export hidecompilelogs=1
        ;;

        'M5Stack_LovyanToyBox')
          export hidecompilelogs=1
          cd LovyanToyBox
        ;;

        'M5Stack-SetWiFi_Mic')
           echo "Duplicating Meta";
           cp -Rf microSD/jpg $M5_SD_BUILD_DIR/
           cp -Rf microSD/json $M5_SD_BUILD_DIR/
        ;;

        'M5Tube')
          #export hidecompilelogs=0
        ;;

        #*)
        #;;

      esac
      export PATH_TO_INO_FILE="$(find ${SDAPP_FOLDER}/${D} -type f -iname *.ino)";

    else

      export hidecompilelogs=1
      case "$D" in

        'M5Stack_LovyanToyBox')
          export hidecompilelogs=1
          cd LovyanToyBox
        ;;

        'M5StackSandbox')
           export hidecompilelogs=1
           cd SWRasterizer
           if [ -d "Sd-Content" ]; then
             cp -Rf Sd-Content/* $M5_SD_BUILD_DIR/
           fi
        ;;

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
        #'M5Stack-Pacman-JoyPSP')
        #;;
        #'M5Stack-SpaceShooter')
        #;;
        #'M5Stack-ESP32-Oscilloscope')
        #;;
        #'M5Stack-NyanCat')
        #;;
        #'M5Stack-Rickroll')
        #;;

        'M5Stack_NyanCat_Ext')
          echo "Renaming file to prevent namespace collision"
          mv M5Stack_NyanCat.ino M5Stack_NyanCat_Ext.ino
          wget -–quiet https://github.com/jimpo/nyancat/raw/master/nyancat.mp3 --output-document=$M5_SD_BUILD_DIR/mp3/NyanCat.mp3
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

        #'SpaceDefense-m5stack')
        #;;

        'M5Stack_FlappyBird_game')
          # put real comments to prevent syntax error
          sed -i -e 's/#By Ponticelli Domenico/\/\/By Ponticelli Domenico/g' M5Stack_FlappyBird.ino
        ;;

        #'M5Stack-PacketMonitor')
        #;;
        #'M5Stack_Sokoban')
        #;;

        'M5Stack-Thermal-Camera')
           echo "Renaming to M5Stack-Thermal-Camera.ino"
           mv thermal_cam_interpolate.ino M5Stack-Thermal-Camera.ino
        ;;

        'mp3-player-m5stack')
          echo "Changing mp3 path in sketch"
          # TODO: fix this
          sed -i 's/createTrackList("\/")/createTrackList("\/mp3")/g' mp3-player-m5stack.ino
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
    cd $SDAPP_FOLDER
  fi
done

ls $M5_SD_BUILD_DIR/jpg -la;
ls $M5_SD_BUILD_DIR/json -la;

# egrep -R M5StackUpdater $SDAPP_FOLDER/*
# egrep -R updateFromFS $SDAPP_FOLDER/*
