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

using namespace CPPUNIT_NS;
using namespace IoUtilities;
using namespace TestUtilities;
using namespace TestUtilities::Literals;
using namespace ConversionUtilities;

/*!
 * \brief The OverallTests class tests the overall functionality of the code generator (CLI and generator itself).
 */
class JsonGeneratorTests : public TestFixture {
    CPPUNIT_TEST_SUITE(JsonGeneratorTests);
    CPPUNIT_TEST(testGeneratorItself);
    CPPUNIT_TEST(testCLI);
    CPPUNIT_TEST(testIncludingGeneratedHeader);
    CPPUNIT_TEST(testNesting);
    CPPUNIT_TEST(testSingleInheritence);
    CPPUNIT_TEST(testMultipleInheritence);
    CPPUNIT_TEST(testCustomSerialization);
    CPPUNIT_TEST(test3rdPartyAdaption);
    CPPUNIT_TEST_SUITE_END();

public:
    JsonGeneratorTests();
    void testGeneratorItself();
    void testCLI();
    void testIncludingGeneratedHeader();
    void testNesting();
    void testSingleInheritence();
    void testMultipleInheritence();
    void testCustomSerialization();
    void test3rdPartyAdaption();

private:
    const vector<string> m_expectedCode;
};

CPPUNIT_TEST_SUITE_REGISTRATION(JsonGeneratorTests);

JsonGeneratorTests::JsonGeneratorTests()
    : m_expectedCode(toArrayOfLines(readFile(testFilePath("some_structs_json_serialization.h"), 3 * 1024)))
{
}

/*!
 * \brief Tests whether the generator works by using it directly.
 */
void JsonGeneratorTests::testGeneratorItself()
{
    const string inputFilePath(testFilePath("some_structs.h"));
    const vector<const char *> inputFiles{ inputFilePath.data() };
    const vector<string> clangOptions{ "-I" CPP_UTILITIES_INCLUDE_DIRS, "-I" RAPIDJSON_INCLUDE_DIRS };

    stringstream buffer;
    JsonSerializationCodeGenerator::Options jsonOptions;
    jsonOptions.additionalClassesArg.occurrenceInfo().emplace_back(0);
    jsonOptions.additionalClassesArg.occurrenceInfo().back().values.emplace_back("TestNamespace2::ThirdPartyStruct");
    CodeFactory factory(TestApplication::appPath(), inputFiles, clangOptions, buffer);
    factory.addGenerator<JsonSerializationCodeGenerator>(jsonOptions);
    CPPUNIT_ASSERT(factory.run());
    assertEqualityLinewise(m_expectedCode, toArrayOfLines(buffer.str()));
}

/*!
 * \brief Tests the generator CLI explicitely.
 * \remarks Only available under UNIX (like) systems so far, because TESTUTILS_ASSERT_EXEC has not been implemented
 *          for other platforms.
 */
void JsonGeneratorTests::testCLI()
{
#ifdef PLATFORM_UNIX
    string stdout, stderr;

    const string inputFilePath(testFilePath("some_structs.h"));
    const char *const args1[] = { PROJECT_NAME, "-i", inputFilePath.data(), "--json-classes", "TestNamespace2::ThirdPartyStruct", "--clang-opt",
        "-I" CPP_UTILITIES_INCLUDE_DIRS, "-I" RAPIDJSON_INCLUDE_DIRS, nullptr };
    TESTUTILS_ASSERT_EXEC(args1);
    assertEqualityLinewise(m_expectedCode, toArrayOfLines(stdout));
#endif
}

/*!
 * \brief Tests whether the generated reflection code actually works.
 * \remarks The following methods do the same. This test case is supposed to be the minimum example.
 */
void JsonGeneratorTests::testIncludingGeneratedHeader()
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

/*!
 * \brief Like testIncludingGeneratedHeader() but also for nested structures.
 */
void JsonGeneratorTests::testNesting()
{
    TestStruct test;
    test.someInt = 42;
    test.someString = "the answer";
    test.yetAnotherString = "but what was the question";
    NestedTestStruct nested;
    nested.nested.emplace_front(vector<TestStruct>{ test });
    nested.deq.emplace_back(3.14);
    const string expectedJSON("{\"nested\":[[{\"someInt\":42,\"someString\":\"the answer\",\"yetAnotherString\":\"but what was the "
                              "question\"}]],\"ptr\":null,\"deq\":[3.14]}");

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
void JsonGeneratorTests::testSingleInheritence()
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
    const DerivedTestStruct parsedTest(JsonSerializable<DerivedTestStruct>::fromJson(expectedJSONForDerived));
    CPPUNIT_ASSERT_EQUAL(test.someInt, parsedTest.someInt);
    CPPUNIT_ASSERT_EQUAL(test.someString, parsedTest.someString);
    CPPUNIT_ASSERT_EQUAL(test.yetAnotherString, parsedTest.yetAnotherString);
    CPPUNIT_ASSERT_EQUAL(test.someBool, parsedTest.someBool);
}

/*!
 * \brief Like testIncludingGeneratedHeader() but also tests multiple inheritence.
 */
