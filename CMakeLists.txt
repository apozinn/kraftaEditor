cmake_minimum_required(VERSION 3.27.7)

project(KraftaEditor
    VERSION 1.0.0
    DESCRIPTION "A fast, lightweight, and cross-platform code editor built with C++ and wxWidgets"
    LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/apozinn/kraftaEditor"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "_d")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SHARED_LIBS "Build as shared libraries" OFF)
option(USE_SYSTEM_WXWIDGETS "Use system wxWidgets installation" OFF)
option(ENABLE_TESTS "Build test suite" OFF)
option(ENABLE_RELEASE_DEBUG_INFO "Include debug info in release builds" ON)
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

include(FetchContent)
include(GNUInstallDirs)

set(wxWidgets_MIN_VERSION "3.3.0")
if(USE_SYSTEM_WXWIDGETS)
    find_package(wxWidgets ${wxWidgets_MIN_VERSION} REQUIRED COMPONENTS core base stc)
    include(${wxWidgets_USE_FILE})
    message(STATUS "Using system wxWidgets v${wxWidgets_VERSION}")
else()
    message(STATUS "Downloading wxWidgets v${wxWidgets_MIN_VERSION} from source")
    set(wxBUILD_SHARED OFF CACHE BOOL "Build wxWidgets as static libraries" FORCE)
    set(wxBUILD_MONOLITHIC OFF CACHE BOOL "Build as separate libraries" FORCE)
    set(wxUSE_WEBVIEW OFF CACHE BOOL "" FORCE)
    set(wxUSE_MEDIACTRL OFF CACHE BOOL "" FORCE)
    set(wxBUILD_COMPATIBILITY "3.2" CACHE STRING "" FORCE)
    FetchContent_Declare(
        wxWidgets
        GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
        GIT_TAG v3.3.0
        GIT_SHALLOW ON
    )
    FetchContent_MakeAvailable(wxWidgets)
endif()

find_package(nlohmann_json 3.11.3 QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "Downloading nlohmann_json v3.11.3 from source")
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(json)
endif()

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE} -extra-arg=-Wno-unknown-warning-option)
        message(STATUS "Enabled clang-tidy static analysis")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
endif()

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "include/*.hpp")

add_executable(kraftaEditor
    ${APP_SOURCES}
    ${APP_HEADERS}
    resources/app.rc
)

target_compile_options(kraftaEditor PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /MP /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror -fdiagnostics-color=always>
)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(kraftaEditor PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_options(kraftaEditor PRIVATE --coverage)
    endif()
endif()

set_target_properties(kraftaEditor PROPERTIES
    CXX_VISIBILITY_PRESET "hidden"
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(kraftaEditor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(kraftaEditor PRIVATE
    wx::core
    wx::stc
    nlohmann_json::nlohmann_json
)

if(APPLE)
    set_target_properties(kraftaEditor PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        MACOSX_BUNDLE_BUNDLE_NAME "Krafta Editor"
        MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.krafta.editor"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    )
elseif(WIN32)
    target_compile_definitions(kraftaEditor PRIVATE wxUSE_NO_MANIFEST=0)
    set_target_properties(kraftaEditor PROPERTIES WIN32_EXECUTABLE TRUE)
    if(BUILD_SHARED_LIBS)
        add_custom_command(TARGET kraftaEditor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:wx::core>
                $<TARGET_FILE_DIR:kraftaEditor>
        )
    endif()
endif()

if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

add_custom_target(run
    COMMAND "$<TARGET_FILE:kraftaEditor>"
    DEPENDS kraftaEditor
    COMMENT "Running Krafta Editor..."
)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(kraftaEditor PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unused-variable -Wno-unused-parameter)
endif()

#-------------------------------------------------------------------------------
# Installation (Linux)
#-------------------------------------------------------------------------------
if(UNIX AND NOT APPLE)
    install(TARGETS kraftaEditor DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/kraftaEditor/assets)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/languages/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/kraftaEditor/languages)
    install(FILES ${CMAKE_SOURCE_DIR}/assets/images/kraftaEditor.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
        RENAME kraftaEditor.png)
    install(FILES ${CMAKE_SOURCE_DIR}/resources/kraftaEditor.desktop
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
endif()

#-------------------------------------------------------------------------------
# Packaging (CPack)
#-------------------------------------------------------------------------------
include(CPack)

set(CPACK_PACKAGE_NAME "kraftaEditor")
set(CPACK_PACKAGE_VENDOR "Okarin Softwares")
set(CPACK_PACKAGE_CONTACT "apozinn@users.noreply.github.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A fast, lightweight, and cross-platform code editor built with wxWidgets")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/apozinn/kraftaEditor")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

set(CPACK_GENERATOR "DEB;TGZ")
set(CPACK_DEBIAN_PACKAGE_SECTION "development")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Apozin <apozinn@users.noreply.github.com>")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
