#!/usr/bin/env bash
#
# It does:
#   cd $OM_ROOT
#     # start oms in a new terminal window:
#   gnome-terminal -e "OM_ROOT=$PWD bin/oms -oms.Listen localhost:4040 -oms.LogRequest"
#     # open UI in browser:
#   xdg-open http://localhost:4040
#
# Environment:
#   OM_ROOT       - openM++ root folder, default is a parent dir: ..
#   OMS_PORT      - oms web-service port to listen, default: 4040
#   OM_X_TERMINAL - which x-terminal to use, default: x-terminal-emulator or gnome-terminal

# set -e
set -m

# set openM++ root folder 
#
self="$(basename "$0")"
sd="$(dirname "$0")"

if [ -z "$OM_ROOT" ] ;
then

  echo "Set openM++ root directory: OM_ROOT"

  if [ -d "${sd}/.." ] ;
  then
    echo pushd "${sd}/.."
    pushd "${sd}/.."
  fi
  
  OM_ROOT="$PWD"

fi

echo "OM_ROOT = $OM_ROOT"

# check if OM_ROOT directory exist
#
if [ ! -d "$OM_ROOT" ] ;
then
  echo "ERROR: openM++ directory not exist: $OM_ROOT"
  echo -n "Press Enter to exit..."
  read any
  exit 1
fi

[ "$OM_ROOT" != "$PWD" ] && pushd $OM_ROOT

export OM_ROOT="$PWD"

# check if OM_ROOT is openM++ root
#
if [ ! -x "bin/oms" ] || [ ! -d "html" ] || [ ! -d "models/bin" ] || [ ! -d "log" ] ;
then
  echo "ERROR: openM++ UI not found at: $OM_ROOT"
  echo -n "Press Enter to exit..."
  read any
  exit 1
fi

# log configuration
#
OMPP_UI_SH_LOG="${OM_ROOT}/log/${self}.log"

echo "Log:       $OMPP_UI_SH_LOG"
echo "OM_ROOT  = $OM_ROOT" >"$OMPP_UI_SH_LOG"

# which x-terminal to use: x-terminal-emulator, gnome-terminal, konsole
#
if [ -z "${OM_X_TERMINAL}" ] && command -v x-terminal-emulator >/dev/null 2>&1 ;
then
  OM_X_TERMINAL="x-terminal-emulator"
fi
if [ -z "${OM_X_TERMINAL}" ] && command -v xfce4-terminal >/dev/null 2>&1 ;
then
  OM_X_TERMINAL="xfce4-terminal"
fi
if [ -z "${OM_X_TERMINAL}" ] && command -v gnome-terminal >/dev/null 2>&1 ;
then
  OM_X_TERMINAL="gnome-terminal"
fi
if [ -z "${OM_X_TERMINAL}" ] && command -v konsole >/dev/null 2>&1 ;
then
  OM_X_TERMINAL="konsole"
fi
if [ -z "${OM_X_TERMINAL}" ] && command -v qterminal >/dev/null 2>&1 ;
then
  OM_X_TERMINAL="qterminal"
fi
if [ -z "${OM_X_TERMINAL}" ] ;
then
  echo "ERROR not found any of: x-terminal-emulator, gnome-terminal, konsole and OM_X_TERMINAL not set" | tee -a "$OMPP_UI_SH_LOG"
  echo -n "Press Enter to exit..."
  read any
  exit 1
fi

# set oms port to listen
#
[ -z "$OMS_PORT" ] && OMS_PORT=4040

export OMS_PORT
echo "OMS_PORT = $OMS_PORT" | tee -a "$OMPP_UI_SH_LOG"

lsof -P -n -iTCP:"$OMS_PORT" -a -sTCP:LISTEN >>"$OMPP_UI_SH_LOG" 2>&1
if [ $? -eq 0 ] ;
then
  echo "FAILED to start oms web-service, port in use: OMS_PORT=$OMS_PORT" | tee -a "$OMPP_UI_SH_LOG"
  echo "If port $OMS_PORT used by oms web-service then stop it first" | tee -a "$OMPP_UI_SH_LOG"
  echo -n "Press Enter to exit..."
  read any
  exit 2
fi

# start oms web-service
#
status=0

if [ "${OM_X_TERMINAL}" != "konsole" ] ;
then
  echo "${OM_X_TERMINAL} -e ./bin/start_oms.sh &" | tee -a "$OMPP_UI_SH_LOG"

  ${OM_X_TERMINAL} -e ./bin/start_oms.sh &
  status=$?
else
  echo bash -c "konsole -e ./bin/start_oms.sh &" | tee -a "$OMPP_UI_SH_LOG"

  bash -c "konsole -e ./bin/start_oms.sh &"
  status=$?
fi

if [ $status -ne 0 ] ;
then
  echo "FAILED to start oms web-service using ${OM_X_TERMINAL}" | tee -a "$OMPP_UI_SH_LOG"
  echo -n "Press Enter to exit..."
  read any
  exit 1
fi
# known issue: test above does not catch exit code of start_oms.sh

# start browser and open UI
#
OMS_URL="http://localhost:${OMS_PORT}" 

echo "Open openM++ UI in browser:" | tee -a "$OMPP_UI_SH_LOG"
echo "xdg-open ${OMS_URL}" | tee -a "$OMPP_UI_SH_LOG"

if ! xdg-open "${OMS_URL}" >> "$OMPP_UI_SH_LOG" 2>&1;
then
  echo "FAILED to open browser at ${OMS_URL}" | tee -a "$OMPP_UI_SH_LOG"
  echo -n "Press Enter to exit..."
  read any
  exit 7
fi
# known issue: xdg-open may return 0 when it is not able to start browser

echo "Done." | tee -a "$OMPP_UI_SH_LOG"

echo -n "Press Enter to exit..."
read any
exit 0
