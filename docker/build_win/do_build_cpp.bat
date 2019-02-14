@echo off
REM build openM++ run-time libraries, omc compiler and models
REM environmemnt variables:
REM  set OM_SKIP_OPENM=yes              (default: no)
REM  set OM_SKIP_MODELS=yes             (default: no)
REM  set OM_BUILD_CONFIGS=Release Debug (default: Release)
REM  set OM_BUILD_PLATFORMS=Win32 x64   (default: Win32)
REM  set OM_MSG_USE=MPI                 (default: EMPTY)
REM  set MODEL_DIRS=modelOne NewCaseBased NewTimeBased NewCaseBased_bilingual NewTimeBased_bilingual IDMM OzProj OzProjGen RiskPaths

setlocal enabledelayedexpansion

set OM_BLD_CFG=Release
set OM_BLD_PLT=Win32

if defined OM_BUILD_CONFIGS   set OM_BLD_CFG=%OM_BUILD_CONFIGS%
if defined OM_BUILD_PLATFORMS set OM_BLD_PLT=%OM_BUILD_PLATFORMS%
if /I "%OM_MSG_USE%"=="MPI"   set OM_P_MPI=-p:OM_MSG_USE=MPI

if /i "%OM_SKIP_OPENM%"=="yes"  set OM_OMPP_SKIP=1
if /i "%OM_SKIP_OPENM%"=="true" set OM_OMPP_SKIP=1

set OM_BLD_MDLS=modelOne NewCaseBased NewTimeBased NewCaseBased_bilingual NewTimeBased_bilingual IDMM OzProj OzProjGen RiskPaths
if defined MODEL_DIRS            set OM_BLD_MDLS=%MODEL_DIRS%
if /i "%OM_SKIP_MODELS%"=="yes"  set OM_BLD_MDLS=
if /i "%OM_SKIP_MODELS%"=="true" set OM_BLD_MDLS=

REM show environment

set START_DT=%DATE% %TIME%
@echo %START_DT%
@echo Build openM++ run-time libraries, omc compiler and models
@echo Environment:
@echo  OM_SKIP_OPENM      = %OM_SKIP_OPENM%
@echo  OM_SKIP_MODELS     = %OM_SKIP_MODELS%
@echo  OM_BUILD_CONFIGS   = %OM_BUILD_CONFIGS%
@echo  OM_BUILD_PLATFORMS = %OM_BUILD_PLATFORMS%
@echo  OM_MSG_USE         = %OM_MSG_USE%
@echo  MODEL_DIRS         = %MODEL_DIRS%
@echo Build configurations: %OM_BLD_CFG%
@echo Build paltforms:      %OM_BLD_PLT%
if defined OM_P_MPI (
  @echo Build cluster version (MPI)
) else (
  @echo Build desktop version (non-MPI)
)

REM get source code from git

if not exist ompp (
  
  @echo git clone http://git.code.sf.net/p/ompp/git ompp
  git clone http://git.code.sf.net/p/ompp/git ompp
  if ERRORLEVEL 1 (
    @echo FAILED.
    EXIT
  ) 
  
) else (
  @echo Skip: git clone
  @echo Skip: git clone >> log\openm-build.log
)

REM push into ompp root and make log directory if not exist

pushd ompp
set   OM_ROOT=%CD%
@echo  OM_ROOT            = %OM_ROOT%

if not exist log mkdir log

REM log build environment 

@echo Log file: log\openm-build.log
@echo %START_DT% > log\openm-build.log
@echo  OM_ROOT            = %OM_ROOT% >> log\openm-build.log
@echo  OM_SKIP_GIT        = %OM_SKIP_GIT% >> log\openm-build.log
@echo  OM_SKIP_OPENM      = %OM_SKIP_OPENM% >> log\openm-build.log
@echo  OM_SKIP_MODELS     = %OM_SKIP_MODELS% >> log\openm-build.log
@echo  OM_BUILD_CONFIGS   = %OM_BUILD_CONFIGS% >> log\openm-build.log
@echo  OM_BUILD_PLATFORMS = %OM_BUILD_PLATFORMS% >> log\openm-build.log
@echo  OM_MSG_USE         = %OM_MSG_USE% >> log\openm-build.log
@echo  MODEL_DIRS         = %MODEL_DIRS% >> log\openm-build.log
@echo Build configurations: %OM_BLD_CFG% >> log\openm-build.log
@echo Build paltforms:      %OM_BLD_PLT% >> log\openm-build.log
if defined OM_P_MPI (
  @echo Build cluster version (MPI) >> log\openm-build.log
) else (
  @echo Build desktop version (non-MPI) >> log\openm-build.log
)

