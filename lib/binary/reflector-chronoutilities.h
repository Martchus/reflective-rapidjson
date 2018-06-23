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

template <> inline void readCustomType<ChronoUtilities::DateTime>(BinaryDeserializer &deserializer, ChronoUtilities::DateTime &dateTime)
{
    deserializer.read(dateTime.ticks());
}

template <> inline void writeCustomType<ChronoUtilities::DateTime>(BinarySerializer &serializer, const ChronoUtilities::DateTime &dateTime)
{
    serializer.write(dateTime.totalTicks());
}

template <> inline void readCustomType<ChronoUtilities::TimeSpan>(BinaryDeserializer &deserializer, ChronoUtilities::TimeSpan &timeSpan)
{
    deserializer.read(timeSpan.ticks());
}

template <> inline void writeCustomType<ChronoUtilities::TimeSpan>(BinarySerializer &serializer, const ChronoUtilities::TimeSpan &timeSpan)
{
    serializer.write(timeSpan.totalTicks());
}

} // namespace BinaryReflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_CHRONO_UTILITIES_H
