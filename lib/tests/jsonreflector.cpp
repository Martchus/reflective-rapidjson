#include "../json/reflector.h"
#include "../json/serializable.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/misc.h>
#include <c++utilities/tests/testutils.h>

using TestUtilities::operator<<; // must be visible prior to the call site
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace CPPUNIT_NS;
using namespace RAPIDJSON_NAMESPACE;
using namespace IoUtilities;
using namespace ConversionUtilities;
using namespace TestUtilities;
using namespace TestUtilities::Literals;
using namespace ReflectiveRapidJSON;

/// \cond

// define some structs for testing serialization
struct TestObject : public JsonSerializable<TestObject> {
    int number;
    double number2;
    vector<int> numbers;
    string text;
    bool boolean;
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
namespace Reflector {

template <> inline void push<TestObject>(const TestObject &reflectable, Value::Object &value, Document::AllocatorType &allocator)
{
    push(reflectable.number, "number", value, allocator);
    push(reflectable.number2, "number2", value, allocator);
    push(reflectable.numbers, "numbers", value, allocator);
    push(reflectable.text, "text", value, allocator);
    push(reflectable.boolean, "boolean", value, allocator);
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

} // namespace Reflector

// namespace Reflector
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
    CPPUNIT_TEST(testDeserializePrimitives);
    CPPUNIT_TEST(testDeserializeSimpleObjects);
    CPPUNIT_TEST(testDeserializeNestedObjects);
    CPPUNIT_TEST(testHandlingParseError);
    CPPUNIT_TEST(testHandlingTypeMismatch);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void experiment();
    void testSerializePrimitives();
    void testSerializeSimpleObjects();
    void testSerializeNestedObjects();
    void testDeserializePrimitives();
    void testDeserializeSimpleObjects();
    void testDeserializeNestedObjects();
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
    Reflector::push<string>("foo"s, array, alloc);
    Reflector::push<const char *>("bar", array, alloc);
    // number
    Reflector::push<int>(25, array, alloc);
    Reflector::push<double>(12.5, array, alloc);
    // array
    Reflector::push<vector<const char *>>({ "foo1", "bar1" }, array, alloc);
    Reflector::push<list<const char *>>({ "foo2", "bar2" }, array, alloc);
    Reflector::push<initializer_list<const char *>>({ "foo3", "bar3" }, array, alloc);
    // boolean
    Reflector::push<bool>(true, array, alloc);
    Reflector::push<bool>(false, array, alloc);

    StringBuffer strbuf;
    Writer<StringBuffer> jsonWriter(strbuf);
    doc.Accept(jsonWriter);
    CPPUNIT_ASSERT_EQUAL(
        "[\"foo\",\"bar\",25,12.5,[\"foo1\",\"bar1\"],[\"foo2\",\"bar2\"],[\"foo3\",\"bar3\"],true,false]"s, string(strbuf.GetString()));
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
    CPPUNIT_ASSERT_EQUAL("{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}"s,
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
        "{\"name\":\"nesting\",\"testObj\":{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}}"s,
        string(nestingObj.toJson().GetString()));
    NestingArray nestingArray;
    nestingArray.name = "nesting2";
    nestingArray.testObjects.emplace_back(testObj);
    nestingArray.testObjects.emplace_back(testObj);
    nestingArray.testObjects.back().number = 43;
    CPPUNIT_ASSERT_EQUAL(
        "{\"name\":\"nesting2\",\"testObjects\":[{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false},{\"number\":43,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}]}"s,
        string(nestingArray.toJson().GetString()));
}

/*!
 * \brief Tests deserializing strings, numbers (int, float, double) and boolean.
 */
void JsonReflectorTests::testDeserializePrimitives()
{
    Document doc(kArrayType);

    doc.Parse("[\"a\", 5, 5e6, \"test\", true, 4.125, false]");
    auto array = doc.GetArray().begin();

    string str1, str2;
    int int1 = 0;
    bool bool1 = false, bool2 = true;
    float float1 = 0.0;
    double double1 = 0.0;
    JsonDeserializationErrors errors;
    Reflector::pull(str1, array, &errors);
    Reflector::pull(int1, array, &errors);
    Reflector::pull(float1, array, &errors);
    Reflector::pull(str2, array, &errors);
    Reflector::pull(bool1, array, &errors);
    Reflector::pull(double1, array, &errors);
    Reflector::pull(bool2, array, &errors);

    CPPUNIT_ASSERT_EQUAL("a"s, str1);
    CPPUNIT_ASSERT_EQUAL(5, int1);
    CPPUNIT_ASSERT_EQUAL(5e6f, float1);
    CPPUNIT_ASSERT_EQUAL("test"s, str2);
    CPPUNIT_ASSERT_EQUAL(true, bool1);
    CPPUNIT_ASSERT_EQUAL(4.125, double1);
    CPPUNIT_ASSERT_EQUAL(false, bool2);
}

/*!
 * \brief Tests deserializing simple objects.
 */
void JsonReflectorTests::testDeserializeSimpleObjects()
{
    const TestObject testObj(
        TestObject::fromJson("{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}"));

    CPPUNIT_ASSERT_EQUAL(42, testObj.number);
    CPPUNIT_ASSERT_EQUAL(3.141592653589793, testObj.number2);
    CPPUNIT_ASSERT_EQUAL(vector<int>({ 1, 2, 3, 4 }), testObj.numbers);
    CPPUNIT_ASSERT_EQUAL("test"s, testObj.text);
    CPPUNIT_ASSERT_EQUAL(false, testObj.boolean);
}

/*!
 * \brief Tests deserializing nested objects and arrays.
 */
void JsonReflectorTests::testDeserializeNestedObjects()
{
    const NestingObject nestingObj(NestingObject::fromJson("{\"name\":\"nesting\",\"testObj\":{\"number\":42,\"number2\":3.141592653589793,"
                                                           "\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}}"));
    const TestObject &testObj = nestingObj.testObj;
    CPPUNIT_ASSERT_EQUAL("nesting"s, nestingObj.name);
    CPPUNIT_ASSERT_EQUAL(42, testObj.number);
    CPPUNIT_ASSERT_EQUAL(3.141592653589793, testObj.number2);
    CPPUNIT_ASSERT_EQUAL(vector<int>({ 1, 2, 3, 4 }), testObj.numbers);
    CPPUNIT_ASSERT_EQUAL("test"s, testObj.text);
    CPPUNIT_ASSERT_EQUAL(false, testObj.boolean);

    const NestingArray nestingArray(NestingArray::fromJson("{\"name\":\"nesting2\",\"testObjects\":[{\"number\":42,\"number2\":3.141592653589793,"
                                                           "\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false},{\"number\":43,\"number2\":3."
                                                           "141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}]}"));
    const vector<TestObject> &testObjects = nestingArray.testObjects;
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
 * \brief Tests whether JsonDeserializationError is thrown on type mismatch.
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
                            "\"test\",\"boolean\":false}}",
        &errors);
    CPPUNIT_ASSERT_EQUAL(1_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(JsonDeserializationErrorKind::TypeMismatch, errors.front().kind);
    CPPUNIT_ASSERT_EQUAL(JsonType::Number, errors.front().expectedType);
    CPPUNIT_ASSERT_EQUAL(JsonType::String, errors.front().actualType);
    CPPUNIT_ASSERT_EQUAL("number"s, string(errors.front().member));
    CPPUNIT_ASSERT_EQUAL("TestObject"s, string(errors.front().record));
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
