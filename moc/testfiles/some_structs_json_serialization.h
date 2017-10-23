namespace ReflectiveRapidJSON {
namespace Reflector {

template <> inline void push<::TestNamespace1::Person>(const ::TestNamespace1::Person &reflectable, ::RAPIDJSON_NAMESPACE::Value::Object &value, ::RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    push(reflectable.age, "age", value, allocator);
    push(reflectable.alive, "alive", value, allocator);
}
template <> inline void pull<::TestNamespace1::Person>(::TestNamespace1::Person &reflectable, const ::RAPIDJSON_NAMESPACE::GenericValue<::RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value)
{
    pull(reflectable.age, "age", value);
    pull(reflectable.alive, "alive", value);
}

} // namespace Reflector
} // namespace ReflectiveRapidJSON
