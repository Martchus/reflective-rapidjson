#include "../binary/reflector-chronoutilities.h"
#include "../binary/reflector.h"
#include "../binary/serializable.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/misc.h>
#include <c++utilities/tests/testutils.h>

using TestUtilities::operator<<; // must be visible prior to the call site
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace CPPUNIT_NS;
using namespace IoUtilities;
using namespace ChronoUtilities;
using namespace ConversionUtilities;
using namespace TestUtilities;
using namespace TestUtilities::Literals;
using namespace ReflectiveRapidJSON;

/// \cond

// define some enums and structs for testing serialization
enum SomeEnum {
    SomeEnumItem1,
    SomeEnumItem2,
    SomeEnumItem3,
};

enum class SomeEnumClass : uint16 {
    Item1,
    Item2,
    Item3,
};

struct TestObjectBinary : public BinarySerializable<TestObjectBinary> {
    int number;
    double number2;
    vector<int> numbers;
    string text;
    bool boolean;
    map<string, int> someMap;
    unordered_map<string, bool> someHash;
    set<string> someSet;
    multiset<string> someMultiset;
    unordered_set<string> someUnorderedSet;
    unordered_multiset<string> someUnorderedMultiset;
    SomeEnum someEnum;
    SomeEnumClass someEnumClass;
    TimeSpan timeSpan;
    DateTime dateTime;
};

struct NestingArrayBinary : public BinarySerializable<NestingArrayBinary> {
    string name;
    vector<TestObjectBinary> testObjects;
};

// pretend serialization code for structs has been generated
namespace ReflectiveRapidJSON {
namespace BinaryReflector {

template <> void readCustomType<TestObjectBinary>(BinaryDeserializer &deserializer, TestObjectBinary &customType)
{
    deserializer.read(customType.number);
    deserializer.read(customType.number2);
    deserializer.read(customType.numbers);
    deserializer.read(customType.text);
    deserializer.read(customType.boolean);
    deserializer.read(customType.someMap);
    deserializer.read(customType.someHash);
    deserializer.read(customType.someSet);
    deserializer.read(customType.someMultiset);
    deserializer.read(customType.someUnorderedSet);
    deserializer.read(customType.someUnorderedMultiset);
    deserializer.read(customType.someEnum);
    deserializer.read(customType.someEnumClass);
    deserializer.read(customType.timeSpan);
    deserializer.read(customType.dateTime);
}

template <> void writeCustomType<TestObjectBinary>(BinarySerializer &serializer, const TestObjectBinary &customType)
{
    serializer.write(customType.number);
    serializer.write(customType.number2);
    serializer.write(customType.numbers);
    serializer.write(customType.text);
    serializer.write(customType.boolean);
    serializer.write(customType.someMap);
    serializer.write(customType.someHash);
    serializer.write(customType.someSet);
    serializer.write(customType.someMultiset);
    serializer.write(customType.someUnorderedSet);
    serializer.write(customType.someUnorderedMultiset);
    serializer.write(customType.someEnum);
    serializer.write(customType.someEnumClass);
    serializer.write(customType.timeSpan);
    serializer.write(customType.dateTime);
}

template <> void readCustomType<NestingArrayBinary>(BinaryDeserializer &deserializer, NestingArrayBinary &customType)
{
    deserializer.read(customType.name);
    deserializer.read(customType.testObjects);
}

template <> void writeCustomType<NestingArrayBinary>(BinarySerializer &serializer, const NestingArrayBinary &customType)
{
    serializer.write(customType.name);
    serializer.write(customType.testObjects);
}

} // namespace BinaryReflector

// namespace BinaryReflector
} // namespace ReflectiveRapidJSON

/// \endcond

/*!
 * \brief The BinaryReflectorTests class tests the (de)serializer.
 * \remarks In these tests, the required reflection code is provided by hand so the generator isn't involved yet.
 */
class BinaryReflectorTests : public TestFixture {
    CPPUNIT_TEST_SUITE(BinaryReflectorTests);
    CPPUNIT_TEST(testSerializeSimpleStruct);
    CPPUNIT_TEST(testDeserializeSimpleStruct);
    CPPUNIT_TEST(testSerializeNestedStruct);
    CPPUNIT_TEST(testDeserializeNestedStruct);
    CPPUNIT_TEST_SUITE_END();

public:
    BinaryReflectorTests();

    void setUp();
    void tearDown();

    void testSerializeSimpleStruct();
    void testDeserializeSimpleStruct();
    void testSerializeNestedStruct();
    void testDeserializeNestedStruct();
    void assertTestObject(const TestObjectBinary &deserialized);

private:
    vector<unsigned char> m_buffer;
    TestObjectBinary m_testObj;
    NestingArrayBinary m_nestedTestObj;
    vector<unsigned char> m_expectedTestObj;
    vector<unsigned char> m_expectedNestedTestObj;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BinaryReflectorTests);

// clang-format off
BinaryReflectorTests::BinaryReflectorTests()
    : m_buffer()
    , m_testObj()
    , m_nestedTestObj()
    , m_expectedTestObj({
               0x00, 0x00, 0x00, 0x05,
               0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
               0x85,
                 0x00, 0x00, 0x00, 0x01,
                 0x00, 0x00, 0x00, 0x02,
                 0x00, 0x00, 0x00, 0x03,
                 0x00, 0x00, 0x00, 0x04,
                 0x00, 0x00, 0x00, 0x05,
               0x89,
                 0x73, 0x6F, 0x6D, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74,
               0x01,
               0x82,
                 0x83, 0x62, 0x61, 0x72, 0x00, 0x00, 0x00, 0x13,
                 0x83, 0x66, 0x6f, 0x6f, 0x00, 0x00, 0x00, 0x11,
               0x80,
               0x83,
                 0x81, 0x31,
                 0x81, 0x32,
                 0x81, 0x33,
               0x84,
                 0x81, 0x31,
                 0x81, 0x32,
                 0x81, 0x32,
                 0x81, 0x33,
               0x80,
               0x80,
               0x00, 0x00, 0x00, 0x01,
               0x00, 0x02,
               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xCD,
               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0xAB,
           })
    , m_expectedNestedTestObj({
               0x93, 0x73, 0x74, 0x72, 0x75, 0x63, 0x74, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20, 0x6e, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67,
               0x82,
                              })
{
}
// clang-format on

