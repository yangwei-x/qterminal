# QTerminal Windows Support

This document describes the Windows support for QTerminal.

## Overview

QTerminal has been modified to support Windows platforms, though with some limitations due to the terminal emulation being primarily designed for Unix-like systems.

## Windows Compatibility Changes

### Main Application (`src/main.cpp`)

- Added Windows-specific includes and compatibility layer
- Implemented Windows version of `getopt_long` for command-line argument parsing
- Added conditional compilation for Windows-specific functions
- Replaced Unix-specific functions with Windows equivalents

### Build System

- Updated `Makefile` with Windows platform detection
- Added Windows-specific build instructions
- Created `cmake/WindowsConfig.cmake` for Windows-specific CMake configuration
- Added Windows compatibility headers in `src/win32/`

### Windows Compatibility Layer

A compatibility layer (`src/win32/windows_compat.h`) provides:

- Mapping of Unix functions to Windows equivalents
- Missing constants and types definitions
- Error handling compatibility
- Sleep function compatibility

## Limitations

### Core Terminal Functionality

The core terminal functionality (qtermwidget) relies heavily on Unix-specific features:

- **PTY (Pseudo Terminal)**: Windows doesn't have native PTY support
- **termios**: Terminal I/O control is Unix-specific
- **fork/exec**: Process creation model differs significantly
- **Signal handling**: Unix signal system is not available on Windows

### Alternative Solutions

For full Windows support, consider:

1. **Windows Subsystem for Linux (WSL)**: Run QTerminal in WSL environment
2. **ConPTY**: Use Windows Console Pseudo Terminal (Windows 10+)
3. **Third-party libraries**: Use libraries like winpty or ConPTY wrapper

## Build Instructions for Windows

### Prerequisites

1. Qt6 SDK (6.4.0 or later)
2. CMake (3.18.0 or later)
3. Visual Studio 2019/2022 or MinGW-w64
4. Git for Windows

### Building

```cmd
# Clone the repository
git clone https://github.com/lxqt/qterminal.git
cd qterminal

# Initialize submodules
git submodule update --init --recursive

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=C:/qterminal ..

# Build
cmake --build . --config Release

# Install (optional)
cmake --install . --config Release
```

### Using the Makefile

```cmd
# Setup development environment (manual installation required)
make dev-setup

# Build the project
make

# Install
make install
```

## Runtime Considerations

When running QTerminal on Windows:

1. **Limited Terminal Features**: Some advanced terminal features may not work
2. **Shell Integration**: Default shell will be Windows Command Prompt or PowerShell
3. **Path Handling**: Windows path separators and conventions apply
4. **Performance**: May be slower than native Windows terminal applications

## Development Notes

### Adding Windows Support

To extend Windows support:

1. Add conditional compilation blocks using `#ifdef _WIN32`
2. Implement Windows-specific versions of Unix functions
3. Use Qt's cross-platform APIs where possible
4. Test on actual Windows systems

### Known Issues

- PTY-related functionality will not work natively on Windows
- Some keyboard shortcuts may conflict with Windows conventions
- File permissions and user management work differently

## Recommended Alternatives

For native Windows terminal experience:

- **Windows Terminal**: Microsoft's modern terminal application
- **ConEmu**: Feature-rich terminal emulator for Windows
- **Mintty**: Lightweight terminal emulator (used by Git Bash)

## Contributing

When contributing Windows-specific code:

1. Use conditional compilation (`#ifdef _WIN32`)
2. Maintain compatibility with Unix systems
3. Document Windows-specific behavior
4. Test on multiple Windows versions
5. Consider using Qt's cross-platform APIs

## License

This Windows compatibility layer is provided under the same license as QTerminal (GPL-2.0).
