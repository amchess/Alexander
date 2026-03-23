@echo off
REM x64 builds begin (GoldDigger Edition)
SET PATH=C:\tools\msys64\mingw64\bin;C:\tools\msys64\usr\bin;%PATH%

echo Inizio compilazione Alexander GoldDigger Edition (Native)...
mingw32-make clean
mingw32-make profile-build ARCH=native COMP=mingw ENV_CXXFLAGS="-DGOLD_DIGGER -DNNUE_EMBEDDING_OFF" -j %Number_Of_Processors%

if not exist alexander.exe (
    echo Errore: Compilazione fallita!
    pause
    exit /b
)

strip alexander.exe
ren alexander.exe Alexander8.2-native-GoldDigger.exe
mingw32-make clean

echo.
echo Compilazione completata con successo!
pause