void BinaryReflectorTests::setUp()
{
    m_testObj.number = 5;
    m_testObj.number2 = 2.5;
    m_testObj.numbers = { 1, 2, 3, 4, 5 };
    m_testObj.text = "some text";
    m_testObj.boolean = true;
    m_testObj.someMap = {
        { "foo", 17 },
        { "bar", 19 },
    };
    m_testObj.someSet = { "1", "2", "3", "2" };
    m_testObj.someMultiset = { "1", "2", "3", "2" };
    m_testObj.someEnum = SomeEnumItem2;
    m_testObj.someEnumClass = SomeEnumClass::Item3;
    m_testObj.timeSpan = TimeSpan(0xABCD);
    m_testObj.dateTime = DateTime(0xEFAB);
    m_nestedTestObj.name = "struct with nesting";
    m_expectedNestedTestObj.reserve(m_expectedNestedTestObj.size() + 2 * m_expectedTestObj.size());
    m_expectedNestedTestObj.insert(m_expectedNestedTestObj.end(), m_expectedTestObj.cbegin(), m_expectedTestObj.cend());
    m_expectedNestedTestObj.insert(m_expectedNestedTestObj.end(), m_expectedTestObj.cbegin(), m_expectedTestObj.cend());
    m_nestedTestObj.testObjects.insert(m_nestedTestObj.testObjects.end(), 2, m_testObj);
}

void BinaryReflectorTests::tearDown()
{
}

void BinaryReflectorTests::testSerializeSimpleStruct()
{
    stringstream stream(ios_base::out | ios_base::binary);
    stream.exceptions(ios_base::failbit | ios_base::badbit);
    m_buffer.resize(m_expectedTestObj.size());
    stream.rdbuf()->pubsetbuf(reinterpret_cast<char *>(m_buffer.data()), static_cast<streamsize>(m_buffer.size()));
    m_testObj.toBinary(stream);

    CPPUNIT_ASSERT_EQUAL(m_expectedTestObj, m_buffer);
}

void BinaryReflectorTests::testDeserializeSimpleStruct()
{
    stringstream stream(ios_base::in | ios_base::binary);
    stream.exceptions(ios_base::failbit | ios_base::badbit);
    stream.rdbuf()->pubsetbuf(reinterpret_cast<char *>(m_expectedTestObj.data()), static_cast<streamsize>(m_expectedTestObj.size()));
    const auto deserialized(TestObjectBinary::fromBinary(stream));
    assertTestObject(deserialized);
}

void BinaryReflectorTests::testSerializeNestedStruct()
{
    stringstream stream(ios_base::out | ios_base::binary);
    stream.exceptions(ios_base::failbit | ios_base::badbit);
    m_buffer.resize(m_expectedNestedTestObj.size());
    stream.rdbuf()->pubsetbuf(reinterpret_cast<char *>(m_buffer.data()), static_cast<streamsize>(m_buffer.size()));
    m_nestedTestObj.toBinary(stream);

    CPPUNIT_ASSERT_EQUAL(m_expectedNestedTestObj, m_buffer);
}

void BinaryReflectorTests::testDeserializeNestedStruct()
{
    stringstream stream(ios_base::in | ios_base::binary);
    stream.exceptions(ios_base::failbit | ios_base::badbit);
    stream.rdbuf()->pubsetbuf(reinterpret_cast<char *>(m_expectedNestedTestObj.data()), static_cast<streamsize>(m_expectedNestedTestObj.size()));

    const auto deserialized(NestingArrayBinary::fromBinary(stream));
    CPPUNIT_ASSERT_EQUAL(m_nestedTestObj.name, deserialized.name);
    for (const auto &testObj : deserialized.testObjects) {
        assertTestObject(testObj);
    }
}

void BinaryReflectorTests::assertTestObject(const TestObjectBinary &deserialized)
{
    CPPUNIT_ASSERT_EQUAL(m_testObj.number, deserialized.number);
    CPPUNIT_ASSERT_EQUAL(m_testObj.number2, deserialized.number2);
    CPPUNIT_ASSERT_EQUAL(m_testObj.numbers, deserialized.numbers);
    CPPUNIT_ASSERT_EQUAL(m_testObj.text, deserialized.text);
    CPPUNIT_ASSERT_EQUAL(m_testObj.boolean, deserialized.boolean);
    CPPUNIT_ASSERT_EQUAL(m_testObj.someMap, deserialized.someMap);
    CPPUNIT_ASSERT_EQUAL(m_testObj.someHash, deserialized.someHash);
    CPPUNIT_ASSERT_EQUAL(m_testObj.someSet, deserialized.someSet);
    CPPUNIT_ASSERT_EQUAL(m_testObj.someMultiset, deserialized.someMultiset);
    CPPUNIT_ASSERT_EQUAL(m_testObj.someUnorderedSet, deserialized.someUnorderedSet);
    CPPUNIT_ASSERT_EQUAL(m_testObj.someUnorderedMultiset, deserialized.someUnorderedMultiset);
}
