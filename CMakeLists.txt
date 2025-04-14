cmake_minimum_required(VERSION 3.20)

project(maze
    VERSION 1.0.0
    DESCRIPTION "Interactive maze pathfinding visualizer"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Options
option(MAZE_BUILD_TESTS "Build test suite" ON)
option(MAZE_BUILD_BENCHMARKS "Build benchmarks" OFF)

# FTXUI for terminal UI
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

# Header-only library interface
add_library(maze_lib INTERFACE)
target_include_directories(maze_lib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Compiler warnings
if(MSVC)
    target_compile_options(maze_lib INTERFACE /W4)
else()
    target_compile_options(maze_lib INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Main executable
add_executable(maze
    src/main.cpp
    src/cell.cpp
)
target_link_libraries(maze PRIVATE
    maze_lib
    ftxui::screen
    ftxui::dom
    ftxui::component
)

# Tests
if(MAZE_BUILD_TESTS)
    enable_testing()

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(maze_tests
        tests/test_cell.cpp
        tests/test_pathfinding.cpp
        tests/test_maze.cpp
    )
    target_link_libraries(maze_tests PRIVATE maze_lib Catch2::Catch2WithMain)

    include(CTest)
    include(Catch)
    catch_discover_tests(maze_tests)
endif()

# Doxygen documentation
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)

    doxygen_add_docs(docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        COMMENT "Generating API documentation"
    )
endif()
