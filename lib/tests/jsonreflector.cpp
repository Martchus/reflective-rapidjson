#include "../jsonreflector.h"
#include "../jsonserializable.h"

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
struct TestObject : public JSONSerializable<TestObject> {
    int number;
    double number2;
    vector<int> numbers;
    string text;
    bool boolean;
};

struct NestingObject : public JSONSerializable<NestingObject> {
    string name;
    TestObject testObj;
};

struct NestingArray : public JSONSerializable<NestingArray> {
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

template <> inline void pull<TestObject>(TestObject &reflectable, const GenericValue<UTF8<char>>::ConstObject &value)
{
    pull(reflectable.number, "number", value);
    pull(reflectable.number2, "number2", value);
    pull(reflectable.numbers, "numbers", value);
    pull(reflectable.text, "text", value);
    pull(reflectable.boolean, "boolean", value);
}

template <> inline void pull<NestingObject>(NestingObject &reflectable, const GenericValue<UTF8<char>>::ConstObject &value)
{
    pull(reflectable.name, "name", value);
    pull(reflectable.testObj, "testObj", value);
}

template <> inline void pull<NestingArray>(NestingArray &reflectable, const GenericValue<UTF8<char>>::ConstObject &value)
{
    pull(reflectable.name, "name", value);
    pull(reflectable.testObjects, "testObjects", value);
}

} // namespace Reflector
} // namespace ReflectiveRapidJSON

/// \endcond

/*!
 * \brief The ReflectorTests class tests RapidJSON wrapper which is used to ease code generation.
 * \remarks In this tests, no reflection or code generation is involved yet.
 */
class JSONReflectorTests : public TestFixture {
    CPPUNIT_TEST_SUITE(JSONReflectorTests);
    CPPUNIT_TEST(experiment);
    CPPUNIT_TEST(testSerializePrimitives);
    CPPUNIT_TEST(testSerializeSimpleObjects);
    CPPUNIT_TEST(testSerializeNestedObjects);
    CPPUNIT_TEST(testDeserializePrimitives);
    CPPUNIT_TEST(testDeserializeSimpleObjects);
    CPPUNIT_TEST(testDeserializeNestedObjects);
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

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(JSONReflectorTests);

void JSONReflectorTests::setUp()
{
}

void JSONReflectorTests::tearDown()
{
}

/*!
 * \brief Not a real test, just some assertions for experimenting with the RapidJSON library.
 */
void JSONReflectorTests::experiment()
{
    Document doc(kArrayType);
    Document::AllocatorType &alloc = doc.GetAllocator();

    /*
    doc.PushBack(25, alloc);
    doc.PushBack(26, alloc);
    doc.SetObject();
    doc.AddMember(StringRef("test"), 27, alloc);

    StringBuffer strbuf;
    Writer<StringBuffer> jsonWriter(strbuf);
    doc.Accept(jsonWriter);
    */

    doc.Parse("[\"a\", 5, \"test\", \"7\"]");
    GenericValue<UTF8<>>::Array a = doc.GetArray();
    CPPUNIT_ASSERT_EQUAL("a"s, string(a[0].GetString()));
    CPPUNIT_ASSERT_EQUAL(5, a[1].GetInt());
    //CPPUNIT_ASSERT_EQUAL(5, a[2].GetInt());
    //CPPUNIT_ASSERT_EQUAL(7, a[3].GetInt());
}

/*!
 * \brief Tests serializing strings, numbers, arrays and boolean.
 */
void JSONReflectorTests::testSerializePrimitives()
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
void JSONReflectorTests::testSerializeSimpleObjects()
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
void JSONReflectorTests::testSerializeNestedObjects()
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
void JSONReflectorTests::testDeserializePrimitives()
{
    Document doc(kArrayType);

    doc.Parse("[\"a\", 5, 5e6, \"test\", true, 4.125, false]");
    auto array = doc.GetArray().begin();

    string str1, str2;
    int int1 = 0;
    bool bool1 = false, bool2 = true;
    float float1 = 0.0;
    double double1 = 0.0;
    Reflector::pull(str1, array);
    Reflector::pull(int1, array);
    Reflector::pull(float1, array);
    Reflector::pull(str2, array);
    Reflector::pull(bool1, array);
    Reflector::pull(double1, array);
    Reflector::pull(bool2, array);

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
void JSONReflectorTests::testDeserializeSimpleObjects()
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
void JSONReflectorTests::testDeserializeNestedObjects()
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
