cmake_minimum_required(VERSION 3.24)

project(ShuTongWen-IME VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(BUILD_ARM64 "Build for ARM64 architecture" OFF)

if(BUILD_ARM64)
    set(CMAKE_SYSTEM_PROCESSOR ARM64)
    set(CMAKE_GENERATOR_PLATFORM ARM64)
endif()

find_package(WindowsSDK REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(GTest REQUIRED)

include_directories(include)
include_directories(third_party/include)
include_directories(${SQLITE3_INCLUDE_DIR})
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(src/core)
add_subdirectory(src/utils)
add_subdirectory(src/platform)
add_subdirectory(src/data)
add_subdirectory(src/ime)
add_subdirectory(src/tsf)
add_subdirectory(src/ipc)
add_subdirectory(src/ui)
add_subdirectory(src/telemetry)
add_subdirectory(src/compatibility)
add_subdirectory(src/watchdog)
add_subdirectory(src/testing)
add_subdirectory(src/tests)