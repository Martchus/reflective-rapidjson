#ifndef REFLECTIVE_RAPIDJSON_JSON_SERIALIZABLE_H
#define REFLECTIVE_RAPIDJSON_JSON_SERIALIZABLE_H

#include "./jsonreflector.h"

#include <rapidjson/document.h>

#include <string>

namespace ReflectiveRapidJSON {

template <typename Type> struct JSONSerializable {
    // RapidJSON-level API
    void push(RAPIDJSON_NAMESPACE::Value &container);
    void push(RAPIDJSON_NAMESPACE::Value &container, const char *name);

    // high-level API
    RAPIDJSON_NAMESPACE::StringBuffer toJson() const;
    static Type fromJson(const char *json, std::size_t jsonSize);
    static Type fromJson(const char *json);
    static Type fromJson(const std::string &json);

    static constexpr const char *qualifiedName = "ReflectiveRapidJSON::JSONSerializable";
};

template <typename Type> void JSONSerializable<Type>::push(RAPIDJSON_NAMESPACE::Value &container)
{
    return Reflector::push<Type>(*this, container);
}

template <typename Type> void JSONSerializable<Type>::push(RAPIDJSON_NAMESPACE::Value &container, const char *name)
{
    return Reflector::push<Type>(*this, name, container);
}

template <typename Type> RAPIDJSON_NAMESPACE::StringBuffer JSONSerializable<Type>::toJson() const
{
    return Reflector::toJson<Type>(static_cast<const Type &>(*this));
}

template <typename Type> Type JSONSerializable<Type>::fromJson(const char *json, std::size_t jsonSize)
{
    return Reflector::fromJson<Type>(json, jsonSize);
}

template <typename Type> Type JSONSerializable<Type>::fromJson(const char *json)
{
    return Reflector::fromJson<Type>(json, std::strlen(json));
}

template <typename Type> Type JSONSerializable<Type>::fromJson(const std::string &json)
{
    return Reflector::fromJson<Type>(json.data(), json.size());
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_SERIALIZABLE_H
