#!/bin/sh

# Credits: https://github.com/lstux/OdroidGO/

BUILDDIR="/tmp/odroid-go-mkfw"


if ! [ -d "${BUILDDIR}" ]; then
  install -d "${BUILDDIR}" || exit 1
fi

cd "${BUILDDIR}/" || exit 2
if ! [ -d "${BUILDDIR}/odroid-go-firmware" ]; then
 cd "${BUILDDIR}/" && git clone https://github.com/othercrashoverride/odroid-go-firmware.git -b factory || exit 3
fi

make -C "${BUILDDIR}/odroid-go-firmware/tools/mkfw" || exit 4

if [ -n "$(which sudo)" ]; then
  sudo install -v -oroot -groot -m755 "${BUILDDIR}/odroid-go-firmware/tools/mkfw/mkfw" "/usr/bin/mkfw" || exit 5
else
  su -c "install -v -oroot -groot -m755 '${BUILDDIR}/odroid-go-firmware/tools/mkfw/mkfw' '/usr/bin/mkfw'" || exit 5
fi

rm -rf "${BUILDDIR}"
 
