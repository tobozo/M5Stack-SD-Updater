#!/bin/bash

/sbin/start-stop-daemon --start --quiet --pidfile /tmp/custom_xvfb_1.pid --make-pidfile --background --exec /usr/bin/Xvfb -- :1 -ac -screen 0 1280x1024x16
sleep 3
export DISPLAY=:1.0
export PATH=$PATH:~/arduino-ide
export logDir=$WORK_DIR/artifacts
export arduino_installed=0
export platformio_installed=0
export needs_firmware=0
export repo_branch=$REPO_BRANCH
export varTagName=`$WORK_SPACE/tools/git-describe.awk $WORK_SPACE`

prn() { printf "$(date '+%Y/%m/%d %H:%M:%S') [$1] "; printf "$2\n"; }
msg() { prn MSG "$1"; }
log() { [ ${VERBOSE} -ge 1 ] || return 0; prn LOG "$1" >&2; }
dbg() { [ ${VERBOSE} -ge 2 ] || return 0; prn DBG "$1" >&2; }
err() { prn ERR "$1" >&2; [ -n "${2}" ] && exit ${2}; }


if [[ "$M5_SD_BUILD_DIR" == "" ]]; then
  echo "[ERROR] Workflow did not set M5_SD_BUILD_DIR variable"
  exit 1
fi

mkdir -p $M5_SD_BUILD_DIR
if [[ "$M5_BURNER_DIR" != "" ]]; then
  mkdir -p $M5_BURNER_DIR/firmware_4MB
fi
mkdir -p ~/Arduino/libraries
mkdir -p $logDir

# set -o xtrace
# set -v


function m5burner_json {
  # $1: path to the JSON file (required, will be overwritten)
  # create JSON file as in M5Burner standards
  if [[ "$1" == "" ]]; then
    echo "[ERROR] No m5burner.json path provided"
    exit 1
  fi

  tee $1 << XXX
{
    "name": "$SDCardAppNameSpace",
    "description": "$repo_desc",
    "keywords": "M5Stack-App-Registry generated firwmare",
    "author": "$repo_author",
    "repository": "$repo_url",
    "firmware_category": [
        {
            "Stack-4MB": {
                "path": "firmware_4MB",
                "device": [
                    "M5Stack 4MB Model (default partition)"
                ],
                "default_baud": 921600
            }
        }
    ],
    "version": "$varTagName",
    "framework": "$platform"
}

XXX

}


function get_arduino_app {
  # $1: URL to the repo (required)
  # $2: repo name (required)
  if [ "$1" == "" ]; then
    echo "[FAIL] Can't install a repo without a URL!!"
    exit 1
  else
    appURL=$1
    # TODO: find out if the URL points to a repo or to a zip file
  fi
  if [ "$2" == "" ]; then
    echo "[FAIL] Can't install a repo without a name!!"
    exit 1
  else
    repoName=$2
  fi
  cd $WORK_DIR
  if [ "$3" == "" ]; then
    echo "[INFO] Cloning $repoName @ $appURL into $WORK_DIR"
    git clone --depth=1 $appURL $repoName
  else
    branchName=$3
    git clone --depth=1 -b $branchName $appURL $repoName
  fi
  cd $repoName
  if [[ "$pre_hook" != "" && "$pre_hook" != "null" ]]; then
    echo "[INFO] Running pre-hook : $pre_hook"
    eval $pre_hook
  fi
}


function gen_arduino_app {
  # requires : $REPO_URL, $REPO_NAME, $REPO_BRANCH, $inofile, $outfile, $M5_SD_BUILD_DIR, $WORK_DIR
  cd $WORK_DIR
  if [ ! -d "$repo_name" ]; then
    get_arduino_app $repo_url $repo_name $repo_branch
  else
    cd $WORK_DIR
    cd $repo_name
  fi
  #if [[ "$pre_hook" != "" && "$pre_hook" != "null" ]]; then
  #  echo "[INFO] Running pre-hook : $pre_hook"
  #  eval $pre_hook
  #fi
  egrep -R M5StackUpdater && egrep -R updateFromFS && export m5enabled=1 || export m5enabled=0;
  if (( $m5enabled == 1 )); then
    echo "[INFO] This app is already using the Sd-Updater library"
  else
    injectupdater
  fi
  copy_assets
  if [ -f $inofile ]; then
    echo "[INFO] Compiling $inofile"
    arduino --preserve-temp-files --verbose-build --verify $inofile >> $logDir/compilation.log
    #movebin "/tmp/arduino_build*"
    newmovebin "/tmp/arduino_build*"
    #if [ -f $M5_SD_BUILD_DIR/$outfile ]; then
    #  echo "[SUCCESS] $inofile has been compiled and saved into $outfile"
    #else
    #  echo "[ERROR] Failed to compile $inofile, aborting"
    #  sleep 5
    #  exit 1
    #fi
    #populatemeta
    newpopulatemeta
  else
    echo "[ERROR] could not compile $outfile from $inofile"
    sleep 5
    exit 1
  fi
  if [[ "$post_hook" != "" && "$post_hook" != "null" ]]; then
    echo "[INFO] Running post-hook : $post_hook"
    eval $post_hook
  fi
}


