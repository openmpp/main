#!/usr/bin/env bash
#
# Linux: start openM++ UI after model build
#   cd models/ModelDir
#   make
#   ../start_ompp_ui_linux.sh
#
# It does:
#   start oms, listen on free port
#   read actual oms URL "http://localhost:12345" from file
#   open UI in browser:
#
#   cd $OM_ROOT
#   ulimit -S -s 65536
#   bin/oms -oms.Listen localhost:0 .... &
#   cat ${PUBLISH_DIR}/${MODEL_NAME}.oms_url.tickle
#   xdg-open http://localhost:12345
#
# Environment:
# MODEL_NAME  - model name, default: current directory name
# OM_ROOT     - openM++ root folder, default: ../..
# PUBLISH_DIR - "publish" directory where model.exe and model.sqlite resides
#               if PUBLISH_DIR is relative then it MUST BE relative to $OM_ROOT
#               default: ${PWD}/ompp-linux/bin
# RELEASE     - if not defined then UI configured to use debug model run template: run.Debug.template.txt

set -e
set -m

# set model name, openM++ root folder and "publish" directory
#     if PUBLISH_DIR is relative then it MUST BE relative to $OM_ROOT
#
[ -z "$MODEL_NAME" ]  && MODEL_NAME="$(basename $PWD)"
[ -z "$OM_ROOT" ]     && OM_ROOT="../.."
[ -z "$PUBLISH_DIR" ] && PUBLISH_DIR="${PWD}/ompp-linux/bin"

# set model root, for example: OM_NewCaseBased=/home/user/NewCaseBased
#
model_root_name=OM_${MODEL_NAME}
model_root_value=$PWD

pushd "$OM_ROOT"
export OM_ROOT="$PWD"
export $model_root_name=$model_root_value

echo "MODEL_NAME  = $MODEL_NAME"
echo "OM_ROOT     = $OM_ROOT"
echo "PUBLISH_DIR = $PUBLISH_DIR"
echo "$model_root_name = $model_root_value"

# check if "publish" directory exist
#
if [ ! -d "$PUBLISH_DIR" ] ;
then
  echo "ERROR: publish directory not exist: $PUBLISH_DIR"
  exit 1
fi

# oms url file and oms start log file
#
OMS_URL_TICKLE="${PUBLISH_DIR}/${MODEL_NAME}.oms_url.tickle"
START_OMPP_UI_LOG="${PUBLISH_DIR}/${MODEL_NAME}.start_ompp_ui.log"

# log configuration
#
echo "MODEL_NAME  = $MODEL_NAME"  >"$START_OMPP_UI_LOG"
echo "OM_ROOT     = $OM_ROOT"     >>"$START_OMPP_UI_LOG"
echo "PUBLISH_DIR = $PUBLISH_DIR" >>"$START_OMPP_UI_LOG"

# large models may require stack limit increase
#
ulimit -S -s 65536
status=$?

if [ $status -ne 0 ] ;
then
  echo "FAILED to set: ulimit -S -s 65536" | tee -a "$START_OMPP_UI_LOG"
  exit $status
fi

# check if model.sqlite exist in "publish" directory
#
if [ ! -f "${PUBLISH_DIR}/${MODEL_NAME}.sqlite" ] ;
then
  echo "ERROR: model SQLite database not found: ${PUBLISH_DIR}/${MODEL_NAME}.sqlite" | tee -a "$START_OMPP_UI_LOG"
  exit 1
fi

# create directory for download and upload from UI
#
if [ ! -d "$PUBLISH_DIR/io/download" ] ;
then
  echo "mkdir -p $PUBLISH_DIR/io/download" | tee -a "$START_OMPP_UI_LOG"
  mkdir -p "$PUBLISH_DIR/io/download"
  if [ $? -ne 0 ] ;
  then
    echo "Warning: error at mkdir -p $PUBLISH_DIR/io/download" | tee -a "$START_OMPP_UI_LOG"
    # do not exit: it is not critical error
  fi
fi

