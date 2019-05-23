#!/bin/bash

export inofile=$SDAPP_FOLDER/../$EXAMPLE.ino
export outfile=$SDAPP_FOLDER/../downloader.h

[ -f $inofile ] && echo "Compiling $inofile"

[ -f $inofile ] && arduino --preserve-temp-files --verbose-build --verify $PWD/examples/$EXAMPLE/$EXAMPLE.ino &>/dev/null
[ -f $inofile ] && find /tmp -name \*.partitions.bin -exec rm {} \; #
[ -f $inofile ] && find /tmp -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/TobozoLauncher.bin \; #
[ -f $inofile ] && cp $M5_SD_BUILD_DIR/TobozoLauncher.bin $M5_SD_BUILD_DIR/menu.bin

[ -f $inofile ] && echo "Compiling $outfile"

[ -f $outfile ] && sed -i -e 's/"\/sd-updater";/"\/sd-updater\/unstable"/g' $outfile
[ -f $outfile ] && arduino --preserve-temp-files --verbose-build --verify $PWD/examples/$EXAMPLE/$EXAMPLE.ino &>/dev/null
[ -f $outfile ] && find /tmp -name \*.partitions.bin -exec rm {} \; #
[ -f $outfile ] && find /tmp -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/BetaLauncher.bin \; # 

echo "Fake Binary" >> $M5_SD_BUILD_DIR/Downloader.bin
echo "Main APPs Compilation successful, now compiling deps"
