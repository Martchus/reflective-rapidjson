#include "../jsonreflector-boosthana.h"
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
struct TestObjectHana : public JSONSerializable<TestObjectHana> {
    //TestObjectHana(){};
    //TestObjectHana(const TestObjectHana &)
    //{
    //    std::cout << "copied!!" << std::endl;
    //};
    BOOST_HANA_DEFINE_STRUCT(TestObjectHana, (int, number), (double, number2), (vector<int>, numbers), (string, text), (bool, boolean));
};

struct NestingObjectHana : public JSONSerializable<NestingObjectHana> {
    BOOST_HANA_DEFINE_STRUCT(NestingObjectHana, (string, name), (TestObjectHana, testObj));
};

struct NestingArrayHana : public JSONSerializable<NestingArrayHana> {
    BOOST_HANA_DEFINE_STRUCT(NestingArrayHana, (string, name), (vector<TestObjectHana>, testObjects));
};

/// \endcond

/*!
 * \brief The ReflectorTests class tests RapidJSON wrapper which is used to ease code generation.
 * \remarks In this tests, no reflection or code generation is involved yet.
 */
class JSONReflectorBoostHanaTests : public TestFixture {
    CPPUNIT_TEST_SUITE(JSONReflectorBoostHanaTests);
    CPPUNIT_TEST(testSerializePrimitives);
    CPPUNIT_TEST(testSerializeSimpleObjects);
    CPPUNIT_TEST(testSerializeNestedObjects);
    CPPUNIT_TEST(testDeserializePrimitives);
    CPPUNIT_TEST(testDeserializeSimpleObjects);
    CPPUNIT_TEST(testDeserializeNestedObjects);
    CPPUNIT_TEST(testHandlingTypeMismatch);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testSerializePrimitives();
    void testSerializeSimpleObjects();
    void testSerializeNestedObjects();
    void testDeserializePrimitives();
    void testDeserializeSimpleObjects();
    void testDeserializeNestedObjects();
    void testHandlingTypeMismatch();

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(JSONReflectorBoostHanaTests);

void JSONReflectorBoostHanaTests::setUp()
{
}

void JSONReflectorBoostHanaTests::tearDown()
{
}

/*!
 * \brief Tests serializing strings, numbers, arrays and boolean.
 */
void JSONReflectorBoostHanaTests::testSerializePrimitives()
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
void JSONReflectorBoostHanaTests::testSerializeSimpleObjects()
{
    TestObjectHana testObj;
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
void JSONReflectorBoostHanaTests::testSerializeNestedObjects()
{
    NestingObjectHana nestingObj;
    nestingObj.name = "nesting";
    TestObjectHana &testObj = nestingObj.testObj;
    testObj.number = 42;
    testObj.number2 = 3.141592653589793;
    testObj.numbers = { 1, 2, 3, 4 };
    testObj.text = "test";
    testObj.boolean = false;
    CPPUNIT_ASSERT_EQUAL(
        "{\"name\":\"nesting\",\"testObj\":{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}}"s,
        string(nestingObj.toJson().GetString()));
    NestingArrayHana nestingArray;
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
void JSONReflectorBoostHanaTests::testDeserializePrimitives()
{
    Document doc(kArrayType);

    doc.Parse("[\"a\", 5, 5e6, \"test\", true, 4.125, false]");
    auto array = doc.GetArray().begin();

    string str1, str2;
    int int1 = 0;
    bool bool1 = false, bool2 = true;
    float float1 = 0.0;
    double double1 = 0.0;
    JSONParseErrors errors;
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
void JSONReflectorBoostHanaTests::testDeserializeSimpleObjects()
{
    const TestObjectHana testObj(
        TestObjectHana::fromJson("{\"number\":42,\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}"));

    CPPUNIT_ASSERT_EQUAL(42, testObj.number);
    CPPUNIT_ASSERT_EQUAL(3.141592653589793, testObj.number2);
    CPPUNIT_ASSERT_EQUAL(vector<int>({ 1, 2, 3, 4 }), testObj.numbers);
    CPPUNIT_ASSERT_EQUAL("test"s, testObj.text);
    CPPUNIT_ASSERT_EQUAL(false, testObj.boolean);
}

/*!
 * \brief Tests deserializing nested objects and arrays.
 */
void JSONReflectorBoostHanaTests::testDeserializeNestedObjects()
{
    const NestingObjectHana nestingObj(NestingObjectHana::fromJson("{\"name\":\"nesting\",\"testObj\":{\"number\":42,\"number2\":3.141592653589793,"
                                                                   "\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}}"));
    const TestObjectHana &testObj = nestingObj.testObj;
    CPPUNIT_ASSERT_EQUAL("nesting"s, nestingObj.name);
    CPPUNIT_ASSERT_EQUAL(42, testObj.number);
    CPPUNIT_ASSERT_EQUAL(3.141592653589793, testObj.number2);
    CPPUNIT_ASSERT_EQUAL(vector<int>({ 1, 2, 3, 4 }), testObj.numbers);
    CPPUNIT_ASSERT_EQUAL("test"s, testObj.text);
    CPPUNIT_ASSERT_EQUAL(false, testObj.boolean);

    const NestingArrayHana nestingArray(
        NestingArrayHana::fromJson("{\"name\":\"nesting2\",\"testObjects\":[{\"number\":42,\"number2\":3.141592653589793,"
                                   "\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false},{\"number\":43,\"number2\":3."
                                   "141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}]}"));
    const vector<TestObjectHana> &testObjects = nestingArray.testObjects;
    CPPUNIT_ASSERT_EQUAL("nesting2"s, nestingArray.name);
    CPPUNIT_ASSERT_EQUAL(2_st, testObjects.size());
    CPPUNIT_ASSERT_EQUAL(42, testObjects[0].number);
    CPPUNIT_ASSERT_EQUAL(43, testObjects[1].number);
    for (const TestObjectHana &testObj : testObjects) {
        CPPUNIT_ASSERT_EQUAL(3.141592653589793, testObj.number2);
        CPPUNIT_ASSERT_EQUAL(vector<int>({ 1, 2, 3, 4 }), testObj.numbers);
        CPPUNIT_ASSERT_EQUAL("test"s, testObj.text);
        CPPUNIT_ASSERT_EQUAL(false, testObj.boolean);
    }
}

/*!
 * \brief Tests whether JSONParseError is thrown on type mismatch.
 */
void JSONReflectorBoostHanaTests::testHandlingTypeMismatch()
{
    JSONParseErrors errors;
    NestingArrayHana::fromJson("{\"name\":\"nesting2\",\"testObjects\":[{\"number\":42,\"number2\":3.141592653589793,"
                               "\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false},{\"number\":43,\"number2\":3."
                               "141592653589793,\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false}]}",
        &errors);
    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());

    NestingObjectHana::fromJson("{\"name\":\"nesting\",\"testObj\":{\"number\":\"42\",\"number2\":3.141592653589793,\"numbers\":[1,2,3,4],\"text\":"
                                "\"test\",\"boolean\":false}}",
        &errors);
    CPPUNIT_ASSERT_EQUAL(1_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(JSONParseErrorKind::TypeMismatch, errors.front().kind);
    CPPUNIT_ASSERT_EQUAL(JSONType::Number, errors.front().expectedType);
    CPPUNIT_ASSERT_EQUAL(JSONType::String, errors.front().actualType);
    CPPUNIT_ASSERT_EQUAL("number"s, string(errors.front().member));
    CPPUNIT_ASSERT_EQUAL("[document]"s, string(errors.front().record));
    errors.clear();

    NestingObjectHana::fromJson("{\"name\":\"nesting\",\"testObj\":{\"number\":42,\"number2\":3.141592653589793,\"numbers\":1,\"text\":"
                                "\"test\",\"boolean\":false}}",
        &errors);
    CPPUNIT_ASSERT_EQUAL(1_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(JSONParseErrorKind::TypeMismatch, errors.front().kind);
    CPPUNIT_ASSERT_EQUAL(JSONType::Array, errors.front().expectedType);
    CPPUNIT_ASSERT_EQUAL(JSONType::Number, errors.front().actualType);
    CPPUNIT_ASSERT_EQUAL("numbers"s, string(errors.front().member));
    CPPUNIT_ASSERT_EQUAL("[document]"s, string(errors.front().record));
    errors.clear();

    NestingObjectHana::fromJson("{\"name\":[],\"testObj\":\"this is not an object\"}", &errors);
    CPPUNIT_ASSERT_EQUAL(2_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(JSONParseErrorKind::TypeMismatch, errors.front().kind);
    CPPUNIT_ASSERT_EQUAL(JSONType::String, errors.front().expectedType);
    CPPUNIT_ASSERT_EQUAL(JSONType::Array, errors.front().actualType);
    CPPUNIT_ASSERT_EQUAL("name"s, string(errors.front().member));
    CPPUNIT_ASSERT_EQUAL("[document]"s, string(errors.front().record));
    CPPUNIT_ASSERT_EQUAL(JSONParseErrorKind::TypeMismatch, errors.back().kind);
    CPPUNIT_ASSERT_EQUAL(JSONType::Object, errors.back().expectedType);
    CPPUNIT_ASSERT_EQUAL(JSONType::String, errors.back().actualType);
    CPPUNIT_ASSERT_EQUAL("testObj"s, string(errors.back().member));
    CPPUNIT_ASSERT_EQUAL("[document]"s, string(errors.back().record));
    errors.clear();

    const NestingArrayHana nestingArray(
        NestingArrayHana::fromJson("{\"name\":\"nesting2\",\"testObjects\":[25,{\"number\":42,\"number2\":3.141592653589793,"
                                   "\"numbers\":[1,2,3,4],\"text\":\"test\",\"boolean\":false},\"foo\",{\"number\":43,\"number2\":3."
                                   "141592653589793,\"numbers\":[1,2,3,4,\"bar\"],\"text\":\"test\",\"boolean\":false}]}",
            &errors));
    CPPUNIT_ASSERT_EQUAL(3_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(JSONParseErrorKind::TypeMismatch, errors[0].kind);
    CPPUNIT_ASSERT_EQUAL(JSONType::Object, errors[0].expectedType);
    CPPUNIT_ASSERT_EQUAL(JSONType::Number, errors[0].actualType);
    CPPUNIT_ASSERT_EQUAL("testObjects"s, string(errors[0].member));
    CPPUNIT_ASSERT_EQUAL("[document]"s, string(errors[0].record));
    CPPUNIT_ASSERT_EQUAL(0_st, errors[0].index);
    CPPUNIT_ASSERT_EQUAL(JSONParseErrorKind::TypeMismatch, errors[1].kind);
    CPPUNIT_ASSERT_EQUAL(JSONType::Object, errors[1].expectedType);
    CPPUNIT_ASSERT_EQUAL(JSONType::String, errors[1].actualType);
    CPPUNIT_ASSERT_EQUAL(2_st, errors[1].index);
    CPPUNIT_ASSERT_EQUAL("testObjects"s, string(errors[1].member));
    CPPUNIT_ASSERT_EQUAL("[document]"s, string(errors[1].record));
    CPPUNIT_ASSERT_EQUAL(JSONParseErrorKind::TypeMismatch, errors[2].kind);
    CPPUNIT_ASSERT_EQUAL(JSONType::Number, errors[2].expectedType);
    CPPUNIT_ASSERT_EQUAL(JSONType::String, errors[2].actualType);
    CPPUNIT_ASSERT_EQUAL("numbers"s, string(errors[2].member));
    CPPUNIT_ASSERT_EQUAL("[document]"s, string(errors[2].record));
    CPPUNIT_ASSERT_EQUAL(4_st, errors[2].index);
    errors.clear();

    errors.throwOn = JSONParseErrors::ThrowOn::TypeMismatch;
    CPPUNIT_ASSERT_THROW(NestingObjectHana::fromJson("{\"name\":[],\"testObj\":\"this is not an object\"}", &errors), JSONParseError);
}
