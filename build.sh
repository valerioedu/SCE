#!/bin/bash

echo "SCE Editor Build Script"
echo "======================="

if grep -q -E "Microsoft|WSL" /proc/version 2>/dev/null || [ -n "$WSL_DISTRO_NAME" ] || [ -f /proc/sys/fs/binfmt_misc/WSLInterop ]; then
    echo "Updating file timestamps to fix clock skew..."
    find . -type f -not -path "*/\.git/*" -exec touch {} \; 2>/dev/null || \
    sudo find . -type f -not -path "*/\.git/*" -exec touch {} \;
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
        
        echo "Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
            
        if ! command -v brew &> /dev/null; then
            echo "Failed to install Homebrew. Please install it manually from https://brew.sh"
            exit 1
        fi
    fi

    if ! command -v cmake &> /dev/null; then
        echo "Installing cmake..."
        brew install cmake
    fi

    if ! command -v pkg-config &> /dev/null; then
        echo "Installing pkg-config..."
        brew install pkg-config
    fi
    
    if ! pkg-config --exists ncurses; then
        echo "Installing ncurses development library..."
        brew install ncurses
    fi

    if ! pkg-config --exists check; then
        echo "Installing Check unit testing framework..."
        brew install check
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
                echo "Would you like to install sudo? (y/n)"
                read -r install_sudo
                if [[ "$install_sudo" == "y" || "$install_sudo" == "Y" ]]; then
                    su -c "pacman -S --noconfirm sudo"
                    PRIV_CMD="sudo"
                fi
            fi
            
            if ! command -v cmake &> /dev/null; then
                echo "Installing cmake..."
                sudo pacman -S --noconfirm cmake
            fi

            if ! command -v pkg-config &> /dev/null; then
                echo "Installing pkg-config..."
                sudo pacman -S --noconfirm pkg-config
            fi
            
            if ! pkg-config --exists ncurses; then
                echo "Installing ncurses development library..."
                sudo pacman -S --noconfirm ncurses
            fi

            if ! pkg-config --exists check; then
                echo "Installing Check unit testing framework..."
                sudo pacman -S --noconfirm check
            fi
            ;;
            
        "dnf")
            echo "Checking dependencies..."
            if ! command -v cmake &> /dev/null; then
                echo "Installing cmake..."
                sudo dnf install -y cmake
            fi

            if ! command -v pkg-config &> /dev/null; then
                echo "Installing pkg-config..."
                sudo dnf install -y pkgconfig
            fi
            
            if ! pkg-config --exists ncurses; then
                echo "Installing ncurses development library..."
                sudo dnf install -y ncurses-devel
            fi

            if ! pkg-config --exists check; then
                echo "Installing Check unit testing framework..."
                sudo dnf install -y check-devel
            fi
            ;;
            
        "apt")
            echo "Checking dependencies..."
            if ! command -v cmake &> /dev/null; then
                echo "Installing cmake..."
                sudo apt install -y cmake
            fi

            if ! command -v pkg-config &> /dev/null; then
                echo "Installing pkg-config..."
                sudo apt install -y pkg-config
            fi
            
            if ! pkg-config --exists ncurses; then
                echo "Installing ncurses development library..."
                sudo apt install -y libncurses-dev
            fi

            if ! pkg-config --exists check; then
                echo "Installing Check unit testing framework..."
                sudo apt install -y check
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

if [ -d "build" ]; then
    echo "Removing old build directory..."
    rm -rf build
    echo "Setting up new build directory..."
else 
    echo "Setting up build directory..."
fi

mkdir -p build
cd build

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
    echo "Clock skew warnings detected! Fixing timestamps..."
    cd ..
    find . -type f -not -path "*/\.git/*" -exec touch {} \; 2>/dev/null || \
    sudo find . -type f -not -path "*/\.git/*" -exec touch {} \;
    cd build
    echo "Rebuilding after timestamp fix..."
    make
fi

if [ "$1" == "--test" ] || [ "$1" == "-t" ]; then
    echo "Running tests..."
    make test

    if [ "$2" == "--no-install" ]; then
        echo "Tests completed. Skipping installation."
        exit 0
    fi
fi

echo "Installing SCE editor..."
sudo make install

echo "Installation complete! You can now run SCE from the command line."
