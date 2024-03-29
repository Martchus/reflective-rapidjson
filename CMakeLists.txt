cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

# set metadata
project(reflective_rapidjson)
set(META_PROJECT_NAME reflective_rapidjson)
set(META_APP_NAME "Reflection for RapidJSON")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/reflective-rapidjson")
set(META_APP_DESCRIPTION "Reflection for serializing/deserializing with RapidJSON")
set(META_APP_CATEGORIES "Utility;")
set(META_GUI_OPTIONAL false)
set(META_VERSION_MAJOR 0)
set(META_VERSION_MINOR 0)
set(META_VERSION_PATCH 16)
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})
set(META_CXX_STANDARD 17)
set(META_ADD_DEFAULT_CPP_UNIT_TEST_APPLICATION ON)

# ensure testing is enabled at this level (and not only for particular sub directories)
enable_testing()

# allow bundling c++utilities
set(BUNDLED_CPP_UTILITIES_PATH OFF CACHE PATH "specifies the (relative) path to the c++utilities sources for building it together with ${META_PROJECT_NAME}")
if(NOT BUNDLED_CPP_UTILITIES_PATH)
    message(STATUS "Using system c++utilities")
else()
    # check whether bundled c++utilities actually exists
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${BUNDLED_CPP_UTILITIES_PATH}")
        # treat the specified path as relative to the current source dir
        set(c++utilities_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${BUNDLED_CPP_UTILITIES_PATH}")
    elseif(IS_DIRECTORY "${BUNDLED_CPP_UTILITIES_PATH}")
        # treat the specified path as absolute
        set(c++utilities_DIR "${BUNDLED_CPP_UTILITIES_PATH}")
    else()
        # consider the specified path invalid
        set(c++utilities_DIR "")
    endif()

    if(c++utilities_DIR)
        add_subdirectory("${BUNDLED_CPP_UTILITIES_PATH}" c++utilities)
        list(APPEND CMAKE_MODULE_PATH "${CPP_UTILITIES_SOURCE_DIR}/cmake/modules")
    else()
        message(FATAL_ERROR "Specified directory for c++utilities sources \"${BUNDLED_CPP_UTILITIES_PATH}\" does not exist.")
    endif()
endif()

# find c++utilities
set(CONFIGURATION_PACKAGE_SUFFIX "" CACHE STRING "sets the suffix for find_package() calls to packages configured via c++utilities")
find_package(c++utilities${CONFIGURATION_PACKAGE_SUFFIX} 5.10.0 REQUIRED)

# find RapidJSON
if(NOT RapidJSON_FOUND)
    find_package(RapidJSON)
endif()
if(NOT RapidJSON_FOUND)
    message(FATAL_ERROR "Unable to find RapidJSON. Since this is the only supported reflection application at this time, it makes no sense to continue.")
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
