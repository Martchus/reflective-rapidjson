#include "../traits.h"
#include "../versioning.h"

#include "../binary/serializable.h"

#include <list>
#include <vector>

// define structs for testing REFLECTIVE_RAPIDJSON_TREAT_AS_…
struct Foo {};
struct Bar {};

// define structs for testing versioning
struct VersionlessBase : public ReflectiveRapidJSON::BinarySerializable<VersionlessBase> {};
struct VersionedDerived : public VersionlessBase, public ReflectiveRapidJSON::BinarySerializable<VersionedDerived, 1> {};
struct VersionedBase : public ReflectiveRapidJSON::BinarySerializable<VersionlessBase, 1> {};
struct VersionlessDerived : public VersionedBase, public ReflectiveRapidJSON::BinarySerializable<VersionlessDerived> {};

namespace ReflectiveRapidJSON {
REFLECTIVE_RAPIDJSON_TREAT_AS_MAP_OR_HASH(Foo);
REFLECTIVE_RAPIDJSON_TREAT_AS_MULTI_MAP_OR_HASH(Foo);
REFLECTIVE_RAPIDJSON_TREAT_AS_SET(Bar);
REFLECTIVE_RAPIDJSON_TREAT_AS_MULTI_SET(Foo);
} // namespace ReflectiveRapidJSON

using namespace std;
using namespace ReflectiveRapidJSON;

// test traits
static_assert(IsArray<vector<int>>::value, "vector mapped to array");
static_assert(IsArray<list<int>>::value, "list mapped to array");
static_assert(!IsArray<set<int>>::value, "set not considered an array");
static_assert(!IsArray<multiset<int>>::value, "multiset not considered an array");
static_assert(IsArrayOrSet<set<int>>::value, "set is array or set");
static_assert(IsArrayOrSet<multiset<int>>::value, "multiset is array or set");
static_assert(IsArrayOrSet<Foo>::value, "Foo is array or set via TreatAsMultiSet");
static_assert(IsArrayOrSet<Bar>::value, "Foo is array or set via TreatAsSet");
static_assert(!IsArrayOrSet<string>::value, "string not mapped to array or set though it is iteratable");
static_assert(IsSet<set<int>>::value, "set mapped to set");
static_assert(IsSet<unordered_set<int>>::value, "unordered_set mapped to set");
static_assert(IsSet<Bar>::value, "Bar mapped to set via TreatAsSet");
static_assert(!IsSet<string>::value, "string not mapped to set");
static_assert(IsMultiSet<unordered_multiset<int>>::value, "multiset");
static_assert(IsMultiSet<Foo>::value, "Foo mapped to multiset via TreatAsMultiSet");
static_assert(!IsMultiSet<string>::value, "string not mapped to multiset");
static_assert(!IsArray<string>::value, "string not mapped to array though it is iteratable");
static_assert(IsMapOrHash<map<string, int>>::value, "map mapped to object");
static_assert(IsMapOrHash<unordered_map<string, int>>::value, "hash mapped to object");
static_assert(!IsMapOrHash<vector<int>>::value, "vector not mapped to object");
static_assert(IsMapOrHash<Foo>::value, "Foo mapped to object via TreatAsMapOrHash");
static_assert(IsMultiMapOrHash<multimap<string, int>>::value, "multimap mapped to object");
static_assert(IsMultiMapOrHash<unordered_multimap<string, int>>::value, "unordered multimap mapped to object");
static_assert(!IsMultiMapOrHash<vector<int>>::value, "vector not mapped to object");
static_assert(IsMultiMapOrHash<Foo>::value, "Foo mapped to object via TreatAsMultiMapOrHash");
static_assert(IsIteratableExceptString<std::vector<int>>::value, "vector is iteratable");
static_assert(!IsIteratableExceptString<std::string>::value, "string not iteratable");
static_assert(!IsIteratableExceptString<std::wstring>::value, "wstring not iteratable");
static_assert(!IsIteratableExceptString<const std::string>::value, "string not iteratable");

// test versioning traits
static_assert(!Versioning<int>::enabled, "versioning for built-in types not enabled");
static_assert(!Versioning<std::string>::enabled, "versioning for standard types not enabled");
static_assert(!Versioning<VersionlessBase>::enabled, "versioning not enabled by default");
static_assert(Versioning<BinarySerializable<VersionedDerived, 1>>::enabled, "versioning enabled if non-zero version parameter specified (derived)");
static_assert(Versioning<VersionedBase>::enabled, "versioning enabled if non-zero version parameter specified (base)");
static_assert(!Versioning<BinarySerializable<VersionlessDerived>>::enabled, "versioning disabled for derived, even if base is versioned");
static_assert(!Versioning<BinarySerializable<Foo, 0>>::enabled, "versioning disabled if zero-version specified");
static_assert(Versioning<BinarySerializable<Foo, 3>>::applyDefaultValue(0) == 3, "default version returned");
static_assert(Versioning<BinarySerializable<Foo, 3>>::applyDefaultValue(2) == 2, "default version overridden");
