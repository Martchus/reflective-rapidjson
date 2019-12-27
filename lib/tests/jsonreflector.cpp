#include "../json/reflector.h"
#include "../json/serializable.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/misc.h>
#include <c++utilities/tests/testutils.h>

using CppUtilities::operator<<; // must be visible prior to the call site
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace CPPUNIT_NS;
using namespace RAPIDJSON_NAMESPACE;
using namespace CppUtilities;
using namespace CppUtilities::Literals;
using namespace ReflectiveRapidJSON;

/// \cond

// define some enums and structs for testing serialization

enum SomeEnum {
    SomeEnumItem1,
    SomeEnumItem2,
    SomeEnumItem3,
};

enum class SomeEnumClass {
    Item1,
    Item2,
    Item3,
};

struct TestObject : public JsonSerializable<TestObject> {
    int number;
    double number2;
    vector<int> numbers;
    string text;
    bool boolean;
    map<string, int> someMap;
    unordered_map<string, bool> someHash;
    multimap<string, int> someMultimap;
    unordered_multimap<string, int> someMultiHash;
    set<string> someSet;
    multiset<string> someMultiset;
    unordered_set<string> someUnorderedSet;
    unordered_multiset<string> someUnorderedMultiset;
    variant<monostate, string, int, float> someVariant;
    variant<string, int, float> anotherVariant;
    variant<string, int, float> yetAnotherVariant;
};

struct NestingObject : public JsonSerializable<NestingObject> {
    string name;
    TestObject testObj;
};

struct NestingArray : public JsonSerializable<NestingArray> {
    string name;
    vector<TestObject> testObjects;
};

// pretend serialization code for structs has been generated
namespace ReflectiveRapidJSON {
namespace JsonReflector {

template <> inline void push<TestObject>(const TestObject &reflectable, Value::Object &value, Document::AllocatorType &allocator)
{
    push(reflectable.number, "number", value, allocator);
    push(reflectable.number2, "number2", value, allocator);
    push(reflectable.numbers, "numbers", value, allocator);
    push(reflectable.text, "text", value, allocator);
    push(reflectable.boolean, "boolean", value, allocator);
    push(reflectable.someMap, "someMap", value, allocator);
    push(reflectable.someHash, "someHash", value, allocator);
    push(reflectable.someMultimap, "someMultimap", value, allocator);
    push(reflectable.someMultiHash, "someMultiHash", value, allocator);
    push(reflectable.someSet, "someSet", value, allocator);
    push(reflectable.someMultiset, "someMultiset", value, allocator);
    push(reflectable.someUnorderedSet, "someUnorderedSet", value, allocator);
    push(reflectable.someUnorderedMultiset, "someUnorderedMultiset", value, allocator);
    push(reflectable.someVariant, "someVariant", value, allocator);
    push(reflectable.anotherVariant, "anotherVariant", value, allocator);
    push(reflectable.yetAnotherVariant, "yetAnotherVariant", value, allocator);
}

template <> inline void push<NestingObject>(const NestingObject &reflectable, Value::Object &value, Document::AllocatorType &allocator)
{
    push(reflectable.name, "name", value, allocator);
    push(reflectable.testObj, "testObj", value, allocator);
}

template <> inline void push<NestingArray>(const NestingArray &reflectable, Value::Object &value, Document::AllocatorType &allocator)
{
    push(reflectable.name, "name", value, allocator);
    push(reflectable.testObjects, "testObjects", value, allocator);
}

template <>
inline void pull<TestObject>(TestObject &reflectable, const GenericValue<UTF8<char>>::ConstObject &value, JsonDeserializationErrors *errors)
{
    const char *previousRecord;
    if (errors) {
        previousRecord = errors->currentRecord;
        errors->currentRecord = "TestObject";
    }
    pull(reflectable.number, "number", value, errors);
    pull(reflectable.number2, "number2", value, errors);
    pull(reflectable.numbers, "numbers", value, errors);
    pull(reflectable.text, "text", value, errors);
    pull(reflectable.boolean, "boolean", value, errors);
    pull(reflectable.someMap, "someMap", value, errors);
    pull(reflectable.someHash, "someHash", value, errors);
    pull(reflectable.someMultimap, "someMultimap", value, errors);
    pull(reflectable.someMultiHash, "someMultiHash", value, errors);
    pull(reflectable.someSet, "someSet", value, errors);
    pull(reflectable.someMultiset, "someMultiset", value, errors);
    pull(reflectable.someUnorderedSet, "someUnorderedSet", value, errors);
    pull(reflectable.someUnorderedMultiset, "someUnorderedMultiset", value, errors);
    pull(reflectable.someVariant, "someVariant", value, errors);
    pull(reflectable.anotherVariant, "anotherVariant", value, errors);
    pull(reflectable.yetAnotherVariant, "yetAnotherVariant", value, errors);
    if (errors) {
        errors->currentRecord = previousRecord;
    }
}

template <>
inline void pull<NestingObject>(NestingObject &reflectable, const GenericValue<UTF8<char>>::ConstObject &value, JsonDeserializationErrors *errors)
{
    const char *previousRecord;
    if (errors) {
        previousRecord = errors->currentRecord;
        errors->currentRecord = "NestingObject";
    }
    pull(reflectable.name, "name", value, errors);
    pull(reflectable.testObj, "testObj", value, errors);
    if (errors) {
        errors->currentRecord = previousRecord;
    }
}

template <>
inline void pull<NestingArray>(NestingArray &reflectable, const GenericValue<UTF8<char>>::ConstObject &value, JsonDeserializationErrors *errors)
{
    const char *previousRecord;
    if (errors) {
        previousRecord = errors->currentRecord;
        errors->currentRecord = "NestingArray";
    }
    pull(reflectable.name, "name", value, errors);
    pull(reflectable.testObjects, "testObjects", value, errors);
    if (errors) {
        errors->currentRecord = previousRecord;
    }
}

} // namespace JsonReflector

// namespace JsonReflector
} // namespace ReflectiveRapidJSON