void JsonGeneratorTests::testMultipleInheritence()
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
    const MultipleDerivedTestStruct parsedTest(JsonSerializable<MultipleDerivedTestStruct>::fromJson(expectedJSONForDerived));
    CPPUNIT_ASSERT_EQUAL(test.someInt, parsedTest.someInt);
    CPPUNIT_ASSERT_EQUAL(test.someString, parsedTest.someString);
    CPPUNIT_ASSERT_EQUAL(test.yetAnotherString, parsedTest.yetAnotherString);
    CPPUNIT_ASSERT_EQUAL(test.someBool, parsedTest.someBool);
    CPPUNIT_ASSERT_EQUAL(test.arrayOfStrings, parsedTest.arrayOfStrings);
}

/*!
 * \brief Like testIncludingGeneratedHeader() but also tests custom (de)serialization.
 */
void JsonGeneratorTests::testCustomSerialization()
{
    const StructWithCustomTypes test;
    const string str("{\"dt\":\"2017-04-02T15:31:21.165125\",\"ts\":\"03:15:19.125\"}");

    // test serialization
    CPPUNIT_ASSERT_EQUAL(str, string(test.toJson().GetString()));

    // test deserialization
    const StructWithCustomTypes parsedTest(StructWithCustomTypes::fromJson(str));
    CPPUNIT_ASSERT_EQUAL(test.dt.toString(), parsedTest.dt.toString());
    CPPUNIT_ASSERT_EQUAL(test.ts.toString(), parsedTest.ts.toString());
}

/*!
 * \brief Tests whether adapting (de)serialization for 3rd party structs works.
 */
void JsonGeneratorTests::test3rdPartyAdaption()
{
    // test whether specializations of AdaptedJsonSerializable are generated
    static_assert(
        ReflectiveRapidJSON::AdaptedJsonSerializable<NotJsonSerializable>::value, "can serialize NotJsonSerializable because of adaption macro");
    static_assert(ReflectiveRapidJSON::AdaptedJsonSerializable<NestedNotJsonSerializable>::value,
        "can serialize NestedNotJsonSerializable because of adaption macro");
    static_assert(!ReflectiveRapidJSON::AdaptedJsonSerializable<OtherNotJsonSerializable>::value,
        "can not serialize OtherNotJsonSerializable because adaption macro missing");
    static_assert(!ReflectiveRapidJSON::AdaptedJsonSerializable<ReallyNotJsonSerializable>::value, "can not serialize ReallyNotJsonSerializable");

    const NotJsonSerializable simple;
    const string strSimple("{\"butSerializableAnyways\":\"useful to adapt 3rd party structs\"}");
    const NestedNotJsonSerializable nested{ { "foo" }, { { "1" }, { "2" }, { "3" } }, { 42, { "bar" } } };
    const string strNested("{\"asMember\":{\"butSerializableAnyways\":\"foo\"},\"asArrayElement\":[{\"butSerializableAnyways\":\"1\"},{"
                           "\"butSerializableAnyways\":\"2\"},{\"butSerializableAnyways\":\"3\"}],\"asTupleElement\":[42,{\"butSerializableAnyways\":"
                           "\"bar\"}]}");

    // test serialization
    CPPUNIT_ASSERT_EQUAL(strSimple, string(ReflectiveRapidJSON::JsonReflector::toJson(simple).GetString()));
    CPPUNIT_ASSERT_EQUAL(strNested, string(ReflectiveRapidJSON::JsonReflector::toJson(nested).GetString()));

    // test deserialization
    JsonDeserializationErrors errors;
    const auto parsedSimple(ReflectiveRapidJSON::JsonReflector::fromJson<NotJsonSerializable>(strSimple, &errors));
    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(simple.butSerializableAnyways, parsedSimple.butSerializableAnyways);
    const auto parsedNested(ReflectiveRapidJSON::JsonReflector::fromJson<NestedNotJsonSerializable>(strNested, &errors));
    CPPUNIT_ASSERT_EQUAL(0_st, errors.size());
    CPPUNIT_ASSERT_EQUAL(nested.asMember.butSerializableAnyways, parsedNested.asMember.butSerializableAnyways);
    CPPUNIT_ASSERT_EQUAL(nested.asMember.butSerializableAnyways, parsedNested.asMember.butSerializableAnyways);
    CPPUNIT_ASSERT_EQUAL(nested.asArrayElement.size(), parsedNested.asArrayElement.size());
    CPPUNIT_ASSERT_EQUAL(nested.asArrayElement.at(0).butSerializableAnyways, parsedNested.asArrayElement.at(0).butSerializableAnyways);
    CPPUNIT_ASSERT_EQUAL(nested.asArrayElement.at(1).butSerializableAnyways, parsedNested.asArrayElement.at(1).butSerializableAnyways);
    CPPUNIT_ASSERT_EQUAL(nested.asArrayElement.at(2).butSerializableAnyways, parsedNested.asArrayElement.at(2).butSerializableAnyways);
    CPPUNIT_ASSERT_EQUAL(get<0>(nested.asTupleElement), get<0>(parsedNested.asTupleElement));
    CPPUNIT_ASSERT_EQUAL(get<1>(nested.asTupleElement).butSerializableAnyways, get<1>(parsedNested.asTupleElement).butSerializableAnyways);
}

// include file required for reflection of TestStruct and other structs defined in structs.h
// NOTE: * generation of this header is triggered using the CMake function add_reflection_generator_invocation()
//       * the include must happen in exactly one translation unit of the project at a point where the structs are defined
#include "reflection/structs.h"

// this file should also be generated via add_reflection_generator_invocation() and hence includeable
// it is included to test the "empty" case when a unit doesn't contain relevant classes
#include "reflection/cppunit.h"
