@echo off

IF "%CD%\" == "%~dp0" (
  cd ..
)
IF "%OM_ROOT%" == "" (
  SET OM_ROOT=%CD%
)
rem cd %OM_ROOT%

START "oms" /MIN bin\oms
START http://localhost:4040

