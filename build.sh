#!/bin/bash

echo "SCE Editor Build Script"
echo "======================="

AUTO_YES=0
for arg in "$@"; do
    if [ "$arg" == "--yes" ] || [ "$arg" == "--ci" ]; then
        AUTO_YES=1
    fi
done

confirm() {
    local message="$1"
    local default="${2:-y}"

    if [ "$AUTO_YES" -eq 1 ]; then
        return 0
    fi
    
    local prompt
    if [ "$default" = "y" ]; then
        prompt="[Y/n]"
    else
        prompt="[y/N]"
    fi
    
    read -p "$message $prompt " response
    
    response=$(echo "$response" | tr '[:upper:]' '[:lower:]')
    
    if [ -z "$response" ]; then
        response=$default
    fi
    
    if [[ "$response" =~ ^(yes|y)$ ]]; then
        return 0
    else
        return 1
    fi
}

if grep -q -E "Microsoft|WSL" /proc/version 2>/dev/null || [ -n "$WSL_DISTRO_NAME" ] || [ -f /proc/sys/fs/binfmt_misc/WSLInterop ]; then
    echo "WSL environment detected."
    if confirm "Fix clock skew by updating file timestamps?"; then
        echo "Updating file timestamps to fix clock skew..."
        find . -type f -not -path "*/\.git/*" -exec touch {} \; 2>/dev/null || \
        sudo find . -type f -not -path "*/\.git/*" -exec touch {} \;
    fi
fi

detect_package_manager() {
    if command -v pacman &> /dev/null; then
        echo "pacman"
    elif command -v dnf &> /dev/null; then
        echo "dnf"
    elif command -v apt &> /dev/null; then
        echo "apt"
    else
        echo "unknown"
    fi
}

if [[ "$OSTYPE" == "darwin"* ]]; then    
    echo "Checking dependencies..."

    if ! command -v brew &> /dev/null; then
        echo "Homebrew is not installed. It's required to install dependencies."
        
        if confirm "Would you like to install Homebrew? (approximately 20-30MB download)"; then
            echo "Installing Homebrew..."
            /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
                
            if ! command -v brew &> /dev/null; then
                echo "Failed to install Homebrew. Please install it manually from https://brew.sh"
                exit 1
            fi
        else
            echo "Homebrew is required for this build script. Please install it manually from https://brew.sh"
            exit 1
        fi
    fi

    if ! command -v cmake &> /dev/null; then
        if confirm "CMake is required but not installed. Would you like to install it? (approximately 20MB)"; then
            echo "Installing cmake..."
            brew install cmake
        else
            echo "CMake is required for building this project. Please install it manually."
            exit 1
        fi
    fi

    if ! command -v pkg-config &> /dev/null; then
        if confirm "pkg-config is required but not installed. Would you like to install it? (approximately 2MB)"; then
            echo "Installing pkg-config..."
            brew install pkg-config
        else
            echo "pkg-config is required for building this project. Please install it manually."
            exit 1
        fi
    fi
    
    if ! pkg-config --exists ncurses; then
        if confirm "ncurses development library is required but not installed. Would you like to install it? (approximately 5MB)"; then
            echo "Installing ncurses development library..."
            brew install ncurses
        else
            echo "ncurses is required for building this project. Please install it manually."
            exit 1
        fi
    fi

    if ! pkg-config --exists check; then
        if confirm "Check unit testing framework is required but not installed. Would you like to install it? (approximately 3MB)"; then
            echo "Installing Check unit testing framework..."
            brew install check
        else
            echo "Check is required for running tests. Please install it manually if you want to run tests."
        fi
    fi
elif [[ "$OSTYPE" == "FreeBSD"* ]] || [[ "$OSTYPE" == "NetBSD"* ]]; then
    echo "Detected BSD system."
    echo "Checking dependencies..."

    if ! command -v pkg &> /dev/null; then
        echo "Error: pkg command not found. Please install dependencies manually:"
        echo "- cmake"
        echo "- pkgconf"
        echo "- ncurses"
        echo "- check"
        exit 1
    fi

    if ! command -v cmake &> /dev/null; then
        if confirm "CMake is required but not installed. Would you like to install it? (approximately 20MB)"; then
            echo "Installing cmake..."
            sudo pkg install -y cmake
        else
            echo "CMake is required for building this project. Please install it manually."
            exit 1
        fi
    fi

    if ! command -v pkg-config &> /dev/null; then
        if confirm "pkgconf is required but not installed. Would you like to install it? (approximately 2MB)"; then
            echo "Installing pkgconf..."
            sudo pkg install -y pkgconf
        else
            echo "pkgconf is required for building this project. Please install it manually."
            exit 1
        fi
    fi
    
    if ! pkg-config --exists ncurses; then
        if confirm "ncurses development library is required but not installed. Would you like to install it? (approximately 5MB)"; then
            echo "Installing ncurses development library..."
            sudo pkg install -y ncurses
        else
            echo "ncurses is required for building this project. Please install it manually."
            exit 1
        fi
    fi

    if ! pkg-config --exists check; then
        if confirm "Check unit testing framework is required but not installed. Would you like to install it? (approximately 3MB)"; then
            echo "Installing Check unit testing framework..."
            sudo pkg install -y check
        else
            echo "Check is required for running tests. Please install it manually if you want to run tests."
        fi
    fi
