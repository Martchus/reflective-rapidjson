#ifndef REFLECTIVE_RAPIDJSON_TESTS_STRUCTS_H
#define REFLECTIVE_RAPIDJSON_TESTS_STRUCTS_H

#include <c++utilities/chrono/datetime.h>
#include <c++utilities/chrono/timespan.h>

// including this header should *not* cause the code generator to generate code for the
// contained structs as well (to prevent violating the one definition rule)
#include "./morestructs.h"

#include "../../lib/json/reflector-chronoutilities.h"
#include "../../lib/json/serializable.h"

#include <deque>
#include <list>
#include <string>
#include <vector>

using namespace std;
using namespace ReflectiveRapidJSON;

/*!
 * \brief The TestStruct struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in JsonGeneratorTests::testIncludingGeneratedHeader();
 */
struct TestStruct : public JsonSerializable<TestStruct> {
    int someInt = 0;
    string someString = "foo";
    string yetAnotherString = "bar";
    static constexpr const char *staticMember = "static members are just ignored";
    static int anotherStaticMember;

private:
    string privateString = "not going to be serialized";
};

class JsonGeneratorTests;

/*!
 * \brief The NestedTestStruct struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in JsonGeneratorTests::testNesting();
 */
struct NestedTestStruct : public JsonSerializable<NestedTestStruct> {
    REFLECTIVE_RAPIDJSON_ENABLE_PRIVATE_MEMBERS(NestedTestStruct);

    friend class JsonGeneratorTests;

    list<vector<TestStruct>> nested;
    unique_ptr<TestStruct> ptr;

private:
    deque<double> deq;
};

/*!
 * \brief The AnotherTestStruct struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in JsonGeneratorTests::testSingleInheritence();
 */
struct AnotherTestStruct : public JsonSerializable<AnotherTestStruct> {
    vector<string> arrayOfStrings{ "a", "b", "cd" };
};

/*!
 * \brief The DerivedTestStruct struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in JsonGeneratorTests::testInheritence();
 */
struct DerivedTestStruct : public TestStruct, public JsonSerializable<DerivedTestStruct> {
    bool someBool = true;
};

/*!
 * \brief The NonSerializable struct should be ignored when used as base class because it isn't serializable.
 */
struct NonSerializable {
    int ignored = 25;
};

/*!
 * \brief The MultipleDerivedTestStruct struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in JsonGeneratorTests::testMultipleInheritence();
 */
struct MultipleDerivedTestStruct : public TestStruct,
                                   public AnotherTestStruct,
                                   public NonSerializable,
                                   public JsonSerializable<MultipleDerivedTestStruct> {
    bool someBool = false;
};

/*!
 * \brief The StructWithCustomTypes struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in JsonGeneratorTests::testCustomSerialization();
 */
struct StructWithCustomTypes : public JsonSerializable<StructWithCustomTypes> {
    ChronoUtilities::DateTime dt = ChronoUtilities::DateTime::fromDateAndTime(2017, 4, 2, 15, 31, 21, 165.125);
    ChronoUtilities::TimeSpan ts = ChronoUtilities::TimeSpan::fromHours(3.25) + ChronoUtilities::TimeSpan::fromSeconds(19.125);
};

/*!
 * \brief The NotJsonSerializable struct is used to test (de)serialization for 3rd party structs (which do not
 *        inherit from JsonSerializable instance). It is used in JsonGeneratorTests::test3rdPartyAdaption().
 * \remarks Imagine this struct would have been defined in a 3rd party header.
 */
struct NotJsonSerializable {
    std::string butSerializableAnyways = "useful to adapt 3rd party structs";
};

/*!
 * \brief The NestedNotJsonSerializable struct is used to test (de)serialization for 3rd party structs (which do not
 *        inherit from JsonSerializable instance). It is used in JsonGeneratorTests::test3rdPartyAdaption().
 * \remarks Imagine this struct would have been defined in a 3rd party header.
 */
struct NestedNotJsonSerializable {
    NotJsonSerializable asMember;
    vector<NotJsonSerializable> asArrayElement;
    tuple<int, NotJsonSerializable> asTupleElement;
};

// make "NotJsonSerializable" and "NestedNotJsonSerializable" serializable
REFLECTIVE_RAPIDJSON_MAKE_JSON_SERIALIZABLE(NotJsonSerializable);
REFLECTIVE_RAPIDJSON_MAKE_JSON_SERIALIZABLE(NestedNotJsonSerializable);

/*!
 * \brief The OtherNotJsonSerializable struct is used to test whether code for (de)serialization is generated for classes explicitely
 *        specified via CMake macro (despite use of REFLECTIVE_RAPIDJSON_ADAPT_JSON_SERIALIZABLE or JsonSerializable is
 *        missing).
 */
struct OtherNotJsonSerializable {
    std::string codeIsGenerated = "for this despite missing REFLECTIVE_RAPIDJSON_ADAPT_JSON_SERIALIZABLE";
};

/*!
 * \brief The ReallyNotJsonSerializable struct is used to tests (de)serialization for 3rd party structs (which do not
 *        inherit from JsonSerializable instance). It is used in JsonGeneratorTests::test3rdPartyAdaption().
 */
struct ReallyNotJsonSerializable {
    std::string notSerializable;
};

//REFLECTIVE_RAPIDJSON_ADAPT_JSON_SERIALIZABLE(NotJsonSerializable);

#endif // REFLECTIVE_RAPIDJSON_TESTS_STRUCTS_H
