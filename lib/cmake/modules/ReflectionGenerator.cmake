cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# prevent multiple inclusion
if(DEFINED REFLECTION_GENERATOR_MODULE_LOADED)
    return()
endif()
set(REFLECTION_GENERATOR_MODULE_LOADED YES)

# find code generator
set(DEFAULT_REFLECTION_GENERATOR_EXECUTABLE "reflective_rapidjson_generator")
set(REFLECTION_GENERATOR_EXECUTABLE "" CACHE FILEPATH "path to executable of reflection generator")
if(REFLECTION_GENERATOR_EXECUTABLE)
    # use custom generator executable
    if(NOT EXISTS "${REFLECTION_GENERATOR_EXECUTABLE}" OR IS_DIRECTORY "${REFLECTION_GENERATOR_EXECUTABLE}")
        message(FATAL_ERROR "The specified code generator executable \"${REFLECTION_GENERATOR_EXECUTABLE}\" is not a file.")
    endif()
elseif(CMAKE_CROSSCOMPILING OR NOT TARGET "${DEFAULT_REFLECTION_GENERATOR_EXECUTABLE}")
    # find native/external "reflective_rapidjson_generator"
    find_program(REFLECTION_GENERATOR_EXECUTABLE "${DEFAULT_REFLECTION_GENERATOR_EXECUTABLE}")
else()
    # use "reflective_rapidjson_generator" target
    set(REFLECTION_GENERATOR_EXECUTABLE "${DEFAULT_REFLECTION_GENERATOR_EXECUTABLE}")
endif()
if(NOT REFLECTION_GENERATOR_EXECUTABLE)
    message(FATAL_ERROR "Unable to find executable of generator for reflection code. Set REFLECTION_GENERATOR_EXECUTABLE to specify the path.")
endif()

# allow to specify a custom include path and use first implicit include directory as default
# (useful for cross-compilation when header files are under custom prefix)
set(REFLECTION_GENERATOR_INCLUDE_DIRECTORIES "${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}" CACHE FILEPATH "include directories for code generator")

# define helper function to add a reflection generator invocation for a specified list of source files
include(CMakeParseArguments)
function(add_reflection_generator_invocation)
    # parse arguments
    set(OPTIONAL_ARGS
    )
    set(ONE_VALUE_ARGS
        OUTPUT_DIRECTORY
        JSON_VISIBILITY
    )
    set(MULTI_VALUE_ARGS
        INPUT_FILES
        GENERATORS
        OUTPUT_LISTS
        CLANG_OPTIONS
        CLANG_OPTIONS_FROM_TARGETS
        JSON_CLASSES)
    cmake_parse_arguments(ARGS "${OPTIONAL_ARGS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    # determine file name or file path if none specified
    if(NOT ARGS_OUTPUT_DIRECTORY)
        set(ARGS_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reflection")
        file(MAKE_DIRECTORY "${ARGS_OUTPUT_DIRECTORY}")
    endif()

    # apply specified REFLECTION_GENERATOR_INCLUDE_DIRECTORIES
    foreach(INCLUDE_DIR ${REFLECTION_GENERATOR_INCLUDE_DIRECTORIES})
        list(APPEND ARGS_CLANG_OPTIONS "-isystem ${INCLUDE_DIR}")
    endforeach()

    # add options required for cross compiling with mingw-w64
    if(MINGW)
        # find MinGW version of stdlib.h to ensure that only this version is processed
        find_file(MINGW_W64_STDLIB_H stdlib.h ${REFLECTION_GENERATOR_INCLUDE_DIRECTORIES})
        if(NOT EXISTS "${MINGW_W64_STDLIB_H}")
            message(FATAL_ERROR "Unable to locate MinGW version of stdlib.h. Ensure it is in REFLECTION_GENERATOR_INCLUDE_DIRECTORIES.")
        endif()

        list(APPEND ARGS_CLANG_OPTIONS
            # allow __declspec
            "-fdeclspec"
            # make sure platform detection works as expected
            "-D_WIN32"
            # ensure libtooling processes the MinGW version of stdlib.h rather than the host version
            # (not sure why specifying REFLECTION_GENERATOR_INCLUDE_DIRECTORIES is not enough to let it find the correct header file)
            "-include ${MINGW_W64_STDLIB_H}"
            # prevent processing of host stdlib.h
            "-D_STDLIB_H"
        )
    endif()

    # TODO: add options for other targets

    # add options to be passed to clang from the specified targets
    if(ARGS_CLANG_OPTIONS_FROM_TARGETS)
        foreach(TARGET_NAME ${ARGS_CLANG_OPTIONS_FROM_TARGETS})
            # add compile flags
            set(PROP "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_FLAGS>")
            list(APPEND ARGS_CLANG_OPTIONS "$<$<BOOL:${PROP}>:$<JOIN:${PROP},$<SEMICOLON>>>")
            # add compile definitions
            set(PROP "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_DEFINITIONS>")
            list(APPEND ARGS_CLANG_OPTIONS "$<$<BOOL:${PROP}>:-D$<JOIN:${PROP},$<SEMICOLON>-D>>")
            # add include directories
            set(PROP "$<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>")
            list(APPEND ARGS_CLANG_OPTIONS "$<$<BOOL:${PROP}>:-I$<JOIN:${PROP},$<SEMICOLON>-I>>")
        endforeach()
    endif()

    # create a custom command for each input file
    foreach(INPUT_FILE ${ARGS_INPUT_FILES})
        # determine the output file
        get_filename_component(OUTPUT_NAME "${INPUT_FILE}" NAME_WE)
        set(OUTPUT_FILE "${ARGS_OUTPUT_DIRECTORY}/${OUTPUT_NAME}.h")
        message(STATUS "Adding generator command for ${INPUT_FILE} producing ${OUTPUT_FILE}")

        # compose the CLI arguments and actually add the custom command
        set(CLI_ARGUMENTS
            --output-file "${OUTPUT_FILE}"
            --input-file "${INPUT_FILE}"
            --generators ${ARGS_GENERATORS}
            --clang-opt ${ARGS_CLANG_OPTIONS}
            --json-classes ${ARGS_JSON_CLASSES}
        )
        if(ARGS_JSON_VISIBILITY)
            list(APPEND CLI_ARGUMENTS --json-visibility "${ARGS_JSON_VISIBILITY}")
        endif()
        add_custom_command(
            OUTPUT "${OUTPUT_FILE}"
            COMMAND "${REFLECTION_GENERATOR_EXECUTABLE}"
            ARGS ${CLI_ARGUMENTS}
            DEPENDS "${INPUT_FILE}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Generating reflection code for ${INPUT_FILE}"
            VERBATIM
        )

        # append the output file to lists specified via OUTPUT_LISTS
        if(ARGS_OUTPUT_LISTS)
            foreach(OUTPUT_LIST ${ARGS_OUTPUT_LISTS})
                list(APPEND "${OUTPUT_LIST}" "${OUTPUT_FILE}")
                set("${OUTPUT_LIST}" "${${OUTPUT_LIST}}" PARENT_SCOPE)
            endforeach()
        endif()
    endforeach()
endfunction()
