@echo off
REM init openM++ build: copy default build script if not exist and start it

if /i "%1" == "cpp" (

  if not exist do_build_cpp.bat (
    @echo copy C:\build_scripts\do_build_cpp.bat .
    copy C:\build_scripts\do_build_cpp.bat .
  ) else (
    @echo Found: %CD%\do_build_cpp.bat
  )

  call do_build_cpp.bat
  EXIT
)

type C:\build_scripts\prompt_cpp.txt
