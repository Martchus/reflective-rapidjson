#ifndef REFLECTIVE_RAPIDJSON_TRAITS
#define REFLECTIVE_RAPIDJSON_TRAITS

#include <c++utilities/misc/traits.h>

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ReflectiveRapidJSON {

namespace Traits = ::CppUtilities::Traits;

// define structs and macros to allow treating custom data types as std::map, std::set, ...
/// \brief \brief The TreatAsMapOrHash class allows treating custom classes as std::map or std::unordered_map.
template <typename T> struct TreatAsMapOrHash : public Traits::Bool<false> {
};
/// \brief \brief The TreatAsMultiMapOrHash class allows treating custom classes as std::multimap or std::unordered_multimap.
template <typename T> struct TreatAsMultiMapOrHash : public Traits::Bool<false> {
};
/// \brief \brief The TreatAsSet class allows treating custom classes as std::set or std::unordered_set.
template <typename T> struct TreatAsSet : public Traits::Bool<false> {
};
/// \brief \brief The TreatAsMultiSet class allows treating custom classes as std::multiset or std::unordered_multiset.
template <typename T> struct TreatAsMultiSet : public Traits::Bool<false> {
};

#define REFLECTIVE_RAPIDJSON_TREAT_AS_MAP_OR_HASH(T)                                                                                                 \
    template <> struct TreatAsMapOrHash<T> : public Traits::Bool<true> {                                                                             \
    }
#define REFLECTIVE_RAPIDJSON_TREAT_AS_MULTI_MAP_OR_HASH(T)                                                                                           \
    template <> struct TreatAsMultiMapOrHash<T> : public Traits::Bool<true> {                                                                        \
    }
#define REFLECTIVE_RAPIDJSON_TREAT_AS_SET(T)                                                                                                         \
    template <> struct TreatAsSet<T> : public Traits::Bool<true> {                                                                                   \
    }
#define REFLECTIVE_RAPIDJSON_TREAT_AS_MULTI_SET(T)                                                                                                   \
    template <> struct TreatAsMultiSet<T> : public Traits::Bool<true> {                                                                              \
    }

// define traits to check for arrays, sets and maps
template <typename Type>
using IsMapOrHash
    = Traits::Any<Traits::IsSpecializationOf<Type, std::map>, Traits::IsSpecializationOf<Type, std::unordered_map>, TreatAsMapOrHash<Type>>;
template <typename Type>
using IsMultiMapOrHash = Traits::Any<Traits::IsSpecializationOf<Type, std::multimap>, Traits::IsSpecializationOf<Type, std::unordered_multimap>,
    TreatAsMultiMapOrHash<Type>>;
template <typename Type>
using IsSet = Traits::Any<Traits::IsSpecializationOf<Type, std::set>, Traits::IsSpecializationOf<Type, std::unordered_set>, TreatAsSet<Type>>;
template <typename Type>
using IsMultiSet
    = Traits::Any<Traits::IsSpecializationOf<Type, std::multiset>, Traits::IsSpecializationOf<Type, std::unordered_multiset>, TreatAsMultiSet<Type>>;
template <typename Type>
using IsArrayOrSet = Traits::Any<Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>,
                                     Traits::Not<IsMapOrHash<Type>>, Traits::Not<IsMultiMapOrHash<Type>>>,
    TreatAsSet<Type>, TreatAsMultiSet<Type>>;
template <typename Type>
using IsArray = Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>,
    Traits::Not<IsMapOrHash<Type>>, Traits::Not<IsSet<Type>>, Traits::Not<IsMultiSet<Type>>>;
template <typename Type>
using IsIteratableExceptString = Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>;

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_TRAITS
