@echo off
REM FreshVoxel Engine - Visual Studio 2022 Solution Generator
REM This script generates the Visual Studio 2022 solution using CMake
REM For detailed build instructions, see BUILD.md

setlocal enabledelayedexpansion

REM Setup logging
set "LOG_DIR=%~dp0logs"
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"

REM Create timestamp for log file
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value 2^>nul') do set "dt=%%a"
if not defined dt (
    REM Fallback if wmic not available
    set "dt=%date:~10,4%%date:~4,2%%date:~7,2%_%time:~0,2%%time:~3,2%%time:~6,2%"
    set "dt=!dt: =0!"
) else (
    set "dt=%dt:~0,8%_%dt:~8,6%"
)
set "LOG_FILE=%LOG_DIR%\generate_vs2022_%dt%.log"

REM Helper function to log both to console and file
call :log "================================================"
call :log "FreshVoxel Engine - Visual Studio 2022 Setup"
call :log "================================================"
call :log ""
call :log "[INFO] Setup started at %date% %time%"
call :log "[INFO] Log file: %LOG_FILE%"
call :log ""

REM Check if CMake is installed
call :log "[STEP] Checking for CMake..."
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    call :log "[ERROR] CMake is not found in PATH"
    call :log ""
    call :log "Please install CMake 3.20 or higher from:"
    call :log "https://cmake.org/download/"
    call :log ""
    call :log "Make sure to add CMake to system PATH during installation."
    pause
    exit /b 1
)

for /f "tokens=*" %%i in ('cmake --version 2^>^&1 ^| findstr /C:"cmake version"') do set "CMAKE_VERSION=%%i"
call :log "[OK] %CMAKE_VERSION%"
call :log ""

REM Create build directory
if not exist "build" (
    call :log "[STEP] Creating build directory..."
    mkdir build
    call :log "[OK] Build directory created"
) else (
    call :log "[INFO] Build directory already exists"
)
call :log ""

cd build

REM Check for vcpkg (project directory first, then parent directory)
call :log "[STEP] Checking for vcpkg..."
set "VCPKG_ROOT="
set "VCPKG_LOCAL=%~dp0vcpkg"
set "VCPKG_PARENT=%~dp0..\vcpkg"

if exist "%VCPKG_LOCAL%\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_ROOT=%VCPKG_LOCAL%"
    call :log "[OK] Found vcpkg in project directory"
    call :log "     Path: %VCPKG_LOCAL%"
) else if exist "%VCPKG_PARENT%\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_ROOT=%VCPKG_PARENT%"
    call :log "[OK] Found vcpkg in parent directory"
    call :log "     Path: %VCPKG_PARENT%"
) else (
    call :log "[ERROR] vcpkg not found"
    call :log ""
    call :log "vcpkg is required for automatic dependency management."
    call :log ""
    call :log "To set up vcpkg from the project root directory:"
    call :log "  1. Clone vcpkg:    git clone https://github.com/microsoft/vcpkg.git"
    call :log "  2. Enter vcpkg:    cd vcpkg"
    call :log "  3. Bootstrap it:   bootstrap-vcpkg.bat"
    call :log "  4. Return to root: cd .."
    call :log "  5. Generate VS:    generate_vs2022.bat"
    call :log ""
    call :log "Or see BUILD.md for detailed setup instructions."
    call :log ""
    pause
    cd ..
    exit /b 1
)
call :log ""

REM Generate Visual Studio 2022 solution with vcpkg
call :log "================================================"
call :log "Generating Visual Studio 2022 solution..."
call :log "================================================"
call :log ""
call :log "[INFO] Configuration: Windows x64 with DirectX 11/12"
call :log "[INFO] CMake Generator: Visual Studio 17 2022"
call :log "[INFO] Architecture: x64"
call :log "[INFO] vcpkg toolchain: %VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
call :log ""
call :log "[INFO] Dependencies will be automatically installed via vcpkg."
call :log "[INFO] This may take 5-15 minutes on first run."
call :log ""
call :log "[STEP] Running CMake configuration..."
call :log ""

REM Create unique temp file in logs directory
set "TEMP_LOG=%LOG_DIR%\cmake_output_%dt%.tmp"

REM Run cmake and log output
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" .. > "%TEMP_LOG%" 2>&1

REM Append cmake output to log and display to console
type "%TEMP_LOG%"
type "%TEMP_LOG%" >> "%LOG_FILE%"
del "%TEMP_LOG%"

if %ERRORLEVEL% NEQ 0 (
    call :log ""
    call :log "[ERROR] CMake generation failed with exit code %ERRORLEVEL%"
    call :log ""
    call :log "Please check the error messages above."
    call :log "Full log saved to: %LOG_FILE%"
    call :log "See BUILD.md for troubleshooting help."
    call :log ""
    call :log "[INFO] Setup completed at %date% %time%"
    cd ..
    pause
    exit /b 1
)

call :log ""
call :log "================================================"
call :log "SUCCESS! Visual Studio 2022 solution generated"
call :log "================================================"
call :log ""
call :log "[OK] Solution file: build\FreshVoxelEngine.sln"
call :log "[OK] Full log saved to: %LOG_FILE%"
call :log ""
call :log "Next steps:"
call :log "  1. Open build\FreshVoxelEngine.sln in Visual Studio 2022"
call :log "  2. In Solution Explorer, right-click 'FreshVoxelEngine' project"
call :log "  3. Select 'Set as Startup Project' (project name will appear bold)"
call :log "  4. Press F7 to build the solution"
call :log "  5. Press F5 to build and run with debugging"
call :log "     Or Ctrl+F5 to run without debugging"
call :log ""
call :log "Or build from command line:"
call :log "  cmake --build build --config Release"
call :log ""
call :log "For detailed instructions, see BUILD.md"
call :log ""
call :log "[INFO] Setup completed successfully at %date% %time%"
call :log ""
call :log "================================================"
call :log "Press any key to exit..."
call :log "================================================"
cd ..
pause
exit /b 0

REM Function to log both to console and file
:log
set "msg=%~1"
echo %msg%
echo %msg% >> "%LOG_FILE%"
goto :eof
