#!/usr/bin/env bash
#
# It does:
#   OM_ROOT=${OM_ROOT} bin/oms -oms.Listen http://localhost:${OMS_PORT} -oms.LogRequest
#
# Environment:
#   OM_ROOT  - openM++ root folder, default: current directory
#   OMS_PORT - oms web-service port to listen, default: 4040

# set -e
set -m

# set openM++ root folder 
#
self=$(basename $0)

if [ -z "$OM_ROOT" ] ;
then

  if [ -x "${self}" ] ;
  then
    echo "pushd .."
    pushd ..
  fi
  
  OM_ROOT="$PWD"

fi

[ "$OM_ROOT" != "$PWD" ] && pushd $OM_ROOT

# start oms web-service
#
[ -z "$OMS_PORT" ] && OMS_PORT=4040

echo "OM_ROOT=$OM_ROOT ./bin/oms -l localhost:${OMS_PORT} -oms.HomeDir models/home -oms.LogRequest"

OM_ROOT=$OM_ROOT ./bin/oms -l localhost:${OMS_PORT} -oms.HomeDir models/home -oms.LogRequest
status=$?

if [ $status -ne 0 ] ;
then
  [ $status -eq 130 ] && echo " oms web-service terminated by Ctrl+C"
  [ $status -ne 130 ] && echo " FAILED to start oms web-service"
fi

echo "."
echo -n "Press Enter to exit..."
read any
exit $status