function install_arduino_lib {
  # $1: URL to the zip file (required)
  # $2: archive name (optional)
  if [ "$1" == "" ]; then
    echo "    [FAIL] Can't install a library without a URL!!"
    exit 1
  else
    repoURL=$1
    # TODO: check that the url points to a zip file
    if [[ "$repoURL" == *.zip ]]; then
      echo "    [INFO] library URL is a zip file"
    else
      echo "    [WARNING] library URL is not a zip file, will be extrapolated"
      if [[ "$repoURL" == *"github"* ]]; then
        if [[ "${repoURL: -1}" == "/" ]]; then
          repoURL=${repoURL}archive/master.zip
        else
          repoURL=$repoURL/archive/master.zip
        fi
      else
        echo "    [ERROR] Can't extrapolate from a non github address, URL needs to point to a zip file"
      fi
    fi
  fi
  if [ "$2" == "" ]; then
    if [[ "$repoURL" == *"github"* ]]; then
      # repoOwner=`echo ${repoURL} | cut -d/ -f4`
      repoName=`echo ${repoURL} | cut -d/ -f5`
      # repoSlug="${repoOwner}_${repoName}"
      zipName="${repoName}.zip"
    else
      #echo ${repoURL} | cut -d/ -f5
      zipName="archive.zip"
    fi
  else
    zipName=$2
  fi
  if [ "$3" == "" ]; then
    #  no dirname provided
    libdir=~/Arduino/libraries
    if [[ "$repoURL" == *"github"* && "$zipName" == "archive.zip" ]]; then
      # TODO extract lib name from repoURL ?
      libdir=~/Arduino/libraries/${zipName%.zip}
    fi
  else
    libdir=~/Arduino/libraries/$3
  fi
  mkdir -p $libdir
  ls -la
  HTTP_RESPONSE=$(curl -L --silent --write-out "HTTPSTATUS:%{http_code}" $repoURL --output $zipName)
  ls $zipName -la
  if [ $? -ne 0 ]; then
    echo "    [FAIL] $? => aborting";
    exit 1;
  fi
  unzip -qq -o -d $libdir $zipName && export zip_worked=true
  if [ "$zip_worked" == "true" ]; then
    echo "    [OK] $repoURL"
    rm $zipName
    ls $libdir -la
  else
    echo "    [FAIL] Bad Zip archive for $repoURL"
    sleep 5
    exit 1
  fi
}


function copy_assets {
  # assets inventory
  if [ "$precompiled" == "1" ]; then
    echo "Assets:"
    echo "-------"
    assetFolders=`jq -r '.repo.precompiled | keys[] as $k | "\($k)"' $appJSONPath`
    for assetFolder in $(echo $assetFolders | sed 's/[[:space:]]/\n/g'); do  # | sed s/:/\\n/
      echo "  [$assetFolder]"
      if [[ "$assetFolder" == "bin" ]]; then
        destpath=$M5_SD_BUILD_DIR
        if [[ "$include_bin_assets" == "0" ]]; then
          continue
        fi
      else
        destpath=$M5_SD_BUILD_DIR/$assetFolder
        mkdir -p $destpath
      fi
      for asset in $(echo "$jsonCode" | jq -r .repo.precompiled.$assetFolder[]); do
        echo "    - $asset"
        # TODO: copy those files to the build folder
        basename_asset=`basename $asset`
        cp "$asset" "$destpath/$basename_asset"
      done
    done
    echo
  fi
}



