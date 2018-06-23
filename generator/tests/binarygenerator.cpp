#include "./helper.h"
#include "./structs.h"

#include "../codefactory.h"
#include "../jsonserializationcodegenerator.h"

#include "resources/config.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/misc.h>
#include <c++utilities/tests/testutils.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>
#include <sstream>

using namespace CPPUNIT_NS;
using namespace IoUtilities;
using namespace TestUtilities;
using namespace TestUtilities::Literals;
using namespace ConversionUtilities;

/*!
 * \brief The BinaryGeneratorTests class tests the binary generator.
 */
class BinaryGeneratorTests : public TestFixture {
    CPPUNIT_TEST_SUITE(BinaryGeneratorTests);
    CPPUNIT_TEST(testSerializationAndDeserialization);
    CPPUNIT_TEST_SUITE_END();

public:
    BinaryGeneratorTests();
    void testSerializationAndDeserialization();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BinaryGeneratorTests);

BinaryGeneratorTests::BinaryGeneratorTests()
{
}

/*!
 * \brief Tests serializing some objects and deserialize them back.
 */
void BinaryGeneratorTests::testSerializationAndDeserialization()
{
    DerivedTestStruct obj;
    obj.someInt = 25;
    obj.someSize = 27;
    obj.someString = "foo";
    obj.someBool = true;

    stringstream stream(ios_base::in | ios_base::out | ios_base::binary);
    stream.exceptions(ios_base::failbit | ios_base::badbit);

    static_cast<BinarySerializable<DerivedTestStruct> &>(obj).toBinary(stream);

    const auto deserializedObj(BinarySerializable<DerivedTestStruct>::fromBinary(stream));
    CPPUNIT_ASSERT_EQUAL(obj.someInt, deserializedObj.someInt);
    CPPUNIT_ASSERT_EQUAL(obj.someSize, deserializedObj.someSize);
    CPPUNIT_ASSERT_EQUAL(obj.someString, deserializedObj.someString);
    CPPUNIT_ASSERT_EQUAL(obj.someBool, deserializedObj.someBool);
}
