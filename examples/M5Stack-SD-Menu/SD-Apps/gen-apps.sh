function movebin {
 find /tmp -name \*.bin -exec cp {} $TRAVIS_BUILD_DIR/build/ \; #<-- you need that backslash before and space after the semicolon
}

function injectupdater {
  awk '/#include <M5Stack.h>/{print;print "#include <M5StackUpdater.h>";next}1' $1 > $1;
  awk '/Wire.begin()/{print;print "  if(digitalRead(BUTTON_A_PIN) == 0) { updateFromFS(SD); ESP.restart(); } ";next}1' $1 > $1;
}


movebin

for D in *; do
  if [ -d "${D}" ]; then
    echo "moving to ${D}";
    cd ${D};
    ls -la
    egrep -R M5StackUpdater && egrep -R updateFromFS && export m5enabled=1 || export m5enabled=0;
    if (( $m5enabled == 1 )); then

      case "$D" in

      'M5-Colours-Demo')
        echo "Downloading/applying Display.h patch"
        wget https://gist.githubusercontent.com/Kongduino/36d152c81bbb1214a2128a2712ecdd18/raw/8ac549b98595856123359cbbd61444f16079bb99/Colours.h
        cat Colours.h >> /home/travis/Arduino/libraries/M5Stack-0.1.7/src/utility/Display.h
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

    movebin
    ls $TRAVIS_BUILD_DIR/build -la;

    cd ..
  fi
done