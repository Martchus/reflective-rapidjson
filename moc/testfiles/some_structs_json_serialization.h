namespace ReflectiveRapidJSON {
namespace Reflector {
template <> inline void push<::TestNamespace1::Person>(const TestNamespace1::Person &reflectable, Value::Object &value, Document::AllocatorType &allocator)
{
    push(reflectable.age, "age", value, allocator);
    push(reflectable.alive, "alive", value, allocator);
}
template <> inline void pull<::TestNamespace1::Person>(TestNamespace1::Person &reflectable, const GenericValue<UTF8<char>>::ConstObject &value)
{
    pull(reflectable.age, "age", value);
    pull(reflectable.alive, "alive", value);
}

} // namespace Reflector
} // namespace ReflectiveRapidJSON
