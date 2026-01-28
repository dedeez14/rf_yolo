# Build Instructions - RF Hunter Enhanced v2.0

Panduan lengkap untuk mem-build dan menjalankan MonsterFinder Enhanced.

## 📋 Prerequisites

### Software yang Dibutuhkan:

1. **Windows OS**
   - Windows 7 atau yang lebih baru
   - Direkomendasikan Windows 10/11

2. **C++ Compiler** (pilih salah satu):
   - **Visual Studio 2019+** (Recommended)
   - **Visual Studio Build Tools**
   - **MinGW-w64**
   - **MSYS2**

3. **RF Online Game**
   - Game harus terinstall
   - Window title harus "RF Online"

## 🔨 Method 1: Build dengan Visual Studio 2019/2022 (Recommended)

### Step 1: Install Visual Studio
1. Download Visual Studio dari: https://visualstudio.microsoft.com/downloads/
2. Install dengan workload:
   - ✅ Desktop development with C++
   - ✅ Windows 10/11 SDK

### Step 2: Buat Project Baru
1. Buka Visual Studio
2. Klik **Create a new project**
3. Cari **Windows Desktop Application** atau **Empty Project**
4. Pilih dan klik **Next**

### Step 3: Configure Project
1. Project name: `MonsterFinderEnhanced`
2. Location: Pilih folder project
3. Klik **Create**

### Step 4: Add Files ke Project
1. Di Solution Explorer, right-click pada **Source Files**
2. Add → Existing Item → Pilih:
   - `MainEnhanced.cpp`
   - `MonsterFinderEnhanced.cpp`
3. Right-click pada **Header Files**
4. Add → Existing Item → Pilih:
   - `MonsterFinderEnhanced.h`

### Step 5: Configure Project Properties
1. Right-click pada project → Properties
2. Set C++ Language Standard:
   - Configuration Properties → C/C++ → Language
   - C++ Language Standard: **ISO C++17 Standard (/std:c++17)** atau lebih baru

3. Set Character Set:
   - Configuration Properties → Advanced
   - Character Set: **Use Multi-Byte Character Set**

### Step 6: Add Library Dependencies
1. Configuration Properties → Linker → Input
2. Additional Dependencies → Add:
   ```
   gdi32.lib
   user32.lib
   comctl32.lib
   ```

### Step 7: Build Project
1. Pilih configuration: **Release** atau **Debug**
2. Pilih platform: **x64** atau **x86** (sesuaikan dengan game)
3. Tekan **F7** atau klik **Build → Build Solution**

### Step 8: Run Executable
1. Output akan ada di: `x64/Release/` atau `x86/Release/`
2. Copy executable ke folder dengan config files:
   - `monsterfinder_config.ini`
   - `monsters.ini`

3. Double-click executable untuk menjalankan

## 🔨 Method 2: Build dengan Visual Studio Build Tools

### Step 1: Install Build Tools
1. Download Visual Studio Build Tools: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
2. Install dengan:
   - ✅ C++ build tools
   - ✅ Windows 10/11 SDK

### Step 2: Open Developer Command Prompt
1. Buka **Developer Command Prompt for VS 2022** dari Start Menu
2. Navigate ke folder project:
   ```cmd
   cd C:\projects\rf\helper\MonsterFinder
   ```

### Step 3: Compile dengan cl.exe
```cmd
cl /EHsc /std:c++17 /Fe:MonsterFinderEnhanced.exe MainEnhanced.cpp MonsterFinderEnhanced.cpp /link gdi32.lib user32.lib comctl32.lib
```

### Step 4: Run
```cmd
MonsterFinderEnhanced.exe
```

## 🔨 Method 3: Build dengan MinGW-w64

### Step 1: Install MinGW-w64
1. Download dari: https://www.mingw-w64.org/downloads/
2. Atau gunakan MSYS2: https://www.msys2.org/
3. Pastikan MinGW ada di PATH

### Step 2: Verify Installation
```cmd
g++ --version
```
Harus menampilkan versi g++

### Step 3: Compile dengan g++
```cmd
g++ -std=c++17 -O2 -o MonsterFinderEnhanced.exe MainEnhanced.cpp MonsterFinderEnhanced.cpp -lgdi32 -luser32 -lcomctl32 -static-libgcc -static-libstdc++
```

