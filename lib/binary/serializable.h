#ifndef REFLECTIVE_RAPIDJSON_BINARY_SERIALIZABLE_H
#define REFLECTIVE_RAPIDJSON_BINARY_SERIALIZABLE_H

/*!
 * \file serializable.h
 * \brief Contains only the definiation of the BinarySerializable template class which makes the reflection
 *        accessible. The actual implementation is found in binaryreflector.h and generated files.
 */

#include "./reflector.h"

#include <iosfwd>
#include <string>

namespace ReflectiveRapidJSON {

/*!
 * \brief The BinarySerializable class provides the CRTP-base for (de)serializable objects.
 */
template <typename Type> struct BinarySerializable {
    void toBinary(std::ostream &outputStream) const;
    void restoreFromBinary(std::istream &inputStream);
    static Type fromBinary(std::istream &inputStream);

    static constexpr const char *qualifiedName = "ReflectiveRapidJSON::BinarySerializable";
};

template <typename Type> inline void BinarySerializable<Type>::toBinary(std::ostream &outputStream) const
{
    BinaryReflector::BinarySerializer(&outputStream).write(static_cast<const Type &>(*this));
}

template <typename Type> inline void BinarySerializable<Type>::restoreFromBinary(std::istream &inputStream)
{
    BinaryReflector::BinaryDeserializer(&inputStream).read(static_cast<Type &>(*this));
}

template <typename Type> Type BinarySerializable<Type>::fromBinary(std::istream &inputStream)
{
    Type object;
    static_cast<BinarySerializable<Type> &>(object).restoreFromBinary(inputStream);
    return object;
}

/*!
 * \def The REFLECTIVE_RAPIDJSON_MAKE_BINARY_SERIALIZABLE macro allows to adapt (de)serialization for types defined in 3rd party header files.
 * \remarks The struct will not have the toBinary() and fromBinary() methods available. Use the corresponding functions in the namespace
 *          ReflectiveRapidJSON::BinaryReflector instead.
 * \todo GCC complains when putting :: before "ReflectiveRapidJSON" namespace: "global qualification of class name is invalid before ':' token"
 *       Find out whether this is a compiler bug or a correct error message.
 */
#define REFLECTIVE_RAPIDJSON_MAKE_BINARY_SERIALIZABLE(T)                                                                                             \
    template <> struct ReflectiveRapidJSON::AdaptedBinarySerializable<T> : Traits::Bool<true> {                                                      \
    }

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_BINARY_SERIALIZABLE_H
