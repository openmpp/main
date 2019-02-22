@echo off
REM openM++ build script
REM copy of any scripts below from docker image done only if such file not already exist
REM you can customize this script or any scripts below in order to change build results

call build-openm.bat
call build-models.bat
call build-go.bat
call build-r.bat
call build-perl.bat
call build-ui.bat

call build-zip.bat
