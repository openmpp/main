@echo off
REM create zip archive of openM++ build from ompp sub-directory: openmpp_win_20180817.zip
REM environmemnt variables:
REM  set OM_MSG_USE=MPI                 (default: EMPTY)
REM  set MODEL_DIRS=modelOne,NewCaseBased,NewTimeBased,NewCaseBased_bilingual,NewTimeBased_bilingual,IDMM,OzProj,OzProjGen,RiskPaths

setlocal enabledelayedexpansion

if /I "%OM_MSG_USE%"=="MPI" set OM_SFX_MPI=_mpi

set OM_BLD_MDLS=modelOne,NewCaseBased,NewTimeBased,NewCaseBased_bilingual,NewTimeBased_bilingual,IDMM,OzProj,OzProjGen,RiskPaths
if defined MODEL_DIRS       set OM_BLD_MDLS=%MODEL_DIRS%

REM push into ompp root and make log directory if not exist

if not exist ompp (
  @echo ERROR: missing source code directory: ompp
  EXIT 1
)

pushd ompp
set   OM_ROOT=%CD%

if not exist log mkdir log

REM get current UTC date

for /F "tokens=1,2 delims==" %%G in ('wmic path Win32_UTCTime get Year^,Month^,Day /value ^| find "="') do (
  set utc_%%G=%%H
)

set /A OM_DATE_STAMP= 10000 * %utc_Year% + 100 * %utc_Month% + %utc_Day%

set OM_DATE_STAMP=%OM_DATE_STAMP:~0,8%
set DEPLOY_DIR=..\openmpp_win%OM_SFX_MPI%_%OM_DATE_STAMP%
set DEPLOY_ZIP=%DEPLOY_DIR%.zip

REM log build environment 

@echo %DATE% %TIME% Pack openM++ build
@echo Environment:
@echo  OM_MSG_USE = %OM_MSG_USE%
@echo  OM_ROOT    = %OM_ROOT%
@echo  MODEL_DIRS = %MODEL_DIRS%
@echo  DEPLOY_DIR = %DEPLOY_DIR%
@echo Pack into: %DEPLOY_ZIP%

@echo Log file: log\build-zip.log
@echo %DATE% %TIME% Pack openM++ build > log\build-zip.log
@echo  OM_MSG_USE = %OM_MSG_USE% >> log\build-zip.log
@echo  OM_ROOT    = %OM_ROOT% >> log\build-zip.log
@echo  MODEL_DIRS = %MODEL_DIRS% >> log\build-zip.log
@echo  DEPLOY_DIR = %DEPLOY_DIR% >> log\build-zip.log
@echo Pack into: %DEPLOY_ZIP% >> log\build-zip.log

REM delete existing pack directory and zip file

if exist %DEPLOY_DIR% (

  @echo Remove existing: %DEPLOY_DIR%
  @echo Remove existing: %DEPLOY_DIR% >> log\build-zip.log
  
  for /L %%k in (1,1,8) do (
    if exist %DEPLOY_DIR% (
      rd /s /q %DEPLOY_DIR% >> log\build-zip.log 2>&1
    )
    if exist %DEPLOY_DIR% (
      ping 127.0.0.1 -n 2 -w 500 >nul
    )
  )

  if exist %DEPLOY_DIR% (
    @echo FAILED to delete: %DEPLOY_DIR%
    @echo FAILED to delete: %DEPLOY_DIR% >> log\build-zip.log
    EXIT 1
  )
)

if exist %DEPLOY_ZIP% (

  @echo Remove existing: %DEPLOY_ZIP%
  @echo Remove existing: %DEPLOY_ZIP% >> log\build-zip.log
  
  for /L %%k in (1,1,8) do (
    if exist %DEPLOY_ZIP% (
      del /f /q %DEPLOY_ZIP% >> log\build-zip.log 2>&1
    )
    if exist %DEPLOY_ZIP% (
      ping 127.0.0.1 -n 2 -w 500 >nul
    )
  )

  if exist %DEPLOY_ZIP% (
    @echo FAIL to delete: %DEPLOY_ZIP%
    @echo FAIL to delete: %DEPLOY_ZIP% >> log\build-zip.log
    EXIT 1
  )
)

