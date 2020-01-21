#!/bin/bash

# Credits: https://github.com/lstux/OdroidGO/

VERBOSE=0
ARDUINO_BOARD="esp32:esp32:odroid_esp32"
SKETCHBOOK_PATH=""

usage() {
  exec >&2
  [ -n "${1}" ] && printf "Error : ${1}\n"
  printf "Usage : $(basename "${0}") [options] {sketch_dir|sketch_file.ino|sketch_file.bin}\n"
  printf "  Create a .fw file for Odroid-Go from arduino sketch\n"
  printf "options:\n"
  printf "  -b build_path  : build in build_path [/tmp/ino2fw]\n"
  printf "  -t tile.png    : use specified image as tile (resized to 86x48px)\n"
  printf "  -l label       : set application label\n"
  printf "  -d description : set application description\n"
  printf "  -v             : increase verbosity level\n"
  printf "  -h             : display this help message\n"
  exit 1
}

prn() { printf "$(date '+%Y/%m/%d %H:%M:%S') [$1] "; printf "$2\n"; }
msg() { prn MSG "$1"; }
log() { [ ${VERBOSE} -ge 1 ] || return 0; prn LOG "$1" >&2; }
dbg() { [ ${VERBOSE} -ge 2 ] || return 0; prn DBG "$1" >&2; }
err() { prn ERR "$1" >&2; [ -n "${2}" ] && exit ${2}; }

checkbin() {
  local bin="${1}" package="${2}"
  which "${1}" >/dev/null 2>&1 && return 0
  err "can't find '${1}' binary in PATH, make sure that '${2}' package is installed" 2
}


FW_TILE=""
FW_LABEL=""
FW_DESC=""
BUILD_PATH="/tmp/ino2fw"
while getopts b:t:l:d:vh opt; do case "${opt}" in
  b) BUILD_PATH="${OPTARG}";;
  t) [ -e "${OPTARG}" ] || usage "[ERROR] FW_TILE unreachable: ${OPT_ARG}"; FW_TILE="${OPTARG}";;
  l) FW_LABEL="${OPTARG}";;
  d) FW_DESC="${OPTARG}";;
  v) VERBOSE=$(expr ${VERBOSE} + 1);;
  *) usage "[WARNING] strange opt arg: ${OPT_ARG}";;
esac; done
shift $(expr ${OPTIND} - 1)

if [ -d "${1}" ]; then
  SKETCH_DIR="$(readlink -m "${1}")"
  SKETCH_FILE="${SKETCH_DIR}/$(basename "${SKETCH_DIR}").ino"
else
  if [ -e "${1}" ]; then
    SKETCH_FILE="$(readlink -m "${1}")"
    SKETCH_DIR="$(dirname "${1}")"
  else
    usage "[ERROR] arg 1 ${1} does not exist"
  fi
fi
[ -e "${SKETCH_FILE}" ] || usage "[ERROR] No '$(basename "${SKETCH_FILE}")' file found in '${SKETCH_DIR}'"

# TODO : what if no .arduino15/preferences.txt is available?
if ! [ -n "${SKETCHBOOK_PATH}" ]; then
  if [ -e "${HOME}/.arduino15/preferences.txt" ]; then
   SKETCHBOOK_PATH="$(sed -n 's/^sketchbook.path=//p' "${HOME}/.arduino15/preferences.txt")"
   log "set sketchbook.path to '${SKETCHBOOK_PATH}'"
  fi
fi

# Create ${BUILD_PATH} directory (will be used as build.path for arduino)
if ! [ -d "${BUILD_PATH}" ]; then
  dbg "creating '${BUILD_PATH}' directory"
  install -d "${BUILD_PATH}"
fi

if [[ $SKETCH_FILE == *".bin" ]]; then
  cp ${SKETCH_FILE} ${BUILD_PATH}/
  SKETCH_BIN="$(basename "${SKETCH_FILE}")"
fi

