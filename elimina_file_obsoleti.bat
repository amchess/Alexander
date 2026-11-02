@echo off
REM Alexander 9: elimina i file della 8.3 che non esistono piu'.
REM Da lanciare nella cartella principale del repository (quella che contiene src\).
cd /d "%~dp0"
if not exist src\ (
    echo cartella src non trovata: lancia lo script dalla radice del repository
    pause
    exit /b 1
)
REM Rinominati in Stockfish 19: shm_linux.h -^> shm_unix.h, thread_win32_osx.h -^> thread_native.h
REM Script per singola architettura e GoldDigger sostituiti da universal.bat / universal.sh
for %%F in (shm_linux.h thread_win32_osx.h makeAVX2.bat makeAVX512.bat makeAll.sh makeAll32.bat makeAll64.bat makeBMI2.bat makeBMI2GD.bat native.bat native.sh nativeGD.bat nativeGD.sh) do (
    if exist "src\%%F" (
        del /q "src\%%F"
        echo eliminato src\%%F
    )
)
echo fatto
pause