else
    PKG_MANAGER=$(detect_package_manager)
    echo "Using package manager: $PKG_MANAGER"
    
    case $PKG_MANAGER in
        "pacman")
            echo "Checking dependencies..."

            if command -v sudo &> /dev/null; then
                PRIV_CMD="sudo"
            else
                echo "sudo not found. Using su instead."
                PRIV_CMD="su -c"
                if confirm "Would you like to install sudo?"; then
                    su -c "pacman -S --noconfirm sudo"
                    PRIV_CMD="sudo"
                fi
            fi

            if ! command -v cmake &> /dev/null; then
                if confirm "CMake is required but not installed. Would you like to install it? (approximately 20MB)"; then
                    echo "Installing cmake..."
                    $PRIV_CMD pacman -S --noconfirm cmake
                else
                    echo "CMake is required for building this project. Please install it manually."
                    exit 1
                fi
            fi

            if ! command -v pkg-config &> /dev/null; then
                if confirm "pkg-config is required but not installed. Would you like to install it? (approximately 2MB)"; then
                    echo "Installing pkg-config..."
                    $PRIV_CMD pacman -S --noconfirm pkg-config
                else
                    echo "pkg-config is required for building this project. Please install it manually."
                    exit 1
                fi
            fi
            
            if ! pkg-config --exists ncurses; then
                if confirm "ncurses development library is required but not installed. Would you like to install it? (approximately 5MB)"; then
                    echo "Installing ncurses development library..."
                    $PRIV_CMD pacman -S --noconfirm ncurses
                else
                    echo "ncurses is required for building this project. Please install it manually."
                    exit 1
                fi
            fi

            if ! pkg-config --exists check; then
                if confirm "Check unit testing framework is required but not installed. Would you like to install it? (approximately 3MB)"; then
                    echo "Installing Check unit testing framework..."
                    $PRIV_CMD pacman -S --noconfirm check
                else
                    echo "Check is required for running tests. Please install it manually if you want to run tests."
                fi
            fi
            ;;
            
        "dnf")
            echo "Checking dependencies..."
            if ! command -v cmake &> /dev/null; then
                if confirm "CMake is required but not installed. Would you like to install it? (approximately 20MB)"; then
                    echo "Installing cmake..."
                    sudo dnf install -y cmake
                else
                    echo "CMake is required for building this project. Please install it manually."
                    exit 1
                fi
            fi

            if ! command -v pkg-config &> /dev/null; then
                if confirm "pkg-config is required but not installed. Would you like to install it? (approximately 2MB)"; then
                    echo "Installing pkg-config..."
                    sudo dnf install -y pkgconfig
                else
                    echo "pkg-config is required for building this project. Please install it manually."
                    exit 1
                fi
            fi
            
            if ! pkg-config --exists ncurses; then
                if confirm "ncurses development library is required but not installed. Would you like to install it? (approximately 5MB)"; then
                    echo "Installing ncurses development library..."
                    sudo dnf install -y ncurses-devel
                else
                    echo "ncurses is required for building this project. Please install it manually."
                    exit 1
                fi
            fi

            if ! pkg-config --exists check; then
                if confirm "Check unit testing framework is required but not installed. Would you like to install it? (approximately 3MB)"; then
                    echo "Installing Check unit testing framework..."
                    sudo dnf install -y check-devel
                else
                    echo "Check is required for running tests. Please install it manually if you want to run tests."
                fi
            fi
            ;;
            
        "apt")
            echo "Checking dependencies..."
            if ! command -v cmake &> /dev/null; then
                if confirm "CMake is required but not installed. Would you like to install it? (approximately 20MB)"; then
                    echo "Installing cmake..."
                    sudo apt install -y cmake
                else
                    echo "CMake is required for building this project. Please install it manually."
                    exit 1
                fi
            fi

            if ! command -v pkg-config &> /dev/null; then
                if confirm "pkg-config is required but not installed. Would you like to install it? (approximately 2MB)"; then
                    echo "Installing pkg-config..."
                    sudo apt install -y pkg-config
                else
                    echo "pkg-config is required for building this project. Please install it manually."
                    exit 1
                fi
            fi
            
            if ! pkg-config --exists ncurses; then
                if confirm "ncurses development library is required but not installed. Would you like to install it? (approximately 5MB)"; then
                    echo "Installing ncurses development library..."
                    sudo apt install -y libncurses-dev
                else
                    echo "ncurses is required for building this project. Please install it manually."
                    exit 1
                fi
            fi

            if ! pkg-config --exists check; then
                if confirm "Check unit testing framework is required but not installed. Would you like to install it? (approximately 3MB)"; then
                    echo "Installing Check unit testing framework..."
                    sudo apt install -y check
                else
                    echo "Check is required for running tests. Please install it manually if you want to run tests."
                fi
            fi
            ;;
            
        *)
            echo "Unsupported package manager. Please install these dependencies manually:"
            echo "- cmake"
            echo "- pkg-config"
            echo "- ncurses development library"
            exit 1
            ;;
    esac
