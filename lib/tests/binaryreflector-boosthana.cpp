#include "../binary/reflector-boosthana.h"
#include "../binary/serializable.h"

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
#include <string>
#include <vector>

using namespace std;
using namespace CPPUNIT_NS;
using namespace RAPIDJSON_NAMESPACE;
using namespace CppUtilities;
using namespace CppUtilities::Literals;
using namespace ReflectiveRapidJSON;

/// \cond

// define some structs for testing serialization
struct TestObjectBinaryHana : public BinarySerializable<TestObjectBinaryHana> {
    BOOST_HANA_DEFINE_STRUCT(TestObjectBinaryHana, (int, number), (double, number2), (vector<int>, numbers), (string, text), (bool, boolean));
};

struct NestingArrayBinaryHana : public BinarySerializable<NestingArrayBinaryHana> {
    BOOST_HANA_DEFINE_STRUCT(NestingArrayBinaryHana, (string, name), (vector<TestObjectBinaryHana>, testObjects));
};

/// \endcond

/*!
 * \brief The BinaryReflectorBoostHanaTests class tests the integration of Boost.Hana with the (de)serializer.
 * \remarks In these tests, the reflection is provided through Boost.Hana so the code generator is not involved.
 */
class BinaryReflectorBoostHanaTests : public TestFixture {
    CPPUNIT_TEST_SUITE(BinaryReflectorBoostHanaTests);
    CPPUNIT_TEST(testSerializingAndDeserializing);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testSerializingAndDeserializing();

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(BinaryReflectorBoostHanaTests);

void BinaryReflectorBoostHanaTests::setUp()
{
}

void BinaryReflectorBoostHanaTests::tearDown()
{
}

void BinaryReflectorBoostHanaTests::testSerializingAndDeserializing()
{
    TestObjectBinaryHana testObject;
    testObject.number = 42;
    testObject.number2 = 1234.25;
    testObject.numbers = { 1, 2, 3, 4, 5 };
    testObject.text = "foo";
    testObject.boolean = true;

    NestingArrayBinaryHana nestingObject;
    nestingObject.name = "bar";
    nestingObject.testObjects.emplace_back(testObject);

    stringstream stream(ios_base::in | ios_base::out | ios_base::binary);
    stream.exceptions(ios_base::failbit | ios_base::badbit);
    nestingObject.toBinary(stream);

    const auto deserializedObject(NestingArrayBinaryHana::fromBinary(stream));
    const auto &deserializedTestObj(deserializedObject.testObjects.at(0));
    CPPUNIT_ASSERT_EQUAL(nestingObject.name, deserializedObject.name);
    CPPUNIT_ASSERT_EQUAL(testObject.number, deserializedTestObj.number);
    CPPUNIT_ASSERT_EQUAL(testObject.number2, deserializedTestObj.number2);
    CPPUNIT_ASSERT_EQUAL(testObject.numbers, deserializedTestObj.numbers);
    CPPUNIT_ASSERT_EQUAL(testObject.text, deserializedTestObj.text);
    CPPUNIT_ASSERT_EQUAL(testObject.boolean, deserializedTestObj.boolean);
}