Options:
- `-std=c++17`: C++17 standard
- `-O2`: Optimization level 2 (good balance)
- `-o`: Output filename
- `-lgdi32 -luser32 -lcomctl32`: Link Windows libraries
- `-static-libgcc -static-libstdc++`: Static link (no dependency on DLLs)

### Step 4: Run
```cmd
MonsterFinderEnhanced.exe
```

## 🔨 Method 4: Build dengan MSYS2

### Step 1: Install MSYS2
1. Download dari: https://www.msys2.org/
2. Install dan follow wizard
3. Open MSYS2 MinGW 64-bit terminal

### Step 2: Update Packages
```bash
pacman -Syu
```

### Step 3: Install Build Tools
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make
```

### Step 4: Compile
```bash
g++ -std=c++17 -O2 -o MonsterFinderEnhanced.exe MainEnhanced.cpp MonsterFinderEnhanced.cpp -lgdi32 -luser32 -lcomctl32 -static
```

### Step 5: Run
```bash
./MonsterFinderEnhanced.exe
```

## 🔨 Method 5: Create Batch Build Script

Buat file `build.bat`:

```batch
@echo off
echo ====================================
echo Building MonsterFinder Enhanced
echo ====================================
echo.

REM Check if g++ exists
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using MinGW g++ compiler...
    g++ --version
    echo.
    echo Compiling...
    g++ -std=c++17 -O2 -o MonsterFinderEnhanced.exe MainEnhanced.cpp MonsterFinderEnhanced.cpp -lgdi32 -luser32 -lcomctl32 -static-libgcc -static-libstdc++
) else (
    REM Check if cl.exe exists
    where cl >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        echo Using MSVC cl.exe compiler...
        cl /EHsc /std:c++17 /Fe:MonsterFinderEnhanced.exe MainEnhanced.cpp MonsterFinderEnhanced.cpp /link gdi32.lib user32.lib comctl32.lib
    ) else (
        echo ERROR: No C++ compiler found!
        echo Please install Visual Studio or MinGW-w64
        pause
        exit /b 1
    )
)

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ====================================
    echo Build SUCCESSFUL!
    echo Executable: MonsterFinderEnhanced.exe
    echo ====================================
) else (
    echo.
    echo ====================================
    echo Build FAILED!
    echo ====================================
)

pause
```

Jalankan dengan double-click `build.bat`

## 🔨 Method 6: Build dengan CMake (Advanced)

### Step 1: Install CMake
Download dari: https://cmake.org/download/

### Step 2: Create CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.15)
project(MonsterFinderEnhanced)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    MainEnhanced.cpp
    MonsterFinderEnhanced.cpp
)

# Headers
set(HEADERS
    MonsterFinderEnhanced.h
)

# Windows-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        gdi32
        user32
        comctl32
    )
endif()

# Create executable
add_executable(MonsterFinderEnhanced ${SOURCES} ${HEADERS})

# Set subsystem to Windows
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
```

### Step 3: Build dengan CMake
```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## ✅ Verification Steps

Setelah build berhasil:

1. **Check executable exists:**
   ```cmd
   dir MonsterFinderEnhanced.exe
   ```

2. **Check dependencies:**
   ```cmd
   dumpbin /dependents MonsterFinderEnhanced.exe
   ```
   atau
   ```cmd
   ldd MonsterFinderEnhanced.exe  # jika ada ldd
   ```

3. **Test run:**
   ```cmd
   MonsterFinderEnhanced.exe
   ```

4. **Verify config files:**
   Pastikan file-file ini ada di folder yang sama:
   - `monsterfinder_config.ini`
   - `monsters.ini`

## 🐛 Common Build Errors & Solutions

### Error 1: 'gdi32.h' not found
**Solution:**
- Install Windows SDK via Visual Studio Installer
- atau check that `Windows.h` is included correctly

### Error 2: LINK : fatal error LNK1104: cannot open file 'gdi32.lib'
**Solution:**
- Add library to Linker → Input → Additional Dependencies
- Check architecture (x86 vs x64)

### Error 3: undefined reference to `CreateWindowEx`
**Solution:**
- Link with `user32.lib` and `gdi32.lib`
- Add: `-lgdi32 -luser32` for MinGW

### Error 4: fatal error C1010: unexpected end of file while looking for precompiled header
**Solution:**
- Disable precompiled headers:
  - Project Properties → C/C++ → Precompiled Headers
  - Not Using Precompiled Headers

### Error 5: 'thread' is not a member of 'std'
**Solution:**
- Use `/std:c++17` or newer
- Ensure threading support is enabled

### Error 6: MSVCRT.lib or MSVCPxx.dll missing
**Solution:**
- Use `-static-libgcc -static-libstdc++` for MinGW
- Or install Visual C++ Redistributable

## 📦 Deployment

### Creating Portable Version:

1. **Copy semua file yang dibutuhkan:**
   ```
   MonsterFinderEnhanced.exe
   monsterfinder_config.ini
   monsters.ini
   README_ENHANCED.md
   ```

2. **Optional: Create logs folder:**
   ```cmd
   mkdir logs
   ```

3. **Test pada clean machine:**
   - Copy ke computer lain tanpa development tools
   - Verify executable runs

### Creating Installer (NSIS):

Create `installer.nsi`:

```nsis
!define APPNAME "RF Hunter Enhanced"
!define COMPANYNAME "MonsterFinder"
!define DESCRIPTION "Advanced RF Online Auto-Hunting Bot"
!define HELPURL "https://github.com/yourrepo"
!define UPDATEURL "https://github.com/yourrepo"
!define ABOUTURL "https://github.com/yourrepo"
!define INSTALLSIZE 5000

