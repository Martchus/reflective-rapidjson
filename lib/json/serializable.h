#ifndef REFLECTIVE_RAPIDJSON_JSON_SERIALIZABLE_H
#define REFLECTIVE_RAPIDJSON_JSON_SERIALIZABLE_H

/*!
 * \file serializable.h
 * \brief Contains only the definiation of the JsonSerializable template class which makes the reflection
 *        accessible. The actual implementation is found in jsonreflector.h and generated files.
 */

#include "./reflector.h"

#include <rapidjson/document.h>

#include <string>

namespace ReflectiveRapidJSON {

/*!
 * \brief The JsonSerializable class provides the CRTP-base for (de)serializable objects.
 */
template <typename Type> struct JsonSerializable {
    // RapidJSON-level API
    void push(RAPIDJSON_NAMESPACE::Value &container);
    void push(RAPIDJSON_NAMESPACE::Value &container, const char *name);

    // high-level API
    RAPIDJSON_NAMESPACE::StringBuffer toJson() const;
    static Type fromJson(const char *json, std::size_t jsonSize, JsonDeserializationErrors *errors = nullptr);
    static Type fromJson(const char *json, JsonDeserializationErrors *errors = nullptr);
    static Type fromJson(const std::string &json, JsonDeserializationErrors *errors = nullptr);

    static constexpr const char *qualifiedName = "ReflectiveRapidJSON::JsonSerializable";
};

/*!
 * \brief Pushes the object to the specified RapidJSON array.
 */
template <typename Type> void JsonSerializable<Type>::push(RAPIDJSON_NAMESPACE::Value &container)
{
    return JsonReflector::push<Type>(*this, container);
}

/*!
 * \brief Pushes the object to the specified RapidJSON object as a member with the specified \a name.
 */
template <typename Type> void JsonSerializable<Type>::push(RAPIDJSON_NAMESPACE::Value &container, const char *name)
{
    return JsonReflector::push<Type>(*this, name, container);
}

/*!
 * \brief Converts the object to its JSON representation.
 * \remarks To obtain a string from the returned buffer, just use its GetString() method.
 */
template <typename Type> RAPIDJSON_NAMESPACE::StringBuffer JsonSerializable<Type>::toJson() const
{
    return JsonReflector::toJson<Type>(static_cast<const Type &>(*this));
}

/*!
 * \brief Constructs a new object from the specified JSON.
 */
template <typename Type> Type JsonSerializable<Type>::fromJson(const char *json, std::size_t jsonSize, JsonDeserializationErrors *errors)
{
    return JsonReflector::fromJson<Type>(json, jsonSize, errors);
}

/*!
 * \brief Constructs a new object from the specified JSON.
 */
template <typename Type> Type JsonSerializable<Type>::fromJson(const char *json, JsonDeserializationErrors *errors)
{
    return JsonReflector::fromJson<Type>(json, std::strlen(json), errors);
}

/*!
 * \brief Constructs a new object from the specified JSON.
 */
template <typename Type> Type JsonSerializable<Type>::fromJson(const std::string &json, JsonDeserializationErrors *errors)
{
    return JsonReflector::fromJson<Type>(json.data(), json.size(), errors);
}

/*!
 * \brief Helps to disambiguate when inheritance is used.
 */
template <typename Type, Traits::EnableIf<std::is_base_of<JsonSerializable<Type>, Type>>* = nullptr> JsonSerializable<Type> &as(Type &serializable)
{
    return static_cast<JsonSerializable<Type> &>(serializable);
}

/*!
 * \brief Helps to disambiguate when inheritance is used.
 */
template <typename Type, Traits::EnableIf<std::is_base_of<JsonSerializable<Type>, Type>>* = nullptr>
const JsonSerializable<Type> &as(const Type &serializable)
{
    return static_cast<const JsonSerializable<Type> &>(serializable);
}

/*!
 * \def The REFLECTIVE_RAPIDJSON_MAKE_JSON_SERIALIZABLE macro allows to adapt (de)serialization for types defined in 3rd party header files.
 * \remarks The struct will not have the toJson() and fromJson() methods available. Use the corresponding functions in the namespace
 *          ReflectiveRapidJSON::JsonReflector instead.
 * \todo GCC complains when putting :: before "ReflectiveRapidJSON" namespace: "global qualification of class name is invalid before ':' token"
 *       Find out whether this is a compiler bug or a correct error message.
 */
#define REFLECTIVE_RAPIDJSON_MAKE_JSON_SERIALIZABLE(T)                                                                                               \
    template <> struct ReflectiveRapidJSON::AdaptedJsonSerializable<T> : Traits::Bool<true> {                                                        \
    }

/*!
 * \def The REFLECTIVE_RAPIDJSON_PUSH_PRIVATE_MEMBERS macro enables serialization of private members.
 * \remarks For an example, see README.md.
 */
#define REFLECTIVE_RAPIDJSON_PUSH_PRIVATE_MEMBERS(T)                                                                                                 \
    friend void ::ReflectiveRapidJSON::JsonReflector::push<T>(                                                                                       \
        const T &reflectable, ::RAPIDJSON_NAMESPACE::Value::Object &value, ::RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)

/*!
 * \def The REFLECTIVE_RAPIDJSON_PULL_PRIVATE_MEMBERS macro enables deserialization of private members.
 * \remarks For an example, see README.md.
 */
#define REFLECTIVE_RAPIDJSON_PULL_PRIVATE_MEMBERS(T)                                                                                                 \
    friend void ::ReflectiveRapidJSON::JsonReflector::pull<T>(T & reflectable,                                                                       \
        const ::RAPIDJSON_NAMESPACE::GenericValue<::RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value,                                            \
        ::ReflectiveRapidJSON::JsonDeserializationErrors *errors)

/*!
 * \def The REFLECTIVE_RAPIDJSON_ENABLE_PRIVATE_MEMBERS macro enables serialization and deserialization of private members.
 * \remarks For an example, see README.md.
 */
#define REFLECTIVE_RAPIDJSON_ENABLE_PRIVATE_MEMBERS(T)                                                                                               \
    REFLECTIVE_RAPIDJSON_PUSH_PRIVATE_MEMBERS(T);                                                                                                    \
    REFLECTIVE_RAPIDJSON_PULL_PRIVATE_MEMBERS(T)

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_SERIALIZABLE_H
