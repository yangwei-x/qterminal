# QTerminal Makefile
# This Makefile provides convenient targets for building the project using CMake

# Variables
BUILD_DIR = build
CMAKE = cmake
MAKE = make

# Detect operating system
ifeq ($(OS),Windows_NT)
    # Windows
    NPROC = $(shell if exist "%NUMBER_OF_PROCESSORS%" (echo %NUMBER_OF_PROCESSORS%) else (echo 4))
    PLATFORM = Windows
    DEV_NULL = nul
    WHICH_CMD = where
    MKDIR = if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
    RM = rmdir /s /q
else
    # Unix-like (Linux, macOS, etc.)
    NPROC = $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    PLATFORM = Unix
    DEV_NULL = /dev/null
    WHICH_CMD = which
    MKDIR = mkdir -p $(BUILD_DIR)
    RM = rm -rf
endif

# Default target
.PHONY: all
all: build-project

# Help target
.PHONY: help
help:
	@echo "QTerminal Build System"
	@echo "======================"
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build the project (default)"
	@echo "  build        - Build the project"
	@echo "  configure    - Configure the project with CMake"
	@echo "  clean        - Clean build directory"
	@echo "  distclean    - Remove build directory completely"
	@echo "  install      - Install the project"
	@echo "  test         - Run tests"
	@echo "  debug        - Build debug version"
	@echo "  release      - Build release version"
	@echo "  rebuild      - Clean and build"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Build options:"
	@echo "  BUILD_TYPE   - Set to Debug or Release (default: Release)"
	@echo "  PREFIX       - Installation prefix (default: /usr/local)"
	@echo "  JOBS         - Number of parallel jobs (default: $(NPROC))"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build release version"
	@echo "  make debug              # Build debug version"
	@echo "  make BUILD_TYPE=Debug   # Build debug version"
	@echo "  make install PREFIX=/opt/qterminal"
	@echo "  make JOBS=4             # Use 4 parallel jobs"

# Build type (default to Release)
BUILD_TYPE ?= Release
PREFIX ?= /usr/local
JOBS ?= $(NPROC)

# Create build directory
$(BUILD_DIR):
	@echo "Creating build directory..."
ifeq ($(OS),Windows_NT)
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
else
	@mkdir -p $(BUILD_DIR)
endif

# Configure target
.PHONY: configure
configure: $(BUILD_DIR)
	@echo "Configuring project with CMake..."
	@cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(PREFIX) \
		..
	@echo "Configuration complete."

# Build target
.PHONY: build-project build
build-project: configure
	@echo "Building project..."
	@cd $(BUILD_DIR) && $(MAKE) -j$(JOBS)
	@echo "Build complete. Executable: $(BUILD_DIR)/qterminal"

# Alias for build-project
build: build-project

# Debug build
.PHONY: debug
debug:
	@$(MAKE) BUILD_TYPE=Debug build-project

# Release build
.PHONY: release
release:
	@$(MAKE) BUILD_TYPE=Release build-project

# Clean target
.PHONY: clean
clean:
	@if [ -d "$(BUILD_DIR)" ]; then \
		echo "Cleaning build directory..."; \
		cd $(BUILD_DIR) && $(MAKE) clean; \
	else \
		echo "Build directory does not exist."; \
	fi

# Dist clean target
.PHONY: distclean
distclean:
	@echo "Removing build directory..."
ifeq ($(OS),Windows_NT)
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
else
	@rm -rf $(BUILD_DIR)
endif
	@echo "Clean complete."

# Rebuild target
.PHONY: rebuild
rebuild: clean build-project

# Install target
.PHONY: install
install: build-project
	@echo "Installing project..."
	@cd $(BUILD_DIR) && $(MAKE) install
	@echo "Installation complete."

# Test target
.PHONY: test
test: build-project
	@echo "Running tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@echo "Tests complete."

# Run the application (for testing in environments with display)
.PHONY: run
run: build-project
	@echo "Running QTerminal..."
	@cd $(BUILD_DIR) && ./qterminal

# Package target (create installation package)
.PHONY: package
package: build-project
	@echo "Creating package..."
	@cd $(BUILD_DIR) && $(MAKE) package
	@echo "Package created."

# Development targets
.PHONY: dev-setup
dev-setup:
	@echo "Setting up development environment..."
ifeq ($(OS),Windows_NT)
	@echo "Windows development setup:"
	@echo "Please install the following manually:"
	@echo "1. Qt6 SDK from https://www.qt.io/download"
	@echo "2. CMake from https://cmake.org/download/"
	@echo "3. Visual Studio 2019/2022 or MinGW-w64"
	@echo "4. Git for Windows"
	@echo "5. Add Qt6, CMake to PATH environment variable"
	@echo "6. For full functionality, use Windows Subsystem for Linux (WSL)"
	@echo "   or install a compatible terminal library"
else
	@echo "Installing development dependencies..."
	@sudo apt update
	@sudo apt install -y qt6-base-dev qt6-base-dev-tools qt6-tools-dev \
		qt6-tools-dev-tools libqt6core6t64 libqt6gui6t64 libqt6widgets6t64 \
		libqt6dbus6t64 qt6-l10n-tools cmake build-essential git \
		libx11-dev pkg-config
endif
	@echo "Development environment setup complete."

# Check dependencies
.PHONY: check-deps
check-deps:
	@echo "Checking dependencies..."
	@command -v cmake >/dev/null 2>&1 || { echo "cmake is required but not installed."; exit 1; }
	@command -v make >/dev/null 2>&1 || { echo "make is required but not installed."; exit 1; }
	@command -v qmake6 >/dev/null 2>&1 || { echo "Qt6 development tools are required but not installed."; exit 1; }
	@echo "All dependencies are available."

# Show build information
.PHONY: info
info:
	@echo "QTerminal Build Information"
	@echo "=========================="
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Build type: $(BUILD_TYPE)"
	@echo "Install prefix: $(PREFIX)"
	@echo "Parallel jobs: $(JOBS)"
	@echo "CMake: $(shell which cmake 2>/dev/null || echo 'not found')"
	@echo "Make: $(shell which make 2>/dev/null || echo 'not found')"
	@echo "Qt6 qmake: $(shell which qmake6 2>/dev/null || echo 'not found')"
	@if [ -f "$(BUILD_DIR)/qterminal" ]; then \
		echo "Executable: $(BUILD_DIR)/qterminal ($(shell stat -c%s $(BUILD_DIR)/qterminal 2>/dev/null | numfmt --to=iec)B)"; \
	else \
		echo "Executable: not built"; \
	fi
	@echo ""

# Git submodule update
.PHONY: update-submodules
update-submodules:
	@echo "Updating git submodules..."
	@git submodule update --init --recursive
	@echo "Submodules updated."

# Lint/format targets (if tools are available)
.PHONY: format
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Formatting source code..."; \
		find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i; \
		echo "Code formatting complete."; \
	else \
		echo "clang-format not available, skipping code formatting."; \
	fi

# Documentation target
.PHONY: docs
docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		echo "Generating documentation..."; \
		doxygen Doxyfile; \
		echo "Documentation generated in docs/"; \
	else \
		echo "doxygen not available, skipping documentation generation."; \
	fi

# Show project status
.PHONY: status
status: info
	@echo "Git status:"
	@git status --short
	@echo ""
	@echo "Git submodule status:"
	@git submodule status
