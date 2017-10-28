namespace ReflectiveRapidJSON {
namespace Reflector {

// define code for (de)serializing TestNamespace1::Person objects
template <> inline void push<::TestNamespace1::Person>(const ::TestNamespace1::Person &reflectable, ::RAPIDJSON_NAMESPACE::Value::Object &value, ::RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    // push base classes
    // push members
    push(reflectable.age, "age", value, allocator);
    push(reflectable.alive, "alive", value, allocator);
}
template <> inline void pull<::TestNamespace1::Person>(::TestNamespace1::Person &reflectable, const ::RAPIDJSON_NAMESPACE::GenericValue<::RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value, JSONParseErrors *errors)
{
    // pull base classes
    // set error context for current record
    const char *previousRecord;
    if (errors) {
        previousRecord = errors->currentRecord;
        errors->currentRecord = "TestNamespace1::Person";
    }
    // pull members
    pull(reflectable.age, "age", value, errors);
    pull(reflectable.alive, "alive", value, errors);
    // restore error context for previous record
    if (errors) {
        errors->currentRecord = previousRecord;
    }
}

} // namespace Reflector
} // namespace ReflectiveRapidJSON