/// \endcond

/*!
 * \brief The JsonReflectorTests class tests RapidJSON wrapper which is used to ease code generation.
 * \remarks In these tests, the required reflection code is provided by hand so the generator isn't involved yet.
 */
class JsonReflectorTests : public TestFixture {
    CPPUNIT_TEST_SUITE(JsonReflectorTests);
    CPPUNIT_TEST(testSerializePrimitives);
    CPPUNIT_TEST(testSerializeSimpleObjects);
    CPPUNIT_TEST(testSerializeNestedObjects);
    CPPUNIT_TEST(testSerializeUniquePtr);
    CPPUNIT_TEST(testSerializeSharedPtr);
    CPPUNIT_TEST(testDeserializePrimitives);
    CPPUNIT_TEST(testDeserializeSimpleObjects);
    CPPUNIT_TEST(testDeserializeNestedObjects);
    CPPUNIT_TEST(testDeserializeUniquePtr);
    CPPUNIT_TEST(testDeserializeSharedPtr);
    CPPUNIT_TEST(testHandlingParseError);
    CPPUNIT_TEST(testHandlingTypeMismatch);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void experiment();
    void testSerializePrimitives();
    void testSerializeSimpleObjects();
    void testSerializeNestedObjects();
    void testSerializeUniquePtr();
    void testSerializeSharedPtr();
    void testDeserializePrimitives();
    void testDeserializeSimpleObjects();
    void testDeserializeNestedObjects();
    void testDeserializeUniquePtr();
    void testDeserializeSharedPtr();
    void testHandlingParseError();
    void testHandlingTypeMismatch();

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(JsonReflectorTests);

void JsonReflectorTests::setUp()
{
}

void JsonReflectorTests::tearDown()
{
}

/*!
 * \brief Tests serializing strings, numbers, arrays and boolean.
 */
void JsonReflectorTests::testSerializePrimitives()
{
    Document doc(kArrayType);
    Document::AllocatorType &alloc = doc.GetAllocator();
    doc.SetArray();
    Document::Array array(doc.GetArray());

    // string
    const string foo("foo"); // musn't be destroyed until JSON is actually written
    JsonReflector::push<string>(foo, array, alloc);
    JsonReflector::push<const char *>("bar", array, alloc);
    // number
    JsonReflector::push<int>(25, array, alloc);
    JsonReflector::push<double>(12.5, array, alloc);
    // enum
    JsonReflector::push<SomeEnum>(SomeEnumItem2, array, alloc);
    JsonReflector::push<SomeEnumClass>(SomeEnumClass::Item2, array, alloc);
    JsonReflector::push<SomeEnumClass>(SomeEnumClass::Item3, array, alloc);
    // array
    JsonReflector::push<vector<const char *>>({ "foo1", "bar1" }, array, alloc);
    JsonReflector::push<list<const char *>>({ "foo2", "bar2" }, array, alloc);
    JsonReflector::push<initializer_list<const char *>>({ "foo3", "bar3" }, array, alloc);
    JsonReflector::push<tuple<int, double>>(make_tuple(2, 413.0), array, alloc);
    // boolean
    JsonReflector::push<bool>(true, array, alloc);
    JsonReflector::push<bool>(false, array, alloc);

    StringBuffer strbuf;
    Writer<StringBuffer> jsonWriter(strbuf);
    doc.Accept(jsonWriter);
    CPPUNIT_ASSERT_EQUAL("[\"foo\",\"bar\",25,12.5,1,1,2,[\"foo1\",\"bar1\"],[\"foo2\",\"bar2\"],[\"foo3\",\"bar3\"],[2,413.0],true,false]"s,
        string(strbuf.GetString()));
}

/*!
 * \brief Tests serializing objects.
 */
void JsonReflectorTests::testSerializeSimpleObjects()
{
    TestObject testObj;
    testObj.number = 42;
    testObj.number2 = 3.141592653589793;
    testObj.numbers = { 1, 2, 3, 4 };
    testObj.text = "test";
    testObj.boolean = false;
    testObj.someMap = { { "a", 1 }, { "b", 2 } };
    testObj.someHash = { { "c", true }, { "d", false } };
    testObj.someMultimap = { { "a", 1 }, { "a", 2 }, { "b", 3 } };
    testObj.someMultiHash = { { "a", 1 } };
    testObj.someSet = { "a", "b", "c" };
    testObj.someMultiset = { "a", "b", "b" };
    testObj.someUnorderedSet = { "a" };
    testObj.someUnorderedMultiset = { "b", "b", "b" };
    testObj.someVariant = std::monostate{};
    testObj.anotherVariant = "foo";
    testObj.yetAnotherVariant = 42;
    CPPUNIT_ASSERT_EQUAL(
        "{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false,\"someMap\":{\"a\":1,\"b\":2},\"someHash\":{\"d\":false,\"c\":true},\"someMultimap\":{\"a\":[1,2],\"b\":[3]},\"someMultiHash\":{\"a\":[1]},\"someSet\":[\"a\",\"b\",\"c\"],\"someMultiset\":[\"a\",\"b\",\"b\"],\"someUnorderedSet\":[\"a\"],\"someUnorderedMultiset\":[\"b\",\"b\",\"b\"],\"someVariant\":{\"index\":0,\"data\":null},\"anotherVariant\":{\"index\":0,\"data\":\"foo\"},\"yetAnotherVariant\":{\"index\":1,\"data\":42}}"s,
        string(testObj.toJson().GetString()));
}

/*!
 * \brief Tests serializing nested object and arrays.
 */
void JsonReflectorTests::testSerializeNestedObjects()
{
    NestingObject nestingObj;
    nestingObj.name = "nesting";
    TestObject &testObj = nestingObj.testObj;
    testObj.number = 42;
    testObj.number2 = 3.141592653589793;
    testObj.numbers = { 1, 2, 3, 4 };
    testObj.text = "test";
    testObj.boolean = false;
    CPPUNIT_ASSERT_EQUAL(
        "{\"name\":\"nesting\",\"testObj\":{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false,\"someMap\":{},\"someHash\":{},\"someMultimap\":{},\"someMultiHash\":{},\"someSet\":[],\"someMultiset\":[],\"someUnorderedSet\":[],\"someUnorderedMultiset\":[],\"someVariant\":{\"index\":0,\"data\":null},\"anotherVariant\":{\"index\":0,\"data\":\"\"},\"yetAnotherVariant\":{\"index\":0,\"data\":\"\"}}}"s,
        string(nestingObj.toJson().GetString()));

    NestingArray nestingArray;
    nestingArray.name = "nesting2";
    nestingArray.testObjects.emplace_back(testObj);
    nestingArray.testObjects.emplace_back(testObj);
    nestingArray.testObjects.back().number = 43;
    CPPUNIT_ASSERT_EQUAL(
        "{\"name\":\"nesting2\",\"testObjects\":[{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false,\"someMap\":{},\"someHash\":{},\"someMultimap\":{},\"someMultiHash\":{},\"someSet\":[],\"someMultiset\":[],\"someUnorderedSet\":[],\"someUnorderedMultiset\":[],\"someVariant\":{\"index\":0,\"data\":null},\"anotherVariant\":{\"index\":0,\"data\":\"\"},\"yetAnotherVariant\":{\"index\":0,\"data\":\"\"}},{\"number\":43,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false,\"someMap\":{},\"someHash\":{},\"someMultimap\":{},\"someMultiHash\":{},\"someSet\":[],\"someMultiset\":[],\"someUnorderedSet\":[],\"someUnorderedMultiset\":[],\"someVariant\":{\"index\":0,\"data\":null},\"anotherVariant\":{\"index\":0,\"data\":\"\"},\"yetAnotherVariant\":{\"index\":0,\"data\":\"\"}}]}"s,
        string(nestingArray.toJson().GetString()));

    vector<TestObject> nestedInVector;
    nestedInVector.emplace_back(testObj);
    CPPUNIT_ASSERT_EQUAL(
        "[{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false,\"someMap\":{},\"someHash\":{},\"someMultimap\":{},\"someMultiHash\":{},\"someSet\":[],\"someMultiset\":[],\"someUnorderedSet\":[],\"someUnorderedMultiset\":[],\"someVariant\":{\"index\":0,\"data\":null},\"anotherVariant\":{\"index\":0,\"data\":\"\"},\"yetAnotherVariant\":{\"index\":0,\"data\":\"\"}}]"s,
        string(JsonReflector::toJson(nestedInVector).GetString()));
}

void JsonReflectorTests::testSerializeUniquePtr()
{
    Document doc(kArrayType);
    Document::AllocatorType &alloc = doc.GetAllocator();
    doc.SetArray();
    Document::Array array(doc.GetArray());

    const auto str = make_unique<string>("foo");
    std::unique_ptr<string> nullStr;
    const auto obj = make_unique<TestObject>();
    obj->number = 42;
    obj->number2 = 3.141592653589793;
    obj->numbers = { 1, 2, 3, 4 };
    obj->text = "bar";
    obj->boolean = false;

    JsonReflector::push(str, array, alloc);
    JsonReflector::push(nullStr, array, alloc);
    JsonReflector::push(obj, array, alloc);

    StringBuffer strbuf;
    Writer<StringBuffer> jsonWriter(strbuf);
    doc.Accept(jsonWriter);
    CPPUNIT_ASSERT_EQUAL(
        "[\"foo\",null,{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"bar\",\"boolean\":false,\"someMap\":{},\"someHash\":{},\"someMultimap\":{},\"someMultiHash\":{},\"someSet\":[],\"someMultiset\":[],\"someUnorderedSet\":[],\"someUnorderedMultiset\":[],\"someVariant\":{\"index\":0,\"data\":null},\"anotherVariant\":{\"index\":0,\"data\":\"\"},\"yetAnotherVariant\":{\"index\":0,\"data\":\"\"}}]"s,
        string(strbuf.GetString()));
}

void JsonReflectorTests::testSerializeSharedPtr()
{
    Document doc(kArrayType);
    Document::AllocatorType &alloc = doc.GetAllocator();
    doc.SetArray();
    Document::Array array(doc.GetArray());

    const auto str = make_shared<string>("foo");
    std::unique_ptr<string> nullStr;
    const auto obj = make_shared<TestObject>();
    obj->number = 42;
    obj->number2 = 3.141592653589793;
    obj->numbers = { 1, 2, 3, 4 };
    obj->text = "bar";
    obj->boolean = false;

    JsonReflector::push(str, array, alloc);
    JsonReflector::push(nullStr, array, alloc);
    JsonReflector::push(obj, array, alloc);

    StringBuffer strbuf;
    Writer<StringBuffer> jsonWriter(strbuf);
    doc.Accept(jsonWriter);
    CPPUNIT_ASSERT_EQUAL(
        "[\"foo\",null,{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"bar\",\"boolean\":false,\"someMap\":{},\"someHash\":{},\"someMultimap\":{},\"someMultiHash\":{},\"someSet\":[],\"someMultiset\":[],\"someUnorderedSet\":[],\"someUnorderedMultiset\":[],\"someVariant\":{\"index\":0,\"data\":null},\"anotherVariant\":{\"index\":0,\"data\":\"\"},\"yetAnotherVariant\":{\"index\":0,\"data\":\"\"}}]"s,
        string(strbuf.GetString()));
}

/*!
 * \brief Tests deserializing strings, numbers (int, float, double) and boolean.
 */
void JsonReflectorTests::testDeserializePrimitives()
{
    Document doc(kArrayType);

    doc.Parse("[\"a\", 5, 5.0, 5e6, 4, \"test\", true, 4.125, false]");
    auto array = doc.GetArray().begin();

    string str1, str2;
    int int1 = 0, int2 = 0;
    bool bool1 = false, bool2 = true;
    float float1 = 0.0f, float2 = 0.0f;
    double double1 = 0.0;
    JsonDeserializationErrors errors;
    JsonReflector::pull(str1, array, &errors);
    JsonReflector::pull(int1, array, &errors);
    JsonReflector::pull(int2, array, &errors);
    JsonReflector::pull(float1, array, &errors);
    JsonReflector::pull(float2, array, &errors);
    JsonReflector::pull(str2, array, &errors);
    JsonReflector::pull(bool1, array, &errors);
    JsonReflector::pull(double1, array, &errors);
    JsonReflector::pull(bool2, array, &errors);

    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());
    CPPUNIT_ASSERT_EQUAL("a"s, str1);
    CPPUNIT_ASSERT_EQUAL(5, int1);
    CPPUNIT_ASSERT_EQUAL(5, int2);
    CPPUNIT_ASSERT_EQUAL(5e6f, float1);
    CPPUNIT_ASSERT_EQUAL(4.f, float2);
    CPPUNIT_ASSERT_EQUAL("test"s, str2);
    CPPUNIT_ASSERT_EQUAL(true, bool1);
    CPPUNIT_ASSERT_EQUAL(4.125, double1);
    CPPUNIT_ASSERT_EQUAL(false, bool2);

