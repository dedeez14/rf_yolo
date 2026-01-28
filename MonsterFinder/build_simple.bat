@echo off
echo ====================================
echo Building MonsterFinder Enhanced
echo ====================================
echo.

REM Set up Visual Studio 2022 (version 18) environment
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

echo.
echo ====================================
echo Compiling with MSVC...
echo ====================================
echo.

REM Compile Enhanced Version with static linking
echo Compiling MonsterFinderEnhanced.cpp...
cl /EHsc /std:c++17 /O2 /MT MainEnhanced.cpp MonsterFinderEnhanced.cpp /Fe:MonsterFinderEnhanced.exe /link gdi32.lib user32.lib comctl32.lib /SUBSYSTEM:CONSOLE

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
    
    REM Check if config files exist
    if not exist monsterfinder_config.ini (
        echo WARNING: monsterfinder_config.ini not found!
    )
    if not exist monsters.ini (
        echo WARNING: monsters.ini not found!
    )
) else (
    echo.
    echo ====================================
    echo Build FAILED!
    echo ====================================
    echo.
    echo Please check error messages above.
)

pause
