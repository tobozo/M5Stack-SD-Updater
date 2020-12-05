#!/bin/bash

# Generate TobozoLauncher.bin and BetaLauncher.bin


cd $TRAVIS_BUILD_DIR;
arduino --pref "compiler.warning_level=none" --save-prefs   &>/dev/null
arduino --pref "build.warn_data_percentage=75" --save-prefs   &>/dev/null
arduino --pref "boardsmanager.additional.urls=https://dl.espressif.com/dl/package_esp32_index.json" --save-prefs   &>/dev/null
arduino --install-boards esp32:esp32 &>/dev/null
arduino --board $BOARD --save-prefs &>/dev/null

export inofile=$SDAPP_FOLDER/../$EXAMPLE.ino
export outfile=$SDAPP_FOLDER/../downloader.h

if [ -f $inofile ]; then
  echo "Compiling $inofile"
  arduino --preserve-temp-files --verbose-build --verify $inofile &>/dev/null
  find /tmp/arduino* -name \*.partitions.bin -exec rm {} \; #
  find /tmp/arduino* -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/TobozoLauncher.bin \; #
  if [ -f $M5_SD_BUILD_DIR/TobozoLauncher.bin ]; then
    cp $M5_SD_BUILD_DIR/TobozoLauncher.bin $M5_SD_BUILD_DIR/menu.bin
  else
    echo "ERROR: Failed to compile $inofile, aborting"
    sleep 5
    exit 1
  fi
else
  echo "ERROR: cannot compile menu.bin"
  sleep 5
  exit 1
fi

if [ -f $outfile ]; then
  echo "Attempting to enable unstable channel by patching $outfile"
  sed -i -e 's/"\/sd-updater"/"\/sd-updater\/unstable"/g' $outfile
  grep unstable $outfile && export patchok=1 || export patchok=0
  if (( $patchok == 1 )); then
    echo "Compiling Beta $inofile"
    arduino --preserve-temp-files --verbose-build --verify $inofile &>/dev/null
    find /tmp/arduino* -name \*.partitions.bin -exec rm {} \; #
    find /tmp/arduino* -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/BetaLauncher.bin \; #
    if [ -f $M5_SD_BUILD_DIR/BetaLauncher.bin ]; then
      # fine
      echo "SUCCESS: compiled BetaLauncher.bin from $inofile"
    else
      echo "ERROR: Failed to compile BetaLauncher.bin from $inofile, aborting"
      sleep 5
      exit 1
    fi
  else
    echo "ERROR: Patching unstable channel failed !!";
    sleep 5
    exit 1
  fi
else
  echo "ERROR: cannot compile BetaLauncher.bin"
  sleep 5
  exit 1
fi

echo "Fake Binary" >> $M5_SD_BUILD_DIR/Downloader.bin
echo "Launchers Compilation successful"
