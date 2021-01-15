@echo off

IF "%CD%\" == "%~dp0" (
  cd ..
)
IF "%OM_ROOT%" == "" (
  set OM_ROOT=%CD%
)

if not exist %OM_ROOT%\models\bin (
  @echo ERROR: missing models bin directory: %OM_ROOT%\models\bin
  exit
  EXIT 1
)

if not exist %OM_ROOT%\html (
  @echo ERROR: missing UI html directory: %OM_ROOT%\html
  exit
  EXIT 1
)

START "oms" /MIN bin\oms
START http://localhost:4040
