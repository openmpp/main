@echo off
REM upgrade models SQLite database files to release 2022-10-12 a.k.a. v1.12.0
REM
REM IMPORTANT:
REM Make a backup copy of your model.sqlite database files before upgrade
REM
REM How to run upgrade script:
REM For example,
REM   if new version of openM++ is in:
REM C:\openmpp_win_20221210\
REM                        bin\
REM                        sql\upgrade_to_20221210\
REM                                               upgrade_win.bat
REM
REM   and previous version of openM++ is in:
REM C:\openmpp_win_20220923\
REM                        bin\
REM                        models\
REM
REM then:
REM
REM option 1. 
REM   copy directory \upgrade_to_20221210\ into old openM++: C:\openmpp_win_20220923\
REM   double click on C:\openmpp_win_20220923\upgrade_to_20221210\upgrade_win.bat
REM
REM option 2. 
REM   open command prompt (not PowerShell) and type:
REM     cd /D C:\openmpp_win_20221210\sql\upgrade_to_20221210
REM     upgrade_win.bat C:\openmpp_win_20220923

setlocal enabledelayedexpansion

REM if there is command line argument it must be previous openM++ version

set dst_root=

if not "%~1" == "" (
  set dst_root=%1
) else (
  set dst_root=%CD%\..
)

REM destination root must have \models\ subfolder
REM destination must have bin\sqlite3.exe or sqlite3 must be in the PATH
REM upgrade sql script must exist

set models_dir=%dst_root%\models

if not exist %models_dir% (
  @echo ERROR: missing models directory: %models_dir%
  pause
  EXIT /B 1
)
@echo Upgrade models at: %models_dir%

set upgrade_sql="%~dp0"upgrade_20221210.sql

if not exist %upgrade_sql% (
  @echo ERROR: missing upgrade script: %upgrade_sql%
  pause
  EXIT /B 1
)
@echo Upgrade script:    %upgrade_sql%

set sqlite3_exe=%dst_root%\bin\sqlite3.exe

if not exist %sqlite3_exe% (

  set sqlite3_exe=sqlite3.exe

  where /Q sqlite3.exe >nul
  if errorlevel 1 (
    @echo ERROR: missing sqlite3.exe
    pause
    EXIT /B 1
  )
)

REM check: do not run upgrade script on the same openM++ 20221210 or older

if exist %dst_root%\sql\upgrade_to_20221210\upgrade_win.bat (
  @echo ERROR: it is look like destination folder is incorrect: %dst_root% 
  @echo ERROR: it must contain version older than 20221210 a.k.a. v1.12.0
  pause
  EXIT /B 1
)

REM for each model.sqlite run upgrade.sql script

for /R "%models_dir%" %%g in (*.sqlite) do (
  @echo %%g

  %sqlite3_exe% %%g < %upgrade_sql% >nul
  if errorlevel 1 (
    @echo ERROR: at %sqlite3_exe% %%g < %upgrade_sql%
    pause
    EXIT /B 1
  )
)

@echo Done.
pause
