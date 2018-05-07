#ifndef REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_BOOST_HANA_H
#define REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_BOOST_HANA_H

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
namespace JsonReflector {

// define function to "push" values to a RapidJSON array or object

template <typename Type, Traits::DisableIf<IsBuiltInType<Type>>*>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    boost::hana::for_each(boost::hana::keys(reflectable), [&reflectable, &value, &allocator](auto key) {
        push(boost::hana::at_key(reflectable, key), boost::hana::to<char const *>(key), value, allocator);
    });
}

// define function to "pull" values from a RapidJSON array or object

template <typename Type, Traits::DisableIf<IsBuiltInType<Type>>*>
void pull(Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value,
    JsonDeserializationErrors *errors)
{
    boost::hana::for_each(boost::hana::keys(reflectable), [&reflectable, &value, &errors](auto key) {
        pull(boost::hana::at_key(reflectable, key), boost::hana::to<char const *>(key), value, errors);
    });
}

} // namespace JsonReflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_BOOST_HANA_H
