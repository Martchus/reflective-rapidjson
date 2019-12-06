cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# metadata
set(META_PROJECT_TYPE library)
set(META_HEADER_ONLY_LIB ON)

# add project files
set(HEADER_FILES traits.h)
set(SRC_FILES)
set(TEST_HEADER_FILES)
set(TEST_SRC_FILES)
set(CMAKE_MODULE_FILES cmake/modules/ReflectionGenerator.cmake)
set(DOC_FILES README.md)

# add JSON-specific sources
if (RapidJSON_FOUND)
    list(APPEND HEADER_FILES json/reflector.h json/reflector-boosthana.h json/reflector-chronoutilities.h
                json/serializable.h json/errorhandling.h)
    list(APPEND TEST_SRC_FILES tests/jsonreflector.cpp tests/jsonreflector-boosthana.cpp
                tests/jsonreflector-chronoutilities.cpp)
endif ()

# add binary (de)serialization specific sources
list(APPEND HEADER_FILES binary/reflector.h binary/reflector-boosthana.h binary/reflector-chronoutilities.h
            binary/serializable.h)
list(APPEND TEST_SRC_FILES tests/traits.cpp tests/binaryreflector.cpp tests/binaryreflector-boosthana.cpp)

# add (only) the CMake module and include dirs for c++utilities because we're not depending on the actual library
list(APPEND CMAKE_MODULE_PATH ${CPP_UTILITIES_MODULE_DIRS})
if (CPP_UTILITIES_SOURCE_DIR)
    list(APPEND PUBLIC_INCLUDE_DIRS $<BUILD_INTERFACE:${CPP_UTILITIES_SOURCE_DIR}/..>
                $<INSTALL_INTERFACE:${CPP_UTILITIES_INCLUDE_DIRS}>)
else ()
    list(APPEND PUBLIC_INCLUDE_DIRS "${CPP_UTILITIES_INCLUDE_DIRS}")
endif ()

# find RapidJSON, also add only the include dirs because RapidJSON is a header-only library
if (RapidJSON_FOUND)
    list(APPEND PUBLIC_INCLUDE_DIRS ${RAPIDJSON_INCLUDE_DIRS})
endif ()

# include modules to apply configuration
include(BasicConfig)
include(LibraryTarget)
include(TestTarget)
include(Doxygen)
include(ConfigHeader)
