@echo off

REM Windows: start openM++ UI from model build directory
REM in order to use copy it into model directory and double click or:
REM
REM   cd models/ModelDir
REM   start_ompp_ui.bat
REM
REM It does:
REM   cd %OM_ROOT%
REM   START %OM_ROOT%\bin\oms.exe -oms.Listen localhost:0 ....
REM   START http://localhost:12345
REM
REM Environment:
REM
REM MODEL_NAME:   model name, default: current directory name
REM
REM OM_ROOT:      openM++ root folder, default: ..\..
REM
REM PUBLISH_DIR:  "publish" directory where model.exe and model.sqlite resides
REM               if PUBLISH_DIR is relative then it MUST BE relative to OM_ROOT
REM               default: ompp\bin
REM
REM OM_CFG_DEFAULT_RUN_TMPL - if defined then UI configured to use this model run template, by default
REM                           if Release model.exe not exist and Debug modelD.exe exist then: run.Debug.template.txt
REM

setlocal enabledelayedexpansion

for %%D in (.) do set model_dir=%%~nxD

IF "%MODEL_NAME%" == "" (
  set MODEL_NAME=%model_dir%
)
@echo MODEL_NAME  = %MODEL_NAME%

IF "%PUBLISH_DIR%" == "" (
  set PUBLISH_DIR=%~dp0%ompp\bin
)
@echo PUBLISH_DIR = %PUBLISH_DIR%

if not exist %PUBLISH_DIR% (
  @echo ERROR: missing model bin directory: %PUBLISH_DIR%
  pause
  EXIT 1
)

set model_root_name=OM_%model_dir%

set model_root_value=%~dp0
if %model_root_value:~-1%==\ set model_root_value=%model_root_value:~0,-1%

set %model_root_name%=%model_root_value%

@echo %model_root_name% = %model_root_value%

REM check if model.exe exist and model.sqlite exist
REM if only Debug modelD.exe exist then set model run template to run.Debug.template.txt

if exist %PUBLISH_DIR%\%MODEL_NAME%.exe (
  set model_exe_name=%MODEL_NAME%.exe
)
if not defined model_exe_name (
  if exist %PUBLISH_DIR%\%MODEL_NAME%D.exe (

    set model_exe_name=%MODEL_NAME%D.exe

    if not defined OM_CFG_DEFAULT_RUN_TMPL (
      set OM_CFG_DEFAULT_RUN_TMPL=run.Debug.template.txt
    )
  )
)
if not defined model_exe_name (
  @echo ERROR: Model.exe not found in %PUBLISH_DIR%
  pause
  EXIT 1
)

if not exist %PUBLISH_DIR%\%MODEL_NAME%.sqlite (
  @echo ERROR: model SQLite database not found: %PUBLISH_DIR%\%MODEL_NAME%.sqlite
  pause
  EXIT 1
)

REM set OM_ROOT if not defined and check if it has UI html, oms and dbcopy

IF "%OM_ROOT%" == "" (
  set OM_ROOT=%~dp0\..\..
)

cd /d %OM_ROOT%
set OM_ROOT=%CD%

@echo OM_ROOT     = %OM_ROOT%

if not exist %OM_ROOT%\html (
  @echo ERROR: missing UI html directory: %OM_ROOT%\html
  pause
  EXIT 1
)

if not exist %OM_ROOT%\bin\oms.exe (
  @echo ERROR: missing openM++ web-service: %OM_ROOT%\bin\oms.exe
  pause
  EXIT 1
)

if not exist %OM_ROOT%\bin\dbcopy.exe (
  @echo ERROR: missing openM++ dbcopy utility: %OM_ROOT%\bin\dbcopy.exe
  pause
  EXIT 1
)

if not exist %OM_ROOT%\props\startOmppUI.ps1 (
  @echo ERROR: missing openM++ UI start script: %OM_ROOT%\props\startOmppUI.ps1
  pause
  EXIT 1
)

REM start oms web-service and UI

set OMS_URL_TICKLE=%PUBLISH_DIR%\%MODEL_NAME%.oms_url.tickle
set START_OMPP_UI_LOG=%PUBLISH_DIR%\%MODEL_NAME%.start_ompp_ui.log

PowerShell.exe -ExecutionPolicy Bypass -File %OM_ROOT%\props\startOmppUI.ps1

