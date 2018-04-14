#export BOARD="espressif:esp32:m5stack-core-esp32:FlashFreq=80"
#export SDAPP_FOLDER=$PWD/examples/M5Stack-SD-Menu/SD-Apps

for D in *; do
  if [ -d "${D}" ]; then
    echo "moving to ${D}";
    cd ${D};
    ls -la
    egrep -R M5StackUpdater && egrep -R updateFromFS && export m5enabled=1 || export m5enabled=0;
    if (( $m5enabled == 1 )); then
      export PATH_TO_INO_FILE="$(find . -type f -iname *.ino)";
      echo "Compiling ${PATH_TO_INO_FILE}";
      arduino --verbose-build --verify --board $BOARD $PATH_TO_INO_FILE >> $SDAPP_FOLDER/out.log;
      ls $TRAVIS_BUILD_DIR/build -la
    else
      echo "Not compiling ${PATH_TO_INO_FILE} as it needs injection first";
    fi
    cd ..
  fi
done
