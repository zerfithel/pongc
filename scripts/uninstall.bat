@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

SET APPDIR=%APPDATA%\pongc
SET BINDIR=%APPDIR%\bin
SET SHAREDIR=%APPDIR%\share
SET SCRIPTDIR=%APPDIR%\scripts

echo [*] Deleting pongc executable...
if exist "%BINDIR%\pongc.exe" (
    del /Q "%BINDIR%\pongc.exe"
)

echo [*] Deleting shared files...
if exist "%SHAREDIR%\*" (
    del /Q "%SHAREDIR%\*"
)

echo [*] Deleting scripts...
if exist "%SCRIPTDIR%\*" (
    del /Q "%SCRIPTDIR%\*"
)

echo [*] Removing directories...
if exist "%BINDIR%" rmdir /S /Q "%BINDIR%"
if exist "%SHAREDIR%" rmdir /S /Q "%SHAREDIR%"
if exist "%SCRIPTDIR%" rmdir /S /Q "%SCRIPTDIR%"

echo [*] Removing main application directory if empty...
if exist "%APPDIR%" rmdir "%APPDIR%" 2>nul

echo [*] Uninstallation complete. Remove .lnk file from desktop
pause
ENDLOCAL
