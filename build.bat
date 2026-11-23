@echo off
setlocal
cd /d "%~dp0"
echo ========================================================
echo Building Wordle Prism (Desktop C Engine)
echo ========================================================

where gcc >nul 2>&1
if errorlevel 1 (
    if exist "C:\MinGW\bin\gcc.exe" (
        set "PATH=C:\MinGW\bin;%PATH%"
    ) else (
        echo [ERROR] GCC compiler not found in PATH or C:\MinGW\bin.
        exit /b 1
    )
)
where windres >nul 2>&1
if errorlevel 1 (
    echo [ERROR] windres tool not found in PATH or C:\MinGW\bin.
    exit /b 1
)

set "TARGET_ARCH="
for /f "delims=" %%A in ('gcc -dumpmachine 2^>nul') do set "TARGET_ARCH=%%A"
if "%TARGET_ARCH%"=="" (
    echo [ERROR] Failed to detect GCC target architecture via "gcc -dumpmachine".
    exit /b 1
)
set "RAYLIB_DIR=raylib-6.0_win32_mingw-w64"
echo(%TARGET_ARCH%| findstr /i "x86_64" >nul
if not errorlevel 1 set "RAYLIB_DIR=raylib-6.0_win64_mingw-w64"

set "CFLAGS=-O3 -Wall -std=c99 -Isrc -Iraylib/%RAYLIB_DIR%/include"
set "LDFLAGS=-Lraylib/%RAYLIB_DIR%/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lm -static -mwindows"

if not exist "raylib\%RAYLIB_DIR%\include\raylib.h" (
    echo [INFO] Raylib SDK not found locally. Downloading %RAYLIB_DIR%...
    if not exist "raylib" mkdir raylib
    curl.exe -fsSL "https://github.com/raysan5/raylib/releases/download/6.0/%RAYLIB_DIR%.zip" -o raylib.zip
    if errorlevel 1 (
        if exist "raylib.zip" del "raylib.zip"
        echo [ERROR] Failed to download Raylib SDK. Please place %RAYLIB_DIR% inside raylib\.
        exit /b 1
    )
    powershell -NoProfile -Command "Expand-Archive -Path 'raylib.zip' -DestinationPath 'raylib' -Force"
    if errorlevel 1 (
        if exist "raylib.zip" del "raylib.zip"
        echo [ERROR] Failed to extract raylib.zip.
        exit /b 1
    )
    del "raylib.zip"
    if not exist "raylib\%RAYLIB_DIR%\include\raylib.h" (
        echo [ERROR] Expected header raylib\%RAYLIB_DIR%\include\raylib.h not found after extraction.
        exit /b 1
    )
)

echo Compiling Windows icon resource...
windres -i src/resource.rc -o src/resource.o
if errorlevel 1 (
    echo [ERROR] Resource compilation failed!
    exit /b 1
)

echo Compiling C source files ^(%TARGET_ARCH%^)...
gcc %CFLAGS% src/main.c src/ui_board.c src/ui_glass.c src/solver.c src/motion.c src/audio_synth.c src/words.c src/shim.c src/resource.o %LDFLAGS% -o wordle_prism.exe
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed! Check compiler errors above.
    exit /b 1
)

echo.
echo ========================================================
echo Build Successful: wordle_prism.exe generated!
echo ========================================================
