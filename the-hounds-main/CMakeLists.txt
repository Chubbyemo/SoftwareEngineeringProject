# Specify the minimum required version of CMake for this project.
# This ensures compatibility with the specified version and newer.
cmake_minimum_required(VERSION 3.6)

# Define the project name.
project(BraendiDog_TheHounds)

# Use C++20
set(CMAKE_CXX_STANDARD 20)

# ================================
# CPM bootstrap (robust)
# ================================
# Download a known CPM version at configure time and include it.
set(CPM_DOWNLOAD_VERSION 0.39.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/CPM.cmake")

if(NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}...")
    file(DOWNLOAD
            https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/CPM.cmake
            "${CPM_DOWNLOAD_LOCATION}"
    )
endif()
# Include CPM to allow adding external dependencies easily.
include("${CPM_DOWNLOAD_LOCATION}")


# --- Option ---
option(ENABLE_COVERAGE "Build with coverage flags" OFF)

function(enable_coverage_for target_name)
  if (ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target_name} PRIVATE
      -O0 -g --coverage
      # Make embedded paths stable (fixes WSL/path issues for gcovr)
      -ffile-prefix-map=${CMAKE_SOURCE_DIR}=.
      -ffile-prefix-map=${CMAKE_BINARY_DIR}=./build
    )
    target_link_options(${target_name} PRIVATE --coverage)
  endif()
endfunction()

# --- Dependencies ---

# Add nlohmann/json for JSON parsing.
CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    VERSION 3.11.3
)

# sockpp
CPMAddPackage(
    NAME sockpp
    GITHUB_REPOSITORY fpagliughi/sockpp
    VERSION 1.0.0
)

# ---------- wxWidgets (platform-specific) ----------
# On macOS, use system/Homebrew wxWidgets to avoid building vendored deps.
# Elsewhere, keep CPM to fetch/build wx.
if(APPLE)
    # Try to detect Homebrew prefix for wxWidgets
    find_program(BREW_EXEC brew)
    if(BREW_EXEC)
        execute_process(
            COMMAND ${BREW_EXEC} --prefix wxwidgets
            OUTPUT_VARIABLE WXBREW_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(WXBREW_PREFIX)
            set(wxWidgets_CONFIG_EXECUTABLE
                "${WXBREW_PREFIX}/bin/wx-config"
                CACHE FILEPATH "Path to wx-config from Homebrew")
            list(APPEND CMAKE_PREFIX_PATH "${WXBREW_PREFIX}")
        endif()
    endif()

    # Find the installed wxWidgets (Cocoa)
    find_package(wxWidgets 3.2 REQUIRED COMPONENTS core base)
    message(STATUS "Using system wxWidgets (macOS): ${wxWidgets_LIBRARIES}")

    # Only include the use-file if CMake provided a valid path
    if(wxWidgets_USE_FILE AND EXISTS "${wxWidgets_USE_FILE}")
        include("${wxWidgets_USE_FILE}")
    endif()

else()
    # Non-macOS: keep CPM to fetch/build wxWidgets as before
    CPMAddPackage(
        NAME wxWidgets
        GITHUB_REPOSITORY wxWidgets/wxWidgets
        VERSION 3.3.1
    )

    # Configure wxWidgets build (non-macOS only)
    set(wxBUILD_SHARED ON  CACHE BOOL "Build wxWidgets as a shared library" FORCE)
    # set(wxBUILD_TESTS  OFF CACHE BOOL "Don't build tests" FORCE)

    find_package(wxWidgets REQUIRED COMPONENTS core base)

    # Only include the use-file if CMake provided a valid path
    if(wxWidgets_USE_FILE AND EXISTS "${wxWidgets_USE_FILE}")
        include("${wxWidgets_USE_FILE}")
    endif()
endif()

# Unify wx linking across platforms:
# - On macOS, FindwxWidgets exposes variables (no wx::core target)
# - Elsewhere (CPM build), imported targets wx::core / wx::base exist
if(APPLE)
    set(GAME_WX_LIBS ${wxWidgets_LIBRARIES})
else()
    set(GAME_WX_LIBS wx::core wx::base)
endif()

# GoogleTest
CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    VERSION 1.15.2
)

