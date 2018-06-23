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

namespace BinaryReflector {
namespace JsonReflector {

} // namespace JsonReflector
} // namespace BinaryReflector

#endif // REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_BOOST_HANA_H
