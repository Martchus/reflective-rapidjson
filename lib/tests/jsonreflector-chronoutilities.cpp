#include "../json/reflector-chronoutilities.h"
#include "../json/serializable.h"

#include <c++utilities/tests/testutils.h>

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
using namespace ChronoUtilities;
using namespace TestUtilities::Literals;
using namespace ReflectiveRapidJSON;

/*!
 * \brief The JsonReflectorChronoUtilitiesTests class tests the custom (de)serialization of the chrono utilities provided
 *        by the C++ utilities library.
 * \remarks In these tests, the (de)serialization is customized so the code generator is not involved.
 */
class JsonReflectorChronoUtilitiesTests : public TestFixture {
    CPPUNIT_TEST_SUITE(JsonReflectorChronoUtilitiesTests);
    CPPUNIT_TEST(testSerializeing);
    CPPUNIT_TEST(testDeserializeing);
    CPPUNIT_TEST(testErroHandling);
    CPPUNIT_TEST_SUITE_END();

public:
    JsonReflectorChronoUtilitiesTests();

    void testSerializeing();
    void testDeserializeing();
    void testErroHandling();

private:
    const DateTime m_dateTime;
    const TimeSpan m_timeSpan;
    const string m_string;
};

CPPUNIT_TEST_SUITE_REGISTRATION(JsonReflectorChronoUtilitiesTests);

JsonReflectorChronoUtilitiesTests::JsonReflectorChronoUtilitiesTests()
    : m_dateTime(DateTime::fromDateAndTime(2017, 4, 2, 15, 31, 21, 165.125))
    , m_timeSpan(TimeSpan::fromHours(3.25) + TimeSpan::fromSeconds(19.125))
    , m_string("[\"2017-04-02T15:31:21.165125\",\"03:15:19.125\"]")
{
}

/*!
 * \brief Tests serializing DateTime and TimeSpan objects.
 */
void JsonReflectorChronoUtilitiesTests::testSerializeing()
{
    Document doc(kArrayType);
    Document::AllocatorType &alloc = doc.GetAllocator();
    doc.SetArray();
    Document::Array array(doc.GetArray());

    JsonReflector::push(m_dateTime, array, alloc);
    JsonReflector::push(m_timeSpan, array, alloc);

    StringBuffer strbuf;
    Writer<StringBuffer> jsonWriter(strbuf);
    doc.Accept(jsonWriter);
    CPPUNIT_ASSERT_EQUAL(m_string, string(strbuf.GetString()));
}

/*!
 * \brief Tests deserializing DateTime and TimeSpan objects.
 */
void JsonReflectorChronoUtilitiesTests::testDeserializeing()
{
    Document doc(kArrayType);

    doc.Parse(m_string.data(), m_string.size());
    auto array = doc.GetArray().begin();

    DateTime dateTime;
    TimeSpan timeSpan;
    JsonDeserializationErrors errors;
    JsonReflector::pull(dateTime, array, &errors);
    JsonReflector::pull(timeSpan, array, &errors);

    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(m_dateTime.toIsoString(), dateTime.toIsoString());
    CPPUNIT_ASSERT_EQUAL(m_timeSpan.toString(), timeSpan.toString());
}

/*!
 * \brief Tests deserializing DateTime and TimeSpan objects (error case).
 */
void JsonReflectorChronoUtilitiesTests::testErroHandling()
{
    Document doc(kArrayType);

    doc.Parse("[\"2017-04-31T15:31:21.165125\",\"03:15:19.125\"]");
    auto array = doc.GetArray().begin();

    DateTime dateTime;
    TimeSpan timeSpan;
    JsonDeserializationErrors errors;
    JsonReflector::pull(dateTime, array, &errors);
    JsonReflector::pull(timeSpan, array, &errors);

    CPPUNIT_ASSERT_EQUAL(1_st, errors.size());
    CPPUNIT_ASSERT(dateTime.isNull());
    CPPUNIT_ASSERT_EQUAL(m_timeSpan.toString(), timeSpan.toString());
}
