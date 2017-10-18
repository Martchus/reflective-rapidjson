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

# allow bundling c++utilities
set(BUNDLED_CPP_UTILITIES_PATH OFF CACHE FILEPATH "specifies the (relative) path to the c++utilities sources for building it together with ${META_PROJECT_NAME}")
if(NOT BUNDLED_CPP_UTILITIES_PATH)
    message(STATUS "Using system c++utilities")
elseif(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${BUNDLED_CPP_UTILITIES_PATH}" OR IS_DIRECTORY "${BUNDLED_CPP_UTILITIES_PATH}")
    add_subdirectory("${BUNDLED_CPP_UTILITIES_PATH}" c++utilities)
    list(APPEND CMAKE_MODULE_PATH ${CPP_UTILITIES_SOURCE_DIR}/cmake/modules)
else()
    message(FATAL_ERROR "Specified directory for c++utilities sources \"${BUNDLED_CPP_UTILITIES_PATH}\" does not exist.")
endif()

# add helper library for using RapidJSON
add_subdirectory(lib)

# add code generator
add_subdirectory(moc)
