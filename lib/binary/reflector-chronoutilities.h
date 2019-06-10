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

template <> inline void readCustomType<CppUtilities::DateTime>(BinaryDeserializer &deserializer, CppUtilities::DateTime &dateTime)
{
    deserializer.read(dateTime.ticks());
}

template <> inline void writeCustomType<CppUtilities::DateTime>(BinarySerializer &serializer, const CppUtilities::DateTime &dateTime)
{
    serializer.write(dateTime.totalTicks());
}

template <> inline void readCustomType<CppUtilities::TimeSpan>(BinaryDeserializer &deserializer, CppUtilities::TimeSpan &timeSpan)
{
    deserializer.read(timeSpan.ticks());
}

template <> inline void writeCustomType<CppUtilities::TimeSpan>(BinarySerializer &serializer, const CppUtilities::TimeSpan &timeSpan)
{
    serializer.write(timeSpan.totalTicks());
}

} // namespace BinaryReflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_CHRONO_UTILITIES_H
