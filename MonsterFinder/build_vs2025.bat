@echo off
echo ====================================
echo Building MonsterFinder Enhanced
echo ====================================
echo.

REM Try to find Visual Studio 2025
set VS2025=

REM Check common VS2025 installation paths
if exist "C:\Program Files\Microsoft Visual Studio\2025\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set VS2025="C:\Program Files\Microsoft Visual Studio\2025\Community\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "C:\Program Files\Microsoft Visual Studio\2025\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set VS2025="C:\Program Files\Microsoft Visual Studio\2025\Professional\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "C:\Program Files\Microsoft Visual Studio\2025\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set VS2025="C:\Program Files\Microsoft Visual Studio\2025\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2025\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set VS2025="C:\Program Files (x86)\Microsoft Visual Studio\2025\Community\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2025\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set VS2025="C:\Program Files (x86)\Microsoft Visual Studio\2025\Professional\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2025\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set VS2025="C:\Program Files (x86)\Microsoft Visual Studio\2025\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

if not defined VS2025 (
    echo ERROR: Visual Studio 2025 not found!
    echo.
    echo Please install Visual Studio 2025 with C++ development tools.
    echo Or use Visual Studio IDE to open and build the project.
    pause
    exit /b 1
)

echo Found Visual Studio 2025
echo ====================================
echo.

REM Set up VS environment
call %VS2025%

echo.
echo ====================================
echo Compiling with MSVC...
echo ====================================
echo.

REM Compile Enhanced Version
echo Compiling MonsterFinderEnhanced.cpp...
cl /EHsc /std:c++17 /O2 /Fe:MonsterFinderEnhanced.exe MainEnhanced.cpp MonsterFinderEnhanced.cpp /link gdi32.lib user32.lib comctl32.lib /SUBSYSTEM:WINDOWS

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ====================================
    echo Build SUCCESSFUL!
    echo Executable: MonsterFinderEnhanced.exe
    echo ====================================
    echo.
    echo Don't forget to copy these files to run the bot:
    echo   - monsterfinder_config.ini
    echo   - monsters.ini
    echo.
) else (
    echo.
    echo ====================================
    echo Build FAILED!
    echo ====================================
    echo.
)

pause
