cmake_minimum_required(VERSION 3.21)   # PROJECT_IS_TOP_LEVEL

# Pin the toolchain ONLY when Synapse is the top-level project, so that
# consuming it as a subproject doesn't override synterm's toolchain.
# (Must be evaluated before project(); these vars are set at configure start.)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_C_COMPILER   "/usr/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
project(synapse VERSION 1.0 LANGUAGES C CXX)
include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# ccache (directory-scoped; only affects Synapse's own targets)
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_C_COMPILER_LAUNCHER  ${CCACHE_PROGRAM})
endif()

option(SYNAPSE_SANITIZE "Build Synapse (and consumers) with AddressSanitizer" ON)

add_link_options("-fuse-ld=lld")

# --- Sources: split main() out of the library ---------------------------------
# Point this at your actual entry-point TU.
set(SYNAPSE_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    CACHE FILEPATH "TU containing Synapse's test main()")

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")

# Keep main() out of the library so consumers provide their own entry point.
list(REMOVE_ITEM SRC_FILES "${SYNAPSE_MAIN}")

# --- The library --------------------------------------------------------------
add_library(synapse ${SRC_FILES})        # STATIC unless -DBUILD_SHARED_LIBS=ON
add_library(synapse::synapse ALIAS synapse)

check_cxx_compiler_flag("-fdiagnostics-absolute-paths" HAS_ABSOLUTE_PATHS)
if(HAS_ABSOLUTE_PATHS)
    target_compile_options(synapse PRIVATE -fdiagnostics-absolute-paths)
endif()
target_compile_options(synapse PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-g -gcolumn-info>)

# ASan as a PUBLIC usage requirement -> propagates to synterm and the sandbox.
if(SYNAPSE_SANITIZE)
    target_compile_options(synapse PUBLIC -fsanitize=address -fno-omit-frame-pointer)
    target_link_options   (synapse PUBLIC -fsanitize=address)
endif()

# Public API headers; vendored deps stay private (see note below).
target_include_directories(synapse PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(synapse PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/include)

# --- Imported third-party targets (GLOBAL so synterm's scope sees them) --------
add_library(glfw STATIC IMPORTED GLOBAL)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/lib/libglfw3.a)

add_library(assimp SHARED IMPORTED GLOBAL)
set_target_properties(assimp PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/lib/libassimp.so)

find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)
target_link_libraries(glfw INTERFACE Threads::Threads X11 GL)

# PRIVATE is correct: for a STATIC lib CMake still forwards these to the final
# link line of any consumer, so synterm resolves them transitively.
target_link_libraries(synapse 
    PUBLIC Freetype::Freetype
    PRIVATE rt dl m glfw assimp)
    

# --- Standalone sandbox: built only when Synapse is top-level ------------------
if(PROJECT_IS_TOP_LEVEL)
    add_executable(synapse_sandbox ${SYNAPSE_MAIN})
    set_target_properties(synapse_sandbox PROPERTIES OUTPUT_NAME synapse)  # keeps ./synapse
    target_link_libraries(synapse_sandbox PRIVATE synapse::synapse)
endif()
