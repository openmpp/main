@echo off

IF "%CD%\" == "%~dp0" (
  cd ..
)
IF "%OM_ROOT%" == "" (
  SET OM_ROOT=%CD%
)

if not exist %OM_ROOT%\models\bin (
  @echo ERROR: missing models bin directory: %OM_ROOT%\models\bin
  PAUSE
  EXIT 1
)

START "oms" /MIN bin\oms
START http://localhost:4040
