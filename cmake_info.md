### Notes on cmake ###

## SPEEDING UP COMPILATION AND BUILD TIMES ##
##

# Using ninja instead of make -- should be faster (x2-x5) #
#
1. In the root (where CMakeLists.txt lives):
> cmake -B build -G Ninja

2. In the build dir
> ninja
(for the current project I redirected the build script to call ninja instead of make).

# Using lld as a linker over ld #
#
1. Install lld
> sudo apt install lld

2. In CMakeListst.txt, put 
    add_link_options("-fuse-ld=lld")

# Caching using ccache #
#
1. 
> sudo apt install ccache

2. In CMakeListst.txt, put 
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    endif()








# Linking with a static library
cmake_minimum_required(VERSION 3.15)
project(cmake_test VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Define paths to where your built raylib files live on your computer
set(RAYLIB_INCLUDE_DIR "/home/iomanip/source/lib/raylib/include") 
set(RAYLIB_LIBRARY_PATH "/home/iomanip/source/lib/raylib//lib/libraylib.a")

# 2. Create a logical wrapper target for your external binary 
add_library(raylib_local STATIC IMPORTED)

# 3. Bind the header path and binary path to our wrapper target
set_target_properties(raylib_local PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}"
    IMPORTED_LOCATION "${RAYLIB_LIBRARY_PATH}"
)

# 4. Set up your executable
add_executable(cmake_test 
    src/main.cpp
    src/inc1/h1.cpp
    src/inc2/h2.cpp
)

target_include_directories(cmake_test PRIVATE 
    src
    src/inc1
    src/inc2
)

# 5. Link against your imported target
# This automatically handles finding raylib.h and linking the binary.
target_link_libraries(cmake_test PRIVATE raylib_local)

# 6. Crucial step for Linux/macOS users: 
# Raylib relies on native graphics frameworks. We must link system backends.
find_package(Threads REQUIRED)
# Linux requirements
target_link_libraries(cmake_test PRIVATE 
    Threads::Threads GL rt dl m X11
)






