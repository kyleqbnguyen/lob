cmake_minimum_required(VERSION 3.20)
project(
  lob
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# lib
option(LOB_BUILD_TESTS "Build tests" ON)

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(${PROJECT_NAME} ${LIB_SOURCES})

target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include>)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

if(LOB_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
