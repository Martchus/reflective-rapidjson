#ifndef REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_BOOST_HANA_H
#define REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_BOOST_HANA_H

/*!
 * \file jsonreflector-boosthana.h
 * \brief Contains generic functions relying on Boost.Hana which can replace the code which would
 *        otherwise had to be generated.
 */

#include "./jsonreflector.h"

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
    boost::hana::for_each(reflectable, [&value, &allocator](auto pair) {
        push(boost::hana::second(pair), boost::hana::to<char const*>(boost::hana::first(pair)), value, allocator);
    });
}

// define functions to "pull" values from a RapidJSON array or object

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void pull(Type &reflectable, RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ValueIterator &value)
{
    boost::hana::for_each(reflectable, [&value](auto pair) {
        pull(boost::hana::second(pair), boost::hana::to<char const*>(boost::hana::first(pair)), value);
    });
}

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void pull(Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value)
{
    boost::hana::for_each(reflectable, [&value](auto pair) {
        pull(boost::hana::second(pair), boost::hana::to<char const*>(boost::hana::first(pair)), value);
    });
}

} // namespace Reflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_BOOST_HANA_H
