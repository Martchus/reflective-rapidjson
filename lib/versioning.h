#ifndef REFLECTIVE_RAPIDJSON_VERSIONING
#define REFLECTIVE_RAPIDJSON_VERSIONING

namespace ReflectiveRapidJSON {

#ifdef REFLECTIVE_RAPIDJSON_GENERATOR
#define REFLECTIVE_RAPIDJSON_CAT_1(a, b) a##b
#define REFLECTIVE_RAPIDJSON_CAT_2(a, b) REFLECTIVE_RAPIDJSON_CAT_1(a, b)
#define REFLECTIVE_RAPIDJSON_AS_OF_VERSION(version) \
    constexpr std::size_t REFLECTIVE_RAPIDJSON_CAT_2(rrjAsOfVersion, __COUNTER__) = version; \
    public
#define REFLECTIVE_RAPIDJSON_UNTIL_VERSION(version) \
    constexpr std::size_t REFLECTIVE_RAPIDJSON_CAT_2(rrjUntilVersion, __COUNTER__) = version; \
    public
#else
#define REFLECTIVE_RAPIDJSON_AS_OF_VERSION(version) public
#define REFLECTIVE_RAPIDJSON_UNTIL_VERSION(version) public
#endif

#ifdef REFLECTIVE_RAPIDJSON_SHORT_MACROS
#define as_of_version(version) REFLECTIVE_RAPIDJSON_AS_OF_VERSION(version)
#define until_version(version) REFLECTIVE_RAPIDJSON_UNTIL_VERSION(version)
#endif

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_TRAITS
