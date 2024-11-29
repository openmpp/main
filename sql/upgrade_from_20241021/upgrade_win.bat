@echo off
REM upgrade models SQLite database files
REM from versions [2023-02-01, 2024-10-21] a.k.a. [v1.14.0, v1.17.5]
REM
REM IMPORTANT:
REM Make a backup copy of your model.sqlite database files before upgrade
REM
REM How to run upgrade script:
REM For example,
REM   if new version of openM++ is in:
REM C:\openmpp_win_next\
REM                     bin\
REM                     sql\upgrade_from_20241021\
REM                                               upgrade_win.bat
REM
REM   and previous version of openM++ is in:
REM C:\openmpp_win_20241021\
REM                         bin\
REM                         models\
REM
REM then:
REM
REM option 1. 
REM   copy directory \upgrade_from_20241021\ into old openM++: C:\openmpp_win_20241021\
REM   double click on C:\openmpp_win_20241021\upgrade_from_20241021\upgrade_win.bat
REM
REM option 2. 
REM   open command prompt (not PowerShell) and type:
REM     cd /D C:\openmpp_win_next\sql\upgrade_from_20241021
REM     upgrade_win.bat C:\openmpp_win_20241021

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

set upgrade_sql="%~dp0"upgrade_from_20241021.sql

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

REM check: do not run upgrade script on the versions after 20241021

if exist %dst_root%\sql\upgrade_from_20241021\upgrade_win.bat (
  @echo ERROR: it is look like destination folder is incorrect: %dst_root% 
  @echo ERROR: it must contain versions [2023-02-01, 2024-10-21] a.k.a. [v1.14.0, v1.17.5]
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
