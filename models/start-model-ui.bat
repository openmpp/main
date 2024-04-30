@echo off

REM Windows: start openM++ UI from model build directory
REM in order to use copy it into model directory and double click or:
REM
REM   cd models/ModelDir
REM   ..\start_ompp_ui.bat
REM
REM It does:
REM   START %OM_ROOT%\bin\oms.exe -oms.Listen localhost:0 ....
REM   START http://localhost:12345
REM
REM Environment:
REM
REM MODEL_NAME:     model name, default: current directory name
REM
REM OM_ROOT:        openM++ root folder, default: ..\..
REM
REM MODEL_FULL_DIR: path to model directory, default: current directory
REM                 for example: C:\openMpp\models\RiskPaths
REM
REM PUBLISH_DIR:    "publish" directory where model.exe and model.sqlite resides
REM                 if PUBLISH_DIR is relative then it MUST BE relative to OM_ROOT
REM                 default: ompp\bin
REM
REM OM_CFG_DEFAULT_RUN_TMPL - if defined then UI configured to use this model run template, by default
REM                           if Release model.exe not exist and Debug modelD.exe exist then: run.Debug.template.txt
REM

setlocal enabledelayedexpansion

IF "%MODEL_FULL_DIR%" == "" (
  set "MODEL_FULL_DIR=%CD%"
)
if "%MODEL_FULL_DIR:~-1%"=="\" set "MODEL_FULL_DIR=%MODEL_FULL_DIR:~0,-1%"

for %%D in ("%MODEL_FULL_DIR%") do set model_dir=%%~nxD

IF "%MODEL_NAME%" == "" (
  set MODEL_NAME=%model_dir%
)
@echo MODEL_NAME  = %MODEL_NAME%

set model_root_name=OM_%MODEL_NAME%
set "model_root_value=%MODEL_FULL_DIR%"

set "%model_root_name%=%model_root_value%"

@echo %model_root_name% = %model_root_value%

IF "%PUBLISH_DIR%" == "" (
  set "PUBLISH_DIR=%MODEL_FULL_DIR%\ompp\bin"
)
@echo PUBLISH_DIR = %PUBLISH_DIR%

if not exist "%PUBLISH_DIR%" (
  @echo ERROR: missing model bin directory: %PUBLISH_DIR%
  pause
  EXIT 1
)

REM check if model.exe exist and model.sqlite exist
REM if only Debug modelD.exe exist then set model run template to run.Debug.template.txt
REM if Debug modelD.exe does not exists then check for MPI model_mpi.exe and for modelD_mpi.exe

if exist "%PUBLISH_DIR%\%MODEL_NAME%.exe" (
  set model_exe_name=%MODEL_NAME%.exe
)
if not defined model_exe_name (
  if exist "%PUBLISH_DIR%\%MODEL_NAME%D.exe" (

    set model_exe_name=%MODEL_NAME%D.exe

    if not defined OM_CFG_DEFAULT_RUN_TMPL (
      set OM_CFG_DEFAULT_RUN_TMPL=run.Debug.template.txt
    )
  )
)
if not defined model_exe_name (
  if exist "%PUBLISH_DIR%\%MODEL_NAME%_mpi.exe" (

    set model_exe_name=%MODEL_NAME%_mpi.exe

    if not defined OM_CFG_DEFAULT_RUN_TMPL (
      set OM_CFG_DEFAULT_RUN_TMPL=mpi.ModelRun.template.txt
    )
  )
)
if not defined model_exe_name (
  if exist "%PUBLISH_DIR%\%MODEL_NAME%D_mpi.exe" (

    set model_exe_name=%MODEL_NAME%D_mpi.exe

    if not defined OM_CFG_DEFAULT_RUN_TMPL (
      set OM_CFG_DEFAULT_RUN_TMPL=mpi.ModelDebug.template.txt
    )
  )
)
if not defined model_exe_name (
  @echo ERROR: Model.exe not found in %PUBLISH_DIR%
  pause
  EXIT 1
)

if not exist "%PUBLISH_DIR%\%MODEL_NAME%.sqlite" (
  @echo ERROR: model SQLite database not found: %PUBLISH_DIR%\%MODEL_NAME%.sqlite
  pause
  EXIT 1
)

REM set OM_ROOT if not defined and check if it has UI html, oms and dbcopy

IF "%OM_ROOT%" == "" (
  set OM_ROOT=%MODEL_FULL_DIR%\..\..
)
@echo OM_ROOT     = %OM_ROOT%

if not exist "%OM_ROOT%\html" (
  @echo ERROR: missing UI html directory: %OM_ROOT%\html
  pause
  EXIT 1
)

if not exist "%OM_ROOT%\bin\oms.exe" (
  @echo ERROR: missing openM++ web-service: %OM_ROOT%\bin\oms.exe
  pause
  EXIT 1
)

if not exist "%OM_ROOT%\bin\dbcopy.exe" (
  @echo ERROR: missing openM++ dbcopy utility: %OM_ROOT%\bin\dbcopy.exe
  pause
  EXIT 1
)

REM create directories for download and upload from UI, errors are not critical and can be ignored

mkdir "%PUBLISH_DIR%\io\download"
mkdir "%PUBLISH_DIR%\io\upload"

REM delete oms URL file if exist

set "OMS_URL_TICKLE=%PUBLISH_DIR%\%MODEL_NAME%.oms_url.tickle"

REM @echo OMS_URL_TICKLE = %OMS_URL_TICKLE%

if exist "%OMS_URL_TICKLE%" (

  del /q %OMS_URL_TICKLE%
  if ERRORLEVEL 1 (
    @echo FAIL to delete: %OMS_URL_TICKLE%
    EXIT 1
  )
)
if exist "%OMS_URL_TICKLE%" (
  @echo FAIL to delete: %OMS_URL_TICKLE%
  EXIT 1
)

REM start oms web-service and UI

set OM_CFG_INI_ALLOW=true
set OM_CFG_INI_ANY_KEY=true
IF "%OM_CFG_TYPE_MAX_LEN%" == "" (
  set OM_CFG_TYPE_MAX_LEN=256
)

START "oms" "%OM_ROOT%\bin\oms.exe" -l localhost:0 -oms.RootDir "%OM_ROOT%" -oms.ModelDir "%PUBLISH_DIR%" -oms.ModelLogDir "%PUBLISH_DIR%" -oms.ModelDocDir "%PUBLISH_DIR%\doc" -oms.UrlSaveTo "%OMS_URL_TICKLE%" -oms.HomeDir "%PUBLISH_DIR%" -oms.AllowDownload -oms.AllowUpload -oms.AllowMicrodata -oms.LogRequest
if ERRORLEVEL 1 (
  @echo FAILED to start %OM_ROOT%\bin\oms.exe
  EXIT 1
)

timeout /T 2 /nobreak >nul

REM read oms url from file and open browser

if not exist "%OMS_URL_TICKLE%" (
  @echo ERROR: oms start failed, URL file not found: %OMS_URL_TICKLE%
  EXIT 1
)

for /F "usebackq tokens=* delims=" %%i in (%OMS_URL_TICKLE%) do (
  set OMS_URL=%%i
)
@echo Starting openM++ UI at: %OMS_URL%

START %OMS_URL%
