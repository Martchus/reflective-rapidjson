#ifndef REFLECTIVE_RAPIDJSON_JSON_SERIALIZABLE_H
#define REFLECTIVE_RAPIDJSON_JSON_SERIALIZABLE_H

/*!
 * \file jsonserializable.h
 * \brief Contains only the definiation of the JSONSerializable template class which makes the reflection
 *        accessible. The actual implementation is found in jsonreflector.h and generated files.
 */

#include "./jsonreflector.h"

#include <rapidjson/document.h>

#include <string>

namespace ReflectiveRapidJSON {

/*!
 * \brief The JSONSerializable class provides the CRTP-base for (de)serializable objects.
 */
template <typename Type> struct JSONSerializable {
    // RapidJSON-level API
    void push(RAPIDJSON_NAMESPACE::Value &container);
    void push(RAPIDJSON_NAMESPACE::Value &container, const char *name);

    // high-level API
    RAPIDJSON_NAMESPACE::StringBuffer toJson() const;
    static Type fromJson(const char *json, std::size_t jsonSize, JSONParseErrors *errors = nullptr);
    static Type fromJson(const char *json, JSONParseErrors *errors = nullptr);
    static Type fromJson(const std::string &json, JSONParseErrors *errors = nullptr);

    static constexpr const char *qualifiedName = "ReflectiveRapidJSON::JSONSerializable";
};

/*!
 * \brief Pushes the object to the specified RapidJSON array.
 */
template <typename Type> void JSONSerializable<Type>::push(RAPIDJSON_NAMESPACE::Value &container)
{
    return Reflector::push<Type>(*this, container);
}

/*!
 * \brief Pushes the object to the specified RapidJSON object as a member with the specified \a name.
 */
template <typename Type> void JSONSerializable<Type>::push(RAPIDJSON_NAMESPACE::Value &container, const char *name)
{
    return Reflector::push<Type>(*this, name, container);
}

/*!
 * \brief Converts the object to its JSON representation.
 * \remarks To obtain a string from the returned buffer, just use its GetString() method.
 */
template <typename Type> RAPIDJSON_NAMESPACE::StringBuffer JSONSerializable<Type>::toJson() const
{
    return Reflector::toJson<Type>(static_cast<const Type &>(*this));
}

/*!
 * \brief Constructs a new object from the specified JSON.
 */
template <typename Type> Type JSONSerializable<Type>::fromJson(const char *json, std::size_t jsonSize, JSONParseErrors *errors)
{
    return Reflector::fromJson<Type>(json, jsonSize, errors);
}

/*!
 * \brief Constructs a new object from the specified JSON.
 */
template <typename Type> Type JSONSerializable<Type>::fromJson(const char *json, JSONParseErrors *errors)
{
    return Reflector::fromJson<Type>(json, std::strlen(json), errors);
}

/*!
 * \brief Constructs a new object from the specified JSON.
 */
template <typename Type> Type JSONSerializable<Type>::fromJson(const std::string &json, JSONParseErrors *errors)
{
    return Reflector::fromJson<Type>(json.data(), json.size(), errors);
}

/*!
 * \brief Helps to disambiguate when inheritance is used.
 */
template <typename Type, Traits::EnableIf<std::is_base_of<JSONSerializable<Type>, Type>>...> JSONSerializable<Type> &as(Type &serializable)
{
    return static_cast<JSONSerializable<Type> &>(serializable);
}

/*!
 * \brief Helps to disambiguate when inheritance is used.
 */
template <typename Type, Traits::EnableIf<std::is_base_of<JSONSerializable<Type>, Type>>...>
const JSONSerializable<Type> &as(const Type &serializable)
{
    return static_cast<const JSONSerializable<Type> &>(serializable);
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_SERIALIZABLE_H
