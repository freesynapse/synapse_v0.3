cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(synapse VERSION 1.0 LANGUAGES C CXX)

include(CheckCXXCompilerFlag)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# Recursively find all .cpp files in the src/ directory
file(GLOB_RECURSE SRC_FILES 
     CONFIGURE_DEPENDS 
     "src/*.cpp"
     "src/*.cc"
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "DEBUG" FORCE)
endif()

add_link_options("-fuse-ld=lld")

add_executable(synapse ${SRC_FILES})

check_cxx_compiler_flag("-fdiagnostics-absolute-paths" HAS_ABSOLUTE_PATHS)
if (HAS_ABSOLUTE_PATHS)
  target_compile_options(synapse PRIVATE -fdiagnostics-absolute-paths)
endif()
target_compile_options(synapse PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-g -gcolumn-info>
)

# GLFW
add_library(glfw STATIC IMPORTED GLOBAL)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/lib/libglfw3.a
)

# assimp
add_library(assimp SHARED IMPORTED GLOBAL)
set_target_properties(assimp PROPERTIES
   IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/lib/libassimp.so
)

target_include_directories(synapse PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/include/
 ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/include/
)

# linking system backends
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)
target_link_libraries(glfw INTERFACE
    Threads::Threads
    X11
    GL
    # Add 'wayland-client' here instead of X11 if you built GLFW for Wayland
)

target_link_libraries(synapse PRIVATE 
    Freetype::Freetype
    rt 
    dl 
    m 
    glfw
    assimp
)