# Try to find a tile if none was specified on cmdline
if ! [ -e "${FW_TILE}" ]; then
  FW_TILE="${SKETCH_DIR}/$(basename "${SKETCH_DIR}").png"
  if [ -e "${FW_TILE}" ]; then
    log "using '${FW_TILE}' as tile"
  else
    FW_TILE="${SKETCH_DIR}/tile.png"
    if [ -e "${FW_TILE}" ]; then
      log "using '${FW_TILE}' as tile"
    else
      log "no tile specified, using default"
      checkbin base64 coreutils
      FW_TILE="${BUILD_PATH}/tile.png"
      sed -e '0,/^### EMBEDDED_BASE64_START tile.png/d' -e '/^### EMBEDDED_BASE64_END tile.png/,$d' "${0}" | base64 --decode > "${FW_TILE}"
    fi
  fi
fi

# Set application label if none was specified on cmdline
if ! [ -n "${FW_LABEL}" ]; then
  FW_LABEL="$(sed -n 's/^[/\* ]*Label *: *//p' "${SKETCH_FILE}")"
  if ! [ -n "${FW_LABEL}" ]; then
    FW_LABEL="$(basename "${SKETCH_DIR}")"
    read -p "Enter application label [${FW_LABEL}] : " label
    [ -n "${label}" ] && FW_LABEL="${label}"
  else
    log "using label found in .ino file : ${FW_LABEL}"
  fi
fi

# Set application description if none was specified on cmdline
if ! [ -n "${FW_DESC}" ]; then
  FW_DESC="$(sed -n 's/^[/\* ]*Description *: *//p' "${SKETCH_FILE}")"
  if ! [ -n "${FW_DESC}" ]; then
    FW_DESC="$(basename "${SKETCH_DIR}") for Odroid-GO"
    read -p "Enter application description [${FW_DESC}] : " desc
    [ -n "${desc}" ] && FW_DESC="${desc}"
  else
    log "using description found in .ino file : ${FW_DESC}"
  fi
fi

if [[ "$SKETCH_BIN" == "" ]]; then

  echo "Compiling ino sketch"
  checkbin arduino arduino
  log "building sketch with arduino"
  dbg "arduino --pref build.path=${BUILD_PATH} --pref sketchbook.path='${SKETCHBOOK_PATH}' --verify '${SKETCH_FILE}'"
  arduino --pref build.path=${BUILD_PATH} --pref sketchbook.path="${SKETCHBOOK_PATH}" --verify "${SKETCH_FILE}" || exit 4

  # Produce fw file using mkfw
  SKETCH_BIN="$(basename "${SKETCH_DIR}").ino.bin"

else

  echo "Skipping build as a bin name has been provided";

fi

# Create "raw" tile using ffmpeg
checkbin ffmpeg ffmpeg
log "creating tile.raw from ${FW_TILE}"
dbg "ffmpeg -i ${FW_TILE} -vf scale=86:48 -f rawvideo -pix_fmt rgb565 '${BUILD_PATH}/tile.raw'"
ffmpeg -i ${FW_TILE} -vf scale=86:48 -f rawvideo -pix_fmt rgb565 "${BUILD_PATH}/tile.raw" || exit 8


BIN_SIZE="$(du -sb "${BUILD_PATH}/${SKETCH_BIN}" | awk '{print $1}')"
log "building fw file"
dbg "mkfw '${FW_DESC}' tile.raw 0 16 ${BIN_SIZE} '${FW_LABEL}' '${SKETCH_BIN}'"
cd "${BUILD_PATH}" && mkfw "${FW_DESC}" tile.raw 0 16 ${BIN_SIZE} "${FW_LABEL}" "${SKETCH_BIN}" || exit 16

log "copying fw file to '${SKETCH_DIR}'"
cp firmware.fw "${SKETCH_DIR}/${FW_LABEL}.fw" || exit 32

ls -la

exit 0


### EMBEDDED_BASE64_START tile.png
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
### EMBEDDED_BASE64_END tile.png
