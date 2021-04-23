#!/bin/bash
#
# Xcode omc project: copy omc release verion to $OM_ROOT/bin
#
set -x
set -e

# do copy only for Release configuration
if [ ${CONFIGURATION} != "Release" ]; then
  echo Skip omc copy, configuration: ${CONFIGURATION}
  exit 0
fi

#
codesign -s - ${TARGET_BUILD_DIR}/omc

cp -p ${TARGET_BUILD_DIR}/omc ${OM_ROOT}/bin/

[ -e ${SRCROOT}/omc.ini ] && cp -p ${SRCROOT}/omc.ini ${OM_ROOT}/bin/
[ -e ${SRCROOT}/omc.message.ini ] && cp -p ${SRCROOT}/omc.message.ini ${OM_ROOT}/bin/

# done
