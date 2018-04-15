function movebin {
 find /tmp -name \*.partitions.bin -exec rm {} \; #<-- you need that backslash before and space after the semicolon
 find /tmp -name \*.ino.bin -exec rename 's/.ino.bin/.bin/' {} \; #
 find /tmp -name \*.bin -exec rename -v 's/(_for)?(_|-)?(M5|m5)_?((S|s)tack)?(-|_)//i' {} \; #
 export BIN_FILE=`basename $( find /tmp -name \*.bin )`
 find /tmp -name \*.bin -exec mv {} $TRAVIS_BUILD_DIR/build/ \; #<-- you need that backslash before and space after the semicolon
 echo $BIN_FILE
}

function injectupdater {
  awk '/#include <M5Stack.h>/{print;print "#include <M5StackUpdater.h>";next}1' $1 > $1;
  awk '/M5.begin()/{print;print "  if(digitalRead(BUTTON_A_PIN) == 0) { updateFromFS(SD); ESP.restart(); } ";next}1' $1 > $1;
}


for D in *; do
  if [ -d "${D}" ]; then
    echo "moving to ${D}";
    cd ${D};
    # ls -la
    egrep -R M5StackUpdater && egrep -R updateFromFS && export m5enabled=1 || export m5enabled=0;
    if (( $m5enabled == 1 )); then

      case "$D" in

      'M5-Colours-Demo')
        echo "Downloading/applying Display.h patch"
        wget https://gist.githubusercontent.com/Kongduino/36d152c81bbb1214a2128a2712ecdd18/raw/8ac549b98595856123359cbbd61444f16079bb99/Colours.h
        cat Colours.h >> /home/travis/Arduino/libraries/M5Stack-0.1.7/src/utility/Display.h
        rm Colours.h
      ;;

      'Pixel-Fun-M5Stack')
        echo "Renaming $D ino file"
        mv PixelFun.ino Pixel-Fun-M5Stack.ino
      ;;
      #*)
      #;;
      esac

    else

      case "$D" in
#      'M5Stack_CrackScreen')
#      ;;
#      'M5Stack-MegaChess')
#      ;;
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
      'M5Stack-Tetris')
         echo "Renaming Tetris to M5Stack-Tetris"
         mv Tetris.ino M5Stack-Tetris.ino
      ;;
#      'SpaceDefense-m5stack')
#      ;;
#      'M5_LoRa_Frequency_Hopping')
#      ;;
#      'M5Stack_FlappyBird_game')
#      ;;
#      'M5Stack-PacketMonitor')
#      ;;
#      'M5Stack_Sokoban')
#      ;;
#      'mp3-player-m5stack')
#      ;;
      esac

      injectupdater $PATH_TO_INO_FILE
    fi

    export PATH_TO_INO_FILE="$(find ${SDAPP_FOLDER}/${D} -type f -iname *.ino)";
    echo "Compiling ${PATH_TO_INO_FILE}";

    arduino --preserve-temp-files --verify --board $BOARD $PATH_TO_INO_FILE >> $SDAPP_FOLDER/out.log;

    export BIN_FILE=`movebin`
    export JPEG_NAME=${BIN_FILE%.bin}.jpg
    # cleanup redundant extensions
    #rename 's/.ino.bin/.bin/' $TRAVIS_BUILD_DIR/build/*.ino.bin
    # remove redundant "M5 Stack" variations from the filename
    #rename -v 's/(_for)?(_|-)?(M5|m5)_?((S|s)tack)?(-|_)//i' $TRAVIS_BUILD_DIR/build/*.bin
    ls $TRAVIS_BUILD_DIR/build -la;

    export REPO_URL=`git config remote.origin.url`
    export REPO_OWNER_URL=`echo ${REPO_URL%/*}`
    export AVATAR_URL=$( wget -O - $REPO_OWNER_URL | grep "avatar width-full" | sed -n 's/.*src="\([^"]*\)".*/\1/p' )
    wget $AVATAR_URL --output-document=$TRAVIS_BUILD_DIR/build/$JPEG_NAME

    #git config remote.origin.url

    cd ..
  fi
done
