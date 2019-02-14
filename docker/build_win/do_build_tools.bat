@echo off
REM build openM++ tools: Go oms web-service, dbcopy utility, R package, Perl utilities and openM++ UI
REM environmemnt variables:
REM  set OM_SKIP_GO=yes   (default: no)
REM  set OM_SKIP_UI=yes   (default: no)
REM  set OM_SKIP_R=yes    (default: no)
REM  set OM_SKIP_PERL=yes (default: no)

setlocal enabledelayedexpansion

if /i "%OM_SKIP_GO%"=="yes"  set OM_GO_SKIP=1
if /i "%OM_SKIP_GO%"=="true" set OM_GO_SKIP=1

if /i "%OM_SKIP_UI%"=="yes"  set OM_UI_SKIP=1
if /i "%OM_SKIP_UI%"=="true" set OM_UI_SKIP=1

if /i "%OM_SKIP_R%"=="yes"  set OM_R_SKIP=1
if /i "%OM_SKIP_R%"=="true" set OM_R_SKIP=1

if /i "%OM_SKIP_PERL%"=="yes"  set OM_PERL_SKIP=1
if /i "%OM_SKIP_PERL%"=="true" set OM_PERL_SKIP=1

REM show environment

set START_DT=%DATE% %TIME%
@echo %START_DT%
@echo Build openM++ tools
@echo Environment:
@echo  OM_SKIP_GO   = %OM_SKIP_GO%
@echo  OM_SKIP_UI   = %OM_SKIP_UI%
@echo  OM_SKIP_R    = %OM_SKIP_R%
@echo  OM_SKIP_PERL = %OM_SKIP_PERL%

REM push into ompp root and make log directory if not exist

if not exist ompp (
  @echo ERROR: missing source code directory: ompp
  EXIT 1
)

pushd ompp
set   OM_ROOT=%CD%
@echo  OM_ROOT      = %OM_ROOT%

if not exist log mkdir log

REM log build environment 

@echo Log file: log\tools-build.log
@echo %START_DT% > log\tools-build.log
@echo  OM_ROOT      = %OM_ROOT% >> log\tools-build.log
@echo  OM_SKIP_GO   = %OM_SKIP_GO% >> log\tools-build.log
@echo  OM_SKIP_UI   = %OM_SKIP_UI% >> log\tools-build.log
@echo  OM_SKIP_R    = %OM_SKIP_R% >> log\tools-build.log
@echo  OM_SKIP_PERL = %OM_SKIP_PERL% >> log\tools-build.log

REM get source code from svn, if not directory not already exist

if not exist ompp-go (
  
  call :do_cmd_line_log log\tools-build.log "svn checkout https://svn.code.sf.net/p/ompp/svn/trunk/ ompp-go/src/go.openmpp.org"
  
) else (
  @echo Skip: svn checkout
  @echo Skip: svn checkout >> log\tools-build.log
)

REM build go oms web-service and dbcopy database utility

if not defined OM_SKIP_GO (

  @echo Build Go web-service and tools
  @echo Build Go web-service and tools >> log\tools-build.log

  pushd ompp-go
  call :do_cmd_line "go get go.openmpp.org/dbcopy"
  call :do_cmd_line "go get go.openmpp.org/oms"
  call :do_cmd_line "go install go.openmpp.org/dbcopy"
  call :do_cmd_line "go install go.openmpp.org/oms"
  popd
  
) else (
  @echo Skip: build Go web-service and database tools
  @echo Skip: build Go web-service and tools >> log\tools-build.log
)

REM build openM++ UI: node, npm, vue.js

if not defined OM_SKIP_UI (

  @echo Build openM++ UI alpha
  @echo Build openM++ UI alpha >> log\tools-build.log

  pushd ompp-ui
  set npm_config_cache=%OM_ROOT%\build\npm-cache
  call :do_npm_call "install"
  call :do_npm_call "run build"
  popd
  
) else (
  @echo Skip: build openM++ UI alpha
  @echo Skip: build openM++ UI alpha >> log\tools-build.log
)

REM build openMpp R package

if not defined OM_SKIP_R (

  @echo Build openMpp R package
  @echo Build openMpp R package >> log\tools-build.log

  pushd R
  call :do_cmd_line "R CMD build openMpp"
  popd
  
) else (
  @echo Skip: build openMpp R package
  @echo Skip: build openMpp R package >> log\tools-build.log
)

REM build Perl utilities

if not defined OM_SKIP_PERL (

  @echo Build Perl utilities
  @echo Build Perl utilities >> log\tools-build.log

  call %PERLROOT%\portableshell /SETENV
  pushd Perl
  call :do_pp_call "-o ../bin/ompp_export_excel.exe ompp_export_excel.pl"
  call :do_pp_call "-o ../bin/ompp_export_csv.exe ompp_export_csv.pl"
  call :do_pp_call "-o ../bin/modgen_export_csv.exe modgen_export_csv.pl"
  call :do_pp_call "-o ../bin/ompp_create_scex.exe ompp_create_scex.pl"
  call :do_pp_call "-o ../bin/patch_modgen12.1_outputs.exe patch_modgen12.1_outputs.pl"
  popd
  
) else (
  @echo Skip: build Perl utilities
  @echo Skip: build Perl utilities >> log\tools-build.log
)

@echo %DATE% %TIME% Done.
@echo %DATE% %TIME% Done. >> log\tools-build.log

popd
goto :eof

REM end of main body, subroutines below

REM helper subroutine to execute command, log it and check errorlevel
REM arguments:
REM  1 = command line

:do_cmd_line
  call :do_cmd_line_log ..\log\tools-build.log %1
exit /b

REM helper subroutine to execute command, log it and check errorlevel
REM arguments:
REM  1 = log file path
REM  2 = command line

:do_cmd_line_log

set c_log=%1
set c_line=%~2

@echo %c_line%
@echo %c_line% >> %c_log%

%c_line% >> %c_log% 2>&1
if ERRORLEVEL 1 (
  @echo FAILED.
  @echo FAILED. >> %c_log%
  EXIT
) 
exit /b

REM helper subroutine to call npm command, log it and check errorlevel
REM arguments:
REM  1 = npm command line arguments

:do_npm_call

set c_line=%~1

@echo npm %c_line%
@echo npm %c_line% >> ..\log\tools-build.log

call npm %c_line% >> ..\log\tools-build.log 2>&1
if ERRORLEVEL 1 (
  @echo FAILED.
  @echo FAILED. >> ..\log\tools-build.log
  EXIT
) 
exit /b

REM helper subroutine to call Perl pp, log it and check errorlevel
REM arguments:
REM  1 = pp command line arguments

:do_pp_call

set c_line=%~1

@echo pp %c_line%
@echo pp %c_line% >> ..\log\tools-build.log

call pp %c_line% >> ..\log\tools-build.log 2>&1
if ERRORLEVEL 1 (
  @echo FAILED.
  @echo FAILED. >> ..\log\tools-build.log
  EXIT
) 
exit /b
