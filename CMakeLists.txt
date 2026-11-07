cmake_minimum_required(VERSION 3.27.7)

project(KraftaEditor
    VERSION 0.9.3.1
    DESCRIPTION "A fast, lean code editor written in C++. Native widgets, LSP built in, 20+ languages, on Linux, Windows and macOS."
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
set(CMAKE_COLOR_DIAGNOSTICS ON)

get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _is_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ------------------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------------------
option(USE_SYSTEM_WXWIDGETS "Use the system-installed wxWidgets"                      ON)
option(KRAFTA_STATIC_MINGW  "MinGW: link runtime/zlib statically (no extra DLLs)"     ON)
option(ENABLE_TESTS         "Build the test suite"                                    OFF)
option(ENABLE_CLANG_TIDY    "Enable clang-tidy static analysis"                       OFF)
option(ENABLE_COVERAGE      "Enable code coverage instrumentation"                    OFF)
option(ENABLE_WERROR        "Treat all warnings as errors"                            OFF)
option(ENABLE_UNITY_BUILD   "Merge source files to speed up compilation"              OFF)

# ------------------------------------------------------------------------------
# Output directories per build configuration
# ------------------------------------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

# ------------------------------------------------------------------------------
# Static runtime on Windows
# ------------------------------------------------------------------------------
if(MSVC)
    # MSVC: CRT estático (sem vcruntime/msvcp DLL)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(wxBUILD_USE_STATIC_RUNTIME ON CACHE BOOL "" FORCE)
endif()

if(MINGW AND KRAFTA_STATIC_MINGW)
    # MinGW (MSYS2): sem isso o .exe exige zlib1.dll, libgcc_s_seh-1.dll,
    # libstdc++-6.dll e libwinpthread-1.dll, que só existem no MSYS2.
    # Vale para o curl e para o wxWidgets (find_package(ZLIB) é compartilhado).
    set(ZLIB_USE_STATIC_LIBS ON)
endif()

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
include(FetchContent)
include(GNUInstallDirs)

# Dependencies must never end up in our install/CPack output
set(CURL_DISABLE_INSTALL ON  CACHE BOOL "" FORCE)
set(wxBUILD_INSTALL      OFF CACHE BOOL "" FORCE)
set(JSON_Install         OFF CACHE BOOL "" FORCE)

# ------------------------------------------------------------------------------
# libcurl — fetched from source, no system dependency required
# ------------------------------------------------------------------------------
set(BUILD_CURL_EXE       OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS    OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING        OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES       OFF CACHE BOOL "" FORCE)
set(HTTP_ONLY            ON  CACHE BOOL "" FORCE)
set(CURL_ENABLE_SSL      ON  CACHE BOOL "" FORCE)
# Optional curl features that would otherwise require extra system libraries
set(CURL_USE_LIBPSL      OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBSSH2     OFF CACHE BOOL "" FORCE)
set(USE_NGHTTP2          OFF CACHE BOOL "" FORCE)
set(CURL_BROTLI          OFF CACHE BOOL "" FORCE)
set(CURL_ZSTD            OFF CACHE BOOL "" FORCE)
set(USE_LIBIDN2          OFF CACHE BOOL "" FORCE)

if(APPLE)
    set(CURL_USE_SECTRANSP ON  CACHE BOOL "" FORCE)
    set(CURL_USE_OPENSSL   OFF CACHE BOOL "" FORCE)
    set(CURL_USE_SCHANNEL  OFF CACHE BOOL "" FORCE)
elseif(WIN32)
    set(CURL_USE_SCHANNEL  ON  CACHE BOOL "" FORCE)
    set(CURL_USE_OPENSSL   OFF CACHE BOOL "" FORCE)
    set(CURL_USE_SECTRANSP OFF CACHE BOOL "" FORCE)
else()
    set(CURL_USE_OPENSSL   ON  CACHE BOOL "" FORCE)
    set(CURL_USE_SCHANNEL  OFF CACHE BOOL "" FORCE)
    set(CURL_USE_SECTRANSP OFF CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG        curl-8_8_0
    GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(curl)

# ------------------------------------------------------------------------------
# wxWidgets — prefer system 3.3+, fall back to FetchContent if too old or absent
# ------------------------------------------------------------------------------
set(wxWidgets_MIN_VERSION "3.3.3")
set(KRAFTA_WX_FROM_SYSTEM OFF)

if(USE_SYSTEM_WXWIDGETS)
    # CONFIG mode provides the imported wx::core / wx::base / wx::stc / wx::aui targets
    find_package(wxWidgets ${wxWidgets_MIN_VERSION} QUIET CONFIG COMPONENTS core base stc aui)

    if(TARGET wx::core AND TARGET wx::stc AND TARGET wx::aui)
        set(KRAFTA_WX_FROM_SYSTEM ON)
        message(STATUS "Using system wxWidgets")
    else()
        message(STATUS "System wxWidgets >= ${wxWidgets_MIN_VERSION} not found, fetching from source")
    endif()
endif()

if(NOT KRAFTA_WX_FROM_SYSTEM)
    message(STATUS "Fetching wxWidgets v${wxWidgets_MIN_VERSION}")
    set(wxBUILD_SHARED        OFF CACHE BOOL "" FORCE)
    set(wxBUILD_MONOLITHIC    OFF CACHE BOOL "" FORCE)
    set(wxBUILD_TESTS         OFF CACHE BOOL "" FORCE)
    set(wxBUILD_SAMPLES       OFF CACHE BOOL "" FORCE)
    set(wxBUILD_DEMOS         OFF CACHE BOOL "" FORCE)
    set(wxUSE_WEBVIEW         OFF CACHE BOOL "" FORCE)
    set(wxUSE_MEDIACTRL       OFF CACHE BOOL "" FORCE)
    set(wxBUILD_COMPATIBILITY "3.2" CACHE STRING "" FORCE)

    if(WIN32)
        # Bibliotecas de imagem/XML embutidas no wxWidgets: no MSYS2 o padrão
        # podia usar libpng/libjpeg/libtiff/libexpat do sistema (outras DLLs).
        set(wxUSE_LIBPNG  builtin CACHE STRING "" FORCE)
        set(wxUSE_LIBJPEG builtin CACHE STRING "" FORCE)
        set(wxUSE_LIBTIFF builtin CACHE STRING "" FORCE)
        set(wxUSE_EXPAT   builtin CACHE STRING "" FORCE)
    endif()

    FetchContent_Declare(
        wxWidgets
        GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
        GIT_TAG        v${wxWidgets_MIN_VERSION}
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(wxWidgets)
endif()

# ------------------------------------------------------------------------------
# nlohmann/json — prefer system package, fall back to FetchContent
# ------------------------------------------------------------------------------
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    ON
    FIND_PACKAGE_ARGS 3.11.3
)
FetchContent_MakeAvailable(json)

# ------------------------------------------------------------------------------
# clang-tidy (optional)
# ------------------------------------------------------------------------------
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

if(NOT APP_SOURCES)
    message(FATAL_ERROR "No source files found in ${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

# Windows resource file (icon + manifest)
set(WIN_RESOURCES "")
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
    enable_language(RC)
    set(WIN_RESOURCES resources/app.rc)
endif()

add_executable(kraftaEditor
    ${APP_SOURCES}
    ${APP_HEADERS}
    ${WIN_RESOURCES}
)

# macOS: o ícone precisa ir para Contents/Resources do bundle
if(APPLE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/AppIcon.icns")
    target_sources(kraftaEditor PRIVATE resources/AppIcon.icns)
    set_source_files_properties(resources/AppIcon.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
endif()

# ------------------------------------------------------------------------------
# Compiler flags
# ------------------------------------------------------------------------------
if(MSVC)
    target_compile_options(kraftaEditor PRIVATE /W4 /MP /permissive- /wd4458 /wd4100 /wd4101)
    if(ENABLE_WERROR)
        target_compile_options(kraftaEditor PRIVATE /WX)
    endif()
else()
    target_compile_options(kraftaEditor PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-variable
        -Wno-unused-parameter
    )
    if(ENABLE_WERROR)
        target_compile_options(kraftaEditor PRIVATE -Werror)
    endif()
endif()

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(kraftaEditor PRIVATE --coverage)
        target_link_options(kraftaEditor PRIVATE --coverage)
    else()
        message(WARNING "ENABLE_COVERAGE is only supported with GCC/Clang")
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
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(kraftaEditor PRIVATE
    wx::aui
    wx::stc
    wx::core
    wx::base
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

# ------------------------------------------------------------------------------
# Platform-specific settings
# ------------------------------------------------------------------------------
# Directory where assets are synced for dev runs
set(KRAFTA_DEV_ASSET_DIR "$<TARGET_FILE_DIR:kraftaEditor>")

if(APPLE)
    set_target_properties(kraftaEditor PROPERTIES
        MACOSX_BUNDLE                      TRUE
        MACOSX_BUNDLE_BUNDLE_NAME          "Krafta Editor"
        MACOSX_BUNDLE_ICON_FILE            "AppIcon.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER       "com.krafta.editor"
        MACOSX_BUNDLE_BUNDLE_VERSION       "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist")
        set_target_properties(kraftaEditor PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist"
        )
    endif()
    set(KRAFTA_ASSET_PATH "../Resources")
    # Binary lives in Contents/MacOS, assets are looked up in Contents/Resources
    set(KRAFTA_DEV_ASSET_DIR "$<TARGET_FILE_DIR:kraftaEditor>/../Resources")

elseif(WIN32)
    target_compile_definitions(kraftaEditor PRIVATE wxUSE_NO_MANIFEST=0)
    set_target_properties(kraftaEditor PROPERTIES WIN32_EXECUTABLE TRUE)
    set(KRAFTA_ASSET_PATH "")

    if(MINGW AND KRAFTA_STATIC_MINGW)
        target_link_options(kraftaEditor PRIVATE
            -static -static-libgcc -static-libstdc++)
    endif()

else()
    set(KRAFTA_ASSET_PATH "${CMAKE_INSTALL_FULL_DATAROOTDIR}/kraftaEditor")
endif()

target_compile_definitions(kraftaEditor PRIVATE
    KRAFTA_ASSET_RELATIVE_PATH="${KRAFTA_ASSET_PATH}"
)

# ------------------------------------------------------------------------------
# Asset directories (shared by the dev sync step and the install rules)
# ------------------------------------------------------------------------------
set(KRAFTA_ASSET_DIRS "")
foreach(ASSET_DIR assets languages i18n config)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${ASSET_DIR}")
        list(APPEND KRAFTA_ASSET_DIRS ${ASSET_DIR})
    else()
        message(WARNING "Asset directory '${ASSET_DIR}' not found; it will not be copied or installed")
    endif()
endforeach()

# Copy assets next to the binary after each build (dev convenience)
foreach(ASSET_DIR IN LISTS KRAFTA_ASSET_DIRS)
    add_custom_command(TARGET kraftaEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/${ASSET_DIR}"
            "${KRAFTA_DEV_ASSET_DIR}/${ASSET_DIR}"
        COMMENT "Syncing ${ASSET_DIR} to output directory"
        VERBATIM
    )
endforeach()

# ------------------------------------------------------------------------------
# Convenience target: cmake --build . --target run
# ------------------------------------------------------------------------------
add_custom_target(run
    COMMAND "$<TARGET_FILE:kraftaEditor>"
    DEPENDS kraftaEditor
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:kraftaEditor>"
    COMMENT "Launching Krafta Editor"
    VERBATIM
)

# ------------------------------------------------------------------------------
# Tests (optional)
# ------------------------------------------------------------------------------
if(ENABLE_TESTS)
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    else()
        message(WARNING "ENABLE_TESTS is ON but tests/CMakeLists.txt does not exist")
    endif()
endif()

# ------------------------------------------------------------------------------
# Installation rules — ONLY the runtime (executable + assets)
# ------------------------------------------------------------------------------
if(APPLE)
    install(TARGETS kraftaEditor BUNDLE DESTINATION . COMPONENT Runtime)
    foreach(ASSET_DIR IN LISTS KRAFTA_ASSET_DIRS)
        install(DIRECTORY ${ASSET_DIR}/
                DESTINATION kraftaEditor.app/Contents/Resources/${ASSET_DIR}
                COMPONENT Runtime)
    endforeach()

elseif(WIN32)
    install(TARGETS kraftaEditor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime)
    foreach(ASSET_DIR IN LISTS KRAFTA_ASSET_DIRS)
        install(DIRECTORY ${ASSET_DIR}/
                DESTINATION ${CMAKE_INSTALL_BINDIR}/${ASSET_DIR}
                COMPONENT Runtime)
    endforeach()
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/.installed")
        install(FILES "${CMAKE_SOURCE_DIR}/resources/.installed"
                DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime)
    endif()

else() # Linux / BSD
    install(TARGETS kraftaEditor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime)
    foreach(ASSET_DIR IN LISTS KRAFTA_ASSET_DIRS)
        install(DIRECTORY ${ASSET_DIR}/
                DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/kraftaEditor/${ASSET_DIR}
                COMPONENT Runtime)
    endforeach()

    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/images/kraftaEditor.svg")
        install(FILES ${CMAKE_SOURCE_DIR}/assets/images/kraftaEditor.svg
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
            RENAME kraftaEditor.svg COMPONENT Runtime
        )
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/krafta-editor.desktop")
        install(FILES ${CMAKE_SOURCE_DIR}/resources/krafta-editor.desktop
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications COMPONENT Runtime
        )
    endif()

    # Refresh desktop/icon caches only on a real install. When building a package
    # (CPack) DESTDIR points to a staging dir and the package's postinst does it.
    install(CODE "
        if(\"\$ENV{DESTDIR}\" STREQUAL \"\")
            message(STATUS \"Updating desktop database...\")
            execute_process(
                COMMAND update-desktop-database -q
                    \"${CMAKE_INSTALL_FULL_DATAROOTDIR}/applications\"
                RESULT_VARIABLE _ddb_result
                ERROR_QUIET
            )
            if(NOT _ddb_result EQUAL 0)
                message(WARNING \"update-desktop-database failed - app may not appear in menu\")
            endif()

            message(STATUS \"Updating icon cache...\")
            execute_process(
                COMMAND gtk-update-icon-cache -f -t
                    \"${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor\"
                RESULT_VARIABLE _icon_result
                ERROR_QUIET
            )
            if(NOT _icon_result EQUAL 0)
                message(WARNING \"gtk-update-icon-cache failed - icon may not appear correctly\")
            endif()
        endif()
    " COMPONENT Runtime)
endif()

# ------------------------------------------------------------------------------
# Packaging (CPack)
# ------------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME                "kraftaEditor")
set(CPACK_PACKAGE_VENDOR              "Okarin Softwares")
set(CPACK_PACKAGE_CONTACT             "apozinn@users.noreply.github.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A fast, lean code editor written in C++. Native widgets, LSP built in, 20+ languages, on Linux, Windows and macOS")
set(CPACK_PACKAGE_HOMEPAGE_URL        "https://github.com/apozinn/kraftaEditor")
set(CPACK_PACKAGE_VERSION             ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY   "Krafta Editor")
set(CPACK_COMPONENTS_ALL              Runtime)

if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
endif()

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")

    set(KRAFTA_ICON_SRC "${CMAKE_SOURCE_DIR}/assets/images/kraftaEditor.ico")
    if(EXISTS "${KRAFTA_ICON_SRC}")
        install(FILES "${KRAFTA_ICON_SRC}"
                DESTINATION ${CMAKE_INSTALL_BINDIR}
                RENAME "kraftaEditor.ico" COMPONENT Runtime)

        set(CPACK_NSIS_CREATE_ICONS_EXTRA "
            CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Krafta Editor.lnk' '$INSTDIR\\\\bin\\\\kraftaEditor.exe' '' '$INSTDIR\\\\bin\\\\kraftaEditor.ico' 0
            CreateShortCut '$DESKTOP\\\\Krafta Editor.lnk' '$INSTDIR\\\\bin\\\\kraftaEditor.exe' '' '$INSTDIR\\\\bin\\\\kraftaEditor.ico' 0
        ")
        set(CPACK_NSIS_DELETE_ICONS_EXTRA "
            Delete '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Krafta Editor.lnk'
            Delete '$DESKTOP\\\\Krafta Editor.lnk'
        ")
    else()
        set(CPACK_PACKAGE_EXECUTABLES "kraftaEditor" "Krafta Editor")
        set(CPACK_CREATE_DESKTOP_LINKS "kraftaEditor")
    endif()

    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\kraftaEditor.exe")

elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")

    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/AppIcon.icns")
        set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources/AppIcon.icns")
    endif()

else()
    set(CPACK_GENERATOR "DEB;TGZ")

    # The binary has CMAKE_INSTALL_FULL_DATAROOTDIR baked in as asset path, so the
    # package must install under the same prefix. For a distributable .deb configure
    # with -DCMAKE_INSTALL_PREFIX=/usr
    set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)   # auto-detect GTK/OpenSSL/etc. dependencies
    set(CPACK_DEBIAN_PACKAGE_SECTION   "devel")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY  "optional")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Apozin <apozinn@users.noreply.github.com>")
    set(CPACK_DEBIAN_FILE_NAME         DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)

    # dpkg only runs maintainer scripts named exactly "postinst" and they must be executable
    set(KRAFTA_DEB_CONTROL_DIR "${CMAKE_BINARY_DIR}/deb-control")
    file(MAKE_DIRECTORY "${KRAFTA_DEB_CONTROL_DIR}")
    file(WRITE "${KRAFTA_DEB_CONTROL_DIR}/postinst"
        "#!/bin/sh\n"
        "set -e\n"
        "if [ -x /usr/bin/update-desktop-database ]; then\n"
        "    /usr/bin/update-desktop-database -q /usr/share/applications || true\n"
        "fi\n"
        "if [ -x /usr/bin/gtk-update-icon-cache ]; then\n"
        "    /usr/bin/gtk-update-icon-cache -f -t /usr/share/icons/hicolor > /dev/null 2>&1 || true\n"
        "fi\n"
        "exit 0\n"
    )
    file(CHMOD "${KRAFTA_DEB_CONTROL_DIR}/postinst"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${KRAFTA_DEB_CONTROL_DIR}/postinst")
endif()

include(CPack)