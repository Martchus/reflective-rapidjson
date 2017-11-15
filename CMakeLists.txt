cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# metadata
set(META_PROJECT_NAME reflective_rapidjson)
set(META_APP_NAME "Reflection for RapidJSON")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "Reflection for serializing/deserializing with RapidJSON")
set(META_APP_CATEGORIES "Utility;")
set(META_GUI_OPTIONAL false)
set(META_VERSION_MAJOR 0)
set(META_VERSION_MINOR 0)
set(META_VERSION_PATCH 1)
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})

# set project name for IDEs like Qt Creator
project(${META_PROJECT_NAME})

# ensure testing is enabled at this level (and not only for particular sub directories)
enable_testing()

# allow bundling c++utilities
set(BUNDLED_CPP_UTILITIES_PATH OFF CACHE PATH "specifies the (relative) path to the c++utilities sources for building it together with ${META_PROJECT_NAME}")
if(NOT BUNDLED_CPP_UTILITIES_PATH)
    message(STATUS "Using system c++utilities")
elseif(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${BUNDLED_CPP_UTILITIES_PATH}" OR IS_DIRECTORY "${BUNDLED_CPP_UTILITIES_PATH}")
    add_subdirectory("${BUNDLED_CPP_UTILITIES_PATH}" c++utilities)
    list(APPEND CMAKE_MODULE_PATH ${CPP_UTILITIES_SOURCE_DIR}/cmake/modules)
else()
    message(FATAL_ERROR "Specified directory for c++utilities sources \"${BUNDLED_CPP_UTILITIES_PATH}\" does not exist.")
endif()

# add header-only library containing JsonSerializable and helper for using RapidJSON
add_subdirectory(lib)
# allow inclusion of CMake modules from that lib in other parts of the project
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/cmake/modules" "${CMAKE_MODULE_PATH}")

# add code generator
option(NO_GENERATOR "disables building the generator (useful for cross-compiling)" OFF)
if(NOT NO_GENERATOR)
    add_subdirectory(generator)
endif()

# propagate binary and source directory of library to parent scope
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(REFLECTIVE_RAPIDJSON_BINARY_DIR "${REFLECTIVE_RAPIDJSON_BINARY_DIR}" PARENT_SCOPE)
    set(REFLECTIVE_RAPIDJSON_SOURCE_DIR "${REFLECTIVE_RAPIDJSON_SOURCE_DIR}" PARENT_SCOPE)
    set(reflective_rapidjson_DIR "${REFLECTIVE_RAPIDJSON_BINARY_DIR}" PARENT_SCOPE)
endif()