if [ ! -d "$PUBLISH_DIR/io/upload" ] ;
then
  echo "mkdir -p $PUBLISH_DIR/io/upload" | tee -a "$START_OMPP_UI_LOG"
  mkdir -p "$PUBLISH_DIR/io/upload"
  if [ $? -ne 0 ] ;
  then
    echo "Warning: error at mkdir -p $PUBLISH_DIR/io/upload" | tee -a "$START_OMPP_UI_LOG"
    # do not exit: it is not critical error
  fi
fi

# default model run template
# if Debug build then use Debug template else no default template
#
[ -z "$RELEASE" ] && [ -z "$OM_CFG_DEFAULT_RUN_TMPL" ] && OM_CFG_DEFAULT_RUN_TMPL="run.Debug.template.txt"

if [ -n "$OM_CFG_DEFAULT_RUN_TMPL" ] ; then
  export OM_CFG_DEFAULT_RUN_TMPL="$OM_CFG_DEFAULT_RUN_TMPL"
  echo "OM_CFG_DEFAULT_RUN_TMPL = $OM_CFG_DEFAULT_RUN_TMPL" | tee -a "$START_OMPP_UI_LOG"
fi

# allow to use $MODEL_NAME.ini file in UI for model run
#
export OM_CFG_INI_ALLOW="true"
export OM_CFG_INI_ANY_KEY="true"
export OM_CFG_TYPE_MAX_LEN=256

# start oms web-service
#
echo "bin/oms" -l localhost:0 -oms.ModelDir "$PUBLISH_DIR" -oms.ModelLogDir "$PUBLISH_DIR" -oms.UrlSaveTo "$OMS_URL_TICKLE" -oms.HomeDir "$PUBLISH_DIR" -oms.ModelDocDir "$PUBLISH_DIR"/doc -oms.AllowDownload -oms.AllowUpload -oms.AllowMicrodata -oms.LogRequest | tee -a "$START_OMPP_UI_LOG"

"bin/oms" -l localhost:0 -oms.ModelDir "$PUBLISH_DIR" -oms.ModelLogDir "$PUBLISH_DIR" -oms.UrlSaveTo "$OMS_URL_TICKLE" -oms.HomeDir "$PUBLISH_DIR" -oms.ModelDocDir "$PUBLISH_DIR"/doc -oms.AllowDownload -oms.AllowUpload -oms.AllowMicrodata -oms.LogRequest \
  >> "$START_OMPP_UI_LOG" 2>&1 & \
  status=$? \
  OMS_PID=$!

if [ $status -ne 0 ] || [ -z "$OMS_PID" ] || ! ps $OMS_PID 1> /dev/null 2>&1 ;
then
  echo FAILED to start oms web-service | tee -a "$START_OMPP_UI_LOG"
  exit 1
fi

echo "OMS_PID     = $OMS_PID" | tee -a "$START_OMPP_UI_LOG"

# read oms url from file
#
sleep 1

echo "cat ${OMS_URL_TICKLE}" | tee -a "$START_OMPP_UI_LOG"

OMS_URL=`cat ${OMS_URL_TICKLE} 2>/dev/null`
if [ $? -ne 0 ] ;
then
  echo "FAILED to read oms url from file: ${OMS_URL_TICKLE}" | tee -a "$START_OMPP_UI_LOG"
  exit 2
fi

echo "oms URL     = ${OMS_URL}" | tee -a "$START_OMPP_UI_LOG"

# start browser and open UI
#
echo "Open openM++ UI in browser:" | tee -a "$START_OMPP_UI_LOG"
echo "xdg-open ${OMS_URL}" | tee -a "$START_OMPP_UI_LOG"

if ! xdg-open "${OMS_URL}" >> "$START_OMPP_UI_LOG" 2>&1;
then
  echo "FAILED to open browser at ${OMS_URL}" | tee -a "$START_OMPP_UI_LOG"
  exit 7
fi
# known issue: xdg-open may return 0 when it is not able to start browser

echo "Done." | tee -a "$START_OMPP_UI_LOG"

popd
exit 0