# --- Clang-Format Integration ---
# Locate the clang-format executable on the system.
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT} -i -style=google
                ${CMAKE_SOURCE_DIR}/src/*/*.cpp
                ${CMAKE_SOURCE_DIR}/tests/*/*.cpp
    )
endif()

# ================================
# Executables
# ================================

# Create a shared library for common code
add_library(BraendiDogShared STATIC
    src/shared/game.cpp
    src/shared/game_types.cpp
    src/shared/game_objects.cpp
    src/shared/messages.cpp
)

target_include_directories(BraendiDogShared PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(BraendiDogShared PUBLIC
    nlohmann_json::nlohmann_json
)

# Client
add_executable(Client
    src/client/MainGamePanel.cpp
    src/client/client.cpp
    src/client/guiHelpers.cpp
    src/client/ConnectionFrame.cpp
    src/client/LobbyFrame.cpp
    src/client/MovePhaseController.cpp
)

target_link_libraries(Client PRIVATE
    BraendiDogShared
    ${GAME_WX_LIBS}
    sockpp
)

# Server
add_executable(Server
    src/server/main.cpp
    src/server/server.cpp
)

target_link_libraries(Server PRIVATE
    BraendiDogShared
    sockpp
)

# Tests
## Test Game (Logic)
add_executable(test_game
    tests/test_game.cpp
)

target_include_directories(test_game PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${wxWidgets_INCLUDE_DIRS}   # adds wx headers on macOS
)

target_link_libraries(test_game PRIVATE
    BraendiDogShared
    gtest gtest_main
)

## Test Game Components
add_executable(test_game_components
    tests/test_game_components.cpp
)

target_include_directories(test_game_components PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${wxWidgets_INCLUDE_DIRS}   # adds wx headers on macOS
)

target_link_libraries(test_game_components PRIVATE
    BraendiDogShared
    gtest gtest_main
)

## Test Messages
add_executable(test_messages
    tests/test_messages.cpp
)

target_include_directories(test_messages PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${wxWidgets_INCLUDE_DIRS}   # adds wx headers on macOS
)

target_link_libraries(test_messages PRIVATE
    BraendiDogShared
    gtest gtest_main
)

# ================================

enable_coverage_for(Client)
enable_coverage_for(Server)
enable_coverage_for(test_game)
enable_coverage_for(test_game_components)
enable_coverage_for(test_messages)

# ================================
# CTest / GoogleTest discovery
# ================================
enable_testing()
include(GoogleTest)

# Discover individual test cases (for detailed CTest output)
gtest_discover_tests(test_game
    DISCOVERY_MODE PRE_TEST
)
gtest_discover_tests(test_game_components
    DISCOVERY_MODE PRE_TEST
)
gtest_discover_tests(test_messages
    DISCOVERY_MODE PRE_TEST
)

# Also add executable-level tests as fallback
add_test(NAME test_game_suite COMMAND test_game)
add_test(NAME test_game_components_suite COMMAND test_game_components)
add_test(NAME test_messages_suite COMMAND test_messages)

#!!!!!!!!!!!!!!!!!!!!!!!!!!
#!!! Test binary for CI !!!
#!!!!!!!!!!!!!!!!!!!!!!!!!!

#Put all executables directly under the build directory (so CI can run ./build/test_game)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#Minimal test binary so CI has something to run
#add_executable(test_game tests/dummy_test.cpp)

#Also copy it to the repo root, because CI later runs ./test_game from there
#add_custom_command(TARGET test_game POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_if_different
#          $<TARGET_FILE:test_game>
#          ${CMAKE_SOURCE_DIR}/test_game
#)

# (Optional) register with CTest so 'ctest' works locally
#enable_testing()
#add_test(NAME test_game COMMAND test_game)
