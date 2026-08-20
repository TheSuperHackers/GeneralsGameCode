@echo off
setlocal
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Install-TECHBYSAKH.ps1" %*
if errorlevel 1 (
  echo.
  echo TECHBYSAKH installation did not complete. Read the error above.
  pause
  exit /b 1
)
echo.
echo Installation complete. Press any key to close.
pause >nul
endlocal