REM create new deploy directory, copy top level sub-directories and files

call :make_dir %DEPLOY_DIR%

echo Copy files: > log\build-zip-copy.log

call :rcopy_files    %DEPLOY_DIR% . "README_win.txt README.txt LICENSE.txt AUTHORS.txt"
call :rcopy_sub_dirs %DEPLOY_DIR% . "docker,etc,Excel,include,licenses,openm,props,R,sql,use"

REM copy openm runtime libraries

call :rcopy_files ^
  %DEPLOY_DIR%\lib ^
  lib ^
  "libopenm%OM_SFX_MPI%.lib libopenm64%OM_SFX_MPI%.lib libopenmD%OM_SFX_MPI%.lib libopenm64D%OM_SFX_MPI%.lib libsqlite.lib libsqlite64.lib libsqliteD.lib libsqlite64D.lib"

REM copy bin directory executables

call :rcopy_files ^
  %DEPLOY_DIR%\bin ^
  bin ^
  "omc.exe omc.message.ini ompp_create_scex.exe ompp_export_csv.exe ompp_export_excel.exe modgen_export_csv.exe patch_modgen11_outputs.exe patch_modgen12_outputs.exe patch_modgen12.1_outputs.exe README_win.txt sqlite3.exe ompp_ui.bat"

REM copy Go bin executables and source code

call :rcopy_files    %DEPLOY_DIR%\bin            ompp-go\bin                "dbcopy.exe oms.exe"
call :rcopy_files    %DEPLOY_DIR%\go.openmpp.org ompp-go\src\go.openmpp.org "LICENSE.txt AUTHORS.txt"
call :rcopy_sub_dirs %DEPLOY_DIR%\go.openmpp.org ompp-go\src\go.openmpp.org "dbcopy,ompp,oms,licenses"

REM copy UI html build and source code

call :rcopy_sub_dirs %DEPLOY_DIR%\html    ompp-ui "dist"
call :rcopy_files    %DEPLOY_DIR%\ompp-ui ompp-ui "*.*"
call :rcopy_sub_dirs %DEPLOY_DIR%\ompp-ui ompp-ui "build,config,src,static"

REM create log directories and models directories

call :make_dir %DEPLOY_DIR%\log
call :make_dir %DEPLOY_DIR%\models\bin
call :make_dir %DEPLOY_DIR%\models\sql
call :make_dir %DEPLOY_DIR%\models\log

