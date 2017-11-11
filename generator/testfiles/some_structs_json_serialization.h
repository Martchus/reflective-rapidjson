namespace ReflectiveRapidJSON {
namespace JsonReflector {

// define code for (de)serializing TestNamespace1::Person objects
template <>  void push<::TestNamespace1::Person>(const ::TestNamespace1::Person &reflectable, ::RAPIDJSON_NAMESPACE::Value::Object &value, ::RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    // push base classes
    // push members
    push(reflectable.age, "age", value, allocator);
    push(reflectable.alive, "alive", value, allocator);
}
template <>  void pull<::TestNamespace1::Person>(::TestNamespace1::Person &reflectable, const ::RAPIDJSON_NAMESPACE::GenericValue<::RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value, JsonDeserializationErrors *errors)
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

// define code for (de)serializing TestNamespace2::ThirdPartyStruct objects
template <>  void push<::TestNamespace2::ThirdPartyStruct>(const ::TestNamespace2::ThirdPartyStruct &reflectable, ::RAPIDJSON_NAMESPACE::Value::Object &value, ::RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    // push base classes
    // push members
    push(reflectable.test1, "test1", value, allocator);
    push(reflectable.test2, "test2", value, allocator);
}
template <>  void pull<::TestNamespace2::ThirdPartyStruct>(::TestNamespace2::ThirdPartyStruct &reflectable, const ::RAPIDJSON_NAMESPACE::GenericValue<::RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value, JsonDeserializationErrors *errors)
{
    // pull base classes
    // set error context for current record
    const char *previousRecord;
    if (errors) {
        previousRecord = errors->currentRecord;
        errors->currentRecord = "TestNamespace2::ThirdPartyStruct";
    }
    // pull members
    pull(reflectable.test1, "test1", value, errors);
    pull(reflectable.test2, "test2", value, errors);
    // restore error context for previous record
    if (errors) {
        errors->currentRecord = previousRecord;
    }
}

} // namespace JsonReflector
} // namespace ReflectiveRapidJSON
