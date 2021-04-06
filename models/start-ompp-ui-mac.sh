#!/usr/bin/env bash
#
# MacOS: start openM++ UI after model build
#   cd models/ModelDir
#   make
#   ../start_ompp_ui_mac.sh
#
# It does:
#   cd $OM_ROOT
#   # start oms, listen on free port:
#   bin/oms -oms.Listen localhost:0 .... &
#   # use lsof to find oms port:
#   lsof -i -a -sTCP:LISTEN -a -p $OMS_PID ....
#   # write oms URL into file:
#   "http://localhost:12345" > ${PUBLISH_DIR}/${MODEL_NAME}.oms_url.tickle
#   # open UI in browser:
#   open http://localhost:12345
#
# Environment:
# MODEL_NAME  - model name, default: current directory name
# OM_ROOT     - openM++ root folder, default: ../..
# PUBLISH_DIR - "publish" directory where model.exe and model.sqlite resides
#               if PUBLISH_DIR is relative then it MUST BE relative to $OM_ROOT
#               default: models/${MODEL_NAME}/ompp-mac/bin
# RELEASE && CONFIGURATION - if both not defined 
#                            then UI configured to use debug model run template: run.Debug.template.txt
# Note:
#   it does work for shell: make RELEASE=1
#   because makefile does create modelD executable if RELEASE not defined
#   Xcode defines CONFIGURATION as "Debug" or "Release" and does not using RELEASE

set -e
set -m

# set model name and openM++ root folder 
#
[ -z "$MODEL_NAME" ] && MODEL_NAME="$(basename $PWD)"
[ -z "$OM_ROOT" ]    && OM_ROOT="../.."

pushd "$OM_ROOT"
export OM_ROOT="$PWD"

# set "publish" directory
# PUBLISH_DIR, if specified, MUST BE relative to $OM_ROOT
#
[ -z "$PUBLISH_DIR" ] && PUBLISH_DIR="models/${MODEL_NAME}/ompp-mac/bin"

echo "MODEL_NAME  = $MODEL_NAME"
echo "OM_ROOT     = $OM_ROOT"
echo "PUBLISH_DIR = $PUBLISH_DIR"

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

# check if model.sqlite exist in "publish" directory
#
if [ ! -f "${PUBLISH_DIR}/${MODEL_NAME}.sqlite" ] ;
then
  echo "ERROR: model SQLite database not found: ${PUBLISH_DIR}/${MODEL_NAME}.sqlite" | tee -a "$START_OMPP_UI_LOG"
  exit 1
fi

# default model run template
# if RELEASE and CONFIGURATION not defined then use Debug template else no default template
#
[ -z "$RELEASE" ] && [ -z "$CONFIGURATION" ] && [ -z "$OM_CFG_DEFAULT_RUN_TMPL" ] && OM_CFG_DEFAULT_RUN_TMPL="run.Debug.template.txt"

if [ -n "$OM_CFG_DEFAULT_RUN_TMPL" ] ; then
  export OM_CFG_DEFAULT_RUN_TMPL="$OM_CFG_DEFAULT_RUN_TMPL"
  echo "OM_CFG_DEFAULT_RUN_TMPL = $OM_CFG_DEFAULT_RUN_TMPL" | tee -a "$START_OMPP_UI_LOG"
fi

# start oms web-service
#
echo "bin/oms" -l localhost:0 -oms.ModelDir "$PUBLISH_DIR" -oms.ModelLogDir "$PUBLISH_DIR" -oms.HomeDir models/home -oms.LogRequest | tee -a "$START_OMPP_UI_LOG"

"bin/oms" -l localhost:0 -oms.ModelDir "$PUBLISH_DIR" -oms.ModelLogDir "$PUBLISH_DIR" -oms.HomeDir models/home -oms.LogRequest \
  >> "$START_OMPP_UI_LOG" 2>&1 & \
  status=$? \
  OMS_PID=$!

if [ $status -ne 0 ] || [ -z "$OMS_PID" ] || ! ps $OMS_PID 1> /dev/null 2>&1 ;
then
  echo FAILED to start oms web-service | tee -a "$START_OMPP_UI_LOG"
  exit 1
fi

echo "OMS_PID     = $OMS_PID" | tee -a "$START_OMPP_UI_LOG"

# get oms port: it would fail if process start failed
#
echo "lsof -P -n -i -a -sTCP:LISTEN -a -p $OMS_PID -F n" | tee -a "$START_OMPP_UI_LOG"

sleep 1    # MacOS is slow

oms_lsof=`lsof -P -n -i -a -sTCP:LISTEN -a -p $OMS_PID -F n`
if [ $? -ne 0 ] ;
then
  echo "FAILED to start oms web-service (port unknown)" | tee -a "$START_OMPP_UI_LOG"
  exit 2
fi

OMS_PORT="${oms_lsof##*:}"

if [ -z "$OMS_PORT" ] ;
then
  echo "FAILED to start oms web-service (port unknown)" | tee -a "$START_OMPP_UI_LOG"
  exit 3
fi

echo "OMS_PORT    = $OMS_PORT" | tee -a "$START_OMPP_UI_LOG"

# start browser and open UI
#
OMS_URL="http://localhost:${OMS_PORT}" 
echo -n "${OMS_URL}" >"$OMS_URL_TICKLE"

echo "Open openM++ UI in browser:" | tee -a "$START_OMPP_UI_LOG"
echo "open ${OMS_URL}" | tee -a "$START_OMPP_UI_LOG"

if ! open "${OMS_URL}" >> "$START_OMPP_UI_LOG" 2>&1;
then
  echo "FAILED to open browser at ${OMS_URL}" | tee -a "$START_OMPP_UI_LOG"
  exit 7
fi

echo "Done." | tee -a "$START_OMPP_UI_LOG"

popd
exit 0
