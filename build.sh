echo "SCE Editor Build Script"
echo "======================="

if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Detected macOS system"
    
    echo "Checking dependencies..."
    if ! command -v cmake &> /dev/null; then
        echo "Installing cmake..."
        brew install cmake
    fi

    if ! command -v pkg-config &> /dev/null; then
        echo "Installing pkg-config..."
        brew install pkg-config
    fi
    
    brew install ncurses
else    
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
fi

echo "Setting up build directory..."
mkdir -p build
cd build

echo "Configuring project..."
cmake ..

echo "Building project..."
make

echo "Installing SCE editor..."
sudo make install

echo "Installation complete! You can now run SCE from the command line."