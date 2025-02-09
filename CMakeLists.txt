cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(KraftaEditor LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(wxBUILD_SHARED OFF)

message(STATUS "Fetching wxWidgets...")

FetchContent_Declare(
   wxWidgets
   GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
   GIT_SHALLOW ON
)
FetchContent_MakeAvailable(wxWidgets)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

set(SRCS main.cpp)

if(APPLE)
    # create bundle on apple compiles
    add_executable(KraftaEditor MACOSX_BUNDLE ${SRCS})

    # Set a custom plist file for the app bundle - needed for Mac OS Retina display
    set_target_properties(KraftaEditor PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
else()
    # the WIN32 is needed for Windows in order for it to look for WinMain
    # instead of the main function. This is ignored on other systems,
    # so it works on all platforms
    add_executable(KraftaEditor WIN32 ${SRCS} KraftaEditor.manifest)
endif()

target_link_libraries(KraftaEditor PRIVATE wxcore wxnet wxstc PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(KraftaEditor PRIVATE "include")