# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.0] - 2025-07-09
### Added
- Windows support using PDCursesMod
- Windows-specific build script (build.ps1) with dependency management
- Support for dirent on Windows for file system operations
- Compatibility with Visual Studio build tools and MSVC compiler
- Windows-specific key bindings and terminal handling

### Changed
- Improved build scripts with user confirmation for all installations
- Updated color handling to be cross-platform compatible
- Modified file path handling for cross-platform compatibility

### Known Limitations
- Git integration features are work-in-progress on Windows
- Color customization has limitations on Windows due to PDCurses constraints
- Some terminal commands may behave differently on Windows

## [1.0.0] - 2025-07-06
### Added
- Initial release of SCE.
- C/C++ syntax highlighting.
- File browser with file and directory creation.
- Built-in configuration editor for customizing colors, tabs, and behavior.
- Git integration with status bar info and status window (F7).
- Support for multiple cursors and bookmarks.
- Undo/redo, search/replace, and automatic indentation.
- Integrated console for running shell commands.
- Command-line support for opening specific files or directories.
- Automatic saving feature.
- Cross-platform compatibility for Linux and macOS.
- Build script (`build.sh`) for easy dependency checking and installation.