#!/usr/bin/env bash
#
# MacOS: start oms web-service and open browser at UI http://localhost:4040

cd "$(dirname "$0")/.."

osascript -e 'on run argv 
  tell app "Terminal" 
    do script "cd " & (item 1 of argv) & " ; OM_CFG_INI_ALLOW=true OM_CFG_INI_ANY_KEY=true OM_CFG_TYPE_MAX_LEN=366 OM_ROOT=$PWD bin/oms -l localhost:4040 -oms.HomeDir models/home -oms.AllowDownload -oms.AllowUpload -oms.AllowMicrodata -oms.PidSaveTo log/oms.pid.txt -oms.LogRequest -OpenM.LogFilePath log/oms.log"
  end tell 
end run' "$PWD"

sleep 1

open http://localhost:4040
