# Windows-specific CMake configuration for QTerminal
# This file provides Windows compatibility additions

# Define Windows-specific compiler flags
if(WIN32)
    # Add Windows-specific definitions
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7+
    add_definitions(-DWINVER=0x0601)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
    
    # Disable specific warnings for Windows
    if(MSVC)
        add_compile_options(/wd4996)  # Disable deprecated function warnings
        add_compile_options(/wd4244)  # Disable conversion warnings
        add_compile_options(/wd4267)  # Disable size_t conversion warnings
    endif()
    
    # Link Windows-specific libraries
    set(WINDOWS_LIBS
        user32
        kernel32
        gdi32
        winspool
        comdlg32
        advapi32
        shell32
        ole32
        oleaut32
        uuid
        odbc32
        odbccp32
    )
    
    # Define Windows-specific include directories
    set(WINDOWS_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/win32
    )
    
    # Create Windows compatibility headers directory if it doesn't exist
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/win32)
    
    message(STATUS "Windows build configuration enabled")
endif()
