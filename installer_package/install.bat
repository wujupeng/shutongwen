@echo off
setlocal enabledelayedexpansion

echo ================================================
echo ShuTongWen IME Installer
echo ================================================
echo.

:: Check if running as administrator
fltmc >nul 2>&1 || (
    echo ERROR: This installer must be run as Administrator!
    echo Please right-click and select "Run as Administrator"
    pause
    exit /b 1
)

set "INSTALL_DIR=%ProgramFiles%\ShuTongWen"
set "IME_DLL=ShuTongWenIME.dll"
set "REG_PATH=HKLM\SOFTWARE\Microsoft\CTF\TIP\{12345678-1234-1234-1234-123456789ABC}"

echo [1/4] Creating installation directory...
mkdir "%INSTALL_DIR%" 2>nul

echo [2/4] Copying IME files...
copy "%~dp0%IME_DLL%" "%INSTALL_DIR%\" /Y >nul
if %errorlevel% neq 0 (
    echo ERROR: Failed to copy IME files!
    pause
    exit /b 1
)

echo [3/4] Registering COM component...
regsvr32 /s "%INSTALL_DIR%\%IME_DLL%"
if %errorlevel% neq 0 (
    echo ERROR: Failed to register COM component!
    pause
    exit /b 1
)

echo [4/4] Creating registry entries...
reg add "%REG_PATH%" /v Description /t REG_SZ /d "ShuTongWen IME" /f >nul
reg add "%REG_PATH%" /v Language /t REG_DWORD /d 0x0804 /f >nul
reg add "%REG_PATH%" /v Profile /t REG_SZ /d "{12345678-1234-1234-1234-123456789ABC}" /f >nul
reg add "%REG_PATH%" /v ShowStatus /t REG_DWORD /d 1 /f >nul
reg add "%REG_PATH%" /v IconPath /t REG_SZ /d "%INSTALL_DIR%\%IME_DLL%,0" /f >nul

echo.
echo ================================================
echo INSTALLATION COMPLETE!
echo ================================================
echo.
echo To enable ShuTongWen IME:
echo 1. Open Settings -^> Time & Language -^> Language & Region
echo 2. Click "Add a language", search for "Chinese (China)"
echo 3. Click "Options", find "ShuTongWen IME" in the input methods list
echo 4. Use Win+Space to switch input methods
echo.
pause
