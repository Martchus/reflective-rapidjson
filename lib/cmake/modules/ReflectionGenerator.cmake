cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# prevent multiple inclusion
if(DEFINED REFLECTION_GENERATOR_MODULE_LOADED)
    return()
endif()
set(REFLECTION_GENERATOR_MODULE_LOADED YES)

# find code generator
set(DEFAULT_REFLECTION_GENERATOR_EXECUTABLE "reflective_rapidjson_generator")
set(CUSTOM_REFLECTION_GENERATOR_EXECUTABLE "" CACHE FILEPATH "path to executable of reflection generator")
if(CUSTOM_REFLECTION_GENERATOR_EXECUTABLE)
    # use custom generator executable
    if(NOT FILE "${CUSTOM_REFLECTION_GENERATOR_EXECUTABLE}")
        message(FATAL_ERROR "The specified code generator executable \"${CUSTOM_REFLECTION_GENERATOR_EXECUTABLE}\" does not exist.")
    endif()
    set(REFLECTION_GENERATOR_EXECUTABLE "${CUSTOM_REFLECTION_GENERATOR_EXECUTABLE}")
elseif(CMAKE_CROSSCOMPILING OR NOT TARGET "${DEFAULT_REFLECTION_GENERATOR_EXECUTABLE}")
    # find native/external "reflective_rapidjson_generator"
    find_program(REFLECTION_GENERATOR_EXECUTABLE "${DEFAULT_REFLECTION_GENERATOR_EXECUTABLE}")
else()
    # use "reflective_rapidjson_generator" target
    set(REFLECTION_GENERATOR_EXECUTABLE "${DEFAULT_REFLECTION_GENERATOR_EXECUTABLE}")
endif()
if(NOT REFLECTION_GENERATOR_EXECUTABLE)
    message(FATAL_ERROR "Unable to find executable of generator for reflection code.")
endif()

# define helper functions
include(CMakeParseArguments)
function(add_reflection_generator_invocation)
    # parse arguments
    set(OPTIONAL_ARGS)
    set(ONE_VALUE_ARGS OUTPUT_DIRECTORY JSON_VISIBILITY)
    set(MULTI_VALUE_ARGS INPUT_FILES GENERATORS OUTPUT_LISTS CLANG_OPTIONS JSON_CLASSES)
    cmake_parse_arguments(ARGS "${OPTIONAL_ARGS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    # determine file name or file path if none specified
    if(NOT ARGS_OUTPUT_DIRECTORY)
        set(ARGS_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reflection")
        file(MAKE_DIRECTORY "${ARGS_OUTPUT_DIRECTORY}")
    endif()

    foreach(INPUT_FILE ${ARGS_INPUT_FILES})
        get_filename_component(OUTPUT_NAME "${INPUT_FILE}" NAME_WE)
        set(OUTPUT_FILE "${ARGS_OUTPUT_DIRECTORY}/${OUTPUT_NAME}.h")
        message(STATUS "Adding generator command for ${INPUT_FILE} producing ${OUTPUT_FILE}")
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
