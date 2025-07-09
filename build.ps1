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

function Install-Dependency {
    param(
        [string]$Name,
        [string]$WingetId
    )
    Write-Host "Checking for $Name..."
    if (-not (Test-CommandExists $Name)) {
        Write-Host "$Name not found. Installing with winget..."
        winget install --id $WingetId -e --accept-package-agreements --accept-source-agreements
        if (-not (Test-CommandExists $Name)) {
            Write-Error "$Name installation failed. Please install it manually."
            exit 1
        }
    } else {
        Write-Host "$Name is already installed."
    }
}

Write-Host "SCE Editor Build Script for Windows"
Write-Host "==================================="

Install-Dependency -Name "git" -WingetId "Git.Git"

$vsInstallerPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vs_installer.exe"
if (-not (Test-Path $vsInstallerPath)) {
    Write-Host "Visual Studio Build Tools not found. Installing..."
    winget install --id Microsoft.VisualStudio.2022.BuildTools --override "--wait --quiet --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64" -e
} else {
    Write-Host "Visual Studio Build Tools detected."
    
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsInstallation = & $vswhere -latest -property installationPath
        
        if ($vsInstallation) {
            Write-Host "Found Visual Studio installation at: $vsInstallation"
            
            $cmakeInstalled = & $vswhere -latest -requires "Microsoft.VisualStudio.Component.VC.CMake.Project" -property installationPath
            
            if ($cmakeInstalled) {
                Write-Host "CMake tools are already installed."
            } else {
                Write-Host "Installing CMake tools..."
                & $vsInstallerPath modify --installPath "$vsInstallation" --add Microsoft.VisualStudio.Component.VC.CMake.Project --quiet
            }
        } else {
            Write-Host "No Visual Studio installation found."
        }
    } else {
        Write-Host "vswhere not found. Cannot detect Visual Studio installation."
    }
}

Write-Host "Checking for Microsoft Visual C++ Redistributable..."
$vcRedistCheck = winget list --id Microsoft.VCRedist.2015+.x64 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Microsoft Visual C++ Redistributable not found. Installing latest version..."
    winget install --id Microsoft.VCRedist.2015+.x64 -e --accept-package-agreements --accept-source-agreements
} else {
    Write-Host "Microsoft Visual C++ Redistributable is installed."
}

if (-not (Test-Path $PDCursesDir)) {
    Write-Host "Downloading PDCursesMod..."
    $zipPath = "$($env:TEMP)\pdcurses.zip"
    Invoke-WebRequest -Uri $PDCursesUrl -OutFile $zipPath
    Write-Host "Extracting PDCursesMod..."
    Expand-Archive -Path $zipPath -DestinationPath $PSScriptRoot
    Rename-Item -Path "$($PSScriptRoot)/PDCursesMod-$($PDCursesVersion)" -NewName "PDCursesMod"
    Remove-Item $zipPath
} else {
    Write-Host "PDCursesMod directory already exists."
}

if (-not (Test-Path $direntDir)) {
    Write-Host "Downloading dirent..."
    $zipPath = "$($env:TEMP)\dirent.zip"
    Invoke-WebRequest -Uri $direntUrl -OutFile $zipPath
    Write-Host "Extracting dirent..."
    Expand-Archive -Path $zipPath -DestinationPath $PSScriptRoot
    Rename-Item -Path "$($PSScriptRoot)/dirent-$($direntVersion)" -NewName "dirent"
    Remove-Item $zipPath
} else {
    Write-Host "dirent directory already exists."
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

Write-Host "Installing SCE editor..."
cmake --build . --config Release --target INSTALL

Write-Host "Installation complete! You can find the executable in the installation directory."
Set-Location ".."