fi

if confirm "Ready to build the SCE Editor. This will create or replace the build directory. Continue?"; then
    if [ -d "build" ]; then
        echo "Removing old build directory..."
        rm -rf build 2>/dev/null || sudo rm -rf build
        echo "Setting up new build directory..."
    else 
        echo "Setting up build directory..."
    fi

    mkdir -p build
    cd build

    if [ -d "$HOME/.sceconfig" ]; then 
        echo "sceconfig directory detected"
    else 
        if confirm "SCE config directory not found. Create it in your home directory?"; then
            echo "Setting up sceconfig directory..."
            mkdir -p "$HOME/.sceconfig"
        else
            echo "SCE config directory is required. Exiting."
            exit 1
        fi
    fi

    echo "Moving sceconfig file into its directory..."
    cp ../.sceconfig "$HOME/.sceconfig"

    BUILD_TEST_FLAG=""
    if [ "$1" == "--test" ] || [ "$1" == "-t" ]; then
        BUILD_TEST_FLAG="-DBUILD_TESTING=ON"
        echo "Testing mode enabled."
    fi

    echo "Configuring project..."
    cmake $BUILD_TEST_FLAG ..

    echo "Building project..."
    BUILD_OUTPUT=$(make 2>&1)
    BUILD_STATUS=$?

    if echo "$BUILD_OUTPUT" | grep -q "Clock skew detected" || echo "$BUILD_OUTPUT" | grep -q "modification time.*in the future"; then
        if confirm "Clock skew warnings detected. Fix timestamps?"; then
            echo "Fixing timestamps..."
            cd ..
            find . -type f -not -path "*/\.git/*" -exec touch {} \; 2>/dev/null || \
            sudo find . -type f -not -path "*/\.git/*" -exec touch {} \;
            cd build
            echo "Rebuilding after timestamp fix..."
            make
        fi
    fi

    if [ "$1" == "--test" ] || [ "$1" == "-t" ]; then
        echo "Running tests..."
        make test

        if [ "$2" == "--no-install" ]; then
            echo "Tests completed. Skipping installation."
            exit 0
        fi
    fi

    if confirm "Ready to install SCE editor. This will copy files to your system. Continue?"; then
        echo "Installing SCE editor..."
        sudo make install
        echo "Installation complete! You can now run SCE from the command line."
    else
        echo "Installation skipped by user."
    fi
else
    echo "Build cancelled by user."
    exit 0
fi


if [ "$1" == "--cleanup" ] || [ "$2" == "--cleanup" ]; then
    if confirm "Would you like to remove source files and build artifacts? This will keep only the installed application."; then
        echo "Cleaning up source files and build artifacts..."
        cd ..
        rm -rf build
        find . -type f -not -path "*/\.git/*" -not -path "*/\.sceconfig/*" \
            -not -name "LICENSE" -not -name "README.MD" -not -name "CHANGELOG.md" \
            -exec rm {} \; 2>/dev/null || \
        sudo find . -type f -not -path "*/\.git/*" -not -path "*/\.sceconfig/*" \
            -not -name "LICENSE" -not -name "README.MD" -not -name "CHANGELOG.md" \
            -exec rm {} \;
        echo "Cleanup complete. Only essential files remain."
    fi
fi