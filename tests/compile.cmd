@echo off
setlocal enabledelayedexpansion

echo.
echo ==================== COMPILING ====================
set "ENC_SRC="
set "DEC_SRC="
for /d %%D in ("src/coders/*") do (
    for %%F in ("src/coders/%%D/*_enc.cpp") do (
        set "ENC_SRC=src/coders/%%D/%%F !ENC_SRC!"
    )
    for %%F in ("src/coders/%%D/*_dec.cpp") do (
        set "DEC_SRC=src/coders/%%D/%%F !DEC_SRC!"
    )
)
g++ -O2 src/encoder.cpp !ENC_SRC! -o encoder.exe
g++ -O2 src/decoder.cpp !DEC_SRC! -o decoder.exe
echo Done.