#!/usr/bin/env bash
#
# MacOS: stop openM++ UI before model build
#   cd models/ModelDir
#   ../stop_ompp_ui_mac.sh
#   make
#
# It does:
#   cd $OM_ROOT
#   # check if oms URL file exist
#   if [ ! f ${PUBLISH_DIR}/${MODEL_NAME}.oms_url.tickle ] ; then exit 0
#   # read oms URL from file
#   oms_url=`cat ${PUBLISH_DIR}/${MODEL_NAME}.oms_url.tickle`
#   # shutdown oms:
#   curl $oms_url/api/admin/shutdown
#
# Environment:
# MODEL_NAME  - model name, default: current directory name
# OM_ROOT     - openM++ root folder, default: ../..
# PUBLISH_DIR - "publish" directory where model.exe and model.sqlite resides
#               if PUBLISH_DIR is relative then it MUST BE relative to $OM_ROOT
#               default: models/${MODEL_NAME}/ompp-mac/bin

set -e

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

echo "MODEL_NAME     = $MODEL_NAME"
echo "OM_ROOT        = $OM_ROOT"
echo "PUBLISH_DIR    = $PUBLISH_DIR"

# if "publish" directory not exist then nothing to stop, exit and return success
#
if [ ! -d "$PUBLISH_DIR" ] ;
then
  echo "Publish directory not exist: $PUBLISH_DIR"
  exit 0
fi

# oms url file and oms stop log file
#
OMS_URL_TICKLE="${PUBLISH_DIR}/${MODEL_NAME}.oms_url.tickle"
STOP_OMPP_UI_LOG="${PUBLISH_DIR}/${MODEL_NAME}.stop_ompp_ui.log"

echo "OMS_URL_TICKLE = $OMS_URL_TICKLE"

# log configuration
#
echo "MODEL_NAME     = $MODEL_NAME"     >"$STOP_OMPP_UI_LOG"
echo "OM_ROOT        = $OM_ROOT"        >>"$STOP_OMPP_UI_LOG"
echo "PUBLISH_DIR    = $PUBLISH_DIR"    >>"$STOP_OMPP_UI_LOG"
echo "OMS_URL_TICKLE = $OMS_URL_TICKLE" >>"$STOP_OMPP_UI_LOG"

# Exit and return success 
# if oms url file not exist in "publish" directory
#
if [ ! -f "${OMS_URL_TICKLE}" ] ;
then
  echo "WARNING: oms URL file not found: ${OMS_URL_TICKLE}" | tee -a "$STOP_OMPP_UI_LOG"
  echo "Done." | tee -a "$STOP_OMPP_UI_LOG"
  exit 0
fi

# read oms url from file
#
echo "cat ${OMS_URL_TICKLE}" | tee -a "$STOP_OMPP_UI_LOG"

oms_url=`cat ${OMS_URL_TICKLE} 2>/dev/null`
if [ $? -ne 0 ] ;
then
  echo "FAILED to read oms url from file: ${OMS_URL_TICKLE}" | tee -a "$STOP_OMPP_UI_LOG"
  exit 1
fi

echo "oms URL        = $oms_url" >>"$STOP_OMPP_UI_LOG"

# check if curl installed
# Ubuntu: apt-get install curl
#
if ! type curl >> "$STOP_OMPP_UI_LOG" 2>&1;
then
  echo "ERROR: curl not found. Please install curl, for example: sudo apt-get install curl" | tee -a "$STOP_OMPP_UI_LOG"
  exit 2
fi

# shutdown oms web-service
# expected retrun from curl: 52 = normal oms shutdown
# if any other non-zero exit code, for exmaple: 7 = Connection refused
# then assume web-service already inactive (eg: system restart)
#
oms_shutdown_url="${oms_url}/api/admin/shutdown"

echo "Stop oms web-service:"    | tee -a "$STOP_OMPP_UI_LOG"
echo "curl -X PUT ${oms_shutdown_url}" | tee -a "$STOP_OMPP_UI_LOG"

status=$(curl -X PUT "${oms_shutdown_url}" >> "$STOP_OMPP_UI_LOG" 2>&1; echo $?)

if [ $status -ne 52 ] && [ $status -ne 0 ] ;
then
  echo "WARNING: unable to stop oms web-service, status=${status}" | tee -a "$STOP_OMPP_UI_LOG"
fi

# remove oms url file
#
echo "unlink ${OMS_URL_TICKLE}" | tee -a "$STOP_OMPP_UI_LOG"

status=$(unlink "${OMS_URL_TICKLE}" >> "$STOP_OMPP_UI_LOG" 2>&1; echo $?)

if [ $status -ne 0 ] ;
then
  echo "WARNING: unable to delete oms url file: ${OMS_URL_TICKLE}, status=${status}" | tee -a "$STOP_OMPP_UI_LOG"
fi

echo "Done." | tee -a "$STOP_OMPP_UI_LOG"

popd
exit 0
