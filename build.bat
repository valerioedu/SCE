@echo off
setlocal

REM ============================================================================
REM  SCE Editor Build Script for Windows
REM ============================================================================
echo SCE Editor Build Script for Windows
echo ===================================

REM --- Check for Administrator privileges for installation ---
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Running with Administrator privileges.
) else (
    echo Administrator privileges are required for dependency and final installation.
    echo Please re-run this script as an Administrator.
    pause
    exit /b 1
)

REM --- Check for dependencies ---
echo Checking for dependencies...

REM Check for winget
where /q winget
if %errorLevel% neq 0 (
    echo winget is not installed or not in PATH.
    echo Please install the App Installer from the Microsoft Store:
    echo ms-windows-store://pdp/?productid=9NBLGGH4NNS1
    pause
    exit /b 1
)

REM Check for Git
where /q git
if %errorLevel% neq 0 (
    echo Git not found. Installing with winget...
    winget install --id Git.Git -e --source winget
) else (
    echo Git found.
)

REM Check for CMake
where /q cmake
if %errorLevel% neq 0 (
    echo CMake not found. Installing with winget...
    winget install --id Kitware.CMake -e --source winget
) else (
    echo CMake found.
)

REM Check for Visual Studio Build Tools
echo Checking for Visual Studio Build Tools (C/C++ compiler)...
call "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath > vs_path.tmp
set /p VS_PATH=<vs_path.tmp
del vs_path.tmp

if not defined VS_PATH (
    echo Visual Studio Build Tools not found.
    echo Please install "Desktop development with C++" from the Visual Studio Installer.
    echo You can download it here: https://visualstudio.microsoft.com/downloads/
    pause
    exit /b 1
)
echo Visual Studio Build Tools found at %VS_PATH%

REM --- Setup build directory ---
if exist "build" (
    echo Removing old build directory...
    rmdir /s /q build
)
echo Setting up new build directory...
mkdir build
cd build

REM --- Configure and Build ---
set BUILD_TEST_FLAG=""
if /i "%1" == "--test" set BUILD_TEST_FLAG=-DBUILD_TESTING=ON
if /i "%1" == "-t" set BUILD_TEST_FLAG=-DBUILD_TESTING=ON

if defined BUILD_TEST_FLAG (
    echo Testing mode enabled.
)

echo Configuring project with CMake...
cmake %BUILD_TEST_FLAG% ..

echo Building project...
cmake --build . --config Release
if %errorLevel% neq 0 (
    echo Build failed.
    exit /b 1
)

REM --- Run Tests if requested ---
if defined BUILD_TEST_FLAG (
    echo Running tests...
    ctest -C Release
)

REM --- Install ---
echo Installing SCE editor...
cmake --install . --config Release
if %errorLevel% neq 0 (
    echo Installation failed.
    exit /b 1
)

echo.
echo Installation complete!
echo You can find the executable in the installation directory.
echo Add it to your PATH to run 'SCE' from anywhere.

endlocal