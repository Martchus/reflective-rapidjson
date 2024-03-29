cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

# metadata
set(META_PROJECT_TYPE library)
set(META_HEADER_ONLY_LIB ON)

# add project files
set(HEADER_FILES traits.h versioning.h)
set(SRC_FILES)
set(TEST_HEADER_FILES)
set(TEST_SRC_FILES)
set(CMAKE_MODULE_FILES cmake/modules/ReflectionGenerator.cmake)
set(DOC_FILES README.md)

# add JSON-specific sources
if (RapidJSON_FOUND)
    list(
        APPEND
        HEADER_FILES
        json/reflector.h
        json/reflector-boosthana.h
        json/reflector-chronoutilities.h
        json/serializable.h
        json/errorhandling.h
        json/errorformatting.h)
    list(APPEND TEST_SRC_FILES tests/jsonreflector.cpp tests/jsonreflector-boosthana.cpp
         tests/jsonreflector-chronoutilities.cpp)
endif ()

# add binary (de)serialization specific sources
list(APPEND HEADER_FILES binary/reflector.h binary/reflector-boosthana.h binary/reflector-chronoutilities.h
     binary/serializable.h)
list(APPEND TEST_SRC_FILES tests/traits.cpp tests/binaryreflector.cpp tests/binaryreflector-boosthana.cpp)

# add (only) the CMake module and include dirs for c++utilities because we're not depending on the actual library
use_cpp_utilities(ONLY_HEADERS VISIBILITY PUBLIC)

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

# export target name so the generator can link against it
set(${META_PROJECT_VARNAME_UPPER}_TARGET_NAME
    "${META_TARGET_NAME}"
    PARENT_SCOPE)
