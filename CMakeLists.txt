cmake_minimum_required(VERSION 3.15)
project(Coroutine VERSION "0.1.0"
  DESCRIPTION "Simple "
  HOMEPAGE_URL "https://github.com/"
  LANGUAGES C CXX ASM)

macro(DumpVariable var)
  message(STATUS "${var}: ${${var}}")
endmacro()

DumpVariable(CMAKE_SYSTEM_NAME)
DumpVariable(CMAKE_SYSTEM_PROCESSOR)

DumpVariable(CMAKE_C_COMPILER)
DumpVariable(CMAKE_CXX_COMPILER)
DumpVariable(CMAKE_ASM_COMPILER)

DumpVariable(CMAKE_C_COMPILER_ID)
DumpVariable(CMAKE_CXX_COMPILER_ID)
DumpVariable(CMAKE_ASM_COMPILER_ID)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
  add_executable(test_coroutine coruntine.cc
                 arch/x86_64/yield_to-x86_64-linux-gnu.S)
else()
  add_executable(test_coroutine coruntine.cc
                 arch/i386/yield_to-i386-linux-gnu.S)
endif()


