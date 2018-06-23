#include "../binary/reflector-boosthana.h"
#include "../binary/serializable.h"

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
struct TestObjectHana : public BinarySerializable<TestObjectHana> {
    BOOST_HANA_DEFINE_STRUCT(TestObjectHana, (int, number), (double, number2), (vector<int>, numbers), (string, text), (bool, boolean));
};

struct NestingObjectHana : public BinarySerializable<NestingObjectHana> {
    BOOST_HANA_DEFINE_STRUCT(NestingObjectHana, (string, name), (TestObjectHana, testObj));
};

struct NestingArrayHana : public BinarySerializable<NestingArrayHana> {
    BOOST_HANA_DEFINE_STRUCT(NestingArrayHana, (string, name), (vector<TestObjectHana>, testObjects));
};

/// \endcond

/*!
 * \brief The BinaryReflectorBoostHanaTests class tests the integration of Boost.Hana with the (de)serializer.
 * \remarks In these tests, the reflection is provided through Boost.Hana so the code generator is not involved.
 */
class BinaryReflectorBoostHanaTests : public TestFixture {
    CPPUNIT_TEST_SUITE(BinaryReflectorBoostHanaTests);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(BinaryReflectorBoostHanaTests);

void BinaryReflectorBoostHanaTests::setUp()
{
}

void BinaryReflectorBoostHanaTests::tearDown()
{
}