    // deserialize primitives as tuple
    tuple<string, int, int, float, float, string, bool, double, bool> arrayAsTuple;
    JsonReflector::pull(arrayAsTuple, doc, &errors);
    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());
    CPPUNIT_ASSERT_EQUAL("a"s, get<0>(arrayAsTuple));
    CPPUNIT_ASSERT_EQUAL(5, get<1>(arrayAsTuple));
    CPPUNIT_ASSERT_EQUAL(5, get<2>(arrayAsTuple));
    CPPUNIT_ASSERT_EQUAL(5e6f, get<3>(arrayAsTuple));
    CPPUNIT_ASSERT_EQUAL(4.f, get<4>(arrayAsTuple));
    CPPUNIT_ASSERT_EQUAL("test"s, get<5>(arrayAsTuple));
    CPPUNIT_ASSERT_EQUAL(true, get<6>(arrayAsTuple));
    CPPUNIT_ASSERT_EQUAL(4.125, get<7>(arrayAsTuple));
    CPPUNIT_ASSERT_EQUAL(false, get<8>(arrayAsTuple));
    tuple<string, int> anotherTuple;
    JsonReflector::pull(anotherTuple, doc, &errors);
    CPPUNIT_ASSERT_EQUAL(1_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::ArraySizeMismatch, errors.front().kind);
}

