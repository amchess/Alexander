@echo off
setlocal enableextensions enabledelayedexpansion

REM ==========================================================================
REM  universal.bat  -  Alexander 9  -  binario UNIVERSAL x86-64 (MinGW/MSYS2)
REM  Da lanciare DENTRO la cartella "src" del sorgente.
REM
REM  ARCH=x86-64-universal compila 8 slice
REM  (x86-64, sse41-popcnt, avx2, bmi2, avxvnni, avx512, vnni512, avx512icl)
REM  e le linka in un unico .exe che sceglie a runtime quella giusta.
REM
REM  Alexander ha la valutazione classica: nessuna rete NNUE da incorporare
REM  o da distribuire insieme all'exe.
REM ==========================================================================

REM --- toolchain: identica al batch nativo ----------------------------------
SET PATH=C:\tools\msys64\mingw64\bin;C:\tools\msys64\usr\bin;%PATH%

REM --- Intel SDE: serve SOLO per il PGO -------------------------------------
REM Il PGO esegue il bench di ogni slice: senza SDE le slice AVX-512 non
REM girano su una CPU che non le supporta, quindi senza SDE niente PGO.
SET "SDE_DIR=C:\tools\sde"

set "TARGET=build"
set "RUN_PREFIX="
if exist "%SDE_DIR%\sde.exe" (
    set "SDE_FWD=%SDE_DIR:\=/%"
    set "RUN_PREFIX=!SDE_FWD!/sde.exe -future --"
    set "TARGET=profile-build"
    echo [universal] Intel SDE trovato: build CON PGO ^(profile-build^)
) else (
    echo [universal] Intel SDE assente in "%SDE_DIR%": build SENZA PGO ^(build^)
)

mingw32-make clean

REM Non passare CXXFLAGS da riga di comando: sostituisce -O3, -m... e -flto.
if defined RUN_PREFIX (
    mingw32-make %TARGET% ARCH=x86-64-universal COMP=mingw RUN_PREFIX="!RUN_PREFIX!" -j %NUMBER_OF_PROCESSORS%
) else (
    mingw32-make %TARGET% ARCH=x86-64-universal COMP=mingw -j %NUMBER_OF_PROCESSORS%
)
if errorlevel 1 goto :fallita
if not exist alexander.exe goto :fallita

strip alexander.exe
REM move /y invece di ren: ren fallisce se l'exe esiste gia' e il clean finale
REM cancellerebbe quello nuovo, lasciando il binario della build precedente.
move /y alexander.exe Alexander9-universal.exe >nul
if errorlevel 1 goto :fallita

REM --- verifica: il bench deve girare con la slice scelta per questa CPU -----
set "CHECKDIR=%TEMP%\alexander_universal_check"
if exist "%CHECKDIR%" rmdir /s /q "%CHECKDIR%"
mkdir "%CHECKDIR%"
copy /y Alexander9-universal.exe "%CHECKDIR%\Alexander9-universal.exe" >nul
pushd "%CHECKDIR%"
Alexander9-universal.exe bench 16 1 8 >nul 2>&1
set "RC=%errorlevel%"
popd
rmdir /s /q "%CHECKDIR%"
if not "%RC%"=="0" (
    echo *** ERRORE: il bench dell'exe universale non termina correttamente ***
    goto :fallita
)
echo [universal] OK: bench eseguito

REM --- verifica: stampa la slice selezionata sulla CPU corrente -------------
Alexander9-universal.exe compiler

mingw32-make clean
echo.
echo [universal] OK: Alexander9-universal.exe
pause
exit /b 0

:fallita
echo.
echo *** BUILD UNIVERSAL FALLITA ***
pause
exit /b 1
