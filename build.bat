@echo off
setlocal enabledelayedexpansion

echo SCE Editor Build Script (Windows)
echo ===================================

if not "%OS%"=="Windows_NT" (
    echo This script is designed for Windows only.
    exit /b 1
)

echo Checking dependencies...

where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo CMake not found. Please install CMake and add it to your PATH.
    echo Download from: https://cmake.org/download/
    echo.
    echo Alternative: Install Visual Studio with C++ tools which includes CMake
    pause
    exit /b 1
)

if exist "build" (
    echo Removing old build directory...
    rmdir /s /q "build" 2>nul
    if exist "build" (
        echo Warning: Could not remove old build directory completely
    )
)

echo Setting up build directory...
mkdir build
if not exist "build" (
    echo Failed to create build directory
    exit /b 1
)

cd build

if not exist "%USERPROFILE%\.sceconfig" (
    echo Setting up sceconfig directory...
    mkdir "%USERPROFILE%\.sceconfig"
)

if exist "..\.sceconfig" (
    echo Moving sceconfig file into its directory...
    copy "..\.sceconfig" "%USERPROFILE%\.sceconfig\" >nul
)

set BUILD_TEST_FLAG=
set NO_INSTALL=0

:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="--test" (
    set BUILD_TEST_FLAG=-DBUILD_TESTING=ON
    echo Testing mode enabled.
) else if /i "%~1"=="-t" (
    set BUILD_TEST_FLAG=-DBUILD_TESTING=ON
    echo Testing mode enabled.
) else if /i "%~1"=="--no-install" (
    set NO_INSTALL=1
)
shift
goto :parse_args

:done_parsing