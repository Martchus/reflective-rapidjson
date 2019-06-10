#ifndef REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_CHRONO_UTILITIES_H
#define REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_CHRONO_UTILITIES_H

/*!
 * \file reflector-chronoutilities.h
 * \brief Contains functions for (de)serializing objects from the chrono utilities provided by the
 *        C++ utilities library.
 * \remarks This file demonstrates implementing custom (de)serialization for specific types.
 */

#include "./reflector.h"

#include <c++utilities/chrono/datetime.h>
#include <c++utilities/chrono/timespan.h>
#include <c++utilities/conversion/conversionexception.h>

namespace ReflectiveRapidJSON {
namespace JsonReflector {

// define functions to "push" values to a RapidJSON array or object

template <>
inline void push<CppUtilities::DateTime>(
    const CppUtilities::DateTime &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    const std::string str(reflectable.toIsoString());
    value.SetString(str.data(), rapidJsonSize(str.size()), allocator);
}

template <>
inline void push<CppUtilities::TimeSpan>(
    const CppUtilities::TimeSpan &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    const std::string str(reflectable.toString());
    value.SetString(str.data(), rapidJsonSize(str.size()), allocator);
}

// define functions to "pull" values from a RapidJSON array or object

template <>
inline void pull<CppUtilities::DateTime>(CppUtilities::DateTime &reflectable,
    const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    std::string str;
    pull(str, value, errors);
    try {
        reflectable = CppUtilities::DateTime::fromIsoStringGmt(str.data());
    } catch (const CppUtilities::ConversionException &) {
        if (errors) {
            errors->reportConversionError(JsonType::String);
        }
    }
}

template <>
inline void pull<CppUtilities::TimeSpan>(CppUtilities::TimeSpan &reflectable,
    const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    std::string str;
    pull(str, value, errors);
    try {
        reflectable = CppUtilities::TimeSpan::fromString(str.data());
    } catch (const CppUtilities::ConversionException &) {
        if (errors) {
            errors->reportConversionError(JsonType::String);
        }
    }
}

} // namespace JsonReflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_CHRONO_UTILITIES_H
