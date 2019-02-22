@echo off
REM build openM++ UI

setlocal enabledelayedexpansion

REM push into ompp root and make log directory if not exist

if not exist ompp (
  @echo ERROR: missing source code directory: ompp
  EXIT 1
)

pushd ompp
set   OM_ROOT=%CD%

if not exist log mkdir log

REM log build environment 

@echo %DATE% %TIME% Build openM++ UI alpha
@echo OM_ROOT = %OM_ROOT%

@echo Log file: log\build-ui.log
@echo %DATE% %TIME% Build openM++ UI alpha > log\build-ui.log
@echo OM_ROOT = %OM_ROOT% >> log\build-ui.log

REM build openM++ UI: node, npm, vue.js

pushd ompp-ui

set npm_config_cache=%OM_ROOT%\build\npm-cache
call :do_npm_call "install"
call :do_npm_call "run build"

popd

@echo %DATE% %TIME% Done.
@echo %DATE% %TIME% Done. >> log\build-ui.log

popd
goto :eof

REM end of main body

REM helper subroutine to call npm command, log it and check errorlevel
REM arguments:
REM  1 = npm command line arguments

:do_npm_call

set c_line=%~1

@echo npm %c_line%
@echo npm %c_line% >> ..\log\build-ui.log

call npm %c_line% >> ..\log\build-ui.log 2>&1
if ERRORLEVEL 1 (
  @echo FAILED.
  @echo FAILED. >> ..\log\build-ui.log
  EXIT
) 
exit /b
