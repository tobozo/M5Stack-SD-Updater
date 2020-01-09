#!/bin/bash

function movebin {
 find /tmp -name \*.partitions.bin -exec rm {} \; #<-- you need that backslash before and space after the semicolon
 #find /tmp -name \*.ino.elf -exec rename 's/.ino.elf/.ino.bin/' {} \; # sometimes arduino produces ELF, sometimes it's BIN
 find /tmp -name \*.ino.bin -exec rename 's/.ino.bin/.bin/' {} \; #
 find /tmp -name \*.bin -exec rename 's/(_for)?(_|-)?(m5)_?(stack)?(-|_)?//ig' {} \; #
 export DIRTY_BIN_FILE=`basename $( find /tmp -name \*.bin )`
 export BIN_FILE="${DIRTY_BIN_FILE^}"
 export DIRTY_FILE_BASENAME=${DIRTY_BIN_FILE%.bin}
 export FILE_BASENAME=${BIN_FILE%.bin}
 find /tmp -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/ \; #<-- you need that backslash before and space after the semicolon
 if [ "$BIN_FILE" != "$DIRTY_BIN_FILE" ]; then
   mv $M5_SD_BUILD_DIR/$DIRTY_BIN_FILE $M5_SD_BUILD_DIR/$BIN_FILE
   echo "[++++] UpperCasedFirst() $DIRTY_BIN_FILE to $BIN_FILE"
 fi
 echo $BIN_FILE
}

function injectupdater {
  export outfile=$PATH_TO_INO_FILE;
  echo "***** Injecting $1"
  awk '/#include <M5Stack.h>/{print;print "#include <M5StackUpdater.h>\nSDUpdater sdUpdater;";next}1' $outfile > tmp && mv tmp $outfile;
  awk '/M5.begin()/{print;print "  if(digitalRead(BUTTON_A_PIN) == 0) { sdUpdater.updateFromFS(SD); ESP.restart(); } ";next}1' $outfile > tmp && mv tmp $outfile;
  # the M5StackUpdater requires Wire.begin(), inject it if necessary
  egrep -R "Wire.begin()" || (awk '/M5.begin()/{print;print "  Wire.begin();";next}1' $outfile > tmp && mv tmp $outfile);
  # the display driver changed, get rid of default rotation in setup
  sed -i -e 's/M5.Lcd.setRotation(0);/\/\//g' $outfile
  # remove any hardcoded credentials so wifi auth can be done from another app (e.g. wifimanager)
  sed -i -e 's/WiFi.begin(ssid, password);/WiFi.begin();/g' $outfile
  sed -i -e 's/WiFi.begin(SSID, PASSWORD);/WiFi.begin();/g' $outfile
  echo "***** Injection successful"
}

function populatemeta {
  echo "***** Populating meta"
  export IMG_NAME=${FILE_BASENAME}_gh.jpg
  export REPO_URL=`git config remote.origin.url`
  export REPO_OWNER_URL=`echo ${REPO_URL%/*}`
  export REPO_USERNAME=$(echo "$REPO_URL" | cut -d "/" -f4)
  export AVATAR_URL=$REPO_OWNER_URL.png?size=200
  export JSONFILE="$M5_SD_BUILD_DIR/json/$FILE_BASENAME.json"
  export IMGFILE="$M5_SD_BUILD_DIR/jpg/$FILE_BASENAME.jpg"
  export AVATARFILE="$M5_SD_BUILD_DIR/jpg/${FILE_BASENAME}_gh.jpg"

  if [ -f $JSONFILE ]; then
    echo "JSON Meta file $JSONFILE exists, should check for contents or leave it be"
  else
    if [ -f "$M5_SD_BUILD_DIR/json/$DIRTY_FILE_BASENAME.json" ]; then
      echo "[++++] UpperCasedFirst() $DIRTY_FILE_BASENAME.json, renaming other meta components"
      mv $M5_SD_BUILD_DIR/json/$DIRTY_FILE_BASENAME.json $JSONFILE
      mv $M5_SD_BUILD_DIR/jpg/$DIRTY_FILE_BASENAME.jpg $IMGFILE &>/dev/null
      mv $M5_SD_BUILD_DIR/jpg/${DIRTY_FILE_BASENAME}_gh.jpg $AVATARFILE &>/dev/null
      sed -i -e "s/$DIRTY_FILE_BASENAME/$FILE_BASENAME/g" $JSONFILE &>/dev/null
    else
      echo "[++++] No $JSONFILE JSON Meta file found, creating from the ether"
      export REPO_SHORTURL=`git.io $REPO_URL`
      if [ "" != "$REPO_SHORTURL" ]; then
        echo "{\"width\":120,\"height\":120, \"authorName\":\"@$REPO_USERNAME\", \"projectURL\": \"$REPO_SHORTURL\",\"credits\":\"$REPO_OWNER_URL\"}" > $JSONFILE
      else
        echo "{\"width\":120,\"height\":120, \"authorName\":\"@$REPO_USERNAME\", \"projectURL\": \"$REPO_URL\",\"credits\":\"$REPO_OWNER_URL\"}" > $JSONFILE
      fi
    fi
  fi
  cat $JSONFILE
  # no gist in URL is valid to retrieve the profile pic
  AVATAR_URL=`sed 's/gist.//g' <<< $AVATAR_URL`
  if [ ! -f $AVATARFILE ]; then
    echo "**** Will download avatar from $AVATAR_URL and save it as $AVATARFILE"
    wget –-quiet $AVATAR_URL --output-document=temp
    convert temp -resize 120x120 $AVATARFILE
    identify $AVATARFILE
    rm temp
  fi
  echo "***** Populating successful"
}
