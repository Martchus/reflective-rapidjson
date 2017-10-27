#include "./helper.h"
#include "./structs.h"

#include "../codefactory.h"

#include "resources/config.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/misc.h>
#include <c++utilities/tests/testutils.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

using namespace CPPUNIT_NS;
using namespace IoUtilities;
using namespace TestUtilities;
using namespace TestUtilities::Literals;
using namespace ConversionUtilities;

/*!
 * \brief The OverallTests class tests the overall functionality of the code generator (CLI and generator itself).
 */
class OverallTests : public TestFixture {
    CPPUNIT_TEST_SUITE(OverallTests);
    CPPUNIT_TEST(testGeneratorItself);
    CPPUNIT_TEST(testCLI);
    CPPUNIT_TEST(testIncludingGeneratedHeader);
    CPPUNIT_TEST(testNesting);
    CPPUNIT_TEST(testSingleInheritence);
    CPPUNIT_TEST(testMultipleInheritence);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testGeneratorItself();
    void testCLI();
    void testIncludingGeneratedHeader();
    void testNesting();
    void testSingleInheritence();
    void testMultipleInheritence();

private:
    vector<string> m_expectedCode;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OverallTests);

void OverallTests::setUp()
{
    m_expectedCode = toArrayOfLines(readFile(testFilePath("some_structs_json_serialization.h"), 1024));
}

void OverallTests::tearDown()
{
}

/*!
 * \brief Tests whether the generator works by using it directly.
 */
void OverallTests::testGeneratorItself()
{
    const string inputFilePath(testFilePath("some_structs.h"));
    const vector<const char *> inputFiles{ inputFilePath.data() };
    const vector<const char *> clangOptions{};

    stringstream buffer;
    CodeFactory factory(TestApplication::appPath(), inputFiles, clangOptions, buffer);
    factory.addGenerator<JSONSerializationCodeGenerator>();
    CPPUNIT_ASSERT(factory.run());
    assertEqualityLinewise(m_expectedCode, toArrayOfLines(buffer.str()));
}

/*!
 * \brief Test the generator via CLI.
 * \remarks Only available under UNIX (like) systems so far, because TESTUTILS_ASSERT_EXEC has not been implemented
 *          for other platforms.
 */
void OverallTests::testCLI()
{
#ifdef PLATFORM_UNIX
    string stdout, stderr;

    const string inputFilePath(testFilePath("some_structs.h"));
    const char *const args1[] = { PROJECT_NAME, "-i", inputFilePath.data(), nullptr };
    TESTUTILS_ASSERT_EXEC(args1);
    assertEqualityLinewise(m_expectedCode, toArrayOfLines(stdout));
#endif
}

/*!
 * \brief Tests whether the generated reflection code actually works.
 */
void OverallTests::testIncludingGeneratedHeader()
{
    TestStruct test;
    test.someInt = 42;
    test.someString = "the answer";
    test.yetAnotherString = "but what was the question";
    const string expectedJSON("{\"someInt\":42,\"someString\":\"the answer\",\"yetAnotherString\":\"but what was the question\"}");

    // test serialization
    CPPUNIT_ASSERT_EQUAL(expectedJSON, string(test.toJson().GetString()));

    // test deserialization
    const TestStruct parsedTest(TestStruct::fromJson(expectedJSON));
    CPPUNIT_ASSERT_EQUAL(test.someInt, parsedTest.someInt);
    CPPUNIT_ASSERT_EQUAL(test.someString, parsedTest.someString);
    CPPUNIT_ASSERT_EQUAL(test.yetAnotherString, parsedTest.yetAnotherString);
}

