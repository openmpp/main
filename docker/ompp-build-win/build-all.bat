@echo off
REM openM++ build script
REM you can customize this script or any scripts below in order to change build results

call build-openm.bat
call build-models.bat
call build-go.bat
call build-r.bat
call build-perl.bat
call build-ui.bat

call build-zip.bat