RequestExecutionLevel admin

InstallDir "$PROGRAMFILES\${APPNAME}"

Page directory
Page instfiles

Section "install"
    SetOutPath $INSTDIR
    File "MonsterFinderEnhanced.exe"
    File "monsterfinder_config.ini"
    File "monsters.ini"
    File "README_ENHANCED.md"
    
    WriteUninstaller "$INSTDIR\uninstall.exe"
    
    CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\MonsterFinderEnhanced.exe"
    CreateDirectory "$SMPROGRAMS\${APPNAME}"
    CreateShortCut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\MonsterFinderEnhanced.exe"
SectionEnd

Section "uninstall"
    Delete $INSTDIR\MonsterFinderEnhanced.exe
    Delete $INSTDIR\monsterfinder_config.ini
    Delete $INSTDIR\monsters.ini
    Delete $INSTDIR\README_ENHANCED.md
    Delete $INSTDIR\uninstall.exe
    RMDir $INSTDIR
    
    Delete "$DESKTOP\${APPNAME}.lnk"
    RMDir "$SMPROGRAMS\${APPNAME}"
SectionEnd
```

Build with NSIS compiler.

## 🚀 Performance Optimization

### Release vs Debug:
- **Debug**: Large file, slow, includes debug symbols
- **Release**: Small file, fast, optimized

### Compiler Optimization Flags:

**For MSVC:**
```cmd
/O2 /GL /Oi /Ot
```

**For MinGW:**
```cmd
-O3 -march=native -flto
```

### Size Optimization:

**MSVC:**
```cmd
/Os /GL
```

**MinGW:**
```cmd
-Os -s
```
`-s` strips symbols, reducing file size

## 📝 Build Checklist

Sebelum menjalankan bot:

- [ ] Compiler terinstall dan di-PATH
- [ ] C++17 atau lebih baru ter-support
- [ ] Windows SDK terinstall (untuk MSVC)
- [ ] Source files sudah di-copy
- [ ] Config files tersedia
- [ ] Build success (no errors)
- [ ] Executable dapat dijalankan
- [ ] RF Online window terdeteksi
- [ ] Monster colors sudah di-set
- [ ] Test run successful

## 🔗 Additional Resources

### Documentation:
- [MSVC Compiler Options](https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options)
- [MinGW Documentation](https://www.mingw-w64.org/)
- [Windows API Reference](https://docs.microsoft.com/en-us/windows/win32/api/)

### Tools:
- [Dependencies Walker](https://github.com/lucasg/Dependencies) - Check DLL dependencies
- [Resource Hacker](http://www.angusj.com/resourcehacker/) - Edit resources
- [PE Explorer](https://www.pe-explorer.com/) - PE file analysis

## ❓ Getting Help

Jika mengalami masalah build:

1. Check error message carefully
2. Verify all prerequisites are installed
3. Check architecture (x86 vs x64)
4. Review compiler version compatibility
5. Try alternative build method

---

**Good luck with your build! 🛠️**
