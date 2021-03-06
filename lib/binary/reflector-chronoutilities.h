#ifndef REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_CHRONO_UTILITIES_H
#define REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_CHRONO_UTILITIES_H

/*!
 * \file reflector-chronoutilities.h
 * \brief Contains functions for (de)serializing objects from the chrono utilities provided by the
 *        C++ utilities library.
 * \remarks This file demonstrates implementing custom (de)serialization for specific types.
 */

#include "./reflector.h"

#include <c++utilities/chrono/datetime.h>
#include <c++utilities/chrono/timespan.h>

namespace ReflectiveRapidJSON {
namespace BinaryReflector {

template <>
inline BinaryVersion readCustomType<CppUtilities::DateTime>(BinaryDeserializer &deserializer, CppUtilities::DateTime &dateTime, BinaryVersion version)
{
    CPP_UTILITIES_UNUSED(version)
    deserializer.read(dateTime.ticks());
    return 0;
}

template <>
inline void writeCustomType<CppUtilities::DateTime>(BinarySerializer &serializer, const CppUtilities::DateTime &dateTime, BinaryVersion version)
{
    CPP_UTILITIES_UNUSED(version)
    serializer.write(dateTime.totalTicks());
}

template <>
inline BinaryVersion readCustomType<CppUtilities::TimeSpan>(BinaryDeserializer &deserializer, CppUtilities::TimeSpan &timeSpan, BinaryVersion version)
{
    CPP_UTILITIES_UNUSED(version)
    deserializer.read(timeSpan.ticks());
    return 0;
}

template <>
inline void writeCustomType<CppUtilities::TimeSpan>(BinarySerializer &serializer, const CppUtilities::TimeSpan &timeSpan, BinaryVersion version)
{
    CPP_UTILITIES_UNUSED(version)
    serializer.write(timeSpan.totalTicks());
}

} // namespace BinaryReflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_CHRONO_UTILITIES_H
