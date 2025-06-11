#!/usr/bin/env bash
#
# MacOS: stop openM++ UI before model build
#   cd models/ModelDir
#   ../stop-ompp-ui-mac.sh
#   make
#
# It does:
#
#   cd $OM_ROOT
#   if [ ! f ${PUBLISH_DIR}/oms.pid.txt ] ; then exit 0
#   oms_pid=`cat ${PUBLISH_DIR}/oms.pid.txt`
#   kill $oms_pid
#
# Environment:
# MODEL_NAME  - model name, default: current directory name
# OM_ROOT     - openM++ root folder, default: ../..
# PUBLISH_DIR - "publish" directory where model.exe and model.sqlite resides
#               if PUBLISH_DIR is relative then it MUST BE relative to $OM_ROOT
#               default: ${PWD}/ompp-mac/bin

set -e

# set model name, openM++ root folder and "publish" directory
#     if PUBLISH_DIR is relative then it MUST BE relative to $OM_ROOT
#
[ -z "$MODEL_NAME" ]  && MODEL_NAME=$(basename "$PWD")
[ -z "$OM_ROOT" ]     && OM_ROOT="../.."
[ -z "$PUBLISH_DIR" ] && PUBLISH_DIR="${PWD}/ompp-mac/bin"

pushd "$OM_ROOT"
export OM_ROOT="$PWD"

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

# oms pid file and oms stop log file
#
OMS_PID_TXT="${PUBLISH_DIR}/oms.pid.txt"
STOP_OMPP_UI_LOG="${PUBLISH_DIR}/${MODEL_NAME}.stop_ompp_ui.log"

echo "OMS_PID_TXT = $OMS_PID_TXT"

# log configuration
#
echo "MODEL_NAME     = $MODEL_NAME"  >"$STOP_OMPP_UI_LOG"
echo "OM_ROOT        = $OM_ROOT"     >>"$STOP_OMPP_UI_LOG"
echo "PUBLISH_DIR    = $PUBLISH_DIR" >>"$STOP_OMPP_UI_LOG"
echo "OMS_PID_TXT    = $OMS_PID_TXT" >>"$STOP_OMPP_UI_LOG"

# Exit and return success 
# if oms pid file not exist in "publish" directory
#
if [ ! -f "${OMS_PID_TXT}" ] ;
then
  echo "WARNING: oms PID file not found: ${OMS_PID_TXT}" | tee -a "$STOP_OMPP_UI_LOG"
  echo "Done." | tee -a "$STOP_OMPP_UI_LOG"
  exit 0
fi

# read oms pid from file
#
echo cat "${OMS_PID_TXT}" | tee -a "$STOP_OMPP_UI_LOG"

oms_pid=`cat "${OMS_PID_TXT}" 2>/dev/null`
if [ $? -ne 0 ] ;
then
  echo "FAILED to read oms pid from file: ${oms_pid}" | tee -a "$STOP_OMPP_UI_LOG"
  exit 1
fi

echo "oms PID        = $oms_pid" >>"$STOP_OMPP_UI_LOG"

# kill oms web-service
#
echo "Kill $oms_pid"    | tee -a "$STOP_OMPP_UI_LOG"

status=$(kill "${oms_pid}" >> "$STOP_OMPP_UI_LOG" 2>&1; echo $?)

if [ $status -ne 0 ] ;
then
  echo "WARNING: unable to stop oms web-service, status=${status}" | tee -a "$STOP_OMPP_UI_LOG"
fi

# remove oms pid file
#
echo "unlink ${OMS_PID_TXT}" | tee -a "$STOP_OMPP_UI_LOG"

status=$(unlink "${OMS_PID_TXT}" >> "$STOP_OMPP_UI_LOG" 2>&1; echo $?)

if [ $status -ne 0 ] ;
then
  echo "WARNING: unable to delete oms PID file: ${OMS_PID_TXT}, status=${status}" | tee -a "$STOP_OMPP_UI_LOG"
fi

echo "Done." | tee -a "$STOP_OMPP_UI_LOG"

popd
exit 0
