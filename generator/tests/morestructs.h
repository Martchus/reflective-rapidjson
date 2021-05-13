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

/*!
 * \brief The PointerTarget struct is used to test the behavior of the binary (de)serialization with smart pointer.
 */
struct PointerTarget : public BinarySerializable<PointerTarget> {
    PointerTarget()
        : n(0xAAAAAAAA)
        , dummy1(0x1111111111111111)
        , dummy2(0x1111111111111111)
        , dummy3(0x1111111111111111)
    {
    }

    PointerTarget(uint32_t n)
        : n(n)
        , dummy1(0x1111111111111111)
        , dummy2(0x1111111111111111)
        , dummy3(0x1111111111111111)
    {
    }

    uint32_t n;
    uint64_t dummy1;
    uint64_t dummy2;
    uint64_t dummy3;
};

/*!
 * \brief The PointerStruct struct is used to test the behavior of the binary (de)serialization with smart pointer.
 */
struct PointerStruct : public BinarySerializable<PointerStruct> {
    std::shared_ptr<PointerTarget> s1;
    std::unique_ptr<PointerTarget> u2;
    std::unique_ptr<PointerTarget> u3;
    std::shared_ptr<PointerTarget> s2;
    std::unique_ptr<PointerTarget> u1;
    std::shared_ptr<PointerTarget> s3;
};

struct AnnotatedStruct : public BinarySerializable<AnnotatedStruct> {
    int anyVersion;
    __attribute__((annotate("cond:version >= 1"))) __attribute__((annotate("foo"))) int newInVersion1;
};

#endif // REFLECTIVE_RAPIDJSON_TESTS_MORE_STRUCTS_H
