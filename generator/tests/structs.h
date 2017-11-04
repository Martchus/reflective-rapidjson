#ifndef REFLECTIVE_RAPIDJSON_TESTS_STRUCTS_H
#define REFLECTIVE_RAPIDJSON_TESTS_STRUCTS_H

#include "../../lib/json/reflector-chronoutilities.h"
#include "../../lib/json/serializable.h"

#include <c++utilities/chrono/datetime.h>
#include <c++utilities/chrono/timespan.h>

#include <deque>
#include <list>
#include <string>
#include <vector>

using namespace std;
using namespace ReflectiveRapidJSON;

/*!
 * \brief The TestStruct struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in OverallTests::testIncludingGeneratedHeader();
 */
struct TestStruct : public JsonSerializable<TestStruct> {
    int someInt = 0;
    string someString = "foo";
    string yetAnotherString = "bar";
};

/*!
 * \brief The NestedTestStruct struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in OverallTests::testNesting();
 */
struct NestedTestStruct : public JsonSerializable<NestedTestStruct> {
    list<vector<TestStruct>> nested;
    deque<double> deq;
};

/*!
 * \brief The AnotherTestStruct struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in OverallTests::testSingleInheritence();
 */
struct AnotherTestStruct : public JsonSerializable<AnotherTestStruct> {
    vector<string> arrayOfStrings{ "a", "b", "cd" };
};

/*!
 * \brief The DerivedTestStruct struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in OverallTests::testInheritence();
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
 *        and toJson() methods. This is asserted in OverallTests::testMultipleInheritence();
 */
struct MultipleDerivedTestStruct : public TestStruct,
                                   public AnotherTestStruct,
                                   public NonSerializable,
                                   public JsonSerializable<MultipleDerivedTestStruct> {
    bool someBool = false;
};

/*!
 * \brief The StructWithCustomTypes struct inherits from JsonSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in OverallTests::testCustomSerialization();
 */
struct StructWithCustomTypes : public JsonSerializable<StructWithCustomTypes> {
    ChronoUtilities::DateTime dt = ChronoUtilities::DateTime::fromDateAndTime(2017, 4, 2, 15, 31, 21, 165.125);
    ChronoUtilities::TimeSpan ts = ChronoUtilities::TimeSpan::fromHours(3.25) + ChronoUtilities::TimeSpan::fromSeconds(19.125);
};

#endif // REFLECTIVE_RAPIDJSON_TESTS_STRUCTS_H
