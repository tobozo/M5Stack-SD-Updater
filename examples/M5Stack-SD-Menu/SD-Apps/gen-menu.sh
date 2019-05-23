#!/bin/bash

arduino --preserve-temp-files --verbose-build --verify $PWD/examples/$EXAMPLE/$EXAMPLE.ino &>/dev/null
find /tmp -name \*.partitions.bin -exec rm {} \; #
find /tmp -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/TobozoLauncher.bin \; #
cp $M5_SD_BUILD_DIR/TobozoLauncher.bin $M5_SD_BUILD_DIR/menu.bin

export $outfile=$PWD/examples/$EXAMPLE/downloader.h
[ -f $outfile ] && sed -i -e 's/"\/sd-updater";/"\/sd-updater\/unstable"/g' $outfile
[ -f $outfile ] && arduino --preserve-temp-files --verbose-build --verify $PWD/examples/$EXAMPLE/$EXAMPLE.ino &>/dev/null
[ -f $outfile ] && find /tmp -name \*.partitions.bin -exec rm {} \; #
[ -f $outfile ] && find /tmp -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/menu_beta.bin \; # 

echo "Fake Binary" >> $M5_SD_BUILD_DIR/Downloader.bin
echo "Main APPs Compilation successful, now compiling deps"
