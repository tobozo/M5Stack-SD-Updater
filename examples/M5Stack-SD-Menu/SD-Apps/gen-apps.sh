#export BOARD="espressif:esp32:m5stack-core-esp32:FlashFreq=80"
#export SDAPP_FOLDER=$PWD/examples/M5Stack-SD-Menu/SD-Apps


wget https://gist.githubusercontent.com/Kongduino/36d152c81bbb1214a2128a2712ecdd18/raw/8ac549b98595856123359cbbd61444f16079bb99/Colours.h
cat Colours.h >> /home/travis/Arduino/libraries/M5Stack-0.1.7/src/utility/Display.h 

for D in *; do
  if [ -d "${D}" ]; then
    echo "moving to ${D}";
    cd ${D};
    ls -la
    egrep -R M5StackUpdater && egrep -R updateFromFS && export m5enabled=1 || export m5enabled=0;
    if (( $m5enabled == 1 )); then
      export PATH_TO_INO_FILE="$(find ${SDAPP_FOLDER}/${D} -type f -iname *.ino)";
      echo "Compiling ${PATH_TO_INO_FILE}";
      arduino --verify --board $BOARD $PATH_TO_INO_FILE >> $SDAPP_FOLDER/out.log;
      ls $TRAVIS_BUILD_DIR/build -la
    else
      echo "Not compiling ${D} as it needs injection first";
    fi
    cd ..
  fi
done
