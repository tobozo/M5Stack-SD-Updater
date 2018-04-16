function movebin {
 find /tmp -name \*.partitions.bin -exec rm {} \; #<-- you need that backslash before and space after the semicolon
 find /tmp -name \*.ino.bin -exec rename 's/.ino.bin/.bin/' {} \; #
 find /tmp -name \*.bin -exec rename 's/(_for)?(_|-)?(m5)_?(stack)?(-|_)?//ig' {} \; #
 export BIN_FILE=`basename $( find /tmp -name \*.bin )`
 find /tmp -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/ \; #<-- you need that backslash before and space after the semicolon
 echo $BIN_FILE
}

function injectupdater {
  export outfile=$1;
  awk '/#include <M5Stack.h>/{print;print "#include <M5StackUpdater.h>";next}1' $outfile > tmp && mv tmp $outfile;
  awk '/M5.begin()/{print;print "  if(digitalRead(BUTTON_A_PIN) == 0) { updateFromFS(SD); ESP.restart(); } ";next}1' $outfile > tmp && mv tmp $outfile;
  # the M5StackUpdater requires Wire.begin(), inject it if necessary
  egrep -R "Wire.begin()" || (awk '/M5.begin()/{print;print "  Wire.begin();";next}1' $outfile > tmp && mv tmp $outfile);
}

function populatemeta {
  export IMG_NAME=${BIN_FILE%.bin}_gh.jpg
  export REPO_URL=`git config remote.origin.url`
  export REPO_OWNER_URL=`echo ${REPO_URL%/*}`
  export AVATAR_URL=$REPO_OWNER_URL.png?size=200
  # no gist in URL is valid to retrieve the profile pic
  AVATAR_URL=`sed 's/gist.//g' <<< $AVATAR_URL`
  echo "**** Will download avatar from $AVATAR_URL and save it as $JPEG_NAME from $BIN_FILE"
  wget $AVATAR_URL --output-document=temp
  convert temp -resize 120x120 $M5_SD_BUILD_DIR/jpg/$IMG_NAME
  identify $M5_SD_BUILD_DIR/jpg/$IMG_NAME
  rm temp
}

cp -R $TRAVIS_BUILD_DIR/examples/M5Stack-SD-Menu/SD-Content/jpg $M5_SD_BUILD_DIR/
cp -R $TRAVIS_BUILD_DIR/examples/M5Stack-SD-Menu/SD-Content/json $M5_SD_BUILD_DIR/


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
         echo "Renaming Tetris to M5Stack-Tetris + changing path to bg image"
         mv Tetris.ino M5Stack-Tetris.ino
         sed -i 's/tetris.jpg/\/jpg\/tetris_bg.jpg/g' M5Stack-Tetris.ino
         cp tetris.jpg $M5_SD_BUILD_DIR/jpg/tetris_bg.jpg
      ;;
#      'SpaceDefense-m5stack')
#      ;;
      'M5_LoRa_Frequency_Hopping')
        outfile=M5_Lora_Frequency_Hopping.ino
        awk '/M5Stack.h/{print;print "#include <M5Widget.h>";next}1' $outfile > tmp && mv tmp $outfile;
        # gotta patch the library too
        outfile=/home/travis/Arduino/libraries/M5Widgets-master/src/M5QRCode.h
        egrep -R "utility/qrcode" $outfile || (sed -i 's/qrcode.h/utility\/qrcode.h/g' $outfile);
      ;;
      'M5Stack_FlappyBird_game')
        # put real comments prevent syntax error
        sed -i -e 's/#By Ponticelli Domenico/\/\/By Ponticelli Domenico/g' M5Stack_FlappyBird.ino
      ;;
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
    echo "**** Compiling ${PATH_TO_INO_FILE}";

    arduino --preserve-temp-files --verify --board $BOARD $PATH_TO_INO_FILE >> $SDAPP_FOLDER/out.log && movebin && populatemeta

    ls $M5_SD_BUILD_DIR -la;

    cd ..
  fi
done

egrep -R M5StackUpdater $SDAPP_FOLDER/* 
egrep -R updateFromFS $SDAPP_FOLDER/*

