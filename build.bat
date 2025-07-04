@echo off
setlocal enabledelayedexpansion

echo SCE Editor Build Script (Windows)
echo ===================================

:: Check if we're in a Windows environment
if not "%OS%"=="Windows_NT" (
    echo This script is designed for Windows only.
    exit /b 1
)

:: Check for required tools
echo Checking dependencies...

:: Check for CMake
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo CMake not found. Please install CMake and add it to your PATH.
    echo Download from: https://cmake.org/download/
    echo.
    echo Alternative: Install Visual Studio with C++ tools which includes CMake
    pause
    exit /b 1
)

:: Check for Visual Studio Build Tools or Visual Studio
where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo Visual Studio compiler (cl.exe) not found.
    echo Please install one of the following:
    echo - Visual Studio 2019 or later with C++ tools
    echo - Visual Studio Build Tools
    echo - Or run this script from a Visual Studio Developer Command Prompt
    echo.
    echo You can also try running: "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    echo (adjust path for your VS version)
    pause
    exit /b 1
)

:: Check for Git (optional but recommended)
where git >nul 2>&1
if %errorlevel% neq 0 (
    echo Warning: Git not found. Some features may not work properly.
    echo Consider installing Git from: https://git-scm.com/download/win
)

:: Clean up old build directory
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

:: Set up sceconfig directory
if not exist "%USERPROFILE%\.sceconfig" (
    echo Setting up sceconfig directory...
    mkdir "%USERPROFILE%\.sceconfig"
)

if exist "..\.sceconfig" (
    echo Moving sceconfig file into its directory...
    copy "..\.sceconfig" "%USERPROFILE%\.sceconfig\" >nul
)

:: Parse command line arguments
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

:: Try to detect Visual Studio version and architecture
if not defined CMAKE_GENERATOR (
    :: Try VS 2022 first
    if exist "C:\Program Files\Microsoft Visual Studio\2022" (
        set CMAKE_GENERATOR="Visual Studio 17 2022"
        set CMAKE_ARCH=-A x64
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019" (
        set CMAKE_GENERATOR="Visual Studio 16 2019"
        set CMAKE_ARCH=-A x64
    ) else (
        echo Using default generator
        set CMAKE_GENERATOR=
        set CMAKE_ARCH=
    )
)

echo Configuring project...
if defined CMAKE_GENERATOR (
    echo Using generator: %CMAKE_GENERATOR%
    cmake %BUILD_TEST_FLAG% %CMAKE_GENERATOR% %CMAKE_ARCH% ..
) else (
    cmake %BUILD_TEST_FLAG% ..
)

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

echo Building project...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Build failed!
    cd ..
    pause
    exit /b 1
)

:: Run tests if requested
if not "%BUILD_TEST_FLAG%"=="" (
    echo Running tests...
    ctest --output-on-failure -C Release
    
    if %NO_INSTALL% equ 1 (
        echo Tests completed. Skipping installation.
        cd ..
        pause
        exit /b 0
    )
)

:: Install (copy to a reasonable location)
echo Installing SCE editor...

:: Create installation directory
set INSTALL_DIR=%LOCALAPPDATA%\SCE
if not exist "%INSTALL_DIR%" (
    mkdir "%INSTALL_DIR%"
)

:: Copy executable
if exist "bin\Release\SCE.exe" (
    copy "bin\Release\SCE.exe" "%INSTALL_DIR%\" >nul
) else if exist "Release\SCE.exe" (
    copy "Release\SCE.exe" "%INSTALL_DIR%\" >nul
) else if exist "SCE.exe" (
    copy "SCE.exe" "%INSTALL_DIR%\" >nul
) else (
    echo Could not find SCE.exe to install
    cd ..
    pause
    exit /b 1
)

:: Add to PATH (user PATH, not system PATH)
echo Adding SCE to user PATH...
for /f "tokens=2*" %%a in ('reg query "HKCU\Environment" /v PATH 2^>nul') do set "USER_PATH=%%b"
if not defined USER_PATH set "USER_PATH="

:: Check if already in PATH
echo !USER_PATH! | find /i "%INSTALL_DIR%" >nul
if %errorlevel% neq 0 (
    if defined USER_PATH (
        reg add "HKCU\Environment" /v PATH /t REG_EXPAND_SZ /d "!USER_PATH!;%INSTALL_DIR%" /f >nul
    ) else (
        reg add "HKCU\Environment" /v PATH /t REG_EXPAND_SZ /d "%INSTALL_DIR%" /f >nul
    )
    echo SCE added to user PATH. You may need to restart your command prompt.
) else (
    echo SCE is already in PATH.
)

cd ..

echo.
echo Installation complete!
echo SCE has been installed to: %INSTALL_DIR%
echo You can now run SCE from the command line (after restarting your command prompt).
echo.
pause