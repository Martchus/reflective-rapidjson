#ifndef REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_H
#define REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_H

/*!
 * \file reflector.h
 * \brief Contains BinaryReader and BinaryWriter supporting binary (de)serialization
 *        of primitive and custom types.
 */

#include "../traits.h"

#include <c++utilities/conversion/conversionexception.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <any>
#include <limits>
#include <memory>
#include <string>
#include <variant>

/// \cond
class BinaryReflectorTests;
/// \endcond

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
using IsBuiltInType = Traits::Any<Traits::IsAnyOf<Type, char, std::uint8_t, bool, std::string, std::int16_t, std::uint16_t, std::int32_t,
                                      std::uint32_t, std::int64_t, std::uint64_t, float, double>,
    Traits::IsIteratable<Type>, Traits::IsSpecializingAnyOf<Type, std::pair, std::unique_ptr, std::shared_ptr>, std::is_enum<Type>, IsVariant<Type>>;
template <typename Type> using IsCustomType = Traits::Not<IsBuiltInType<Type>>;

class BinaryDeserializer;
class BinarySerializer;

template <typename Type, Traits::EnableIf<IsCustomType<Type>> * = nullptr> void readCustomType(BinaryDeserializer &deserializer, Type &customType);
template <typename Type, Traits::EnableIf<IsCustomType<Type>> * = nullptr> void writeCustomType(BinarySerializer &serializer, const Type &customType);

class BinaryDeserializer : public CppUtilities::BinaryReader {
    friend class ::BinaryReflectorTests;

public:
    explicit BinaryDeserializer(std::istream *stream);

    using CppUtilities::BinaryReader::read;
    template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> * = nullptr> void read(Type &pair);
    template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::unique_ptr>> * = nullptr> void read(Type &pointer);
    template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::shared_ptr>> * = nullptr> void read(Type &pointer);
    template <typename Type, Traits::EnableIf<IsArray<Type>, Traits::IsResizable<Type>> * = nullptr> void read(Type &iteratable);
    template <typename Type, Traits::EnableIfAny<IsMapOrHash<Type>, IsMultiMapOrHash<Type>> * = nullptr> void read(Type &iteratable);
    template <typename Type,
        Traits::EnableIf<IsIteratableExceptString<Type>,
            Traits::None<IsMapOrHash<Type>, IsMultiMapOrHash<Type>, Traits::All<IsArray<Type>, Traits::IsResizable<Type>>>> * = nullptr>
    void read(Type &iteratable);
    template <typename Type, Traits::EnableIf<std::is_enum<Type>> * = nullptr> void read(Type &enumValue);
    template <typename Type, Traits::EnableIf<IsVariant<Type>> * = nullptr> void read(Type &variant);
    template <typename Type, Traits::EnableIf<IsCustomType<Type>> * = nullptr> void read(Type &customType);

private:
    std::unordered_map<std::uint64_t, std::any> m_pointer;
};

class BinarySerializer : public CppUtilities::BinaryWriter {
    friend class ::BinaryReflectorTests;

public:
    explicit BinarySerializer(std::ostream *stream);

    using CppUtilities::BinaryWriter::write;
    template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> * = nullptr> void write(const Type &pair);
    template <typename Type, Traits::EnableIf<Traits::IsSpecializingAnyOf<Type, std::unique_ptr>> * = nullptr> void write(const Type &pointer);
    template <typename Type, Traits::EnableIf<Traits::IsSpecializingAnyOf<Type, std::shared_ptr>> * = nullptr> void write(const Type &pointer);
    template <typename Type, Traits::EnableIf<IsIteratableExceptString<Type>, Traits::HasSize<Type>> * = nullptr> void write(const Type &iteratable);
    template <typename Type, Traits::EnableIf<std::is_enum<Type>> * = nullptr> void write(const Type &enumValue);
    template <typename Type, Traits::EnableIf<IsVariant<Type>> * = nullptr> void write(const Type &variant);
    template <typename Type, Traits::EnableIf<IsCustomType<Type>> * = nullptr> void write(const Type &customType);

private:
    std::unordered_map<std::uint64_t, bool> m_pointer;
};

inline BinaryDeserializer::BinaryDeserializer(std::istream *stream)
    : CppUtilities::BinaryReader(stream)
{
}

template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> *> void BinaryDeserializer::read(Type &pair)
{
    read(pair.first);
    read(pair.second);
}

template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::unique_ptr>> *> void BinaryDeserializer::read(Type &pointer)
{
    if (!readBool()) {
        pointer.reset();
        return;
    }
    pointer = std::make_unique<typename Type::element_type>();
    read(*pointer);
}

template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::shared_ptr>> *> void BinaryDeserializer::read(Type &pointer)
{
    auto mode = readByte();
    if (!mode) {
        // pointer not set
        pointer.reset();
        return;
    }

    const auto id = (mode & 0x4) ? readUInt64BE() : readVariableLengthUIntBE(); // the 3rd bit being flagged indicates a big ID
    if ((mode & 0x3) == 1) {
        // first occurrence: make a new pointer
        m_pointer[id] = pointer = std::make_shared<typename Type::element_type>();
        read(*pointer);
        return;
    }
    // further occurrences: copy previous pointer
    try {
        pointer = std::any_cast<Type>(m_pointer[id]);
    } catch (const std::bad_any_cast &) {
        throw CppUtilities::ConversionException("Referenced pointer type does not match");
    }
}