REM copy models
REM modelOne is special case, it does not have modgen or any code/*.ompp or parameters/*.mpp files

for %%m in (%OM_BLD_MDLS%) do (
  
  set MDL_DIR=%%m
  @echo Copy model: %%m
  @echo Copy model: %%m >> log\build-zip.log
  
  if /i "!MDL_DIR:modelOne=!"=="!MDL_DIR!" (
  
    call :rcopy_sub_dirs %DEPLOY_DIR%\models\%%m        models\%%m          "code,parameters"
    call :rcopy_files    %DEPLOY_DIR%\models\%%m        models\%%m          "*.sln"
    call :rcopy_files    %DEPLOY_DIR%\models\%%m\modgen models\%%m\modgen   "*.vcxproj *.vcxproj.filters *.props"
    call :do_copy_files  %DEPLOY_DIR%\models\sql        models\%%m\ompp\src\*.sql
    
  ) else (
  
    call :rcopy_sub_dirs %DEPLOY_DIR%\models\%%m models\%%m  "csv"
    call :rcopy_files    %DEPLOY_DIR%\models\%%m models\%%m  "*.sln *.cpp *.h *.ini *.sql *.txt"
    call :do_copy_files  %DEPLOY_DIR%\models\sql models\%%m\*.sql
  )
  
  call :rcopy_files %DEPLOY_DIR%\models\%%m\ompp models\%%m\ompp "*.vcxproj *.vcxproj.filters *.props"
    
  call :rcopy_files ^
    %DEPLOY_DIR%\models\bin ^
    models\%%m\ompp\bin ^
    "!MDL_DIR!%OM_SFX_MPI%.exe !MDL_DIR!64%OM_SFX_MPI%.exe !MDL_DIR!.sqlite !MDL_DIR!*.ini"
)

call :rcopy_files    %DEPLOY_DIR%\models          models          ".gitignore"
call :rcopy_sub_dirs %DEPLOY_DIR%\models          models          "microdata"

REM create zip archive from deployment directory

@echo Build completed on: %OM_DATE_STAMP:~0,4%-%OM_DATE_STAMP:~4,2%-%OM_DATE_STAMP:~6,2% > %DEPLOY_DIR%\build_date.txt

@echo Create %DEPLOY_ZIP%
@echo Create %DEPLOY_ZIP% >> log\build-zip.log

C:\7zip\7z.exe a -tzip %DEPLOY_ZIP% %DEPLOY_DIR%\*
if ERRORLEVEL 1 (
  @echo FAIL to create %DEPLOY_ZIP%
  @echo FAIL to create %DEPLOY_ZIP% >> log\build-zip.log
  EXIT
)

@echo %DATE% %TIME% Done.
@echo %DATE% %TIME% Done. >> log\build-zip.log

popd
goto :eof

REM end of main body

REM copy file(s), using copy command
REM arguments:
REM  1 = destination directory
REM  2 = source file(s) path, can be wildcard

:do_copy_files

set dst_dir=%1
set src_path=%2

@echo Copy files from: %src_path%
@echo Copy files from: %src_path% >> log\build-zip.log

copy /b %src_path% %dst_dir% >> log\build-zip-copy.log
if ERRORLEVEL 2 (
  @echo FAIL to copy: %src_path%
  @echo FAIL to copy: %src_path% >> log\build-zip.log
  EXIT
)
exit /b

REM robocopy file(s)
REM arguments:
REM  1 = destination directory
REM  2 = source directory
REM  3 = file name(s), space separated, must be "quoted", can be wildcard

:rcopy_files

set dst_dir=%1
set src_dir=%2
set fnames=%~3

@echo Copy files from: %src_dir%
@echo Copy files from: %src_dir% >> log\build-zip.log

robocopy /njh /njs /np %src_dir% %dst_dir% %fnames% >> log\build-zip-copy.log
if ERRORLEVEL 2 (
@echo %ERRORLEVEL%
  @echo FAIL to copy: %fnames%
  @echo FAIL to copy: %fnames% >> log\build-zip.log
  EXIT
)
exit /b

REM robocopy sub-directories
REM arguments:
REM  1 = destination directory
REM  2 = source root directory
REM  2 = source sub-directories, comma or space separated list, must be "quoted"

:rcopy_sub_dirs

set dst_dir=%1
set src_root=%2
set src_dir_lst=%~3

for %%d in (%src_dir_lst%) do (

  @echo Copy directory: %%d
  @echo Copy directory: %%d >> log\build-zip.log
  
  robocopy /s /e /njh /njs /np %src_root%\%%d %dst_dir%\%%d >> log\build-zip-copy.log
  if ERRORLEVEL 2 (
    @echo FAIL to copy: %%d
    @echo FAIL to copy: %%d >> log\build-zip.log
    EXIT
  )
)
exit /b

REM create directory, exit on error, do log
REM arguments:
REM  1 = destination directory

:make_dir

set dst_dir=%1

@echo Create directory: %dst_dir%
@echo Create directory: %dst_dir% >> log\build-zip.log

mkdir %dst_dir%
if ERRORLEVEL 1 (
  @echo FAIL to create: %dst_dir%
  @echo FAIL to create: %dst_dir% >> log\build-zip.log
  EXIT
)
exit /b