function get_remote_app {

  if [[ "$1" == "" ]]; then
    echo "[ERROR] No appJSONPath provided"
    exit 1
  fi

  appJSONPath=/tmp/app.json
  echo "[INFO] Fetching meta file $1 into $appJSONPath"
  HTTP_RESPONSE=$(curl -L --silent --write-out "HTTPSTATUS:%{http_code}" $1 --output $appJSONPath)
  if [ $? -ne 0 ]; then
    echo "[FAIL] $? => aborting";
    sleep 5
    exit 1;
  fi
  source=0
  precompiled=0
  has_deps=0
  has_source=0
  jsonCode=`cat $appJSONPath`
  # echo "Fetched json code:\n${jsonCode}\n"
  repo_url=`echo "${jsonCode}" | jq -r .repo.url`
  repo_name=`echo "${jsonCode}" | jq -r .repo.name`
  repo_desc=`echo "${jsonCode}" | jq -r .repo.description`
  repo_author=`echo "${jsonCode}" | jq -r .author.name`
  repo_updated_at=`echo "${jsonCode}" | jq -r .repo.updated_at`
  libDepsResp=`echo "$jsonCode" | jq -r .repo.source.lib_deps`
  sourceFilesResp=`echo "$jsonCode" | jq -r .repo.source.ino`
  expectedOutFileName=`echo "$jsonCode" | jq -r .repo.source.bin_name`
  SDCardAppNameSpace=`echo "$jsonCode" | jq -r .repo.source.sd_namespace`

  platform=`echo "$jsonCode" | jq -r .repo.source.platform`
  pre_hook=`echo "$jsonCode" | jq -r '.repo.source | .["pre-hook"]'`
  post_hook=`echo "$jsonCode" | jq -r '.repo.source | .["post-hook"]'`
  app_name=`echo "$jsonCode" | jq -r .repo.name`

  # collect bundle types
  for row in $(echo "${jsonCode}" | jq -r .repo.type[]); do
    echo "Found available bundle format: ${row}"
    if [ "${row}" == "source" ]; then
      source=1 # json has source tree
    fi
    if [ "${row}" == "precompiled" ]; then
      precompiled=1 # json has bin tree
    fi
  done
  echo
  # evaluate if the source version has library dependencies and is usable
  if [[ "$libDepsResp" == "null" || "$libDepsResp" == "" || "$libDepsResp" == '[]' ]]; then
    has_deps=0 # json has no library deps
  else
    has_deps=1 # json has library deps
  fi
  if [ "$sourceFilesResp" == "null" ]; then
    echo "[INFO] Json has no ino_file"
    has_source=0 # json has no ino_file
  else
    has_source=1 # json has ino_file
    export ino_array=`echo "$jsonCode" | jq -r .repo.source.ino[]`
    export array_size=`echo $ino_array | wc -l`
  fi

  if [ "$has_source" == "1" ]; then
    if [[ "$expectedOutFileName" == "null"  || "$expectedOutFileName" == ""  ]]; then
      if [[ "$SDCardAppNameSpace" == "null"  || "$SDCardAppNameSpace" == ""  ]]; then
        echo "[ERROR] Neither bin_name nor sd_namespace found in JSON"
        echo "[ERROR] Please implement repo.source.bin_name or repo.source.sd_namespace in package $1"
        echo "[INFO] TODO: hook delete and regen JSON"
        exit 1
      else
        expectedOutFileName="$SDCardAppNameSpace.bin"
        echo "[INFO] Using namespace : $SDCardAppNameSpace";
        echo "[WARNING] Forced bin_name from namespace : $SDCardAppNameSpace"
      fi
    else
      echo "[INFO] Using bin_name = $expectedOutFileName"
      if [[ "$SDCardAppNameSpace" == "null"  || "$SDCardAppNameSpace" == ""  ]]; then
        SDCardAppNameSpace=${expectedOutFileName%.bin}
        echo "[WARNING] Forced namespace from bin_name : $SDCardAppNameSpace"
      else
        echo "[INFO] Using namespace : $SDCardAppNameSpace";
      fi
    fi
  else
    SDCardAppNameSpace=${repo_name}
  fi

  include_bin_assets=0

  # build information
  if [ "$source" == "1" ]; then
    echo "Source code:"
    echo "------------"
    echo "  Platform: $platform"
    if [[ $platform == *"arduino"* ]]; then
      board=`echo "$jsonCode" | jq -r .repo.source.board`
      install_arduino
      get_arduino_app $repo_url $repo_name $repo_branch
      # install_arduino_libraries
      echo "  Board: $board"
      if [ "$has_deps" == "1" ]; then
        echo "    Library requirements:"
        for libraryURL in $(echo "${jsonCode}" | jq -r .repo.source.lib_deps[]); do
          install_arduino_lib $libraryURL
        done
      else
        install_arduino_libraries
      fi
      if [ "$has_source" == "1" ]; then
        if [[ "$inofile_forced" != "" ]]; then
          # echo "  [FORCED] Will conpile $inofile_forced"
          inofile=$inofile_forced
          gen_arduino_app
        else
          if [ $array_size > 1 ]; then
            for inofile in $(echo $ino_array | sed 's/\n/\n/g'); do
              # echo "  [ARRAY] Will compile $inofile"
              gen_arduino_app
              break
            done
          else
            inofile=$ino_array
            # echo "  [INFO] Will compile $ino_array"
            gen_arduino_app
          fi
        fi
      else
        # JSON says there's no ino/cpp source on this repo, use the binaries
        # get_arduino_app $repo_url $repo_name
        echo "Can't use the source on this one, let's hope the binaries are fine"
        include_bin_assets=1
        copy_assets
      fi

    else
      # platformio ?
      # get_arduino_app $repo_url $repo_name
      if [ "$source" == "1" ]; then
        get_arduino_app $repo_url $repo_name $repo_branch
        install_platformio
        if [ "$has_deps" == "1" ]; then
          echo "    Library requirements:"
          for libraryURL in $(echo "${jsonCode}" | jq -r .repo.source.lib_deps[]); do
            echo "    [INFO] installing library $libraryURL"
            python -m platformio  lib install $libraryURL
          done
        fi
        gen_platformio_app
      else
        echo "Can't use the source on this one, let's hope the binaries are fine"
        include_bin_assets=1
        copy_assets
      fi
    fi

    echo
  fi

  find $M5_SD_BUILD_DIR
  cd $M5_SD_BUILD_DIR

  if [[ "${FILE_BASENAME}" != "" ]]; then
    zipFileName=${FILE_BASENAME}.zip
  else
    zipFileName=${SDCardAppNameSpace}.zip
  fi

  zip -r $zipFileName ./*
  rm $M5_SD_BUILD_DIR/*.bin

  if [[ "$needs_firmware" != "0" ]]; then

    case $APP_BOARD in
      m5stack)
        echo "[INFO] Project needs M5Burner zip file too"
        cd $M5_BURNER_DIR
        m5burner_json m5burner.json
        # m5BurnerDirName=${SDCardAppNameSpace}-v${varTagName}/${SDCardAppNameSpace}
        # mkdir -p ${m5BurnerDirName} && mv firmware_4MB m5burner.json ${m5BurnerDirName}/
        zipFileName="M5Burner-$zipFileName"
        # cd ${m5BurnerDirName}
        # zip -r $zipFileName ./*
        zip -r $zipFileName *
        #zip -jrq $zipFileName ${m5BurnerDirName}
        cp $zipFileName $M5_SD_BUILD_DIR/
        echo "[INFO] M5Burner zip file created: $zipFileName"
      ;;
      odroid)
        echo "[INFO] Project needs OdroidFW zip file too"
        if [ ! -f $IMGFILE ]; then
          echo "[ERROR] can't make a firmware without a pic"
          exit 1
        fi
        cd $M5_BURNER_DIR

        # git clone https://github.com/othercrashoverride/odroid-go-firmware.git -b factory
        # cd odroid-go-firmware/tools/mkfw
        # make
        # chmod +x mkfw


        # git clone https://github.com/lstux/OdroidGO/
        # chmod +x OdroidGO/ino2fw/mkfw-build.sh OdroidGO/ino2fw/ino2fw.sh
        $WORK_SPACE/tools/mkfw-build.sh
        ls $WORK_DIR/$repo_name/$inofile -la
        ls $IMGFILE -la
        set -o xtrace
        set -v
        #echo "[INFO] sending command: $WORK_SPACE/tools/ino2fw.sh -t $IMGFILE -l $SDCardAppNameSpace -d \"${repo_desc}\" $WORK_DIR/$repo_name/$inofile"
        #$WORK_SPACE/tools/ino2fw.sh -t $IMGFILE -l $SDCardAppNameSpace -d "${repo_desc}" $WORK_DIR/$repo_name/$inofile
        # cp $M5_SD_BUILD_DIR/$expectedOutFileName firmware.bin
        echo "[INFO] sending command: $WORK_SPACE/tools/ino2fw.sh -t $IMGFILE -l $SDCardAppNameSpace -d \"${repo_desc}\" $M5_BURNER_DIR/$expectedOutFileName"
        $WORK_SPACE/tools/ino2fw.sh -t $IMGFILE -l $SDCardAppNameSpace -d "${repo_desc}" $M5_BURNER_DIR/$expectedOutFileName

        pwd
        ls -la

        cd $M5_BURNER_DIR
        # cp $M5_BURNER_DIR/$expectedOutFileName
        echo "$M5_BURNER_DIR :"
        ls $M5_BURNER_DIR -la

        zipFileName="OdroidFW-$zipFileName"
        zip -r $zipFileName ${SDCardAppNameSpace}.fw
        cp $zipFileName $M5_SD_BUILD_DIR/

        #ls -la

        #cp $zipFileName $M5_SD_BUILD_DIR/
        #    ino2fw.sh [options] {sketch_dir|sketch_file.ino}
        #      Create a .fw file for Odroid-Go from arduino sketch
        #    options:
        #      -b build_path  : build in build_path [/tmp/ino2fw]
        #      -t tile.png    : use specified image as tile (resized to 86x48px)
        #      -l label       : set application label
        #      -d description : set application description
        #      -v             : increase verbosity level
        #      -h             : display help message



        #ffmpeg -i $IMGFILE -f rawvideo -pix_fmt rgb565 tile.raw
        #cp $M5_SD_BUILD_DIR/$expectedOutFileName firmware.bin
        #ls -la
        #filesize=$(stat -c%s "$M5_BURNER_DIR/$expectedOutFileName")
        #blocks=$(($filesize/64))
        #normsize=$(((1+$blocks)*64))
        #echo "[INFO] will mkfw '$expectedOutFileName' with hardkernel's tool"
        #echo "[TODO] ./mkfw ${SDCardAppNameSpace} tile.raw 0 16 $normsize ${SDCardAppNameSpace} $M5_BURNER_DIR/$expectedOutFileName"
        ## ./mkfw test tile.raw 0 16 1048576 app $expectedOutFileName
        #dbg "./mkfw ${SDCardAppNameSpace} tile.raw 0 16 $normsize ${SDCardAppNameSpace} $expectedOutFileName"
        #mv firmware.fw $M5_BURNER_DIR/${SDCardAppNameSpace}.fw
        #cd $M5_BURNER_DIR
        #zipFileName="OdroidFW-$zipFileName"
        #zip -r $zipFileName $expectedOutFileName
        #cp $zipFileName $M5_SD_BUILD_DIR/
        echo "[INFO] OdroidFW zip file created: $zipFileName"

      ;;
      *)
        echo "[ERROR] bad APP_BOARD value: $APP_BOARD"
        exit 1
      ;;
    esac

  fi

  cd $M5_SD_BUILD_DIR

}



function install_platformio {
  egrep -R M5StackUpdater && egrep -R updateFromFS && export m5enabled=1 || export m5enabled=0;
  if (( $m5enabled == 1 )); then
    echo "[INFO] This app is already using the Sd-Updater library"
  else
    if [[ "$inofile" == "" ]]; then
      export ino_array=`echo "$jsonCode" | jq -r .repo.source.ino[]`
      # export array_size=`echo $ino_array | wc -l`
      inofile=$ino_array
    fi
    injectupdater
  fi
  if [[ "$platformio_installed" != "0" ]]; then
    echo "plaformio already installed"
    return
  fi
  pip install -U https://github.com/platformio/platformio/archive/develop.zip
  python -m platformio platform install https://github.com/platformio/platform-espressif32.git#feature/stage
  python -m platformio lib install m5stack-sd-updater
  python -m platformio lib install https://github.com/tobozo/ESP32-Chimera-Core
  platformio_installed=1
}


function gen_platformio_app {
  if [[ "$pre_hook" != "" && "$pre_hook" != "null" ]]; then
    echo "[INFO] Running pre-hook : $pre_hook"
    eval $pre_hook
  fi
  copy_assets
  echo "[INFO] Compiling $app_name"
  python -m platformio run  # >> $logDir/compilation.log
  #movebin ".pio/"
  #if [ -f $M5_SD_BUILD_DIR/$outfile ]; then
  #  echo "[SUCCESS] $inofile has been compiled and saved into $outfile"
  #else
  #  echo "[ERROR] Failed to compile $inofile, aborting"
  #  sleep 5
  #  exit 1
  #fi
  #populatemeta
  newmovebin ".pio/"
  newpopulatemeta
  if [[ "$post_hook" != "" && "$post_hook" != "null" ]]; then
    echo "[INFO] Running pre-hook : $post_hook"
    eval $post_hook
  fi
}




function install_arduino_libraries {
  install_arduino_lib https://github.com/adafruit/Adafruit_NeoPixel/archive/master.zip Adafruit_NeoPixel.zip
  install_arduino_lib https://github.com/adafruit/Adafruit_AMG88xx/archive/1.0.2.zip Adafruit_AMG88xx.zip
  install_arduino_lib https://github.com/bblanchon/ArduinoJson/archive/6.x.zip Arduinojson-master.zip
  install_arduino_lib https://github.com/tobozo/M5StackSAM/archive/patch-1.zip M5StackSAM-master.zip
  install_arduino_lib https://github.com/earlephilhower/ESP8266Audio/archive/master.zip ESP8266Audio.zip
  install_arduino_lib https://github.com/Seeed-Studio/Grove_BMP280/archive/1.0.1.zip Grove_BMP280.zip
  install_arduino_lib https://github.com/Gianbacchio/ESP8266_Spiram/archive/master.zip ESP8266_Spiram.zip
  install_arduino_lib http://www.buildlog.net/blog/wp-content/uploads/2018/02/Game_Audio.zip Game_Audio.zip Game_Audio
  install_arduino_lib https://github.com/lovyan03/M5Stack_TreeView/archive/master.zip M5Stack_TreeView.zip
  install_arduino_lib https://github.com/lovyan03/M5Stack_OnScreenKeyboard/archive/master.zip M5Stack_OnScreenKeyboard.zip
  install_arduino_lib https://github.com/kosme/arduinoFFT/archive/master.zip arduinoFFT.zip
}


function install_arduino {
  if [[ "$arduino_installed" != "0" ]]; then
    echo "arduino already installed"
    return
  fi
  cd $WORK_DIR
  if [[ "$platform" != "" ]]; then
    BOARD="esp32:esp32:$board:FlashFreq=80"
  else
    echo "[INFO] Using default board"
    BOARD="esp32:esp32:m5stack-core-esp32:FlashFreq=80"
  fi

  if [[ "$platform" != "" ]]; then
    export JAVA_ARGS="-Djavax.jmdns.level=OFF"
    wget --quiet "http://downloads.arduino.cc/$platform-linux64.tar.xz" # &>/dev/null 2>&1
    if [ -f $platform-linux64.tar.xz ]; then
      echo "[OK] Downloaded $platform-linux64.tar.xz"
    else
      echo "[FAIL] Failed to download $platform-linux64.tar.xz"
      sleep 5
      exit 1
    fi
    tar xf $platform-linux64.tar.xz  &>/dev/null
    mv $platform ~/arduino-ide
    rm $platform-linux64.tar.xz
    # export PATH=$PATH:~/arduino-ide

    releaseAddr=`sed "s/\/tag\//\/download\//g"<<<$(curl -s -D - https://github.com/espressif/arduino-esp32/releases/latest/ -o /dev/null | grep  -oP 'Location: \K.*(?=\r)')/package_esp32_index.json`

    echo "[OK] Successfully unpacked $platform-linux64.tar.xz and added ~/arduino-ide to PATH"
    arduino --pref "compiler.warning_level=none" --save-prefs   &>/dev/null
    arduino --pref "build.warn_data_percentage=75" --save-prefs   &>/dev/null
    arduino --pref "boardsmanager.additional.urls=$releaseAddr" --save-prefs   &>/dev/null
    # arduino --pref "boardsmanager.additional.urls=https://dl.espressif.com/dl/package_esp32_index.json" --save-prefs   &>/dev/null
    arduino --install-boards esp32:esp32 &>/dev/null
    arduino --board $BOARD --save-prefs &>/dev/null

    echo "[OK] Successfully installed esp32 board in Arduino IDE"

    # mkdir -p ~/Arduino/libraries
    wget --quiet https://github.com/tobozo/M5Stack-SD-Updater/archive/$SD_UPDATER_BRANCH.zip --output-document=SD-Updater.zip
    unzip -qq -d ~/Arduino/libraries SD-Updater.zip

    echo "[OK] Successfully installed SD-Updater library from $SD_UPDATER_BRANCH branch"

    # TODO: move this somewhere else
    cd ~/Arduino/libraries
    git clone $M5_CORE_URL

    echo "[OK] Successfully installed M5 Core from $M5_CORE_URL"
    export arduino_installed=1
  else

    echo "[FAIL] NO platform provided !!"
    sleep 5
    exit 1

  fi

}



#
# function parse_yaml (lol, you wish)
#
# https://stackoverflow.com/questions/5014632/how-can-i-parse-a-yaml-file-from-a-linux-shell-script
#
# usage: parse_yaml sample.yml
#
# Data example:
#
# ## global definitions
# global:
#   debug: yes
#   verbose: no
#   debugging:
#     detailed: no
#     header: "debugging started"
# output:
#    file: "yes"
#
# Will output:
#
# global_debug="yes"
# global_verbose="no"
# global_debugging_detailed="no"
# global_debugging_header="debugging started"
# output_file="yes"
#
#

function parse_yaml {
   local prefix=$2
   local s='[[:space:]]*' w='[a-zA-Z0-9_]*' fs=$(echo @|tr @ '\034')
   sed -ne "s|^\($s\):|\1|" \
        -e "s|^\($s\)\($w\)$s:$s[\"']\(.*\)[\"']$s\$|\1$fs\2$fs\3|p" \
        -e "s|^\($s\)\($w\)$s:$s\(.*\)$s\$|\1$fs\2$fs\3|p"  $1 |
   awk -F$fs '{
      indent = length($1)/2;
      vname[indent] = $2;
      for (i in vname) {if (i > indent) {delete vname[i]}}
      if (length($3) > 0) {
         vn=""; for (i=0; i<indent; i++) {vn=(vn)(vname[i])("_")}
         printf("%s%s%s=\"%s\"\n", "'$prefix'",vn, $2, $3);
      }
   }'
}


function newmovebin {
  if [ "$1" == "" ]; then
    binpath="/tmp/arduino_build*"
  else
    binpath=$1
  fi
  echo "[INFO] Searching $binpath"

  if [[ "$needs_firmware" != "0" ]]; then  # if [[ "$M5_BURNER_DIR" != "" ]]; then

    case $APP_BOARD in
      m5stack)
        echo "[INFO] Building M5Burner firmware package"
        find ~/.arduino15/packages/esp32/ -name "boot_app0.bin" -exec cp {} $M5_BURNER_DIR/firmware_4MB/boot_0xe000.bin \; #
        find ~/.arduino15/packages/esp32/ -name "bootloader_qio_80m.bin" -exec cp {} $M5_BURNER_DIR/firmware_4MB/bootloader_0x1000.bin \; #
        find $binpath -name \*partitions.bin -exec mv {} $M5_BURNER_DIR/firmware_4MB/partitions_0x8000.bin \; #
        find $binpath -name \*.bin -exec cp {} $M5_BURNER_DIR/firmware_4MB/${SDCardAppNameSpace}_0x10000.bin \; #
        echo "[INFO] Resuming on SD package"
      ;;
      odroid)
        echo "[INFO] Building Odroid-Go FW package"
        find $binpath -name \*.bin -exec cp {} $M5_BURNER_DIR/$expectedOutFileName \; #<-- you need that backslash before and space after the semicolon
        # TODO:
        # git clone https://github.com/othercrashoverride/odroid-go-firmware.git -b factory
        # cd odroid-go-firmware/tools/mkfw
        # make
      ;;
      *)
        echo "[ERROR] bad APP_BOARD value: $APP_BOARD"
        exit 1
      ;;
    esac

  else
    find $binpath -name \*partitions.bin -exec rm {} \; #<-- you need that backslash before and space after the semicolon
  fi

  find $binpath -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/$expectedOutFileName \; #<-- you need that backslash before and space after the semicolon
  echo "[INFO] Kept bin file: $expectedOutFileName"
  if [ ! -f $M5_SD_BUILD_DIR/$expectedOutFileName ]; then
    echo "[ERROR] bin copy failed : $M5_SD_BUILD_DIR/$expectedOutFileName"
    echo "[TODO]: escalate this"
    exit 1
  fi
}



function movebin {
  binpath=$1
  if [ "$binpath" == "" ]; then
    echo "[ERROR] Can't move bin file without path"
    exit 1
  fi
  # remove the "partitions.bin" file first
  find $binpath -name \*partitions.bin -exec rm {} \; #<-- you need that backslash before and space after the semicolon
  # blind rename ".ino.bin" to ".bin" (Arduino IDE does that)
  find $binpath -name \*.ino.bin -exec rename -v 's/.ino.bin/.bin/' {} \; # wut
  if [[ "$platform" == "platformio" || "$DIRTY_BIN_FILE" == "firmware.bin" ]]; then
    n=`find $binpath -name *.bin`
    dn=$(dirname $n)
    fn=$(basename $n)
    cfn=`echo "$fn" | perl -pe 's/(_for)?(_|-)?(m5)_?(stack)?(-|_)?//ig'`
    mv "$n" "${dn}/${app_name^}.bin"
    export DIRTY_BIN_FILE=`basename $( find $binpath -name \*.bin )`
    export BIN_FILE="${app_name^}.bin"
    export DIRTY_FILE_BASENAME=${app_name%.bin}
    # echo ${hello//[0-9]/}
  else

    if [ "$expectedOutFileName" == "null" ]; then
      # use sketch default bin filename
      find $binpath -name \*.bin -exec rename -v 's/(_for)?(_|-)?(m5)_?(stack)?(-|_)?//ig' {} \; #
      export DIRTY_BIN_FILE=`basename $( find $binpath -name \*.bin )`
      export BIN_FILE="${DIRTY_BIN_FILE^}"
      export DIRTY_FILE_BASENAME=${DIRTY_BIN_FILE%.bin}
    else
      # use inherited bin filename
      n=`find $binpath -name *.bin`
      dn=$(dirname $n)
      mv "$n" "${dn}/${expectedOutFileName}"
      export DIRTY_BIN_FILE=`basename $( find $binpath -name \*.bin )`
      export BIN_FILE="$expectedOutFileName"
      export DIRTY_FILE_BASENAME=${expectedOutFileName%.bin}
    fi
  fi
  export FILE_BASENAME=${BIN_FILE%.bin}
  find $binpath -name \*.bin -exec mv {} $M5_SD_BUILD_DIR/$BIN_FILE \; #<-- you need that backslash before and space after the semicolon
  #if [ "$BIN_FILE" != "$DIRTY_BIN_FILE" ]; then
  #  mv $M5_SD_BUILD_DIR/$DIRTY_BIN_FILE $M5_SD_BUILD_DIR/$BIN_FILE
  #  echo "[++++] UpperCasedFirst() $DIRTY_BIN_FILE to $BIN_FILE"
  #fi
  echo "[INFO] Keeping bin file: $BIN_FILE"
  export outfile=$BIN_FILE
}


function injectupdater {
  echo "[INFO] Injecting target $inofile"
  awk '/#include <M5Stack.h>/{print;print "#include <M5StackUpdater.h>\nSDUpdater sdUpdater;";next}1' $inofile > tmp && mv tmp $inofile;
  awk '/M5.begin()/{print;print "  if(digitalRead(BUTTON_A_PIN) == 0) { sdUpdater.updateFromFS(SD); ESP.restart(); } ";next}1' $inofile > tmp && mv tmp $inofile;
  # the M5StackUpdater requires Wire.begin(), inject it if necessary
  egrep -R "Wire.begin()" || (awk '/M5.begin()/{print;print "  Wire.begin();";next}1' $inofile > tmp && mv tmp $inofile);
  # the display driver changed, get rid of default rotation in setup
  sed -i -e 's/M5.Lcd.setRotation(0);/\/\//g' $inofile
  # remove any hardcoded credentials so wifi auth can be done from another app (e.g. wifimanager)
  sed -i -e 's/WiFi.begin(ssid, password);/WiFi.begin();/g' $inofile
  sed -i -e 's/WiFi.begin(SSID, PASSWORD);/WiFi.begin();/g' $inofile
  echo "[OK] Injection successful"
}


function newpopulatemeta {
  # $SDCardAppNameSpace
  echo "***** Populating meta"
  IMG_NAME=${SDCardAppNameSpace}_gh.jpg
  REPO_URL=`git config remote.origin.url`
  REPO_OWNER_URL=`echo ${REPO_URL%/*}`
  REPO_USERNAME=$(echo "$REPO_URL" | cut -d "/" -f4)
  AVATAR_URL=$REPO_OWNER_URL.png?size=200
  JSONFILE="$M5_SD_BUILD_DIR/json/$SDCardAppNameSpace.json"
  IMGFILE="$M5_SD_BUILD_DIR/jpg/$SDCardAppNameSpace.jpg"
  AVATARFILE="$M5_SD_BUILD_DIR/jpg/${SDCardAppNameSpace}_gh.jpg"
  if [ ! -f $JSONFILE ]; then
    echo "[WARNING] JSON Meta file $JSONFILE does not exists, will be created from the ether"
    mkdir -p $M5_SD_BUILD_DIR/json
    REPO_SHORTURL=`git.io $REPO_URL`
    if [ "" != "$REPO_SHORTURL" ]; then
      echo "{\"width\":120,\"height\":120, \"authorName\":\"@$REPO_USERNAME\", \"projectURL\": \"$REPO_SHORTURL\",\"credits\":\"$REPO_OWNER_URL\"}" > $JSONFILE
    else
      echo "{\"width\":120,\"height\":120, \"authorName\":\"@$REPO_USERNAME\", \"projectURL\": \"$REPO_URL\",\"credits\":\"$REPO_OWNER_URL\"}" > $JSONFILE
    fi
  fi
  if [ ! -f $AVATARFILE ]; then
    mkdir -p $M5_SD_BUILD_DIR/jpg
    AVATAR_URL=`sed 's/gist.//g' <<< $AVATAR_URL` # no gist in URL is valid to retrieve the profile pic
    echo "[WARNING] No avatar file found, will download from $AVATAR_URL and save it as $AVATARFILE"
    #wget --quiet $AVATAR_URL --output-document=temp
    wget $AVATAR_URL --output-document=temp
    convert temp -resize 120x120 -type TrueColor $AVATARFILE
    identify $AVATARFILE
    rm temp

  fi
  if [ ! -f $IMGFILE ]; then
    mkdir -p $M5_SD_BUILD_DIR/jpg
    # TODO: search in SD-Apps-folder.zip
    echo "[TODO] search in SD-Apps-folder.zip"
    echo "[WARNING] No img file found, will use a copy of avatar file $AVATARFILE"
    cp $AVATARFILE $IMGFILE
  fi
  echo "***** Populating successful"
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
      mkdir -p $M5_SD_BUILD_DIR/json
      mkdir -p $M5_SD_BUILD_DIR/jpg
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
    wget --quiet $AVATAR_URL --output-document=temp
    convert temp -resize 120x120 $AVATARFILE
    identify $AVATARFILE
    rm temp
  fi
  echo "***** Populating successful"
}
