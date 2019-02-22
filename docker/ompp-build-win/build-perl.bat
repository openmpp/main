@echo off
REM build openM++ Perl utilities

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

@echo %DATE% %TIME% Build Perl utilities
@echo OM_ROOT = %OM_ROOT%

@echo Log file: log\build-perl.log
@echo %DATE% %TIME% Build Perl utilities > log\build-perl.log
@echo OM_ROOT = %OM_ROOT% >> log\build-perl.log

REM build Perl utilities

call %PERLROOT%\portableshell /SETENV
pushd Perl
call :do_pp_call "-o ../bin/ompp_export_excel.exe ompp_export_excel.pl"
call :do_pp_call "-o ../bin/ompp_export_csv.exe ompp_export_csv.pl"
call :do_pp_call "-o ../bin/modgen_export_csv.exe modgen_export_csv.pl"
call :do_pp_call "-o ../bin/ompp_create_scex.exe ompp_create_scex.pl"
call :do_pp_call "-o ../bin/patch_modgen11_outputs.exe patch_modgen11_outputs.pl"
call :do_pp_call "-o ../bin/patch_modgen12_outputs.exe patch_modgen12_outputs.pl"
call :do_pp_call "-o ../bin/patch_modgen12.1_outputs.exe patch_modgen12.1_outputs.pl"

popd

if not exist bin\sqlite3.exe (
  @echo copy %SQLITE_EXE_DIR%\sqlite3.exe bin
  copy %SQLITE_EXE_DIR%\sqlite3.exe bin
)

@echo %DATE% %TIME% Done.
@echo %DATE% %TIME% Done. >> log\build-perl.log

popd
goto :eof

REM end of main body

REM helper subroutine to call Perl pp, log it and check errorlevel
REM arguments:
REM  1 = pp command line arguments

:do_pp_call

set c_line=%~1

@echo pp %c_line%
@echo pp %c_line% >> ..\log\build-perl.log

call pp %c_line% >> ..\log\build-perl.log 2>&1
if ERRORLEVEL 1 (
  @echo FAILED.
  @echo FAILED. >> ..\log\build-perl.log
  EXIT
) 
exit /b
