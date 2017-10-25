cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# metadata
set(META_PROJECT_NAME reflective_rapidjson_generator)
set(META_PROJECT_TYPE application)
set(LINK_TESTS_AGAINST_APP_TARGET ON)

# add project files
set(HEADER_FILES
    codegenerator.h
    codefactory.h
    frontendaction.h
    consumer.h
    visitor.h
    clangversionabstraction.h
)
set(SRC_FILES
    codegenerator.cpp
    codefactory.cpp
    frontendaction.cpp
    consumer.cpp
    clangversionabstraction.cpp
    visitor.cpp
    main.cpp
)
set(TEST_HEADER_FILES
)
set(TEST_SRC_FILES
    tests/cppunit.cpp
    tests/overall.cpp
)

# find c++utilities
find_package(c++utilities 4.11.0 REQUIRED)
use_cpp_utilities()

# find libclang
find_package(Clang REQUIRED)
list(APPEND PRIVATE_LIBRARIES clangTooling)

# also add reflective_rapidjson
list(APPEND PRIVATE_LIBRARIES reflective_rapidjson)

# trigger code generator for tests
include(ReflectionGenerator)
add_reflection_generator_invocation(
    INPUT_FILES tests/overall.cpp tests/cppunit.cpp
    GENERATORS json
    OUTPUT_LISTS TEST_HEADER_FILES
)

# include modules to apply configuration
include(BasicConfig)
include(WindowsResources)
include(AppTarget)
include(TestTarget)
include(ShellCompletion)
include(Doxygen)
include(ConfigHeader)
