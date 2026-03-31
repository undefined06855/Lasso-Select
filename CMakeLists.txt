cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

project(LassoSelect VERSION 1.0.0)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

set(CLIPPER2_UTILS OFF)
set(CLIPPER2_EXAMPLES OFF)
set(CLIPPER2_TESTS OFF)
CPMAddPackage("gh:AngusJohnson/Clipper2#642390d")
add_subdirectory(${Clipper2_SOURCE_DIR}/CPP)
target_link_libraries(${PROJECT_NAME} Clipper2)
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND WIN32)
    target_compile_definitions(Clipper2 PRIVATE _ITERATOR_DEBUG_LEVEL=0)
endif()


setup_geode_mod(${PROJECT_NAME})
