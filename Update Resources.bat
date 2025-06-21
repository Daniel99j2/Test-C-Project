@echo off
setlocal

set "SOURCE_DIR=%~dp0src\resources"
set "DEST_DIR=%~dp0cmake-build-debug\src\resources"

if not exist "%DEST_DIR%" (
    mkdir "%DEST_DIR%"
)

xcopy "%SOURCE_DIR%\*" "%DEST_DIR%\" /E /D /Y /I

endlocal
