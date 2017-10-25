#include "../codefactory.h"

#include "../../lib/jsonserializable.h"

#include "resources/config.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/misc.h>
#include <c++utilities/tests/testutils.h>

using TestUtilities::operator<<; // must be visible prior to the call site
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace CPPUNIT_NS;
using namespace IoUtilities;
using namespace TestUtilities;
using namespace ConversionUtilities;
using namespace ReflectiveRapidJSON;

/*!
 * \brief The TestStruct struct inherits from JSONSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in OverallTests::testIncludingGeneratedHeader();
 */
struct TestStruct : public JSONSerializable<TestStruct> {
    int someInt = 0;
    string someString = "foo";
    string yetAnotherString = "bar";
};

/*!
 * \brief The OverallTests class tests the overall functionality of the code generator (CLI and generator itself).
 */
class OverallTests : public TestFixture {
    CPPUNIT_TEST_SUITE(OverallTests);
    CPPUNIT_TEST(testGeneratorItself);
    CPPUNIT_TEST(testCLI);
    CPPUNIT_TEST(testIncludingGeneratedHeader);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testGeneratorItself();
    void testCLI();
    void testIncludingGeneratedHeader();

private:
    vector<string> m_expectedCode;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OverallTests);

/*!
 * \brief Asserts equality of two iteratables printing the differing indices.
 */
template <typename Iteratable, Traits::EnableIf<Traits::IsIteratable<Iteratable>, Traits::Not<Traits::IsString<Iteratable>>>...>
inline void assertEqualityLinewise(const Iteratable &iteratable1, const Iteratable &iteratable2)
{
    std::vector<std::string> differentLines;
    std::size_t currentLine = 0;

    for (auto i1 = iteratable1.cbegin(), i2 = iteratable2.cbegin(); i1 != iteratable1.cend() || i2 != iteratable2.cend(); ++currentLine) {
        if (i1 != iteratable1.cend() && i2 != iteratable2.cend()) {
            if (*i1 != *i2) {
                differentLines.push_back(numberToString(currentLine));
            }
            ++i1, ++i2;
        } else if (i1 != iteratable1.cend()) {
            differentLines.push_back(numberToString(currentLine));
            ++i1;
        } else if (i2 != iteratable1.cend()) {
            differentLines.push_back(numberToString(currentLine));
            ++i2;
        }
    }
    if (!differentLines.empty()) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("the following lines differ: " + joinStrings(differentLines, ", "), iteratable1, iteratable2);
    }
}

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
    CPPUNIT_ASSERT(factory.readAST());
    CPPUNIT_ASSERT(factory.generate());
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

// include file required for reflection of TestStruct; generation of this header is triggered using
// the CMake function add_reflection_generator_invocation()
#include "reflection.h"