/*!
 * \brief Tests deserializing simple objects.
 */
void JsonReflectorTests::testDeserializeSimpleObjects()
{
    const auto testObj
        = TestObject::fromJson("{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":"
                               "false,\"someMap\":{\"a\":1,\"b\":2},\"someHash\":{\"c\":true,\"d\":false},\"someMultimap\":{\"a\":[1,2],\"b\":[3]},"
                               "\"someMultiHash\":{\"a\":[4,5],\"b\":[6]},"
                               "\"someSet\":[\"a\",\"b\"],\"someMultiset\":["
                               "\"a\",\"a\"],\"someUnorderedSet\":[\"a\",\"b\"],\"someUnorderedMultiset\":[\"a\",\"a\"],\"someVariant\":{\"index\":0,"
                               "\"data\":null},\"anotherVariant\":{\"index\":0,\"data\":\"foo\"},\"yetAnotherVariant\":{\"index\":1,\"data\":42}}");

    CPPUNIT_ASSERT_EQUAL(42, testObj.number);
    CPPUNIT_ASSERT_EQUAL(3.141592653589793, testObj.number2);
    CPPUNIT_ASSERT_EQUAL(vector<int>({ 1, 2, 3, 4 }), testObj.numbers);
    CPPUNIT_ASSERT_EQUAL("test"s, testObj.text);
    CPPUNIT_ASSERT_EQUAL(false, testObj.boolean);
    const map<string, int> expectedMap{ { "a", 1 }, { "b", 2 } };
    CPPUNIT_ASSERT_EQUAL(expectedMap, testObj.someMap);
    const unordered_map<string, bool> expectedHash{ { "c", true }, { "d", false } };
    CPPUNIT_ASSERT_EQUAL(expectedHash, testObj.someHash);
    const multimap<string, int> expectedMultiMap{ { "a", 1 }, { "a", 2 }, { "b", 3 } };
    CPPUNIT_ASSERT_EQUAL(expectedMultiMap, testObj.someMultimap);
    const unordered_multimap<string, int> expectedUnorderedMultiMap{ { "a", 4 }, { "a", 5 }, { "b", 6 } };
    CPPUNIT_ASSERT_EQUAL(expectedUnorderedMultiMap, testObj.someMultiHash);
    CPPUNIT_ASSERT_EQUAL(set<string>({ "a", "b" }), testObj.someSet);
    CPPUNIT_ASSERT_EQUAL(multiset<string>({ "a", "a" }), testObj.someMultiset);
    CPPUNIT_ASSERT_EQUAL(unordered_set<string>({ "a", "b" }), testObj.someUnorderedSet);
    CPPUNIT_ASSERT_EQUAL(unordered_multiset<string>({ "a", "a" }), testObj.someUnorderedMultiset);
    CPPUNIT_ASSERT_EQUAL(0_st, testObj.someVariant.index());
    CPPUNIT_ASSERT_EQUAL(0_st, testObj.anotherVariant.index());
    CPPUNIT_ASSERT_EQUAL("foo"s, std::get<0>(testObj.anotherVariant));
    CPPUNIT_ASSERT_EQUAL(1_st, testObj.yetAnotherVariant.index());
    CPPUNIT_ASSERT_EQUAL(42, std::get<1>(testObj.yetAnotherVariant));
}

