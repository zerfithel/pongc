@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

SET APPDIR=%APPDATA%\pongc
SET BINDIR=%APPDIR%\bin
SET SHAREDIR=%APPDIR%\share
SET SCRIPTDIR=%APPDIR%\scripts
SET DESKTOP=%USERPROFILE%\Desktop

mkdir "%BINDIR%" >nul 2>&1
mkdir "%SHAREDIR%" >nul 2>&1
mkdir "%SCRIPTDIR%" >nul 2>&1

move /Y "build\pongc.exe" "%BINDIR%\pongc.exe"
for %%f in (docs\*) do (
  copy /Y "%%f" "%SHAREDIR%"
)

copy /Y "LICENSE.txt" "%SHAREDIR%\LICENSE.txt"

copy /Y "uninstall.bat" "%SCRIPTDIR%\uninstall.bat"

powershell -Command ^
"$s=(New-Object -COM WScript.Shell).CreateShortcut('%DESKTOP%\pongc.lnk'); ^
$s.TargetPath='%BINDIR%\pongc.exe'; ^
$s.IconLocation='%BINDIR%\pongc.exe'; ^
$s.Save()"

ENDLOCAL
pause
