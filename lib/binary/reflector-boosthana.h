#ifndef REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_BOOST_HANA_H
#define REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_BOOST_HANA_H

/*!
 * \file reflector-boosthana.h
 * \brief Contains generic functions relying on Boost.Hana which can replace the code which would
 *        otherwise had to be generated.
 * \remarks
 * These functions use boost::hana::keys() and boost::hana::at_key() rather than the "plain"
 * for-loop shown in the introspection examples of the Boost.Hana documentation. The reason is that
 * the "plain" for-loop involves making copies. This costs performance and - more importantly - prevents
 * modifying the actual object.
 */

#include "./reflector.h"

#include <boost/hana/adapt_struct.hpp>
#include <boost/hana/at_key.hpp>
#include <boost/hana/define_struct.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/intersection.hpp>
#include <boost/hana/keys.hpp>

namespace ReflectiveRapidJSON {
namespace BinaryReflector {

template <typename Type, Traits::EnableIf<IsCustomType<Type>> *> void readCustomType(BinaryDeserializer &deserializer, Type &customType)
{
    boost::hana::for_each(
        boost::hana::keys(customType), [&deserializer, &customType](auto key) { deserializer.read(boost::hana::at_key(customType, key)); });
}

template <typename Type, Traits::EnableIf<IsCustomType<Type>> *> void writeCustomType(BinarySerializer &serializer, const Type &customType, BinaryVersion version)
{
    boost::hana::for_each(
        boost::hana::keys(customType), [&serializer, &customType](auto key) { serializer.write(boost::hana::at_key(customType, key)); });
}

} // namespace BinaryReflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_BOOST_HANA_H
