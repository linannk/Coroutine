cmake_minimum_required(VERSION 3.15)
project(Coroutine VERSION "0.1.0"
  DESCRIPTION "Simple "
  HOMEPAGE_URL "https://github.com/"
  LANGUAGES C CXX ASM)

add_executable(test_coroutine coruntine.cc yield_to.S)

