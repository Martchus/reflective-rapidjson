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

// TODO: find out which header files are actually relevant rather than including the master
#include <boost/hana.hpp>

namespace ReflectiveRapidJSON {
namespace Reflector {

// define functions to "push" values to a RapidJSON array or object

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    boost::hana::for_each(boost::hana::keys(reflectable), [&reflectable, &value, &allocator](auto key) {
        push(boost::hana::at_key(reflectable, key), boost::hana::to<char const *>(key), value, allocator);
    });
}

// define functions to "pull" values from a RapidJSON array or object

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void pull(
    Type &reflectable, RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ValueIterator &value, JsonDeserializationErrors *errors)
{
    boost::hana::for_each(boost::hana::keys(reflectable), [&reflectable, &value, &errors](auto key) {
        pull(boost::hana::at_key(reflectable, key), boost::hana::to<char const *>(key), value, errors);
    });
}

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void pull(Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value,
    JsonDeserializationErrors *errors)
{
    boost::hana::for_each(boost::hana::keys(reflectable), [&reflectable, &value, &errors](auto key) {
        pull(boost::hana::at_key(reflectable, key), boost::hana::to<char const *>(key), value, errors);
    });
}

} // namespace Reflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_BOOST_HANA_H