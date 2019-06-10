#include "./helper.h"
#include "./morestructs.h"
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
#include <memory>
#include <sstream>

using namespace CPPUNIT_NS;
using namespace CppUtilities;
using namespace CppUtilities::Literals;

/*!
 * \brief The BinaryGeneratorTests class tests the binary generator.
 */
class BinaryGeneratorTests : public TestFixture {
    CPPUNIT_TEST_SUITE(BinaryGeneratorTests);
    CPPUNIT_TEST(testSerializationAndDeserialization);
    CPPUNIT_TEST(testPointerHandling);
    CPPUNIT_TEST_SUITE_END();

public:
    BinaryGeneratorTests();
    void testSerializationAndDeserialization();
    void testPointerHandling();
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

/*!
 * \brief Tests handling of std::unique_ptr and std::shared_ptr.
 *
 * In particular, the same object referred by 2 related std::shared_ptr instances
 *
 * - should be serialized only once
 * - and result in 2 related std::shared_ptr instances again after deserialization.
 */
void BinaryGeneratorTests::testPointerHandling()
{
    PointerStruct ps;
    ps.s1 = make_shared<PointerTarget>(0xF1F2F3F3);
    ps.s2 = ps.s1;
    ps.s3 = make_shared<PointerTarget>(0xBBBBBBBB);
    ps.u1 = make_unique<PointerTarget>(0xF1F2F3F4);
    ps.u2 = make_unique<PointerTarget>(0xDDDDDDDD);
    ps.u3 = make_unique<PointerTarget>(0xEEEEEEEE);

    // check whether shared pointer are "wired" as expected
    ++ps.s1->n; // should affect s2 but not s3
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xF1F2F3F4), asHexNumber<uint32_t>(ps.s1->n));
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xF1F2F3F4), asHexNumber<uint32_t>(ps.s2->n));
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xBBBBBBBB), asHexNumber<uint32_t>(ps.s3->n));

    // serialize and deserialize
    stringstream stream(ios_base::in | ios_base::out | ios_base::binary);
    stream.exceptions(ios_base::failbit | ios_base::badbit);
    ps.toBinary(stream);
    const auto deserializedPs(PointerStruct::fromBinary(stream));

    // check shared pointer
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xF1F2F3F4), asHexNumber<uint32_t>(deserializedPs.s1->n));
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xF1F2F3F4), asHexNumber<uint32_t>(deserializedPs.s2->n));
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xBBBBBBBB), asHexNumber<uint32_t>(deserializedPs.s3->n));
    ++deserializedPs.s1->n; // should affect s2 but not s3
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xF1F2F3F5), asHexNumber<uint32_t>(deserializedPs.s1->n));
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xF1F2F3F5), asHexNumber<uint32_t>(deserializedPs.s2->n));
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xBBBBBBBB), asHexNumber<uint32_t>(deserializedPs.s3->n));

    // check unique pointer
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xF1F2F3F4), asHexNumber<uint32_t>(deserializedPs.u1->n));
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xDDDDDDDD), asHexNumber<uint32_t>(deserializedPs.u2->n));
    CPPUNIT_ASSERT_EQUAL(asHexNumber<uint32_t>(0xEEEEEEEE), asHexNumber<uint32_t>(deserializedPs.u3->n));
}
