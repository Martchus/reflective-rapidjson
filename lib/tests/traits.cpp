#include "../traits.h"

#include <list>
#include <vector>

using namespace std;
using namespace ReflectiveRapidJSON;

// test traits
static_assert(IsArray<vector<int>>::value, "vector mapped to array");
static_assert(IsArray<list<int>>::value, "list mapped to array");
static_assert(!IsArray<set<int>>::value, "set not considered an array");
static_assert(!IsArray<multiset<int>>::value, "multiset not considered an array");
static_assert(IsArrayOrSet<set<int>>::value, "set is array or set");
static_assert(IsArrayOrSet<multiset<int>>::value, "multiset is array or set");
static_assert(IsSet<unordered_set<int>>::value, "set");
static_assert(IsMultiSet<unordered_multiset<int>>::value, "multiset");
static_assert(!IsArray<string>::value, "string not mapped to array though it is iteratable");
static_assert(IsMapOrHash<map<string, int>>::value, "map mapped to object");
static_assert(IsMapOrHash<unordered_map<string, int>>::value, "hash mapped to object");
static_assert(!IsMapOrHash<vector<int>>::value, "vector not mapped to object");
