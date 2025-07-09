#Requires -RunAsAdministrator

$ProjectName = "SCE Editor"
$PDCursesVersion = "4.5.2"
$direntVersion = "1.25"
$PDCursesUrl = "https://github.com/Bill-Gray/PDCursesMod/archive/refs/tags/v4.5.2.zip"
$direntUrl = "https://github.com/tronkko/dirent/archive/refs/tags/1.25.zip"
$PDCursesDir = "$($PSScriptRoot)/PDCursesMod"
$direntDir = "$($PSScriptRoot)/dirent"

function Test-CommandExists {
    param($command)
    return (Get-Command $command -ErrorAction SilentlyContinue)
}

function Get-UserConfirmation {
    param(
        [string]$Message,
        [switch]$AllowSkip
    )
    
    $choices = if ($AllowSkip) { "&Yes", "&No", "&Skip" } else { "&Yes", "&No" }
    $defaultChoice = 0
    
    $decision = $Host.UI.PromptForChoice("Confirmation Required", $Message, $choices, $defaultChoice)
    
    if ($AllowSkip) {
        switch ($decision) {
            0 { return $true }
            1 { return $false }
            2 { return "Skip" }
        }
    } else {
        return $decision -eq 0
    }
}

function Install-Dependency {
    param(
        [string]$Name,
        [string]$WingetId
    )
    Write-Host "Checking for $Name..."
    if (-not (Test-CommandExists $Name)) {
        $confirm = Get-UserConfirmation -Message "$Name not found. Do you want to install it? (approximately 100-300MB download)"
        if ($confirm) {
            Write-Host "Installing $Name with winget..."
            winget install --id $WingetId -e --accept-package-agreements --accept-source-agreements
            if (-not (Test-CommandExists $Name)) {
                Write-Error "$Name installation failed. Please install it manually."
                exit 1
            }
        } else {
            Write-Error "$Name is required but not installed. Please install it manually."
            exit 1
        }
    } else {
        Write-Host "$Name is already installed."
    }
}

Write-Host "SCE Editor Build Script for Windows"
Write-Host "==================================="

Install-Dependency -Name "git" -WingetId "Git.Git"

Write-Host "Checking for MSVC compiler and CMake..."

$vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsInstallPath = $null

if (Test-Path $vswherePath) {
    $vsInstallPath = & $vswherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
}

if ($vsInstallPath) {
    Write-Host "Visual Studio installation found at: $vsInstallPath"
    
    $vcvarsallPath = Join-Path $vsInstallPath "VC\Auxiliary\Build\vcvarsall.bat"
    if (Test-Path $vcvarsallPath) {
        Write-Host "Initializing Visual Studio build environment..."
        $tempFile = [System.IO.Path]::GetTempFileName()
        cmd /c "call `"$vcvarsallPath`" x64 && set > `"$tempFile`""
        
        Get-Content $tempFile | ForEach-Object {
            if ($_ -match "^(.*?)=(.*)$") {
                $name = $matches[1]
                $value = $matches[2]
                Set-Item -Path "env:$name" -Value $value
            }
        }
        Remove-Item $tempFile
        
        $clPath = Get-Command "cl" -ErrorAction SilentlyContinue
        if ($clPath) {
            Write-Host "MSVC compiler (cl.exe) is now available at: $($clPath.Source)"
        } else {
            Write-Error "Failed to initialize MSVC compiler. Please check your Visual Studio installation."
            exit 1
        }
    } else {
        Write-Error "Visual Studio C++ tools not found. Please install them through Visual Studio Installer."
        exit 1
    }
} else {
    Write-Host "MSVC compiler not found in PATH."
    $confirm = Get-UserConfirmation -Message "Visual Studio Build Tools are required but not installed. Would you like to install them? (This is a large download, approximately 1.5GB)"
    
    if ($confirm) {
        Write-Host "Installing Visual Studio Build Tools..."
        winget install --id Microsoft.VisualStudio.2022.BuildTools --override "--wait --quiet --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.VC.CMake.Project" -e

        Start-Sleep -Seconds 5
        $vsInstallPath = & $vswherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        
        if ($vsInstallPath) {
            $vcvarsallPath = Join-Path $vsInstallPath "VC\Auxiliary\Build\vcvarsall.bat"
            if (Test-Path $vcvarsallPath) {
                Write-Host "Initializing Visual Studio build environment..."
                $tempFile = [System.IO.Path]::GetTempFileName()
                cmd /c "call `"$vcvarsallPath`" x64 && set > `"$tempFile`""
                
                Get-Content $tempFile | ForEach-Object {
                    if ($_ -match "^(.*?)=(.*)$") {
                        $name = $matches[1]
                        $value = $matches[2]
                        Set-Item -Path "env:$name" -Value $value
                    }
                }
                Remove-Item $tempFile
                
                $clPath = Get-Command "cl" -ErrorAction SilentlyContinue
                if ($clPath) {
                    Write-Host "MSVC compiler (cl.exe) is now available at: $($clPath.Source)"
                } else {
                    Write-Error "Failed to initialize MSVC compiler. Please check your Visual Studio installation."
                    exit 1
                }
            }
        } else {
            Write-Error "Visual Studio Build Tools installation failed or couldn't be detected."
            Write-Error "Please install manually from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022"
            exit 1
        }
    } else {
        Write-Error "Visual Studio Build Tools are required for building this project."
        Write-Error "Please install manually from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022"
        exit 1
    }
}

