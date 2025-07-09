# SCE - Simple Code Editor

SCE, pronounced C, is a lightweight, terminal-based code editor built with ncurses, designed for young C developers who want to approach CLI editors for the first time. It's designed to provide a straightforward yet powerful editing experience with syntax highlighting, file browsing, and basic IDE features.

![General Example](screenshots/Screenshot%202025-06-08%20115935.png)

## Features

- Syntax highlighting for C/C++
- File browser with directory navigation
- Variable and type definition detection and highlighting
- Automatic indentation
- Undo/redo functionality
- Search and replace
- Terminal integration
- Memory-efficient design
- Responsive UI with status bar
- Automatic window resizing
- Configuration Editor
- Git integration
- Cross-platform (Linux, macOS, Windows and BSD)
- Multiple cursors
- Multiple bookmarks
- Automatic saving

## Installation

### Dependencies

- ncurses (Unix Systems) or PDCursesMod (Windows)
- cmake
- pkg-config
- C compiler
- git (optional, for Git integration features)
- Homebrew for macOS
- MSVC and Visual studio cmake for Windows, which need to be installed manually

But don't worry, the build script will check and install them for you!

### Homebrew (for macOS and Linux)

You can install SCE using Homebrew.

```bash
# First, tap the repository
brew tap valerioedu/sce

# Now, install SCE
brew install sce
```

### Build from source

#### Unix Systems

```bash
# Clone the repository
git clone -b v1.1.0 https://github.com/valerioedu/SCE
cd SCE

# Build using the provided script
chmod +x build.sh
./build.sh              #or sudo ./build.sh
```

#### Windows

```bash
# Clone the repository
git clone -b v1.1.0 https://github.com/valerioedu/SCE
cd SCE

# Build using the provided script (requires administrator privileges)
./build.ps1
```

The build script will handle the installation of dependencies, compilation, and system installation.

## Windows Support

SCE now supports Windows through PDCursesMod. Please note the following Windows-specific limitations:

- Git Integration: Git features are currently a work in progress on Windows
- Color Customization: Custom colors are limited on Windows due to PDCurses limitations
- Terminal Commands: Some terminal commands may behave differently on Windows

These features are being actively developed and will be improved in future releases.

## Usage

```bash
# Open SCE in the current directory
SCE

# Generates a new file in memory if not present in path
SCE file.c

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
- F4 / Ctrl+S: Save file
- F7: Git status window
- F9: Open console
- Ctrl+F: Search within file
- Ctrl+Z: Undo
- Ctrl+R: Save current cursor
- Ctrl+B: Clear saved cursors
- Ctrl+E: Edit saved cursors/current cursor
- Ctrl+Left Arrow: Move cursor to the beginning of the previous word
- Ctrl+Right Arrow: Move cursor to the beginning of the next word
- Ctrl+Up Arrow: Move to previous cursor
- Ctrl+Down Arrow: Move to next cursor

- Alt+Right Arrow: Goes to the end of the line
- Alt+Left Arrow:  Goes to the start of the line
- Esc / Ctrl+Q: Exit editor
- Tab: Insert tab

## Multiple Cursors and Bookmarks

SCE supports working with multiple cursors:

- Ctrl+R: Save the current cursor position
- Ctrl+B: Clears all the saved cursors
- Ctrl+Up Arrow: Navigate to the nearest saved cursor position above
- Ctrl+Down Arrow: Navigate to the nearest saved cursor position below
- Ctrl+E: Switch between editing all the curors or just the current one

Multiple cursors allow you to quickly jump between different positions in your file.

## Console Commands

- term: open a shell instance
- config: configure your options
- search: equivalent to CTRL+F
- exit/quit: exits the program
- load: loads last automatic save
- goto \<Lines\> \<Cols\>: move to (Lines, Cols) position
- goto end: move to the end of the file

![Goto Example](screenshots/Screenshot%202025-06-08%20114142.png)

## File Explorer

- ↑/↓: Navigate through files and directories
- Enter: Open file or enter directory
- Backspace: Go up one directory level
- Esc: Return to editor
- F1: Create new directory
- F2: Create new file
- F3: Open selected file
- H: Show help

![File Explorer Example](screenshots/Screenshot%202025-06-08%20114105.png)

## Git Integration

When working in a Git repository, SCE provides:

- Status bar information showing repository name, current branch and git user
- Git status window (F7) to view pending changes and commit history
- Automatic detection of Git repositories

![Git Example](screenshots/Screenshot%202025-06-08%20114123.png)

## Search

- Enter: Search for text
- ↑/↓: Navigate between matches
- X: Replace the current match
- Esc: Cancel search

![Search Example](screenshots/Screenshot%202025-06-08%20114201.png)

## Configuration

SCE is fully customizable through its built-in configuration editor:

- Navigate options with Up/Down arrows
- Edit values with Enter
- Save changes with F10

![Config Example](screenshots/Screenshot%202025-06-08%20114223.png)

## .sceconfig Directory:

The shell script will automatically create a .sceconfig directory in your home environment which will store your configuration file (.sceconfig), and your latest automatic save (save.sce).

### Configurable Options:

- **Tab Size**: Number of spaces for tab indentation
- **Parentheses Autocompletion**: Toggle automatic closing of parentheses
- **Quotes Autocompletion**: Toggle automatic closing of quotation marks
- **Automatic Saving**: Toggle automatic saving
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

SCE is optimized for memory efficiency, with dynamic allocation used for the undo history system to minimize memory footprint while maintaining functionality. Soon a benchmark directory will be implemented.

```bash
# Memory usage with a 700-line C file open
924211 valerio+  20   0    3104   2048   1664 S   0.0   0.0   0:00.05 SCE

# Memory usage with a 2,000-line file open
924416 valerio+  20   0    3764   2816   1792 S   0.0   0.0   0:00.11 SCE
```

With only ~2 MB of resident memory used even while editing substantial source files (~700 lines of code), SCE is suitable for resource-constrained environments while maintaining full functionality. For reference, without any plugins and without any file open: 

- Nano Virt: 4468   Res: 3328  Shr: 2176
- Vim  Virt: 17580  Res: 9344  Shr: 5760

## System Requirements

### Compatible Operating Systems
- Linux distributions (Debian, Ubuntu, Kali Linux, Fedora, Arch)
- macOS 10.13 or later
- Windows 10/11 with PowerShell

## Aknowledgements
SCE would not be possible without these valuable libraries:

- [PDCursesMod](https://github.com/Bill-Gray/PDCursesMod) by Bill Gray - A wonderful curses implementation for Windows
- [dirent](https://github.com/tronkko/dirent) by Toni Ronkko - An exceptional port for the dirent header

## License

SCE is licensed under the GNU General Public License v3.0 (GPL-3.0). See the LICENSE file for details.

## Contributing

Contributions are welcome! Feel free to submit pull requests or open issues to improve SCE. For any clarification such as more documentation, reach me out creating an issue!
