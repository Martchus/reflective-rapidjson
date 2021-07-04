#ifndef REFLECTIVE_RAPIDJSON_VERSIONING
#define REFLECTIVE_RAPIDJSON_VERSIONING

#include <c++utilities/misc/traits.h>

namespace ReflectiveRapidJSON {

#ifdef REFLECTIVE_RAPIDJSON_GENERATOR
#define REFLECTIVE_RAPIDJSON_CAT_1(a, b) a##b
#define REFLECTIVE_RAPIDJSON_CAT_2(a, b) REFLECTIVE_RAPIDJSON_CAT_1(a, b)
#define REFLECTIVE_RAPIDJSON_AS_OF_VERSION(version)                                                                                                  \
    static constexpr std::size_t REFLECTIVE_RAPIDJSON_CAT_2(rrjAsOfVersion, __COUNTER__) = version;                                                  \
public
#define REFLECTIVE_RAPIDJSON_UNTIL_VERSION(version)                                                                                                  \
    static constexpr std::size_t REFLECTIVE_RAPIDJSON_CAT_2(rrjUntilVersion, __COUNTER__) = version;                                                 \
public
#else
#define REFLECTIVE_RAPIDJSON_AS_OF_VERSION(version) public
#define REFLECTIVE_RAPIDJSON_UNTIL_VERSION(version) public
#endif

#ifdef REFLECTIVE_RAPIDJSON_SHORT_MACROS
#define as_of_version(version) REFLECTIVE_RAPIDJSON_AS_OF_VERSION(version)
#define until_version(version) REFLECTIVE_RAPIDJSON_UNTIL_VERSION(version)
#endif

CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(IsVersioned, T::version);
//CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(IsVersioned, T::versioningEnabled(std::declval<T &>()));

//using BinaryVersion = std::uint64_t;

//template <typename Type, BinaryVersion v = 0> struct BinarySerializable;
//CPP_UTILITIES_TRAITS_DEFINE_TYPE_CHECK(IsVersioned, static_cast<T &>(std::declval<T &>()).version);

template <typename Type, bool Condition = IsVersioned<Type>::value> struct Versioning {
    static constexpr auto enabled = false;
};

template <typename Type> struct Versioning<Type, true> {
    static constexpr auto enabled = Type::version != 0;
    static constexpr auto serializationDefault = Type::version;
    static constexpr auto applyDefault(decltype(serializationDefault) version)
    {
        return version ? version : serializationDefault;
    }
};

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_TRAITS
