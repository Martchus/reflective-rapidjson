cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# prevent multiple inclusion
if(DEFINED REFLECTION_GENERATOR_MODULE_LOADED)
    return()
endif()
set(REFLECTION_GENERATOR_MODULE_LOADED YES)

# find code generator
set(REFLECTION_GENERATOR_EXECUTABLE reflective_rapidjson_moc)
if(CMAKE_CROSSCOMPILING OR NOT TARGET "${REFLECTION_GENERATOR_EXECUTABLE}")
    # find "reflective_rapidjson_moc" from path
    find_program(REFLECTION_GENERATOR_EXECUTABLE "${REFLECTION_GENERATOR_EXECUTABLE}")
endif()
if(NOT REFLECTION_GENERATOR_EXECUTABLE)
    message(FATAL_ERROR "Unable to find executable of generator for reflection code.")
endif()

# define helper functions
include(CMakeParseArguments)
function(add_reflection_generator_invocation)
    # parse arguments
    set(OPTIONAL_ARGS)
    set(ONE_VALUE_ARGS OUTPUT_FILE OUTPUT_NAME)
    set(MULTI_VALUE_ARGS INPUT_FILES GENERATORS OUTPUT_LISTS)
    cmake_parse_arguments(ARGS "${OPTIONAL_ARGS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    # determine file name or file path if none specified
    if(OUTPUT_FILE AND OUTPUT_NAME)
        message(FATAL_ERROR "Specify either OUTPUT_NAME or OUTPUT_FILE but not both.")
    endif()
    if(NOT ARGS_OUTPUT_FILE)
        if(NOT ARGS_OUTPUT_NAME)
            set(ARGS_OUTPUT_NAME "reflection.h")
        endif()
        set(ARGS_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_NAME}")
    endif()

    add_custom_command(
        OUTPUT "${ARGS_OUTPUT_FILE}"
        COMMAND "${REFLECTION_GENERATOR_EXECUTABLE}"
            -o "${ARGS_OUTPUT_FILE}"
            -i ${ARGS_INPUT_FILES}
            -g ${ARGS_GENERATORS}
        DEPENDS "${ARGS_INPUT_FILES}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Generating reflection code"
    )

    # append the output file to lists specified via OUTPUT_LISTS
    if(ARGS_OUTPUT_LISTS)
        foreach(OUTPUT_LIST ${ARGS_OUTPUT_LISTS})
            set("${OUTPUT_LIST}" "${${OUTPUT_LIST}};${ARGS_OUTPUT_FILE}" PARENT_SCOPE)
        endforeach()
    endif()
endfunction()