/*!
 * \brief Tests deserializing nested objects and arrays.
 */
void JsonReflectorTests::testDeserializeNestedObjects()
{
    JsonDeserializationErrors errors;
    const NestingObject nestingObj(NestingObject::fromJson("{\"name\":\"nesting\",\"testObj\":{\"number\":42,\"number2\":3.141592653589793,"
                                                           "\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}}",
        &errors));
    const TestObject &testObj = nestingObj.testObj;
    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());
    CPPUNIT_ASSERT_EQUAL("nesting"s, nestingObj.name);
    CPPUNIT_ASSERT_EQUAL(42, testObj.number);
    CPPUNIT_ASSERT_EQUAL(3.141592653589793, testObj.number2);
    CPPUNIT_ASSERT_EQUAL(vector<int>({ 1, 2, 3, 4 }), testObj.numbers);
    CPPUNIT_ASSERT_EQUAL("test"s, testObj.text);
    CPPUNIT_ASSERT_EQUAL(false, testObj.boolean);

    const NestingArray nestingArray(NestingArray::fromJson("{\"name\":\"nesting2\",\"testObjects\":[{\"number\":42,\"number2\":3.141592653589793,"
                                                           "\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false},{\"number\":43,\"number2\":3."
                                                           "141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}]}",
        &errors));
    const vector<TestObject> &testObjects = nestingArray.testObjects;
    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());
    CPPUNIT_ASSERT_EQUAL("nesting2"s, nestingArray.name);
    CPPUNIT_ASSERT_EQUAL(2_st, testObjects.size());
    CPPUNIT_ASSERT_EQUAL(42, testObjects[0].number);
    CPPUNIT_ASSERT_EQUAL(43, testObjects[1].number);
    for (const TestObject &testObj : testObjects) {
        CPPUNIT_ASSERT_EQUAL(3.141592653589793, testObj.number2);
        CPPUNIT_ASSERT_EQUAL(vector<int>({ 1, 2, 3, 4 }), testObj.numbers);
        CPPUNIT_ASSERT_EQUAL("test"s, testObj.text);
        CPPUNIT_ASSERT_EQUAL(false, testObj.boolean);
    }

    const auto nestedInVector(JsonReflector::fromJson<vector<TestObject>>(
        "[{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false,\"someMap\":{},\"someHash\":{}}]",
        &errors));
    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(1_st, nestedInVector.size());
    CPPUNIT_ASSERT_EQUAL(42, nestedInVector[0].number);
    CPPUNIT_ASSERT_EQUAL(4_st, nestedInVector[0].numbers.size());
    CPPUNIT_ASSERT_EQUAL("test"s, nestedInVector[0].text);
}

