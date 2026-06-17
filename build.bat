@echo off
echo ========================================================
echo Building Wordle Prism (Desktop C Engine)
echo ========================================================

set CC=C:\MinGW\bin\gcc.exe
set CFLAGS=-O3 -Wall -std=c99 -Isrc -Iraylib/raylib-6.0_win32_mingw-w64/include
set LDFLAGS=-Lraylib/raylib-6.0_win32_mingw-w64/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lm -mwindows

echo Compiling C source files...
%CC% %CFLAGS% src/main.c src/ui_board.c src/ui_glass.c src/solver.c src/motion.c src/audio_synth.c src/words.c src/shim.c %LDFLAGS% -o wordle_prism.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================================
    echo Build Successful: wordle_prism.exe generated!
    echo ========================================================
) else (
    echo.
    echo [ERROR] Build failed! Check compiler errors above.
)
