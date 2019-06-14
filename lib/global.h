// Created via CMake from template global.h.in
// WARNING! Any changes to this file will be overwritten by the next CMake run!

#ifndef REFLECTIVE_RAPIDJSON_GLOBAL
#define REFLECTIVE_RAPIDJSON_GLOBAL

#include <c++utilities/application/global.h>

#ifdef REFLECTIVE_RAPIDJSON_STATIC
#define REFLECTIVE_RAPIDJSON_EXPORT
#define REFLECTIVE_RAPIDJSON_IMPORT
#else
#define REFLECTIVE_RAPIDJSON_EXPORT CPP_UTILITIES_GENERIC_LIB_EXPORT
#define REFLECTIVE_RAPIDJSON_IMPORT CPP_UTILITIES_GENERIC_LIB_IMPORT
#endif

/*!
 * \def REFLECTIVE_RAPIDJSON_EXPORT
 * \brief Marks the symbol to be exported by the reflective_rapidjson library.
 */

/*!
 * \def REFLECTIVE_RAPIDJSON_IMPORT
 * \brief Marks the symbol to be imported from the reflective_rapidjson library.
 */

#endif // REFLECTIVE_RAPIDJSON_GLOBAL