void OverallTests::testNesting()
{
    TestStruct test;
    test.someInt = 42;
    test.someString = "the answer";
    test.yetAnotherString = "but what was the question";
    NestedTestStruct nested;
    nested.nested.emplace_front(vector<TestStruct>{ test });
    nested.deq.emplace_back(3.14);
    const string expectedJSON(
        "{\"nested\":[[{\"someInt\":42,\"someString\":\"the answer\",\"yetAnotherString\":\"but what was the question\"}]],\"deq\":[3.14]}");

    // test serialization
    CPPUNIT_ASSERT_EQUAL(expectedJSON, string(nested.toJson().GetString()));

    // test deserialization
    const NestedTestStruct parsedNested(NestedTestStruct::fromJson(expectedJSON));
    CPPUNIT_ASSERT_EQUAL(1_st, parsedNested.nested.size());
    CPPUNIT_ASSERT_EQUAL(1_st, parsedNested.nested.front().size());
    CPPUNIT_ASSERT_EQUAL(1_st, parsedNested.deq.size());
    const TestStruct &parsedTest(parsedNested.nested.front().front());
    CPPUNIT_ASSERT_EQUAL(test.someInt, parsedTest.someInt);
    CPPUNIT_ASSERT_EQUAL(test.someString, parsedTest.someString);
    CPPUNIT_ASSERT_EQUAL(test.yetAnotherString, parsedTest.yetAnotherString);
    CPPUNIT_ASSERT_EQUAL(3.14, parsedNested.deq.front());
}

/*!
 * \brief Like testIncludingGeneratedHeader() but also tests single inheritence.
 */
void OverallTests::testSingleInheritence()
{
    DerivedTestStruct test;
    test.someInt = 42;
    test.someString = "the answer";
    test.yetAnotherString = "but what was the question";
    test.someBool = false;
    const string expectedJSONForBase("{\"someInt\":42,\"someString\":\"the answer\",\"yetAnotherString\":\"but what was the question\"}");
    const string expectedJSONForDerived(
        "{\"someInt\":42,\"someString\":\"the answer\",\"yetAnotherString\":\"but what was the question\",\"someBool\":false}");

    // test serialization
    CPPUNIT_ASSERT_EQUAL(expectedJSONForBase, string(as<TestStruct>(test).toJson().GetString()));
    CPPUNIT_ASSERT_EQUAL(expectedJSONForDerived, string(as<DerivedTestStruct>(test).toJson().GetString()));

    // test deserialization
    const DerivedTestStruct parsedTest(JSONSerializable<DerivedTestStruct>::fromJson(expectedJSONForDerived));
    CPPUNIT_ASSERT_EQUAL(test.someInt, parsedTest.someInt);
    CPPUNIT_ASSERT_EQUAL(test.someString, parsedTest.someString);
    CPPUNIT_ASSERT_EQUAL(test.yetAnotherString, parsedTest.yetAnotherString);
    CPPUNIT_ASSERT_EQUAL(test.someBool, parsedTest.someBool);
}

/*!
 * \brief Like testIncludingGeneratedHeader() but also tests multiple inheritence.
 */
void OverallTests::testMultipleInheritence()
{
    MultipleDerivedTestStruct test;
    test.someInt = 42;
    test.someString = "the answer";
    test.yetAnotherString = "but what was the question";
    test.someBool = false;
    test.arrayOfStrings = { "array", "of", "strings" };
    const string expectedJSONForDerived("{\"someInt\":42,\"someString\":\"the answer\",\"yetAnotherString\":\"but what was the "
                                        "question\",\"arrayOfStrings\":[\"array\",\"of\",\"strings\"],\"someBool\":false}");

    // test serialization
    CPPUNIT_ASSERT_EQUAL(expectedJSONForDerived, string(as<MultipleDerivedTestStruct>(test).toJson().GetString()));

    // test deserialization
    const MultipleDerivedTestStruct parsedTest(JSONSerializable<MultipleDerivedTestStruct>::fromJson(expectedJSONForDerived));
    CPPUNIT_ASSERT_EQUAL(test.someInt, parsedTest.someInt);
    CPPUNIT_ASSERT_EQUAL(test.someString, parsedTest.someString);
    CPPUNIT_ASSERT_EQUAL(test.yetAnotherString, parsedTest.yetAnotherString);
    CPPUNIT_ASSERT_EQUAL(test.someBool, parsedTest.someBool);
    CPPUNIT_ASSERT_EQUAL(test.arrayOfStrings, parsedTest.arrayOfStrings);
}

// include file required for reflection of TestStruct and other structs defined in structs.h
// NOTE: * generation of this header is triggered using the CMake function add_reflection_generator_invocation()
//       * the include must happen in exactly one translation unit of the project at a point where the structs are defined
#include "reflection/structs.h"

// this file should also be generated via add_reflection_generator_invocation() and hence includeable
// it is included to test the "empty" case when a unit doesn't contain relevant classes
#include "reflection/cppunit.h"