void JsonReflectorTests::testDeserializeUniquePtr()
{
    Document doc(kArrayType);
    doc.Parse("[\"foo\",null,{\"text\":\"bar\"}]");
    auto array = doc.GetArray().begin();

    unique_ptr<string> str;
    unique_ptr<string> nullStr;
    unique_ptr<TestObject> obj;
    JsonDeserializationErrors errors;
    JsonReflector::pull(str, array, &errors);
    JsonReflector::pull(nullStr, array, &errors);
    JsonReflector::pull(obj, array, &errors);

    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());
    CPPUNIT_ASSERT(str);
    CPPUNIT_ASSERT_EQUAL("foo"s, *str);
    CPPUNIT_ASSERT(!nullStr);
    CPPUNIT_ASSERT(obj);
    CPPUNIT_ASSERT_EQUAL("bar"s, obj->text);
}

void JsonReflectorTests::testDeserializeSharedPtr()
{
    Document doc(kArrayType);
    doc.Parse("[\"foo\",null,{\"text\":\"bar\"}]");
    auto array = doc.GetArray().begin();

    shared_ptr<string> str;
    shared_ptr<string> nullStr;
    shared_ptr<TestObject> obj;
    JsonDeserializationErrors errors;
    JsonReflector::pull(str, array, &errors);
    JsonReflector::pull(nullStr, array, &errors);
    JsonReflector::pull(obj, array, &errors);

    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());
    CPPUNIT_ASSERT(str);
    CPPUNIT_ASSERT_EQUAL("foo"s, *str);
    CPPUNIT_ASSERT(!nullStr);
    CPPUNIT_ASSERT(obj);
    CPPUNIT_ASSERT_EQUAL("bar"s, obj->text);
}

