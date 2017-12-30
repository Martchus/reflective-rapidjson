#ifndef REFLECTIVE_RAPIDJSON_TESTS_MORE_STRUCTS_H
#define REFLECTIVE_RAPIDJSON_TESTS_MORE_STRUCTS_H

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
struct IncludedStruct : public JsonSerializable<IncludedStruct> {
    int someInt = 0;
};

#endif // REFLECTIVE_RAPIDJSON_TESTS_MORE_STRUCTS_H
