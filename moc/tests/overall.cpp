#include "../generator.h"

#include "resources/config.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/misc.h>
#include <c++utilities/tests/testutils.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace CPPUNIT_NS;
using namespace IoUtilities;
using namespace ConversionUtilities;
using namespace TestUtilities;
using namespace ReflectiveRapidJSON;

class OverallTests : public TestFixture {
    CPPUNIT_TEST_SUITE(OverallTests);
    CPPUNIT_TEST(testGenerator);
    CPPUNIT_TEST(testCLI);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testGenerator();
    void testCLI();

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(OverallTests);

void OverallTests::setUp()
{
}

void OverallTests::tearDown()
{
}

void OverallTests::testGenerator()
{
    const string inputFilePath(testFilePath("some_structs.h"));
    stringstream buffer;
    generateReflectionCode({ inputFilePath.data() }, buffer);
    CPPUNIT_ASSERT_EQUAL("test"s, buffer.str());
}

void OverallTests::testCLI()
{
#ifdef PLATFORM_UNIX
    const string inputFilePath(testFilePath("some_structs.h"));
    string stdout, stderr;
    const char *const args1[] = { PROJECT_NAME, "-i", inputFilePath.data(), nullptr };
    TESTUTILS_ASSERT_EXEC(args1);
    CPPUNIT_ASSERT_EQUAL("test"s, stdout);
#endif
}
