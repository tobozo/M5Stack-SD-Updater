#export BOARD="espressif:esp32:m5stack-core-esp32:FlashFreq=80"
#export SDAPP_FOLDER=$PWD/examples/M5Stack-SD-Menu/SD-Apps

for D in *; do
  if [ -d "${D}" ]; then
    echo "${D}";
    cd ${D};
    egrep -R M5StackUpdater && egrep -R updateFromFS && export m5enabled=1 || export m5enabled=0;
    if (( $m5enabled == 1 )); then
      export PATH_TO_INO_FILE="$(find . -type f -iname *.ino)";
      echo $PATH_TO_INO_FILE;
      arduino --verbose-build --verify --board $BOARD $PATH_TO_INO_FILE;
    else
      echo bof;
    fi
    cd ..
  fi
done
