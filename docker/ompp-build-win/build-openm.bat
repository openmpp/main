@echo off
REM build openM++ run-time libraries and omc compiler
REM environmemnt variables:
REM  set OM_BUILD_CONFIGS=Release,Debug (default: Release,Debug)
REM  set OM_BUILD_PLATFORMS=Win32,x64   (default: Win32,x64)
REM  set OM_MSG_USE=MPI                 (default: EMPTY)

setlocal enabledelayedexpansion

set OM_BLD_CFG=Release,Debug
set OM_BLD_PLT=Win32,x64

if defined OM_BUILD_CONFIGS   set OM_BLD_CFG=%OM_BUILD_CONFIGS%
if defined OM_BUILD_PLATFORMS set OM_BLD_PLT=%OM_BUILD_PLATFORMS%
if /I "%OM_MSG_USE%"=="MPI"   set OM_P_MPI=-p:OM_MSG_USE=MPI

REM show environment

set START_DT=%DATE% %TIME%
@echo %START_DT% Build openM++ run-time libraries and omc compiler
@echo Environment:
@echo  OM_BUILD_CONFIGS   = %OM_BUILD_CONFIGS%
@echo  OM_BUILD_PLATFORMS = %OM_BUILD_PLATFORMS%
@echo  OM_MSG_USE         = %OM_MSG_USE%
@echo Build configurations: %OM_BLD_CFG%
@echo Build paltforms:      %OM_BLD_PLT%
if defined OM_P_MPI (
  @echo Build cluster version: using MPI
) else (
  @echo Build desktop version: non-MPI
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
)

REM push into ompp root and make log directory if not exist

pushd ompp
set   OM_ROOT=%CD%
@echo  OM_ROOT            = %OM_ROOT%

if not exist log mkdir log

REM log build environment 

@echo Log file: log\build-openm.log
@echo %START_DT% Build openM++ run-time libraries and omc compiler > log\build-openm.log
@echo  OM_BUILD_CONFIGS   = %OM_BUILD_CONFIGS% >> log\build-openm.log
@echo  OM_BUILD_PLATFORMS = %OM_BUILD_PLATFORMS% >> log\build-openm.log
@echo  OM_MSG_USE         = %OM_MSG_USE% >> log\build-openm.log
@echo  OM_ROOT            = %OM_ROOT% >> log\build-openm.log
@echo Build configurations: %OM_BLD_CFG% >> log\build-openm.log
@echo Build paltforms:      %OM_BLD_PLT% >> log\build-openm.log
if defined OM_P_MPI (
  @echo Build cluster version: using MPI >> log\build-openm.log
) else (
  @echo Build desktop version: non-MPI >> log\build-openm.log
)

REM build c++ run-time libraries and omc compiler

pushd openm
for %%c in (%OM_BLD_CFG%) do (
  for %%p in (%OM_BLD_PLT%) do (
    call :make_openm_sln "%OM_P_MPI% -p:Configuration=%%c -p:Platform=%%p openm.sln"
  )
)
popd

@echo %DATE% %TIME% Done.
@echo %DATE% %TIME% Done. >> log\build-openm.log

popd
goto :eof

REM end of main body

REM build openm solution subroutine
REM arguments: 
REM  1 = msbuild command line arguments

:make_openm_sln

set mk_args=%~1
@echo msbuild %mk_args%
@echo msbuild %mk_args% >> ..\log\build-openm.log

msbuild %mk_args% >> ..\log\build-openm.log 2>&1
if ERRORLEVEL 1 (
  @echo FAILED.
  @echo FAILED. >> ..\log\build-openm.log
  EXIT
) 
exit /b