REM build c++ run-time libraries and omc compiler

if not defined OM_OMPP_SKIP (

  @echo Build openm run-time libraries and omc compiler
  @echo Build openm run-time libraries and omc compiler >> log\openm-build.log
  
  pushd openm
  for %%c in (%OM_BLD_CFG%) do (
    for %%p in (%OM_BLD_PLT%) do (
      call :make_openm_sln "%OM_P_MPI% -p:Configuration=%%c -p:Platform=%%p openm.sln"
    )
  )
  popd
  
  @echo Completed: build openm run-time libraries and omc compiler
  @echo Completed: build openm run-time libraries and omc compiler >> log\openm-build.log

) else (
  @echo Skip: build openm run-time libraries and omc compiler
  @echo Skip: build openm run-time libraries and omc compiler >> log\openm-build.log
)

REM build models, log files: models\log\ModelName-configuration-platform.log

if defined OM_BLD_MDLS (
  @echo Build models: %OM_BLD_MDLS%
  @echo Build models: %OM_BLD_MDLS% >> log\openm-build.log
) else (
  @echo Skip: build models
  @echo Skip: build models >> log\openm-build.log
)

pushd models

for %%m in (%OM_BLD_MDLS%) do (

  @echo Build: %%m
  @echo Build: %%m >> ..\log\openm-build.log
  set MDL_RUN=
  
  for %%c in (%OM_BLD_CFG%) do (
    for %%p in (%OM_BLD_PLT%) do (

      set MDL_EXE=%%m
      if /i "%%p"=="x64" set MDL_EXE=!MDL_EXE!64
      if /i "%%c"=="Debug" set MDL_EXE=!MDL_EXE!D
      if not defined OM_BLD_EMPTY (
        if defined OM_P_MPI set MDL_EXE=!MDL_EXE!_mpi
      )
      
      set MDL_P_ARGS=-p:Configuration=%%c -p:Platform=%%p %%m-ompp.sln
      set MDL_DIR=%%m
      if /i not "!MDL_DIR:_bilingual=!"=="!MDL_DIR!" (
        set MDL_P_ARGS=-p:Configuration=%%c -p:Platform=%%p !MDL_DIR:_bilingual=!-ompp.sln
      )
      if /i not "!MDL_DIR:modelOne=!"=="!MDL_DIR!" (
        if not defined MDL_ONE_EXE set MDL_ONE_EXE=!MDL_EXE!
	set MDL_RUN=1
        set MDL_P_ARGS=-p:Configuration=%%c -p:Platform=%%p %%m.sln
      )

      set MDL_LOG=%%m-%%c-%%p.log
      if defined %OM_P_MPI% set MDL_LOG=%%m_mpi-%%c-%%p.log
      
      REM build the model
      
      call :make_model_sln %%m !MDL_LOG! "%OM_P_MPI% !MDL_P_ARGS!"
      
      REM run the model after first build, if not modelOne
      
      if not defined MDL_RUN (

        pushd %%m\ompp\bin
        
        @echo Run: !MDL_EXE! -OpenM.ProgressPercent 25
        @echo Run: !MDL_EXE! -OpenM.ProgressPercent 25 >> ..\..\..\..\log\openm-build.log
        !MDL_EXE! -OpenM.ProgressPercent 25 > ..\..\..\log\!MDL_EXE!.log 2>&1
        if ERRORLEVEL 1 (
          @echo FAILED.
          @echo FAILED. >> ..\..\..\..\log\openm-build.log
          EXIT
        )
        popd
        set MDL_RUN=1
      )
    )
  )
)

REM create modelOne.sqlite database and run modelOne

