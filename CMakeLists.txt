cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#-------------------------------------------------------------------------------
# Project Metadata
#-------------------------------------------------------------------------------
project(KraftaEditor
    VERSION 1.0.0
    DESCRIPTION "A fast, lightweight, and cross-platform code editor built with C++ and wxWidgets"
    LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/apozinn/kraftaEditor"
)

#-------------------------------------------------------------------------------
# Compiler Configuration
#-------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "_d")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#-------------------------------------------------------------------------------
# Build Options
#-------------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build as shared libraries" OFF)
option(USE_SYSTEM_WXWIDGETS "Use system wxWidgets installation" OFF)
option(ENABLE_TESTS "Build test suite" OFF)
option(ENABLE_RELEASE_DEBUG_INFO "Include debug info in release builds" ON)
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

#-------------------------------------------------------------------------------
# Output Directories
#-------------------------------------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

#-------------------------------------------------------------------------------
# Dependency Management
#-------------------------------------------------------------------------------
include(FetchContent)
include(GNUInstallDirs)

# wxWidgets Configuration
set(wxWidgets_MIN_VERSION "3.3.0")
if(USE_SYSTEM_WXWIDGETS)
    find_package(wxWidgets ${wxWidgets_MIN_VERSION} REQUIRED COMPONENTS core base stc)
    if(NOT wxWidgets_FOUND)
        message(FATAL_ERROR "wxWidgets not found, please install it or set USE_SYSTEM_WXWIDGETS to OFF")
    endif()
    include(${wxWidgets_USE_FILE})
    message(STATUS "Using system wxWidgets v${wxWidgets_VERSION}")
else()
    message(STATUS "Downloading wxWidgets v${wxWidgets_MIN_VERSION} from source")
    
    # --- CRITICAL static build settings ---
    # 1. Force wxWidgets to build as static libraries (even if BUILD_SHARED_LIBS=ON in your project)
    set(wxBUILD_SHARED OFF CACHE BOOL "Build wxWidgets as static libraries" FORCE)
    
    # 2. Size optimizations
    set(wxBUILD_MONOLITHIC OFF CACHE BOOL "Build as separate libraries (reduces unused code)" FORCE)
    set(wxUSE_WEBVIEW OFF CACHE BOOL "" FORCE)
    set(wxUSE_MEDIACTRL OFF CACHE BOOL "" FORCE)
    
    # 3. Compatibility and feature flags
    set(wxBUILD_COMPATIBILITY "3.2" CACHE STRING "" FORCE)
    set(wxBUILD_OPTIONS "std,no_aui,no_help,no_qa,no_ribbon" CACHE STRING "Disable unused features" FORCE)
    
    # 4. Download and build wxWidgets
    FetchContent_Declare(
        wxWidgets
        GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
        GIT_TAG v3.3.0
        GIT_SHALLOW ON
        CMAKE_ARGS 
            -DwxBUILD_SHARED=OFF  # Ensure static linking
            -DwxBUILD_MONOLITHIC=OFF
            -DwxUSE_WEBVIEW=OFF
            -DwxBUILD_COMPATIBILITY=3.2
    )
    
    FetchContent_MakeAvailable(wxWidgets)
endif()

# JSON Library Configuration
find_package(nlohmann_json 3.11.2 QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "Downloading nlohmann_json v3.11.2 from source")
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

#-------------------------------------------------------------------------------
# Static Analysis
#-------------------------------------------------------------------------------
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE} -extra-arg=-Wno-unknown-warning-option)
        message(STATUS "Enabled clang-tidy static analysis")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
endif()

#-------------------------------------------------------------------------------
# Target Definition
#-------------------------------------------------------------------------------
# List source files explicitly or use CONFIGURE_DEPENDS
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
) 

file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS
    "include/*.hpp"
)

add_executable(KraftaEditor 
    ${APP_SOURCES}
    ${APP_HEADERS}
)

# Platform-agnostic compiler flags
target_compile_options(KraftaEditor PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        /WX
        /permissive-
        /MP  # Enable parallel builds
    >
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -fdiagnostics-color=always  # Colorful diagnostics
    >
)

# Code coverage support
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(KraftaEditor PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_options(KraftaEditor PRIVATE --coverage)
    else()
        message(WARNING "Code coverage is not supported for this compiler")
    endif()
endif()

# Target properties
set_target_properties(KraftaEditor PROPERTIES
    CXX_VISIBILITY_PRESET "hidden"
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

# Include directories
target_include_directories(KraftaEditor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Link libraries
target_link_libraries(KraftaEditor PRIVATE
    wx::core
    wx::stc
    nlohmann_json::nlohmann_json
)

#-------------------------------------------------------------------------------
# Platform-Specific Configuration
#-------------------------------------------------------------------------------
if(APPLE)
    # macOS Bundle Settings
    set_target_properties(KraftaEditor PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        MACOSX_BUNDLE_BUNDLE_NAME "Krafta Editor"
        MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.krafta.editor"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    )
    
    # Bundle Resources
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
        set_source_files_properties(
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/AppIcon.icns"
            PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources
        )
        target_sources(KraftaEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/assets/AppIcon.icns")
    endif()
    
elseif(WIN32)
    # Windows Configuration
    set_target_properties(KraftaEditor PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/MANIFEST:EMBED"
    )
    
    # Windows Manifest and Resources
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/KraftaEditor.manifest")
        target_sources(KraftaEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/KraftaEditor.manifest")
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
        target_sources(KraftaEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
    endif()
    
    # Copy DLLs for shared builds
    if(BUILD_SHARED_LIBS)
        add_custom_command(TARGET KraftaEditor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:wx::core>
                $<TARGET_FILE_DIR:KraftaEditor>
            COMMENT "Copying wxWidgets DLL to output directory"
        )
    endif()
endif()

#-------------------------------------------------------------------------------
# Testing
#-------------------------------------------------------------------------------
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

#-------------------------------------------------------------------------------
# Developer Convenience
#-------------------------------------------------------------------------------
add_custom_target(run
    COMMAND "$<TARGET_FILE:KraftaEditor>"
    DEPENDS KraftaEditor
    COMMENT "Running Krafta Editor..."
)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    target_compile_options(KraftaEditor PRIVATE /W4 /WX)
else()
    target_compile_options(KraftaEditor PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()