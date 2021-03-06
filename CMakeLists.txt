cmake_minimum_required(VERSION 3.1)

# Ignore CMake tests which are broken with clang + static compilation
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

project(sppl)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DEBUG "Debug mode" ON)
option(STATIC "Compile with static libs" OFF)
option(LINE "Lines support in bison and flex" ON)
option(CCPP "C++ Compiler" ON)
option(CHASKELL "Haskell Compiler" OFF)
option(CGNUASM "GNU Assembly Compiler" OFF)
option(CLLVM "LLVM Intermediate Representation Compiler" OFF)
option(PROFILER "Profiler description" OFF)
option(OPTIMIZER "Optimizer description" ON)
option(RUNTIME "Runtime" ON)
option(INTERPRETER "JIT interpreter using LLVM" OFF)
option(JIT "LLVM JIT Compiler" OFF)
option(TESTS "Tests" OFF)

# Enable C++14 and toggle use of static libraries
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIE")
if(STATIC)
    set(BUILD_SHARED_LIBS Off)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
else()
    set(BUILD_SHARED_LIBS On)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("Compiling with LLVM/Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS")
    # Fix compilation with Clang
    if(STATIC)
        set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
        set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message("Compiling with GNU/GCC")
else()
    message("Compiling with ${CMAKE_CXX_COMPILER_ID}")
endif()

if(DEBUG)
    add_definitions(-DSPPLDEBUG)
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -pedantic -Wno-unused-parameter")
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set C/C++ defines from CMake options
if (CCPP)
    add_definitions(-DCCPP)
endif()
if (CHASKELL)
    add_definitions(-DCHASKELL)
endif()
if (CGNUASM)
    add_definitions(-DCGNUASM)
endif()
if (CLLVM)
    add_definitions(-DCLLVM)
endif()
if (OPTIMIZER)
    add_definitions(-DOPTIMIZER)
endif()
if (TESTS)
    add_definitions(-DTESTS)
endif()

include_directories(
        src/common
        src/semantics
        src/parser
        src/codegen/c
        src/codegen/cpar
        src/codegen/haskell
        src/codegen/gnuasm
        src/codegen/llvm
        src/codegen/pprinter
        src/compiler
        src/optimizer
        src/jit
)

add_subdirectory(src)
add_subdirectory(tools)
if (TESTS)
    add_subdirectory(tests)
endif()
