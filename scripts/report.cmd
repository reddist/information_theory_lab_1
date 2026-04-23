@echo off
setlocal enabledelayedexpansion


echo.
echo ==================== COMPILING ====================
g++ -O2 src/report.cpp -o report.exe
echo Done.


echo.
echo ==================== REPORT ====================
for %%f in (dataset\*) do (
    echo %%~nxf
    .\report dataset\%%~nxf zip\%%~nxf_zip
    echo.
)
