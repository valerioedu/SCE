echo "SCE Editor Build Script"
echo "======================="

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

echo "Setting up build directory..."
mkdir -p build
cd build

echo "Configuring project..."
cmake ..

echo "Building project..."
make

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