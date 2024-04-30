@echo off

REM Windows: start openM++ UI from model/ompp/bin directory
REM
REM Environment:
REM
REM MODEL_NAME:    model name, default: current directory name
REM
REM OM_ROOT:       openM++ root folder, default: ..\..
REM
REM PUBLISH_DIR:   "publish" directory where model.exe and model.sqlite resides
REM                if PUBLISH_DIR is relative then it MUST BE relative to OM_ROOT
REM                default: ompp\bin
REM
REM OM_CFG_DEFAULT_RUN_TMPL:  if defined then UI configured to use this model run template, by default
REM                           if Release model.exe not exist and Debug modelD.exe exist then: run.Debug.template.txt
REM

setlocal enabledelayedexpansion

REM check MODEL_NAME and OM_ROOT environment

IF "%MODEL_NAME%" == "" (
  @echo ERROR: empty MODEL_NAME environment variable
  EXIT 1
)
@echo MODEL_NAME     = %MODEL_NAME%

IF "%OM_ROOT%" == "" (
  @echo ERROR: empty OM_ROOT environment variable
  EXIT 1
)
@echo OM_ROOT        = %OM_ROOT%

IF "%PUBLISH_DIR%" == "" (
  @echo ERROR: empty PUBLISH_DIR environment variable
  EXIT 1
)
@echo PUBLISH_DIR    = %PUBLISH_DIR%

if not exist "%OM_ROOT%\html" (
  @echo ERROR: missing UI html directory: %OM_ROOT%\html
  EXIT 1
)
if not exist "%OM_ROOT%\bin\oms.exe" (
  @echo ERROR: missing openM++ web-service: %OM_ROOT%\bin\oms.exe
  EXIT 1
)
if not exist "%OM_ROOT%\bin\dbcopy.exe" (
  @echo ERROR: missing openM++ dbcopy utility: %OM_ROOT%\bin\dbcopy.exe
  EXIT 1
)

REM check if model.exe exist and model.sqlite exist
REM if only Debug modelD.exe exist then set model run template to run.Debug.template.txt

if exist "%MODEL_NAME%.exe" (
  set model_exe_name=%MODEL_NAME%.exe
)
if not defined model_exe_name (
  if exist "%MODEL_NAME%D.exe" (

    set model_exe_name=%MODEL_NAME%D.exe

    if not defined OM_CFG_DEFAULT_RUN_TMPL (
      set OM_CFG_DEFAULT_RUN_TMPL=run.Debug.template.txt
    )
  )
)
if not defined model_exe_name (
  if exist "%MODEL_NAME%_mpi.exe" (

    set model_exe_name=%MODEL_NAME%_mpi.exe

    if not defined OM_CFG_DEFAULT_RUN_TMPL (
      set OM_CFG_DEFAULT_RUN_TMPL=mpi.ModelRun.template.txt
    )
  )
)
if not defined model_exe_name (
  if exist "%MODEL_NAME%D_mpi.exe" (

    set model_exe_name=%MODEL_NAME%D_mpi.exe

    if not defined OM_CFG_DEFAULT_RUN_TMPL (
      set OM_CFG_DEFAULT_RUN_TMPL=mpi.ModelDebug.template.txt
    )
  )
)
if not defined model_exe_name (
  @echo ERROR: %MODEL_NAME% exe not found
  EXIT 1
)

if not exist "%MODEL_NAME%.sqlite" (
  @echo ERROR: model SQLite database not found: %MODEL_NAME%.sqlite
  EXIT 1
)

REM create directories for download and upload from UI, errors are not critical and can be ignored

mkdir "io\download"
mkdir "io\upload"

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

REM set OM_ModelName=/path/to/model/dir

set model_root_name=OM_%MODEL_NAME%
set "%model_root_name%=%PUBLISH_DIR%"

@echo %model_root_name% = %PUBLISH_DIR%

@echo "%OM_ROOT%\bin\oms.exe" -l localhost:0 -oms.RootDir "%OM_ROOT%" -oms.ModelDir "%PUBLISH_DIR%" -oms.ModelLogDir "%PUBLISH_DIR%" -oms.UrlSaveTo "%OMS_URL_TICKLE%" -oms.HtmlDir "%OM_ROOT%\html" -oms.EtcDir "%OM_ROOT%\etc" -oms.HomeDir "%PUBLISH_DIR%" -oms.ModelDocDir "%PUBLISH_DIR%\doc" -oms.AllowDownload -oms.AllowUpload -oms.AllowMicrodata -oms.LogRequest -OpenM.LogToFile

START "oms" "%OM_ROOT%\bin\oms.exe" -l localhost:0 -oms.RootDir "%OM_ROOT%" -oms.ModelDir "%PUBLISH_DIR%" -oms.ModelLogDir "%PUBLISH_DIR%" -oms.UrlSaveTo "%OMS_URL_TICKLE%" -oms.HtmlDir "%OM_ROOT%\html" -oms.EtcDir "%OM_ROOT%\etc" -oms.HomeDir "%PUBLISH_DIR%" -oms.ModelDocDir "%PUBLISH_DIR%\doc" -oms.AllowDownload -oms.AllowUpload -oms.AllowMicrodata -oms.LogRequest -OpenM.LogToFile
if ERRORLEVEL 1 (
  @echo FAILED to start %OM_ROOT%\bin\oms.exe
  EXIT 1
)

START /wait timeout /T 2 /nobreak >nul

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
