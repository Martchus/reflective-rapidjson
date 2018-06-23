#ifndef REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_H
#define REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_H

/*!
 * \file reflector.h
 * \brief Contains BinaryReader and BinaryWriter supporting binary (de)serialization
 *        of primitive and custom types.
 */

#include "../traits.h"

#include <c++utilities/conversion/types.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <limits>
#include <memory>
#include <string>
#include <tuple>

namespace ReflectiveRapidJSON {

/*!
 * \brief The AdaptedBinarySerializable class allows considering 3rd party classes as serializable.
 */
template <typename T> struct AdaptedBinarySerializable : public Traits::Bool<false> {
    static constexpr const char *name = "AdaptedBinarySerializable";
    static constexpr const char *qualifiedName = "ReflectiveRapidJSON::AdaptedBinarySerializable";
};

template <typename Type> struct BinarySerializable;

/*!
 * \brief The BinaryReflector namespace contains BinaryReader and BinaryWriter for automatic binary (de)serialization.
 */
namespace BinaryReflector {

// define traits to distinguish between "built-in" types like int, std::string, std::vector, ... and custom structs/classes
template <typename Type>
using IsBuiltInType = Traits::Any<Traits::IsAnyOf<Type, char, byte, bool, std::string, int16, uint16, int32, uint32, int64, uint64, float32, float64>,
    Traits::IsIteratable<Type>, Traits::IsSpecializationOf<Type, std::pair>, std::is_enum<Type>>;
template <typename Type> using IsCustomType = Traits::Not<IsBuiltInType<Type>>;
template <typename Type>
using IsSerializable = Traits::All<
    Traits::Any<Traits::Not<Traits::IsComplete<Type>>, std::is_base_of<BinarySerializable<Type>, Type>, AdaptedBinarySerializable<Type>>,
    Traits::Not<IsBuiltInType<Type>>>;

class BinaryDeserializer;
class BinarySerializer;

template <typename Type, Traits::EnableIf<IsCustomType<Type>> * = nullptr> void readCustomType(BinaryDeserializer &deserializer, Type &customType);
template <typename Type, Traits::EnableIf<IsCustomType<Type>> * = nullptr> void writeCustomType(BinarySerializer &serializer, const Type &customType);

class BinaryDeserializer : public IoUtilities::BinaryReader {
public:
    BinaryDeserializer(std::istream *stream);

    using IoUtilities::BinaryReader::read;
    template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> * = nullptr> void read(Type &pair);
    template <typename Type, Traits::EnableIf<IsArray<Type>, Traits::IsResizable<Type>> * = nullptr> void read(Type &iteratable);
    template <typename Type, Traits::EnableIf<IsMapOrHash<Type>> * = nullptr> void read(Type &iteratable);
    template <typename Type,
        Traits::EnableIf<IsIteratableExceptString<Type>, Traits::None<IsMapOrHash<Type>, Traits::All<IsArray<Type>, Traits::IsResizable<Type>>>>
            * = nullptr>
    void read(Type &iteratable);
    template <typename Type, Traits::EnableIf<std::is_enum<Type>> * = nullptr> void read(Type &customType);
    template <typename Type, Traits::EnableIf<IsCustomType<Type>> * = nullptr> void read(Type &customType);
};

class BinarySerializer : public IoUtilities::BinaryWriter {
public:
    BinarySerializer(std::ostream *stream);

    using IoUtilities::BinaryWriter::write;
    template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> * = nullptr> void write(const Type &pair);
    template <typename Type, Traits::EnableIf<IsIteratableExceptString<Type>, Traits::HasSize<Type>> * = nullptr> void write(const Type &iteratable);
    template <typename Type, Traits::EnableIf<std::is_enum<Type>> * = nullptr> void write(const Type &customType);
    template <typename Type, Traits::EnableIf<IsCustomType<Type>> * = nullptr> void write(const Type &customType);
};

inline BinaryDeserializer::BinaryDeserializer(std::istream *stream)
    : IoUtilities::BinaryReader(stream)
{
}

template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> *> void BinaryDeserializer::read(Type &pair)
{
    read(pair.first);
    read(pair.second);
}

template <typename Type, Traits::EnableIf<IsArray<Type>, Traits::IsResizable<Type>> *> void BinaryDeserializer::read(Type &iteratable)
{
    const auto size = readVariableLengthUIntBE();
    iteratable.resize(size);
    for (auto &element : iteratable) {
        read(element);
    }
}

template <typename Type, Traits::EnableIf<IsMapOrHash<Type>> *> void BinaryDeserializer::read(Type &iteratable)
{
    const auto size = readVariableLengthUIntBE();
    for (size_t i = 0; i != size; ++i) {
        std::pair<typename std::remove_const<typename Type::value_type::first_type>::type, typename Type::value_type::second_type> value;
        read(value);
        iteratable.emplace(std::move(value));
    }
}

template <typename Type,
    Traits::EnableIf<IsIteratableExceptString<Type>, Traits::None<IsMapOrHash<Type>, Traits::All<IsArray<Type>, Traits::IsResizable<Type>>>> *>
void BinaryDeserializer::read(Type &iteratable)
{
    const auto size = readVariableLengthUIntBE();
    for (size_t i = 0; i != size; ++i) {
        typename Type::value_type value;
        read(value);
        iteratable.emplace(std::move(value));
    }
}

template <typename Type, Traits::EnableIf<std::is_enum<Type>> *> void BinaryDeserializer::read(Type &enumValue)
{
    typename std::underlying_type<Type>::type value;
    read(value);
    enumValue = static_cast<Type>(value);
}

template <typename Type, Traits::EnableIf<IsCustomType<Type>> *> void BinaryDeserializer::read(Type &customType)
{
    readCustomType(*this, customType);
}

inline BinarySerializer::BinarySerializer(std::ostream *stream)
    : IoUtilities::BinaryWriter(stream)
{
}

template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> *> void BinarySerializer::write(const Type &pair)
{
    write(pair.first);
    write(pair.second);
}

template <typename Type, Traits::EnableIf<IsIteratableExceptString<Type>, Traits::HasSize<Type>> *>
void BinarySerializer::write(const Type &iteratable)
{
    writeVariableLengthUIntBE(iteratable.size());
    for (const auto &element : iteratable) {
        write(element);
    }
}

template <typename Type, Traits::EnableIf<std::is_enum<Type>> *> void BinarySerializer::write(const Type &enumValue)
{
    write(static_cast<typename std::underlying_type<Type>::type>(enumValue));
}

template <typename Type, Traits::EnableIf<IsCustomType<Type>> *> void BinarySerializer::write(const Type &customType)
{
    writeCustomType(*this, customType);
}

} // namespace BinaryReflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_H
