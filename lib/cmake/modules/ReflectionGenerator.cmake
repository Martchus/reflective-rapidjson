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
    find_program(REFLECTION_GENERATOR_EXECUTABLE
        "${DEFAULT_REFLECTION_GENERATOR_EXECUTABLE}"
        PATHS "/usr/bin" "/bin"
    )
else()
    # use "reflective_rapidjson_generator" target
    set(REFLECTION_GENERATOR_EXECUTABLE "${DEFAULT_REFLECTION_GENERATOR_EXECUTABLE}")
endif()
if(NOT REFLECTION_GENERATOR_EXECUTABLE)
    message(FATAL_ERROR "Unable to find executable of generator for reflection code. Set REFLECTION_GENERATOR_EXECUTABLE to specify the path.")
endif()

# determine Clang's resource directory
set(REFLECTION_GENERATOR_CLANG_RESOURCE_DIR "" CACHE PATH "directory containing Clang's builtin headers, usually /usr/lib/clang/version")
if(NOT REFLECTION_GENERATOR_CLANG_RESOURCE_DIR)
    if(NOT REFLECTION_GENERATOR_CLANG_BIN)
        find_program(REFLECTION_GENERATOR_CLANG_BIN clang
            NAMES clang++
            PATHS "/usr/bin" "/bin"
        )
        if(NOT REFLECTION_GENERATOR_CLANG_BIN)
            message(FATAL_ERROR "Unable to find the clang executable to determine Clang's resource directory")
        endif()
    endif()
    exec_program(${REFLECTION_GENERATOR_CLANG_BIN} ARGS -print-resource-dir OUTPUT_VARIABLE REFLECTION_GENERATOR_CLANG_RESOURCE_DIR)
endif()
if(NOT REFLECTION_GENERATOR_CLANG_RESOURCE_DIR OR NOT IS_DIRECTORY "${REFLECTION_GENERATOR_CLANG_RESOURCE_DIR}")
    message(FATAL_ERROR "Unable to find Clang's resource directory. Set REFLECTION_GENERATOR_CLANG_RESOURCE_DIR manually to the corresponding path (usually /usr/lib/clang/\$version).")
endif()
message(STATUS "Using ${REFLECTION_GENERATOR_CLANG_RESOURCE_DIR} as Clang's resource directory for Reflective RapidJSON")

# allow to specify a custom include paths (useful for cross-compilation when header files are under custom prefix)
set(REFLECTION_GENERATOR_INCLUDE_DIRECTORIES "" CACHE FILEPATH "include directories for code generator")

# allow to specify a custom platform tiple (useful for cross-compilation to specify the target platform)
set(REFLECTION_GENERATOR_TRIPLE "" CACHE STRING "platform triple for code generator")

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

    # specify Clang's resource directory
    list(APPEND ARGS_CLANG_OPTIONS -resource-dir "${REFLECTION_GENERATOR_CLANG_RESOURCE_DIR}")

    # apply specified REFLECTION_GENERATOR_TRIPLET
    if(REFLECTION_GENERATOR_TRIPLE)
        list(APPEND ARGS_CLANG_OPTIONS
            -Xclang -triple
            -Xclang "${REFLECTION_GENERATOR_TRIPLE}"
        )
    endif()

    # apply specified REFLECTION_GENERATOR_INCLUDE_DIRECTORIES
    foreach(INCLUDE_DIR ${REFLECTION_GENERATOR_INCLUDE_DIRECTORIES})
        if(NOT IS_DIRECTORY "${INCLUDE_DIR}")
            message(FATAL_ERROR "Specified include directory \"${INCLUDE_DIR})\" for reflection generator doesn't exists.")
        endif()
        list(APPEND ARGS_CLANG_OPTIONS -I "${INCLUDE_DIR}")
    endforeach()

    # add workaround for cross compiling with mingw-w64 to prevent host stdlib.h being included
    # (not sure why specifying REFLECTION_GENERATOR_INCLUDE_DIRECTORIES is not enough to let it find this particular header file)
    if(MINGW)
        # find MinGW version of stdlib.h
        find_file(MINGW_W64_STDLIB_H stdlib.h ${REFLECTION_GENERATOR_INCLUDE_DIRECTORIES})
        if(NOT EXISTS "${MINGW_W64_STDLIB_H}")
            message(FATAL_ERROR "Unable to locate MinGW version of stdlib.h. Ensure it is in REFLECTION_GENERATOR_INCLUDE_DIRECTORIES.")
        endif()

        # ensure libtooling includes the MinGW version of stdlib.h rather than the host version
        list(APPEND ARGS_CLANG_OPTIONS
            -include "${MINGW_W64_STDLIB_H}"
            -D_STDLIB_H # prevent processing of host stdlib.h
        )
    endif()

    # add options to be passed to clang from the specified targets
    if(ARGS_CLANG_OPTIONS_FROM_TARGETS)
        foreach(TARGET_NAME ${ARGS_CLANG_OPTIONS_FROM_TARGETS})
            # set c++ standard
            list(APPEND CLANG_TIDY_CXX_FLAGS "-std=c++$<TARGET_PROPERTY:${TARGET_NAME},CXX_STANDARD>")
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

        # prevent Qt's code generator to be executed on the files generated by this code generator
        set_property(SOURCE "${OUTPUT_FILE}" PROPERTY SKIP_AUTOGEN ON)

        # append the output file to lists specified via OUTPUT_LISTS
        if(ARGS_OUTPUT_LISTS)
            foreach(OUTPUT_LIST ${ARGS_OUTPUT_LISTS})
                list(APPEND "${OUTPUT_LIST}" "${OUTPUT_FILE}")
                set("${OUTPUT_LIST}" "${${OUTPUT_LIST}}" PARENT_SCOPE)
            endforeach()
        endif()
    endforeach()
endfunction()
