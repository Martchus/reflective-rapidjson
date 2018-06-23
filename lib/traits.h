#ifndef REFLECTIVE_RAPIDJSON_TRAITS
#define REFLECTIVE_RAPIDJSON_TRAITS

#include <c++utilities/misc/traits.h>

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ReflectiveRapidJSON {

// define traits to check for arrays, sets and maps
template <typename Type>
using IsMapOrHash = Traits::Any<Traits::IsSpecializationOf<Type, std::map>, Traits::IsSpecializationOf<Type, std::unordered_map>>;
template <typename Type> using IsSet = Traits::Any<Traits::IsSpecializationOf<Type, std::set>, Traits::IsSpecializationOf<Type, std::unordered_set>>;
template <typename Type>
using IsMultiSet = Traits::Any<Traits::IsSpecializationOf<Type, std::multiset>, Traits::IsSpecializationOf<Type, std::unordered_multiset>>;
template <typename Type>
using IsArrayOrSet
    = Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>, Traits::Not<IsMapOrHash<Type>>>;
template <typename Type>
using IsArray = Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>,
    Traits::Not<IsMapOrHash<Type>>, Traits::Not<IsSet<Type>>, Traits::Not<IsMultiSet<Type>>>;
template <typename Type>
using IsIteratableExceptString = Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>;

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_TRAITS
