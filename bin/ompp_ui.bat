@echo off

IF "%OM_ROOT%" == "" (
  set "OM_ROOT=%~dp0.."
)

echo "OM_ROOT:" %OM_ROOT%

if not exist "%OM_ROOT%\models\bin" (
  @echo ERROR: missing models bin directory: %OM_ROOT%\models\bin
  pause
  EXIT 1
)

if not exist "%OM_ROOT%\html" (
  @echo ERROR: missing UI html directory: %OM_ROOT%\html
  pause
  EXIT 1
)

set OM_CFG_INI_ALLOW=true
set OM_CFG_INI_ANY_KEY=true
IF "%OM_CFG_TYPE_MAX_LEN%" == "" (
  set OM_CFG_TYPE_MAX_LEN=256
)

START "oms" "%OM_ROOT%\bin\oms" -oms.RootDir "%OM_ROOT%" -oms.HomeDir models\home -oms.AllowDownload -oms.AllowUpload -oms.AllowMicrodata -oms.LogRequest

START http://localhost:4040
