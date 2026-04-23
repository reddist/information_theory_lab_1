@echo off
setlocal enabledelayedexpansion
for /f %%a in ('echo prompt $E^| cmd') do set "ESC=%%a"
set "GREEN=!ESC![32m"
set "RED=!ESC![31m"
set "RESET=!ESC![0m"


cmd /c ".\scripts\compile.cmd"


echo.
echo ==================== ENCODING ====================
if not exist zip mkdir zip
for %%f in (dataset\*) do (
    echo Encoding %%~nxf...
    .\encoder dataset\%%~nxf zip\%%~nxf_zip
)


echo.
echo ==================== DECODING ====================
if not exist dec mkdir dec
for %%f in (dataset\*) do (
    echo Decoding %%~nxf...
    .\decoder zip\%%~nxf_zip dec\%%~nxf_dec
)


echo.
echo =================== COMPARING ====================
for %%f in (dataset\*) do (
    set "msg=Comparing dataset\%%~nxf dec\%%~nxf_dec"
    set "msg=!msg!............................................................"
    <nul set /p "=!msg:~0,65!"
    fc /b dataset\%%~nxf dec\%%~nxf_dec >nul 2>&1 && (echo !GREEN!PASSED!RESET!) || (echo !RED!FAILED!RESET!)
)
