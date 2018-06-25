#ifndef REFLECTIVE_RAPIDJSON_TESTS_MORE_STRUCTS_H
#define REFLECTIVE_RAPIDJSON_TESTS_MORE_STRUCTS_H

#include "../../lib/binary/serializable.h"
#include "../../lib/json/serializable.h"

using namespace std;
using namespace ReflectiveRapidJSON;

/*!
 * \brief The IncludedStruct struct is used to test whether only code for directly included
 *        structs is generated.
 *
 * The containing header file morestructs.h is indirectly included by struct.h. When
 * generating code for struct.h, the code generator should ignore this struct.
 *
 * \remarks This is important to prevent violating the one definition rule.
 */
struct IncludedStruct : public JsonSerializable<IncludedStruct>, public BinarySerializable<IncludedStruct> {
    int someInt = 0;
};

/*!
 * \brief The ConstStruct struct is used to test handling of const members.
 * \remarks Those members should be ignored when deserializing.
 */
struct ConstStruct : public JsonSerializable<ConstStruct>, public BinarySerializable<IncludedStruct> {
    int modifiableInt = 23;
    const int constInt = 42;
};

#endif // REFLECTIVE_RAPIDJSON_TESTS_MORE_STRUCTS_H