if defined MDL_ONE_EXE (

  pushd modelOne\ompp\bin

  @echo Create modelOne.sqlite
  @echo Create modelOne.sqlite >> ..\..\..\..\log\openm-build.log
  
  if exist modelOne.sqlite (
    del modelOne.sqlite
    if ERRORLEVEL 1 (
      @echo FAILED.
      @echo FAILED. >> ..\..\..\..\log\openm-build.log
      EXIT
    )
  )
  
  (%SQLITE_EXE_DIR%\sqlite3 modelOne.sqlite < ..\..\..\..\sql\create_db.sql && ^
%SQLITE_EXE_DIR%\sqlite3 modelOne.sqlite < ..\..\..\..\sql\insert_default.sql && ^
%SQLITE_EXE_DIR%\sqlite3 modelOne.sqlite < ..\..\..\..\sql\sqlite\optional_meta_views_sqlite.sql && ^
%SQLITE_EXE_DIR%\sqlite3 modelOne.sqlite < ..\..\modelOne_create_model.sql && ^
%SQLITE_EXE_DIR%\sqlite3 modelOne.sqlite < ..\..\modelOne_insert_param.sql && ^
%SQLITE_EXE_DIR%\sqlite3 modelOne.sqlite < ..\..\modelOne_optional_views.sql) >> ..\..\..\..\log\openm-build.log 2>&1
  if ERRORLEVEL 1 (
    @echo FAILED.
    @echo FAILED. >> ..\..\..\..\log\openm-build.log
    EXIT
  )

  @echo Run: %MDL_ONE_EXE%
  @echo Run: %MDL_ONE_EXE% >> ..\..\..\..\log\openm-build.log
  
  (%MDL_ONE_EXE% -OpenM.RunName Default && ^
%MDL_ONE_EXE% -OpenM.SubValues 4 -OpenM.Threads 4 -OpenM.RunName "Default-4" && ^
%MDL_ONE_EXE% -OpenM.TaskName taskOne -OpenM.TaskRunName "First Task Run" && ^
%MDL_ONE_EXE% -SubValue.salaryFull csv -OpenM.SubValues 2 -OpenM.ParamDir ../../csv ^
  -OpenM.RunName "Parameter sub-values 2 from csv" && ^
%MDL_ONE_EXE% -OpenM.SubValues 4 -OpenM.Threads 4 -SubValue.baseSalary db -SubValue.salaryFull db -SubValue.filePath db -SubValue.isOldAge db ^
  -OpenM.RunName "Parameter sub-values 4") > ..\..\..\log\%MDL_ONE_EXE%.log 2>&1
  if ERRORLEVEL 1 (
    @echo FAILED.
    @echo FAILED. >> ..\..\..\..\log\openm-build.log
    EXIT
  )
  
  popd
)

if defined OM_BLD_MDLS (
  @echo Completed build models: %OM_BLD_MDLS%
  @echo Completed build models: %OM_BLD_MDLS% >> log\openm-build.log
)

@echo %DATE% %TIME% Done.
@echo %DATE% %TIME% Done. >> log\openm-build.log

popd
goto :eof

REM end of main body, subroutines below

REM build openm solution subroutine
REM arguments: 
REM  1 = msbuild command line arguments

:make_openm_sln

set mk_args=%~1
@echo msbuild %mk_args%
@echo msbuild %mk_args% >> ..\log\openm-build.log

msbuild %mk_args% >> ..\log\openm-build.log 2>&1
if ERRORLEVEL 1 (
  @echo FAILED.
  @echo FAILED. >> ..\log\openm-build.log
  EXIT
) 
exit /b

REM build model subroutine
REM arguments:
REM  1 = model directory
REM  2 = model build log file name
REM  3 = msbuild command line arguments

:make_model_sln

set m_dir=%1
set m_log=%2
set mk_args=%~3

pushd %m_dir%

@echo msbuild %mk_args%
@echo msbuild %mk_args% >> ..\..\log\openm-build.log

msbuild %mk_args% >..\log\%m_log% 2>&1
if ERRORLEVEL 1 (
  @echo FAILED.
  @echo FAILED. >> ..\..\log\openm-build.log
  EXIT
)
popd
exit /b
