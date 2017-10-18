#ifndef REFLECTIVE_RAPIDJSON_REFLECTABLE_H
#define REFLECTIVE_RAPIDJSON_REFLECTABLE_H

#include "./reflect.h"

#include <rapidjson/document.h>

#include <string>

namespace ReflectiveRapidJSON {

template <typename Type> struct Reflectable {
    // RapidJSON-level API
    void push(RAPIDJSON_NAMESPACE::Value &container);
    void push(RAPIDJSON_NAMESPACE::Value &container, const char *name);

    // high-level API
    RAPIDJSON_NAMESPACE::StringBuffer toJson() const;
    static Type fromJson(const char *json, std::size_t jsonSize);
    static Type fromJson(const char *json);
    static Type fromJson(const std::string &json);
};

template <typename Type> void Reflectable<Type>::push(RAPIDJSON_NAMESPACE::Value &container)
{
    return Reflector::push<Type>(*this, container);
}

template <typename Type> void Reflectable<Type>::push(RAPIDJSON_NAMESPACE::Value &container, const char *name)
{
    return Reflector::push<Type>(*this, name, container);
}

template <typename Type> RAPIDJSON_NAMESPACE::StringBuffer Reflectable<Type>::toJson() const
{
    return Reflector::toJson<Type>(static_cast<const Type &>(*this));
}

template <typename Type> Type Reflectable<Type>::fromJson(const char *json, std::size_t jsonSize)
{
    return Reflector::fromJson<Type>(json, jsonSize);
}

template <typename Type> Type Reflectable<Type>::fromJson(const char *json)
{
    return Reflector::fromJson<Type>(json, std::strlen(json));
}

template <typename Type> Type Reflectable<Type>::fromJson(const std::string &json)
{
    return Reflector::fromJson<Type>(json.data(), json.size());
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_REFLECTABLE_H
