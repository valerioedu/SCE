# SCE - Simple Code Editor

SCE, pronounced as C, is a lightweight, terminal-based code editor built with ncurses. It's designed to provide a straightforward yet powerful editing experience with syntax highlighting, file browsing, and basic IDE features.

## Features

- Syntax highlighting for C/C++
- File browser with directory navigation
- Variable detection and highlighting
- Undo/redo functionality
- Search and replace
- Terminal integration
- Memory-efficient design
- Responsive UI with status bar
- Configuration Editor

## Installation

### Dependencies

- ncurses
- cmake
- pkg-config
- C compiler

### Build from source
```bash
# Clone the repository
git clone https://github.com/yourusername/SCE2.git
cd SCE2

# Build using the provided script
chmod +x build.sh
./build.sh
```

The build script will handle the installation of dependencies, compilation, and system installation.

## Usage

```bash
# Open SCE in the current directory
SCE

# Open a specific file
SCE path/to/file.c

# Open a specific directory
SCE path/to/directory

# Show help
SCE -h

# Show version information
SCE -v
```

## Key Bindings

- F1: Help screen
- F2: Info page
- F3: File explorer
- F4: Save file
- F9: Open console
- Ctrl+F: Search within file
- Ctrl+Z: Undo
- Esc: Exit editor
- Tab: Insert tab

## Console Commands

- term: open a shell instance
- config: configure your options
- search: equivalent to cTRL+F
- goto <Lines> <Cols>: move to (Lines, Cols) position

## File Explorer

- ↑/↓: Navigate through files and directories
- Enter: Open file or enter directory
- Backspace: Go up one directory level
- Esc: Return to editor
- F1: Create new directory
- F2: Create new file
- F3: Open selected file
- H: Show help

## Search

- Enter: Search for text
- ↑/↓: Navigate between matches
- X: Replace the current match
- Esc: Cancel search

## Configuration

SCE is fully customizable through its built-in configuration editor:

- Press F5 to access the configuration menu
- Navigate options with Up/Down arrows
- Edit values with Enter
- Save changes with F10

### Configurable Options:

- **Tab Size**: Number of spaces for tab indentation
- **Parentheses Autocompletion**: Toggle automatic closing of parentheses
- **Quotes Autocompletion**: Toggle automatic closing of quotation marks
- **Color Schemes**: Customize syntax highlighting colors:
  - Comments
  - Types
  - Control flow keywords
  - Variables
  - Functions
  - Numbers
  - Parentheses
  - Strings
  - Typedefs
  - Escape sequences
- **Default Path**: Set your preferred starting directory

All settings are stored in the `.sceconfig` file in your home directory.

## Memory Usage

SCE is optimized for memory efficiency, with dynamic allocation used for the undo history system to minimize memory footprint while maintaining functionality.

## License

SCE is licensed under the GNU General Public License v3.0 (GPL-3.0). See the LICENSE file for details.

## Contributing

Contributions are welcome! Feel free to submit pull requests or open issues to improve SCE. For any clarification such as more documentation, reach me out creating an issue!