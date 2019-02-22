@echo off
REM copy build scripts if any not exist in curent directory

setlocal enabledelayedexpansion

call :do_copy_script build-all.bat
call :do_copy_script build-openm.bat
call :do_copy_script build-models.bat
call :do_copy_script build-go.bat
call :do_copy_script build-r.bat
call :do_copy_script build-perl.bat
call :do_copy_script build-ui.bat
call :do_copy_script build-zip.bat

goto :eof

REM end of main body

REM helper subroutine
REM if not exist then copy %OM_SCRIPTS_DIR%\file into current disrectory, exit if copy failed
REM arguments:
REM  1 = file name

:do_copy_script

if exist %1 (
  @echo Skip copy of existing: %CD%\%1
  exit /b
)

@echo copy %OM_SCRIPTS_DIR%\%1 .
copy %OM_SCRIPTS_DIR%\%1 .
if ERRORLEVEL 1 (
  @echo FAILED copy %1
  EXIT
)

exit /b