$cmakePath = Get-Command "cmake" -ErrorAction SilentlyContinue
if (-not $cmakePath) {
    $confirm = Get-UserConfirmation -Message "CMake is required but not found. Would you like to install it? (approximately 30MB download)"
    
    if ($confirm) {
        Write-Host "Installing CMake..."
        winget install --id Kitware.CMake -e
        
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
        
        $cmakePath = Get-Command "cmake" -ErrorAction SilentlyContinue
        if (-not $cmakePath) {
            Write-Error "CMake installation failed. Please install manually."
            exit 1
        }
    } else {
        Write-Error "CMake is required for building this project. Please install manually."
        exit 1
    }
}

Write-Host "Checking for Microsoft Visual C++ Redistributable..."
$vcRedistCheck = winget list --id Microsoft.VCRedist.2015+.x64 2>$null
if ($LASTEXITCODE -ne 0) {
    $confirm = Get-UserConfirmation -Message "Microsoft Visual C++ Redistributable is required but not found. Would you like to install it? (approximately 25MB download)"
    
    if ($confirm) {
        Write-Host "Installing Microsoft Visual C++ Redistributable..."
        winget install --id Microsoft.VCRedist.2015+.x64 -e --accept-package-agreements --accept-source-agreements
    } else {
        Write-Error "Microsoft Visual C++ Redistributable is required for running this application."
        exit 1
    }
} else {
    Write-Host "Microsoft Visual C++ Redistributable is installed."
}

if (-not (Test-Path $PDCursesDir)) {
    $confirm = Get-UserConfirmation -Message "PDCursesMod library needs to be downloaded. Would you like to download it now? (approximately 2MB)"
    
    if ($confirm) {
        Write-Host "Downloading PDCursesMod..."
        $zipPath = "$($env:TEMP)\pdcurses.zip"
        Invoke-WebRequest -Uri $PDCursesUrl -OutFile $zipPath
        Write-Host "Extracting PDCursesMod..."
        Expand-Archive -Path $zipPath -DestinationPath $PSScriptRoot
        Rename-Item -Path "$($PSScriptRoot)/PDCursesMod-$($PDCursesVersion)" -NewName "PDCursesMod"
        Remove-Item $zipPath
    } else {
        Write-Error "PDCursesMod is required for building this project."
        exit 1
    }
} else {
    Write-Host "PDCursesMod directory already exists."
}

if (-not (Test-Path $direntDir)) {
    $confirm = Get-UserConfirmation -Message "dirent library needs to be downloaded. Would you like to download it now? (approximately 1MB)"
    
    if ($confirm) {
        Write-Host "Downloading dirent..."
        $zipPath = "$($env:TEMP)\dirent.zip"
        Invoke-WebRequest -Uri $direntUrl -OutFile $zipPath
        Write-Host "Extracting dirent..."
        Expand-Archive -Path $zipPath -DestinationPath $PSScriptRoot
        Rename-Item -Path "$($PSScriptRoot)/dirent-$($direntVersion)" -NewName "dirent"
        Remove-Item $zipPath
    } else {
        Write-Error "dirent is required for building this project."
        exit 1
    }
} else {
    Write-Host "dirent directory already exists."
}

$confirm = Get-UserConfirmation -Message "Ready to build the SCE Editor. This will create or replace the build directory. Continue?"
if (-not $confirm) {
    Write-Host "Build cancelled by user."
    exit 0
}

if (Test-Path "build") {
    Write-Host "Removing old build directory..."
    Remove-Item -Recurse -Force "build"
}
Write-Host "Creating build directory..."
New-Item -ItemType Directory -Force "build" | Out-Null
Set-Location "build"

$configDir = "$HOME/.sceconfig"
if (-not (Test-Path $configDir)) {
    Write-Host "Creating .sceconfig directory..."
    New-Item -ItemType Directory -Force $configDir | Out-Null
}
Write-Host "Copying .sceconfig file..."
Copy-Item -Path "../.sceconfig" -Destination $configDir -Force

$cmakeArgs = @("..", "-A", "x64", "-DPDCURSES_DIR=$($PDCursesDir)")
if ($args -contains "--test" -or $args -contains "-t") {
    $cmakeArgs += "-DBUILD_TESTING=ON"
    Write-Host "Test mode enabled."
}

Write-Host "Configuring project with CMake..."
cmake $cmakeArgs

Write-Host "Building project..."
cmake --build . --config Release

if ($args -contains "--test" -or $args -contains "-t") {
    Write-Host "Running tests..."
    ctest --output-on-failure
}

$confirm = Get-UserConfirmation -Message "Ready to install SCE Editor. This will copy files to your system. Continue?"
if ($confirm) {
    Write-Host "Installing SCE editor..."
    cmake --build . --config Release --target INSTALL
    Write-Host "Installation complete! You can find the executable in the installation directory."
} else {
    Write-Host "Installation skipped by user."
}

Set-Location ".."