/*!
 * \brief Tests whether RAPIDJSON_NAMESPACE::ParseResult is thrown correctly when passing invalid JSON to fromJSON().
 */
void JsonReflectorTests::testHandlingParseError()
{
    try {
        NestingObject::fromJson("{\"name\":nesting\",\"testObj\":{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":"
                                "\"test\",\"boolean\":false}}");
        CPPUNIT_FAIL("expected ParseResult thrown");
    } catch (const RAPIDJSON_NAMESPACE::ParseResult &res) {
        CPPUNIT_ASSERT_EQUAL(RAPIDJSON_NAMESPACE::kParseErrorValueInvalid, res.Code());
        CPPUNIT_ASSERT_EQUAL(9_st, res.Offset());
    }
}

/*!
 * \brief Tests whether errors are added on type mismatch and in other cases.
 */
void JsonReflectorTests::testHandlingTypeMismatch()
{
    JsonDeserializationErrors errors;
    NestingArray::fromJson("{\"name\":\"nesting2\",\"testObjects\":[{\"number\":42,\"number2\":3.141592653589793,"
                           "\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false},{\"number\":43,\"number2\":3."
                           "141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}]}",
        &errors);
    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());

    NestingObject::fromJson("{\"name\":\"nesting\",\"testObj\":{\"number\":\"42\",\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":"
                            "\"test\",\"boolean\":false,\"someSet\":[\"a\",\"a\"],\"someMultiset\":[\"a\",\"a\"],\"someUnorderedSet\":[\"a\",\"a\"],"
                            "\"someUnorderedMultiset\":[\"a\",\"a\"]}}",
        &errors);
    CPPUNIT_ASSERT_EQUAL(3_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::TypeMismatch, errors.front().kind);
    CPPUNIT_ASSERT_EQUAL(JsonType::Number, errors.front().expectedType);
    CPPUNIT_ASSERT_EQUAL(JsonType::String, errors.front().actualType);
    CPPUNIT_ASSERT_EQUAL("number"s, string(errors.front().member));
    CPPUNIT_ASSERT_EQUAL("TestObject"s, string(errors.front().record));
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::UnexpectedDuplicate, errors[1].kind);
    CPPUNIT_ASSERT_EQUAL(JsonType::Array, errors[1].expectedType);
    CPPUNIT_ASSERT_EQUAL(JsonType::Array, errors[1].actualType);
    CPPUNIT_ASSERT_EQUAL("someSet"s, string(errors[1].member));
    CPPUNIT_ASSERT_EQUAL("TestObject"s, string(errors[1].record));
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::UnexpectedDuplicate, errors[2].kind);
    CPPUNIT_ASSERT_EQUAL(JsonType::Array, errors[2].expectedType);
    CPPUNIT_ASSERT_EQUAL(JsonType::Array, errors[2].actualType);
    CPPUNIT_ASSERT_EQUAL("someUnorderedSet"s, string(errors[2].member));
    CPPUNIT_ASSERT_EQUAL("TestObject"s, string(errors[2].record));
    errors.clear();

    NestingObject::fromJson("{\"name\":\"nesting\",\"testObj\":{\"number\":42,\"number2\":3.141592653589793,\"numbers\":1,\"text\":"
                            "\"test\",\"boolean\":false}}",
        &errors);
    CPPUNIT_ASSERT_EQUAL(1_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::TypeMismatch, errors.front().kind);
    CPPUNIT_ASSERT_EQUAL(JsonType::Array, errors.front().expectedType);
    CPPUNIT_ASSERT_EQUAL(JsonType::Number, errors.front().actualType);
    CPPUNIT_ASSERT_EQUAL("numbers"s, string(errors.front().member));
    CPPUNIT_ASSERT_EQUAL("TestObject"s, string(errors.front().record));
    errors.clear();

    NestingObject::fromJson("{\"name\":[],\"testObj\":\"this is not an object\"}", &errors);
    CPPUNIT_ASSERT_EQUAL(2_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::TypeMismatch, errors.front().kind);
    CPPUNIT_ASSERT_EQUAL(JsonType::String, errors.front().expectedType);
    CPPUNIT_ASSERT_EQUAL(JsonType::Array, errors.front().actualType);
    CPPUNIT_ASSERT_EQUAL("name"s, string(errors.front().member));
    CPPUNIT_ASSERT_EQUAL("NestingObject"s, string(errors.front().record));
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::TypeMismatch, errors.back().kind);
    CPPUNIT_ASSERT_EQUAL(JsonType::Object, errors.back().expectedType);
    CPPUNIT_ASSERT_EQUAL(JsonType::String, errors.back().actualType);
    CPPUNIT_ASSERT_EQUAL("testObj"s, string(errors.back().member));
    CPPUNIT_ASSERT_EQUAL("NestingObject"s, string(errors.back().record));
    errors.clear();

    const NestingArray nestingArray(
        NestingArray::fromJson("{\"name\":\"nesting2\",\"testObjects\":[25,{\"number\":42,\"number2\":3.141592653589793,"
                               "\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false},\"foo\",{\"number\":43,\"number2\":3."
                               "141592653589793,\"numbers\":[1,2,3,4,\"bar\"],\"text\":\"test\",\"boolean\":false}]}",
            &errors));
    CPPUNIT_ASSERT_EQUAL(3_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::TypeMismatch, errors[0].kind);
    CPPUNIT_ASSERT_EQUAL(JsonType::Object, errors[0].expectedType);
    CPPUNIT_ASSERT_EQUAL(JsonType::Number, errors[0].actualType);
    CPPUNIT_ASSERT_EQUAL("testObjects"s, string(errors[0].member));
    CPPUNIT_ASSERT_EQUAL("NestingArray"s, string(errors[0].record));
    CPPUNIT_ASSERT_EQUAL(0_st, errors[0].index);
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::TypeMismatch, errors[1].kind);
    CPPUNIT_ASSERT_EQUAL(JsonType::Object, errors[1].expectedType);
    CPPUNIT_ASSERT_EQUAL(JsonType::String, errors[1].actualType);
    CPPUNIT_ASSERT_EQUAL(2_st, errors[1].index);
    CPPUNIT_ASSERT_EQUAL("testObjects"s, string(errors[1].member));
    CPPUNIT_ASSERT_EQUAL("NestingArray"s, string(errors[1].record));
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::TypeMismatch, errors[2].kind);
    CPPUNIT_ASSERT_EQUAL(JsonType::Number, errors[2].expectedType);
    CPPUNIT_ASSERT_EQUAL(JsonType::String, errors[2].actualType);
    CPPUNIT_ASSERT_EQUAL("numbers"s, string(errors[2].member));
    CPPUNIT_ASSERT_EQUAL("TestObject"s, string(errors[2].record));
    CPPUNIT_ASSERT_EQUAL(4_st, errors[2].index);
    errors.clear();

    errors.throwOn = JsonDeserializationErrors::ThrowOn::TypeMismatch;
    CPPUNIT_ASSERT_THROW(NestingObject::fromJson("{\"name\":[],\"testObj\":\"this is not an object\"}", &errors), JsonDeserializationError);
}
