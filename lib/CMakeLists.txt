cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# metadata
set(META_PROJECT_TYPE library)

# add project files
set(HEADER_FILES
    json/reflector.h
    json/reflector-boosthana.h
    json/reflector-chronoutilities.h
    json/serializable.h
    json/errorhandling.h
)
set(SRC_FILES
)
set(TEST_HEADER_FILES
)
set(TEST_SRC_FILES
    tests/cppunit.cpp
    tests/jsonreflector.cpp
    tests/jsonreflector-boosthana.cpp
    tests/jsonreflector-chronoutilities.cpp
)
set(CMAKE_MODULE_FILES
    cmake/modules/ReflectionGenerator.cmake
)
set(DOC_FILES
    README.md
)

# add (only) the include dirs for c++utilities because we're not depending on the actual library
list(APPEND PUBLIC_SHARED_INCLUDE_DIRS "${CPP_UTILITIES_INCLUDE_DIRS}")
list(APPEND PUBLIC_STATIC_INCLUDE_DIRS "${CPP_UTILITIES_INCLUDE_DIRS}")

# find RapidJSON, also add only the include dirs because RapidJSON is a header-only library
find_package(RapidJSON)
if(RapidJSON_FOUND)
    list(APPEND PUBLIC_SHARED_INCLUDE_DIRS ${RAPIDJSON_INCLUDE_DIRS})
    list(APPEND PUBLIC_STATIC_INCLUDE_DIRS ${RAPIDJSON_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Unable to find RapidJSON. Since this is the only reflection application supported at this time it makes no sense to continue.")
endif()

# include modules to apply configuration
include(BasicConfig)
include(WindowsResources)
include(LibraryTarget)
include(TestTarget)
include(Doxygen)
include(ConfigHeader)
