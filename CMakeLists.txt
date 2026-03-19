cmake_minimum_required(VERSION 3.27.7)

project(KraftaEditor
    VERSION 1.0.0
    DESCRIPTION "A fast, lightweight, and cross-platform code editor built with C++ and wxWidgets"
    LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/apozinn/kraftaEditor"
)

# ------------------------------------------------------------------------------
# C++ standard
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "_d")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------------------
option(USE_SYSTEM_WXWIDGETS "Use the system-installed wxWidgets"       OFF)
option(ENABLE_TESTS         "Build the test suite"                      OFF)
option(ENABLE_CLANG_TIDY    "Enable clang-tidy static analysis"         OFF)
option(ENABLE_COVERAGE      "Enable code coverage instrumentation"       OFF)
option(ENABLE_WERROR        "Treat all warnings as errors"               OFF)
option(ENABLE_UNITY_BUILD   "Merge source files to speed up compilation" ON)

# ------------------------------------------------------------------------------
# Output directories per build configuration
# ------------------------------------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

# ------------------------------------------------------------------------------
# Static runtime on Windows (removes msvcp/vcruntime DLL dependency)
# ------------------------------------------------------------------------------
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ------------------------------------------------------------------------------
# LTO — enabled only for Release to avoid debug build overhead
# ------------------------------------------------------------------------------
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_output)
if(lto_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    message(STATUS "LTO enabled for Release builds")
else()
    message(STATUS "LTO not supported: ${lto_output}")
endif()

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
include(FetchContent)
include(GNUInstallDirs)

# wxWidgets — prefer system 3.3+, fall back to FetchContent if too old or absent
set(wxWidgets_MIN_VERSION "3.3.0")
if(USE_SYSTEM_WXWIDGETS)
    find_package(wxWidgets ${wxWidgets_MIN_VERSION} QUIET COMPONENTS core base stc)

    if(wxWidgets_FOUND)
        include(${wxWidgets_USE_FILE})
        message(STATUS "Using system wxWidgets v${wxWidgets_VERSION}")
    else()
        # System version is absent or older than 3.3 — fall back to FetchContent
        message(STATUS "System wxWidgets >= ${wxWidgets_MIN_VERSION} not found, fetching from source")
        set(USE_SYSTEM_WXWIDGETS OFF CACHE BOOL "" FORCE)
    endif()
endif()

if(NOT USE_SYSTEM_WXWIDGETS)
    message(STATUS "Fetching wxWidgets v${wxWidgets_MIN_VERSION}")
    set(wxBUILD_SHARED       OFF CACHE BOOL "" FORCE)
    set(wxBUILD_MONOLITHIC   OFF CACHE BOOL "" FORCE)
    set(wxUSE_WEBVIEW        OFF CACHE BOOL "" FORCE)
    set(wxUSE_MEDIACTRL      OFF CACHE BOOL "" FORCE)
    set(wxBUILD_COMPATIBILITY "3.2" CACHE STRING "" FORCE)
    FetchContent_Declare(
        wxWidgets
        GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
        GIT_TAG        v3.3.0
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(wxWidgets)
endif()

# nlohmann/json — prefer system package, fall back to FetchContent
find_package(nlohmann_json 3.11.3 QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "Fetching nlohmann_json v3.11.3")
    FetchContent_Declare(
        json
        GIT_REPOSITORY          https://github.com/nlohmann/json.git
        GIT_TAG                 v3.11.3
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(json)
endif()

# clang-tidy (optional)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE} -extra-arg=-Wno-unknown-warning-option)
        message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
endif()

# ------------------------------------------------------------------------------
# Sources and executable
# ------------------------------------------------------------------------------
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "include/*.hpp")

# Windows resource file (icon + manifest) — ignored on other platforms
set(WIN_RESOURCES "")
if(WIN32)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
        set(WIN_RESOURCES resources/app.rc)
    endif()
endif()

add_executable(kraftaEditor
    ${APP_SOURCES}
    ${APP_HEADERS}
    ${WIN_RESOURCES}
)

# ------------------------------------------------------------------------------
# Compiler flags
# ------------------------------------------------------------------------------
target_compile_options(kraftaEditor PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /MP /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -Wall -Wextra -Wpedantic
        -Wno-unused-variable -Wno-unused-parameter
        -fdiagnostics-color=always>
    # -Werror is opt-in to keep CI strict without breaking local builds
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:${ENABLE_WERROR}>>:/WX>
    $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<BOOL:${ENABLE_WERROR}>>:-Werror>
)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(kraftaEditor PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_options(kraftaEditor PRIVATE --coverage)
    endif()
endif()

# ------------------------------------------------------------------------------
# Target properties
# ------------------------------------------------------------------------------
set_target_properties(kraftaEditor PROPERTIES
    CXX_VISIBILITY_PRESET     "hidden"
    VISIBILITY_INLINES_HIDDEN ON
    UNITY_BUILD               ${ENABLE_UNITY_BUILD}
)

target_include_directories(kraftaEditor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(kraftaEditor PRIVATE
    wx::core
    wx::stc
    wx::base
    nlohmann_json::nlohmann_json
)

# ------------------------------------------------------------------------------
# Platform-specific settings
# ------------------------------------------------------------------------------
if(APPLE)
    set_target_properties(kraftaEditor PROPERTIES
        MACOSX_BUNDLE                      TRUE
        MACOSX_BUNDLE_INFO_PLIST           "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        MACOSX_BUNDLE_BUNDLE_NAME          "Krafta Editor"
        MACOSX_BUNDLE_ICON_FILE            "AppIcon.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER       "com.krafta.editor"
        MACOSX_BUNDLE_BUNDLE_VERSION       "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    )
    # Inside a .app bundle, Resources sit one level above the MacOS binary
    set(KRAFTA_ASSET_PATH "../Resources")

elseif(WIN32)
    target_compile_definitions(kraftaEditor PRIVATE wxUSE_NO_MANIFEST=0)
    set_target_properties(kraftaEditor PROPERTIES WIN32_EXECUTABLE TRUE)
    set(KRAFTA_ASSET_PATH "assets")

else() # Linux / BSD
    # Absolute path resolved at startup via the installed data directory
    set(KRAFTA_ASSET_PATH "${CMAKE_INSTALL_FULL_DATADIR}/kraftaEditor")
endif()

target_compile_definitions(kraftaEditor PRIVATE
    KRAFTA_ASSET_RELATIVE_PATH="${KRAFTA_ASSET_PATH}"
)

# ------------------------------------------------------------------------------
# Copy assets next to the binary after each build (dev convenience)
# ------------------------------------------------------------------------------
foreach(ASSET_DIR assets languages i18n config)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${ASSET_DIR}")
        add_custom_command(TARGET kraftaEditor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/${ASSET_DIR}"
                "$<TARGET_FILE_DIR:kraftaEditor>/${ASSET_DIR}"
            COMMENT "Syncing ${ASSET_DIR} to output directory"
        )
    endif()
endforeach()

# ------------------------------------------------------------------------------
# Convenience target: cmake --build . --target run
# ------------------------------------------------------------------------------
add_custom_target(run
    COMMAND "$<TARGET_FILE:kraftaEditor>"
    DEPENDS kraftaEditor
    COMMENT "Launching Krafta Editor"
)

# ------------------------------------------------------------------------------
# Tests (optional)
# ------------------------------------------------------------------------------
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ------------------------------------------------------------------------------
# Installation rules
# ------------------------------------------------------------------------------
if(APPLE)
    install(TARGETS kraftaEditor BUNDLE DESTINATION .)
    install(DIRECTORY assets/    DESTINATION kraftaEditor.app/Contents/Resources/assets)
    install(DIRECTORY languages/ DESTINATION kraftaEditor.app/Contents/Resources/languages)
    install(DIRECTORY i18n/ DESTINATION kraftaEditor.app/Contents/Resources/i18n)
    install(DIRECTORY config/    DESTINATION kraftaEditor.app/Contents/Resources/config)

elseif(WIN32)
    install(TARGETS kraftaEditor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY assets/    DESTINATION ${CMAKE_INSTALL_DATADIR}/kraftaEditor/assets)
    install(DIRECTORY languages/ DESTINATION ${CMAKE_INSTALL_DATADIR}/kraftaEditor/languages)
    install(DIRECTORY i18n/ DESTINATION ${CMAKE_INSTALL_DATADIR}/kraftaEditor/i18n)
    install(DIRECTORY config/    DESTINATION ${CMAKE_INSTALL_DATADIR}/kraftaEditor/config)

else() # Linux / BSD
    install(TARGETS kraftaEditor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY assets/    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/kraftaEditor/assets)
    install(DIRECTORY languages/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/kraftaEditor/languages)
    install(DIRECTORY i18n/      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/kraftaEditor/i18n)
    install(DIRECTORY config/    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/kraftaEditor/config)

    install(FILES ${CMAKE_SOURCE_DIR}/assets/images/kraftaEditor.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
        RENAME kraftaEditor.png
    )

    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/krafta-editor.desktop")
        install(FILES ${CMAKE_SOURCE_DIR}/resources/krafta-editor.desktop
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
        )
    endif()

    install(CODE "
        message(STATUS \"Updating desktop database...\")
        execute_process(
            COMMAND update-desktop-database
                \"\$ENV{DESTDIR}${CMAKE_INSTALL_FULL_DATAROOTDIR}/applications\"
            RESULT_VARIABLE _ddb_result
        )
        if(NOT _ddb_result EQUAL 0)
            message(WARNING \"update-desktop-database failed — app may not appear in menu\")
        endif()

        message(STATUS \"Updating icon cache...\")
        execute_process(
            COMMAND gtk-update-icon-cache -f -t
                \"\$ENV{DESTDIR}${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor\"
            RESULT_VARIABLE _icon_result
        )
        if(NOT _icon_result EQUAL 0)
            message(WARNING \"gtk-update-icon-cache failed — icon may not appear correctly\")
        endif()
    ")
endif()

# ------------------------------------------------------------------------------
# Packaging (CPack)
# ------------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME                "kraftaEditor")
set(CPACK_PACKAGE_VENDOR              "Okarin Softwares")
set(CPACK_PACKAGE_CONTACT             "apozinn@users.noreply.github.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A fast, lightweight, and cross-platform code editor")
set(CPACK_PACKAGE_HOMEPAGE_URL        "https://github.com/apozinn/kraftaEditor")
set(CPACK_PACKAGE_VERSION             ${PROJECT_VERSION})

if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
endif()

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "DEB;TGZ")
    # SHLIBDEPS disabled: wxWidgets is statically linked, no shared-lib deps to resolve
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS  OFF)
    set(CPACK_DEBIAN_PACKAGE_SECTION    "development")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY   "optional")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Apozin <apozinn@users.noreply.github.com>")
    set(CPACK_DEBIAN_FILE_NAME          DEB-DEFAULT)
endif()

include(CPack)