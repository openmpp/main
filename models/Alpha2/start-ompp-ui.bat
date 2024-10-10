IF "%MODEL_FULL_DIR%" == "" set "MODEL_FULL_DIR=%~dp0"

IF "%OM_ROOT%" == "" (
  "%~dp0..\start-model-ui.bat"
) ELSE (
  "%OM_ROOT%\models\start-model-ui.bat"
)