template <typename Type, Traits::EnableIf<IsArray<Type>, Traits::IsResizable<Type>> *> void BinaryDeserializer::read(Type &iteratable)
{
    const auto size = readVariableLengthUIntBE();
    iteratable.resize(size);
    for (auto &element : iteratable) {
        read(element);
    }
}

template <typename Type, Traits::EnableIfAny<IsMapOrHash<Type>, IsMultiMapOrHash<Type>> *> void BinaryDeserializer::read(Type &iteratable)
{
    const auto size = readVariableLengthUIntBE();
    for (size_t i = 0; i != size; ++i) {
        std::pair<typename std::remove_const<typename Type::value_type::first_type>::type, typename Type::value_type::second_type> value;
        read(value);
        iteratable.emplace(std::move(value));
    }
}

template <typename Type,
    Traits::EnableIf<IsIteratableExceptString<Type>,
        Traits::None<IsMapOrHash<Type>, IsMultiMapOrHash<Type>, Traits::All<IsArray<Type>, Traits::IsResizable<Type>>>> *>
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

/// \cond
namespace Detail {
template <typename Variant, std::size_t compiletimeIndex = 0>
void readVariantValueByRuntimeIndex(std::size_t runtimeIndex, Variant &variant, BinaryDeserializer &deserializer)
{
    if constexpr (compiletimeIndex < std::variant_size_v<Variant>) {
        if (compiletimeIndex == runtimeIndex) {
            if constexpr (std::is_same_v<std::variant_alternative_t<compiletimeIndex, Variant>, std::monostate>) {
                variant = std::monostate{};
            } else {
                deserializer.read(variant.template emplace<compiletimeIndex>());
            }
        } else {
            readVariantValueByRuntimeIndex<Variant, compiletimeIndex + 1>(runtimeIndex, variant, deserializer);
        }
    } else {
        throw CppUtilities::ConversionException("Variant index is out of expected range");
    }
}
} // namespace Detail
/// \endcond

template <typename Type, Traits::EnableIf<IsVariant<Type>> *> void BinaryDeserializer::read(Type &variant)
{
    Detail::readVariantValueByRuntimeIndex(readByte(), variant, *this);
}

template <typename Type, Traits::EnableIf<IsCustomType<Type>> *> void BinaryDeserializer::read(Type &customType)
{
    readCustomType(*this, customType);
}

inline BinarySerializer::BinarySerializer(std::ostream *stream)
    : CppUtilities::BinaryWriter(stream)
{
}

template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> *> void BinarySerializer::write(const Type &pair)
{
    write(pair.first);
    write(pair.second);
}

template <typename Type, Traits::EnableIf<Traits::IsSpecializingAnyOf<Type, std::unique_ptr>> *> void BinarySerializer::write(const Type &pointer)
{
    const bool hasValue = pointer != nullptr;
    writeBool(hasValue);
    if (hasValue) {
        write(*pointer);
    }
}

template <typename Type, Traits::EnableIf<Traits::IsSpecializingAnyOf<Type, std::shared_ptr>> *> void BinarySerializer::write(const Type &pointer)
{
    if (pointer == nullptr) {
        writeByte(0);
        return;
    }
    const auto id = reinterpret_cast<std::uintptr_t>(pointer.get());
    const auto bigId = id >= 0x80000000000000;
    auto &alreadyWritten = m_pointer[id];
    std::uint8_t mode = alreadyWritten ? 2 : 1;
    if (bigId) {
        mode = mode | 0x4; // "flag" 3rd bit to indicate big ID
    }
    writeByte(mode);
    if (bigId) {
        writeUInt64BE(id);
    } else {
        writeVariableLengthUIntBE(id);
    }
    if (!alreadyWritten) {
        alreadyWritten = true;
        write(*pointer);
    }
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

template <typename Type, Traits::EnableIf<IsVariant<Type>> *> void BinarySerializer::write(const Type &variant)
{
    static_assert(std::variant_size_v<Type> < std::numeric_limits<std::uint8_t>::max(), "index will not exceed limit");
    writeByte(static_cast<std::uint8_t>(variant.index()));
    std::visit(
        [this](const auto &valueOfActualType) {
            if constexpr (!std::is_same_v<std::decay_t<decltype(valueOfActualType)>, std::monostate>) {
                write(valueOfActualType);
            } else {
                CPP_UTILITIES_UNUSED(this)
            }
        },
        variant);
}

template <typename Type, Traits::EnableIf<IsCustomType<Type>> *> void BinarySerializer::write(const Type &customType)
{
    writeCustomType(*this, customType);
}

} // namespace BinaryReflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_BINARY_REFLECTOR_H
