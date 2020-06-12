#!/bin/bash
#
# Xcode project to build models: copy model.sqlite and *.ini into target dir 
#
set -x
set -e

MODEL_SQLITE=${MODEL_NAME}.sqlite

if [ "$MODEL_SQLITE" = '.sqlite' ]; then
  echo "Invalid (empty) model name. Use .xconfig to set model name."
  exit 1
fi

if [ ! -f ${OMC_OUT_DIR}/${MODEL_SQLITE} ]; then
  echo Model.sqlite not found: ${OMC_OUT_DIR}/${MODEL_SQLITE}
  exit 1
fi

# move newly created model.sqlite
mv -f ${OMC_OUT_DIR}/${MODEL_SQLITE} ${PUBLISH_DIR}/

# if copy model.ini and model messages:
if [ -e ${MODEL_NAME}.ini ] ; then cp -pf ${MODEL_NAME}.ini ${PUBLISH_DIR}/ ; fi
if [ -e ${OMC_OUT_DIR}/${MODEL_NAME}.message.ini ] ; then cp -pf ${OMC_OUT_DIR}/${MODEL_NAME}.message.ini ${PUBLISH_DIR}/ ; fi

# done
