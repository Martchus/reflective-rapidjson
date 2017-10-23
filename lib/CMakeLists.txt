cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# metadata
set(META_PROJECT_TYPE library)

# add project files
set(HEADER_FILES
    jsonreflector.h
    jsonserializable.h
)
set(SRC_FILES
)
set(TEST_HEADER_FILES
)
set(TEST_SRC_FILES
    tests/cppunit.cpp
    tests/jsonreflector.cpp
)
set(CMAKE_MODULE_FILES
    cmake/modules/ReflectionGenerator.cmake
)
set(DOC_FILES
    README.md
)

# find c++utilities
find_package(c++utilities 4.6.0 REQUIRED)
use_cpp_utilities()

# include modules to apply configuration
include(BasicConfig)
include(WindowsResources)
include(LibraryTarget)
include(TestTarget)
include(Doxygen)
include(ConfigHeader)
