#ifndef REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_H
#define REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_H

/*!
 * \file reflector.h
 * \brief Contains functions to (de)serialize basic types such as int, double, bool, std::string,
 *        std::vector, ... with RapidJSON.
 */

#include "../traits.h"

#include <c++utilities/application/global.h>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

#include "./errorhandling.h"

namespace ReflectiveRapidJSON {

template <typename Type> struct JsonSerializable;

/*!
 * \brief The AdaptedJsonSerializable class allows considering 3rd party classes as serializable.
 */
template <typename T> struct AdaptedJsonSerializable : public Traits::Bool<false> {
    static constexpr const char *name = "AdaptedJsonSerializable";
    static constexpr const char *qualifiedName = "ReflectiveRapidJSON::AdaptedJsonSerializable";
};

/*!
 * \brief The JsonReflector namespace contains helper functions to ease the use of RapidJSON for automatic (de)serialization.
 */
namespace JsonReflector {

/*!
 * \brief Casts the specified \a size to the size type used by RapidJSON ensuring no overflow happens.
 */
constexpr RAPIDJSON_NAMESPACE::SizeType rapidJsonSize(std::size_t size)
{
    return size > std::numeric_limits<RAPIDJSON_NAMESPACE::SizeType>::max() ? std::numeric_limits<RAPIDJSON_NAMESPACE::SizeType>::max()
                                                                            : static_cast<RAPIDJSON_NAMESPACE::SizeType>(size);
}

/*!
 * \brief Serializes the specified JSON \a document.
 */
inline RAPIDJSON_NAMESPACE::StringBuffer serializeJsonDocToString(RAPIDJSON_NAMESPACE::Document &document)
{
    RAPIDJSON_NAMESPACE::StringBuffer buffer;
    RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer;
}

/*!
 * \brief Parses the specified JSON string.
 */
inline RAPIDJSON_NAMESPACE::Document parseJsonDocFromString(const char *json, std::size_t jsonSize)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kObjectType);
    const RAPIDJSON_NAMESPACE::ParseResult parseRes = document.Parse(json, jsonSize);
    if (parseRes.IsError()) {
        throw parseRes;
    }
    return document;
}

// define traits to distinguish between "built-in" types like int, std::string, std::vector, ... and custom structs/classes
template <typename Type>
using IsBuiltInType = Traits::Any<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>, std::is_enum<Type>,
    Traits::IsSpecializationOf<Type, std::tuple>, Traits::IsSpecializationOf<Type, std::pair>, Traits::IsIteratable<Type>,
    Traits::IsSpecializationOf<Type, std::unique_ptr>, Traits::IsSpecializationOf<Type, std::shared_ptr>,
    Traits::IsSpecializationOf<Type, std::weak_ptr>, IsVariant<Type>>;
template <typename Type> using IsCustomType = Traits::Not<IsBuiltInType<Type>>;

// define trait to check for custom structs/classes which are JSON serializable
// NOTE: the check for Traits::IsComplete is required because std::is_base_of fails for incomplete types when using GCC
template <typename Type>
using IsJsonSerializable
    = Traits::Any<Traits::Not<Traits::IsComplete<Type>>, std::is_base_of<JsonSerializable<Type>, Type>, AdaptedJsonSerializable<Type>>;

// define functions to "push" values to a RapidJSON array or object

/*!
 * \brief Pushes the specified \a reflectable to the specified value.
 */
template <typename Type, Traits::DisableIf<IsBuiltInType<Type>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator);

/*!
 * \brief Pushes the \a reflectable to the specified array.
 */
template <typename Type, Traits::DisableIf<IsJsonSerializable<Type>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator);

/*!
 * \brief Pushes the \a reflectable which has a custom type to the specified array.
 */
template <typename Type, Traits::EnableIf<IsJsonSerializable<Type>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator);

/*!
 * \brief Pushes the specified \a reflectable which has custom type as a member to the specified object.
 */
template <typename Type, Traits::EnableIf<IsJsonSerializable<Type>> * = nullptr>
void push(
    const Type &reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator);

/*!
 * \brief Pushes the specified \a reflectable as a member to the specified object.
 */
template <typename Type, Traits::DisableIf<IsJsonSerializable<Type>> * = nullptr>
void push(
    const Type &reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator);

/*!
 * \brief Pushes the \a reflectable which has a custom type to the specified object.
 * \remarks The definition of this function must be provided by the code generator or Boost.Hana.
 */
template <typename Type, Traits::DisableIf<IsBuiltInType<Type>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator);

/*!
 * \brief Pushes the specified \a reflectable which has a custom type to the specified value.
 */
template <typename Type, Traits::DisableIf<IsBuiltInType<Type>> *>
inline void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.SetObject();
    RAPIDJSON_NAMESPACE::Value::Object obj(value.GetObject());
    push(reflectable, obj, allocator);
}

/*!
 * \brief Pushes the specified integer/float/boolean to the specified value.
 */
template <typename Type, Traits::EnableIfAny<std::is_integral<Type>, std::is_floating_point<Type>> * = nullptr>
inline void push(Type reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.Set(reflectable, allocator);
}

/*!
 * \brief Pushes the specified enumeration item to the specified value.
 */
template <typename Type, Traits::EnableIfAny<std::is_enum<Type>> * = nullptr>
inline void push(Type reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.Set(static_cast<Traits::Conditional<std::is_unsigned<typename std::underlying_type<Type>::type>, std::uint64_t, std::int64_t>>(reflectable),
        allocator);
}

/*!
 * \brief Pushes the specified C-string to the specified value.
 */
template <typename Type, Traits::EnableIfAny<std::is_same<Type, const char *>, std::is_same<Type, const char *const &>> * = nullptr>
inline void push(Type reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    if (reflectable) {
        value.SetString(reflectable, allocator);
    } else {
        value.SetNull();
    }
}

/*!
 * \brief Pushes the specified std::string_view to the specified value.
 */
template <typename Type, Traits::EnableIf<std::is_same<Type, std::string_view>> * = nullptr>
inline void push(Type reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    if (reflectable.data()) {
        value.SetString(reflectable.data(), rapidJsonSize(reflectable.size()), allocator);
    } else {
        value.SetNull();
    }
}

/*!
 * \brief Pushes the specified std::string to the specified value.
 */
template <typename Type, Traits::EnableIf<std::is_same<Type, std::string>> * = nullptr>
inline void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.SetString(reflectable.data(), rapidJsonSize(reflectable.size()), allocator);
}

/*!
 * \brief Pushes the specified iteratable (eg. std::vector, std::list) to the specified value.
 */
template <typename Type, Traits::EnableIf<IsArrayOrSet<Type>, Traits::HasSize<Type>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.SetArray();
    RAPIDJSON_NAMESPACE::Value::Array array(value.GetArray());
    array.Reserve(rapidJsonSize(reflectable.size()), allocator);
    for (const auto &item : reflectable) {
        push(item, array, allocator);
    }
}

/*!
 * \brief Pushes the specified iteratable list (eg. std::vector, std::list) to the specified value.
 */
template <typename Type, Traits::EnableIf<IsArrayOrSet<Type>, Traits::Not<Traits::HasSize<Type>>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.SetArray();
    RAPIDJSON_NAMESPACE::Value::Array array(value.GetArray());
    for (const auto &item : reflectable) {
        push(item, array, allocator);
    }
}

/*!
 * \brief Pushes the specified map (std::map, std::unordered_map) to the specified value.
 */
template <typename Type, Traits::EnableIfAny<IsMapOrHash<Type>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.SetObject();
    RAPIDJSON_NAMESPACE::Value::Object object(value.GetObject());
    for (const auto &item : reflectable) {
        push(item.second, item.first.data(), object, allocator);
    }
}

/*!
 * \brief Pushes the specified multimap (std::multimap, std::unordered_multimap) to the specified value.
 */
template <typename Type, Traits::EnableIfAny<IsMultiMapOrHash<Type>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.SetObject();
    for (const auto &item : reflectable) {
        const auto memberName = RAPIDJSON_NAMESPACE::Value::StringRefType(item.first.data(), rapidJsonSize(item.first.size()));
        const auto existingMember = value.FindMember(memberName);
        const auto arrayAlreadyExists
            = existingMember != value.MemberEnd() && existingMember->value.GetType() == RAPIDJSON_NAMESPACE::Type::kArrayType;
        auto newArrayValue = RAPIDJSON_NAMESPACE::Value{ RAPIDJSON_NAMESPACE::kArrayType };
        RAPIDJSON_NAMESPACE::Value::Array array = arrayAlreadyExists ? existingMember->value.GetArray() : newArrayValue.GetArray();
        push(item.second, array, allocator);
        if (!arrayAlreadyExists) {
            value.AddMember(memberName, newArrayValue, allocator);
        }
    }
}

namespace Detail {

/*!
 * \brief The TuplePushHelper class helps serializing tuples to JSON arrays.
 */
template <class Tuple, std::size_t N> struct TuplePushHelper {
    static void push(const Tuple &tuple, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
    {
        TuplePushHelper<Tuple, N - 1>::push(tuple, value, allocator);
        JsonReflector::push(std::get<N - 1>(tuple), value, allocator);
    }
};

template <class Tuple> struct TuplePushHelper<Tuple, 1> {
    static void push(const Tuple &tuple, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
    {
        JsonReflector::push(std::get<0>(tuple), value, allocator);
    }
};
} // namespace Detail

/*!
 * \brief Pushes the specified tuple to the specified value.
 */
template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::tuple>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.SetArray();
    RAPIDJSON_NAMESPACE::Value::Array array(value.GetArray());
    array.Reserve(std::tuple_size<Type>::value, allocator);
    Detail::TuplePushHelper<Type, std::tuple_size<Type>::value>::push(reflectable, array, allocator);
}

/*!
 * \brief Pushes the specified pair to the specified value.
 */
template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.SetArray();
    RAPIDJSON_NAMESPACE::Value::Array array(value.GetArray());
    array.Reserve(2, allocator);
    push(reflectable.first, array, allocator);
    push(reflectable.second, array, allocator);
}

/*!
 * \brief Pushes the specified unique_ptr, shared_ptr or weak_ptr to the specified value.
 */
template <typename Type,
    Traits::EnableIfAny<Traits::IsSpecializationOf<Type, std::unique_ptr>, Traits::IsSpecializationOf<Type, std::shared_ptr>,
        Traits::IsSpecializationOf<Type, std::weak_ptr>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    if (!reflectable) {
        value.SetNull();
        return;
    }
    push(*reflectable, value, allocator);
}

/*!
 * \brief Pushes the specified variant to the specified value.
 */
template <typename Type, Traits::EnableIf<IsVariant<Type>> * = nullptr>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    if (reflectable.valueless_by_exception()) {
        value.SetNull();
        return;
    }

    RAPIDJSON_NAMESPACE::Value index, data;
    index.SetUint64(reflectable.index());
    std::visit(
        [&data, &allocator](const auto &reflectableOfActualType) {
            if constexpr (!std::is_same_v<std::decay_t<decltype(reflectableOfActualType)>, std::monostate>) {
                push(reflectableOfActualType, data, allocator);
            } else {
                CPP_UTILITIES_UNUSED(data)
                CPP_UTILITIES_UNUSED(allocator)
            }
        },
        reflectable);

    value.SetObject();
    value.AddMember("index", index, allocator);
    value.AddMember("data", data, allocator);
}

/*!
 * \brief Pushes the specified \a reflectable which has a custom type to the specified array.
 */
template <typename Type, Traits::EnableIf<IsJsonSerializable<Type>> *>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    RAPIDJSON_NAMESPACE::Value objectValue(RAPIDJSON_NAMESPACE::kObjectType);
    RAPIDJSON_NAMESPACE::Value::Object object(objectValue.GetObject());
    push(reflectable, object, allocator);
    value.PushBack(objectValue, allocator);
}

/*!
 * \brief Pushes the specified \a reflectable to the specified array.
 */
template <typename Type, Traits::DisableIf<IsJsonSerializable<Type>> *>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    RAPIDJSON_NAMESPACE::Value genericValue;
    push(reflectable, genericValue, allocator);
    value.PushBack(genericValue, allocator);
}

/*!
 * \brief Pushes the specified \a reflectable which has custom type as a member to the specified object.
 */
template <typename Type, Traits::EnableIf<IsJsonSerializable<Type>> *>
void push(
    const Type &reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    RAPIDJSON_NAMESPACE::Value objectValue(RAPIDJSON_NAMESPACE::kObjectType);
    RAPIDJSON_NAMESPACE::Value::Object object(objectValue.GetObject());
    push(reflectable, object, allocator);
    value.AddMember(RAPIDJSON_NAMESPACE::StringRef(name), objectValue, allocator);
}

/*!
 * \brief Pushes the specified \a reflectable as a member to the specified object.
 */
template <typename Type, Traits::DisableIf<IsJsonSerializable<Type>> *>
void push(
    const Type &reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    RAPIDJSON_NAMESPACE::Value genericValue;
    push(reflectable, genericValue, allocator);
    value.AddMember(RAPIDJSON_NAMESPACE::StringRef(name), genericValue, allocator);
}

// define functions to "pull" values from a RapidJSON array or object

/*!
 * \brief Pulls the \a reflectable which has a custom type from the specified object.
 * \remarks The definition of this function must be provided by the code generator or Boost.Hana.
 */
template <typename Type, Traits::DisableIf<IsBuiltInType<Type>> * = nullptr>
void pull(Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value,
    JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the \a reflectable which has a custom type from the specified value which is supposed and checked to contain an object.
 */
template <typename Type, Traits::DisableIf<IsBuiltInType<Type>> * = nullptr>
void pull(Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is an iteratable without reserve() method from the specified value which is checked to contain an array.
 */
template <typename Type, Traits::EnableIf<IsArrayOrSet<Type>, Traits::Not<Traits::IsReservable<Type>>> * = nullptr>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is an iteratable with reserve() method from the specified value which is checked to contain an array.
 */
template <typename Type, Traits::EnableIf<IsArrayOrSet<Type>, Traits::IsReservable<Type>> * = nullptr>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is an array/vector/list from the specified array. The \a reflectable is cleared before.
 */
template <typename Type, Traits::EnableIf<IsArray<Type>> * = nullptr>
void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstArray array, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is a set from the specified array. The \a reflectable is cleared before.
 */
template <typename Type, Traits::EnableIf<IsSet<Type>> * = nullptr>
void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstArray array, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is a multiset from the specified array. The \a reflectable is cleared before.
 */
template <typename Type, Traits::EnableIf<IsMultiSet<Type>> * = nullptr>
void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstArray array, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is a map from the specified value which is checked to contain an object.
 */
template <typename Type, Traits::EnableIf<IsMapOrHash<Type>> * = nullptr>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is a multimap from the specified value which is checked to contain an object.
 */
template <typename Type, Traits::EnableIf<IsMultiMapOrHash<Type>> * = nullptr>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is a tuple from the specified value which is checked to contain an array.
 */
template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::tuple>> * = nullptr>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is a pair from the specified value which is checked to contain an array.
 */
template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> * = nullptr>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is a unique_ptr from the specified value which might be null.
 */
template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::unique_ptr>> * = nullptr>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is a shared_ptr from the specified value which might be null.
 */
template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::shared_ptr>> * = nullptr>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable which is a variant from the specified value which might be null.
 */
template <typename Type, Traits::EnableIf<IsVariant<Type>> * = nullptr>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified \a reflectable from the specified value iterator which is checked to contain the right type.
 */
template <typename Type>
inline void pull(
    Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ValueIterator &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the specified member of \a reflectable which has a custom type from the specified object.
 * \remarks It is checked whether the object actually contains the member. If not, the missing member is ignored. So currently all members
 *          are optional.
 */
template <typename Type>
inline void pull(Type &reflectable, const char *name, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value,
    JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the \a reflectable which has a custom type from the specified value which is supposed and checked to contain an object.
 */
template <typename Type, Traits::DisableIf<IsBuiltInType<Type>> *>
void pull(Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors);

/*!
 * \brief Pulls the integer or float from the specified value which is supposed and checked to contain the right type.
 */
template <typename Type,
    Traits::EnableIf<Traits::Not<std::is_same<Type, bool>>, Traits::Any<std::is_integral<Type>, std::is_floating_point<Type>>> * = nullptr>
inline void pull(
    Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsNumber()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    reflectable = value.Is<Type>() ? value.Get<Type>() : static_cast<Type>(value.GetDouble());
}

/*!
 * \brief Pulls the boolean from the specified value which is supposed and checked to contain the right type.
 */
template <typename Type, Traits::EnableIf<std::is_same<Type, bool>> * = nullptr>
inline void pull(
    Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsBool()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    reflectable = value.GetBool();
}

/*!
 * \brief Pulls the specified enumeration item from the specified value which is supposed and checked to be compatible with the underlying type.
 * \remarks It is *not* checked, whether \a value is actually a valid enum item.
 */
template <typename Type, Traits::EnableIfAny<std::is_enum<Type>> * = nullptr>
inline void pull(
    Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    using ExpectedType = Traits::Conditional<std::is_unsigned<typename std::underlying_type<Type>::type>, std::uint64_t, std::int64_t>;
    if (!value.Is<ExpectedType>()) {
        if (errors) {
            errors->reportTypeMismatch<ExpectedType>(value.GetType());
        }
        return;
    }
    reflectable = static_cast<Type>(value.Get<ExpectedType>());
}

/*!
 * \brief Pulls the std::string from the specified value which is supposed and checked to contain a string.
 */
template <typename Type, Traits::EnableIf<std::is_same<Type, std::string>> * = nullptr>
inline void pull(
    Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsString()) {
        if (errors) {
            errors->reportTypeMismatch<std::string>(value.GetType());
        }
        return;
    }
    reflectable = value.GetString();
}

/*!
 * \brief Checks whether the specified value contains a string.
 * \remarks Does not actually store the value since the ownership would not be clear (see README.md).
 */
template <typename Type,
    Traits::EnableIfAny<std::is_same<Type, const char *>, std::is_same<Type, const char *const &>, std::is_same<Type, std::string_view>> * = nullptr>
inline void pull(Type &, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsString()) {
        if (errors) {
            errors->reportTypeMismatch<std::string>(value.GetType());
        }
        return;
    }
}

/*!
 * \brief Pulls the specified \a reflectable which is an iteratable without reserve() method from the specified value which is checked to contain an array.
 */
template <typename Type, Traits::EnableIf<IsArrayOrSet<Type>, Traits::Not<Traits::IsReservable<Type>>> *>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsArray()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    pull(reflectable, value.GetArray(), errors);
}

/*!
 * \brief Pulls the specified \a reflectable which is an iteratable with reserve() method from the specified value which is checked to contain an array.
 */
template <typename Type, Traits::EnableIf<IsArrayOrSet<Type>, Traits::IsReservable<Type>> *>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsArray()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    auto array = value.GetArray();
    reflectable.reserve(array.Size());
    pull(reflectable, array, errors);
}

/*!
 * \brief Pulls the specified \a reflectable which is an array/vector/list from the specified array. The \a reflectable is cleared before.
 */
template <typename Type, Traits::EnableIf<IsArray<Type>> *>
void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstArray array, JsonDeserializationErrors *errors)
{
    // clear previous contents of the array
    reflectable.clear();

    // pull all array elements of the specified value
    std::size_t index = 0;
    for (const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &item : array) {
        // set error context for current index
        if (errors) {
            errors->currentIndex = index;
        }
        ++index;
        reflectable.emplace_back();
        pull(reflectable.back(), item, errors);
    }

    // clear error context
    if (errors) {
        errors->currentIndex = JsonDeserializationError::noIndex;
    }
}

/*!
 * \brief Pulls the specified \a reflectable which is a multiset from the specified array. The \a reflectable is cleared before.
 */
template <typename Type, Traits::EnableIf<IsMultiSet<Type>> *>
void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstArray array, JsonDeserializationErrors *errors)
{
    // clear previous contents of the array
    reflectable.clear();

    // pull all array elements of the specified value
    std::size_t index = 0;
    for (const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &item : array) {
        // set error context for current index
        if (errors) {
            errors->currentIndex = index;
        }
        ++index;
        typename Type::value_type itemObj;
        pull(itemObj, item, errors);
        reflectable.emplace(std::move(itemObj));
    }

    // clear error context
    if (errors) {
        errors->currentIndex = JsonDeserializationError::noIndex;
    }
}

/*!
 * \brief Pulls the specified \a reflectable which is a set from the specified array. The \a reflectable is cleared before.
 */
template <typename Type, Traits::EnableIf<IsSet<Type>> *>
void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstArray array, JsonDeserializationErrors *errors)
{
    // clear previous contents of the array
    reflectable.clear();

    // pull all array elements of the specified value
    std::size_t index = 0;
    for (const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &item : array) {
        // set error context for current index
        if (errors) {
            errors->currentIndex = index;
        }
        ++index;
        typename Type::value_type itemObj;
        pull(itemObj, item, errors);
        if (!reflectable.emplace(std::move(itemObj)).second) {
            errors->reportUnexpectedDuplicate(JsonType::Array);
        }
    }

    // clear error context
    if (errors) {
        errors->currentIndex = JsonDeserializationError::noIndex;
    }
}

/*!
 * \brief Pulls the specified \a reflectable which is a map from the specified value which is checked to contain an object.
 */
template <typename Type, Traits::EnableIf<IsMapOrHash<Type>> *>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsObject()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    auto obj = value.GetObject();
    for (auto i = obj.MemberBegin(), end = obj.MemberEnd(); i != end; ++i) {
        pull(reflectable[i->name.GetString()], i->value, errors);
    }
}

/*!
 * \brief Pulls the specified \a reflectable which is a multimap from the specified value which is checked to contain an object.
 */
template <typename Type, Traits::EnableIf<IsMultiMapOrHash<Type>> *>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsObject()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    auto obj = value.GetObject();
    for (auto i = obj.MemberBegin(), end = obj.MemberEnd(); i != end; ++i) {
        if (i->value.GetType() != RAPIDJSON_NAMESPACE::kArrayType) {
            auto insertedIterator = reflectable.insert(typename Type::value_type(i->name.GetString(), typename Type::mapped_type()));
            pull(insertedIterator->second, i->value, errors);
            continue;
        }
        const auto array = i->value.GetArray();
        for (const auto &arrayValue : array) {
            auto insertedIterator = reflectable.insert(typename Type::value_type(i->name.GetString(), typename Type::mapped_type()));
            pull(insertedIterator->second, arrayValue, errors);
        }
    }
}

namespace Detail {

/*!
 * \brief The TuplePullHelper class helps deserializing tuples from JSON arrays.
 * \remarks Assumes that the array bounds have been checked before (to match the size of the tuple).
 */
template <class Tuple, std::size_t N> struct TuplePullHelper {
    static void pull(Tuple &tuple, const RAPIDJSON_NAMESPACE::Value::ConstArray value, JsonDeserializationErrors *errors)
    {
        TuplePullHelper<Tuple, N - 1>::pull(tuple, value, errors);
        JsonReflector::pull<typename std::tuple_element<N - 1, Tuple>::type>(std::get<N - 1>(tuple), value[N - 1], errors);
    }
};

template <class Tuple> struct TuplePullHelper<Tuple, 1> {
    static void pull(Tuple &tuple, const RAPIDJSON_NAMESPACE::Value::ConstArray value, JsonDeserializationErrors *errors)
    {
        JsonReflector::pull<typename std::tuple_element<0, Tuple>::type>(std::get<0>(tuple), value[0], errors);
    }
};
} // namespace Detail

/*!
 * \brief Pulls the specified \a reflectable which is a tuple from the specified value which is checked to contain an array.
 */
template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::tuple>> *>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsArray()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    const auto array = value.GetArray();
    if (array.Size() != std::tuple_size<Type>::value) {
        if (errors) {
            // FIXME: report expected and actual size
            errors->reportArraySizeMismatch();
        }
        return;
    }
    Detail::TuplePullHelper<Type, std::tuple_size<Type>::value>::pull(reflectable, array, errors);
}

/*!
 * \brief Pulls the specified \a reflectable which is a pair from the specified value which is checked to contain an array.
 */
template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::pair>> *>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsArray()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    const auto array = value.GetArray();
    if (array.Size() != 2) {
        if (errors) {
            // FIXME: report expected and actual size
            errors->reportArraySizeMismatch();
        }
        return;
    }
    pull(reflectable.first, array[0], errors);
    pull(reflectable.second, array[1], errors);
}

/*!
 * \brief Pulls the specified \a reflectable which is a unique_ptr from the specified value which might be null.
 */
template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::unique_ptr>> *>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (value.IsNull()) {
        reflectable.reset();
        return;
    }
    reflectable = std::make_unique<typename Type::element_type>();
    pull(*reflectable, value, errors);
}

/*!
 * \brief Pulls the specified \a reflectable which is a shared_ptr from the specified value which might be null.
 */
template <typename Type, Traits::EnableIf<Traits::IsSpecializationOf<Type, std::shared_ptr>> *>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (value.IsNull()) {
        reflectable.reset();
        return;
    }
    reflectable = std::make_shared<typename Type::element_type>();
    pull(*reflectable, value, errors);
}

/// \cond
namespace Detail {
template <typename Variant, std::size_t compiletimeIndex = 0>
void assignVariantValueByRuntimeIndex(std::size_t runtimeIndex, Variant &variant,
    const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if constexpr (compiletimeIndex < std::variant_size_v<Variant>) {
        if (compiletimeIndex == runtimeIndex) {
            if constexpr (std::is_same_v<std::variant_alternative_t<compiletimeIndex, Variant>, std::monostate>) {
                variant = std::monostate{};
            } else {
                pull(variant.template emplace<compiletimeIndex>(), value, errors);
            }
        } else {
            assignVariantValueByRuntimeIndex<Variant, compiletimeIndex + 1>(runtimeIndex, variant, value, errors);
        }
    } else {
        if (errors) {
            errors->emplace_back(JsonDeserializationErrorKind::InvalidVariantIndex, JsonType::Number, JsonType::Number, errors->currentRecord,
                errors->currentMember, errors->currentIndex);
        }
    }
}
} // namespace Detail
/// \endcond

/*!
 * \brief Pulls the specified \a reflectable which is a variant from the specified value which might be null.
 */
template <typename Type, Traits::EnableIf<IsVariant<Type>> *>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsObject()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }

    auto obj = value.GetObject();
    auto indexIterator = obj.FindMember("index");
    auto dataIterator = obj.FindMember("data");
    if (indexIterator == obj.MemberEnd() || dataIterator == obj.MemberEnd()) {
        if (errors) {
            errors->emplace_back(JsonDeserializationErrorKind::InvalidVariantObject, JsonType::Object, JsonType::Object, errors->currentRecord,
                errors->currentMember, errors->currentIndex);
        }
        return;
    }
    const auto &indexValue = indexIterator->value;
    if (!indexValue.IsUint64()) {
        if (errors) {
            errors->emplace_back(JsonDeserializationErrorKind::InvalidVariantIndex, JsonType::Number, jsonType(indexValue.GetType()),
                errors->currentRecord, errors->currentMember, errors->currentIndex);
        }
        return;
    }
    Detail::assignVariantValueByRuntimeIndex(indexValue.GetUint64(), reflectable, dataIterator->value, errors);
}

/*!
 * \brief Pulls the specified \a reflectable from the specified value iterator which is checked to contain the right type.
 */
template <typename Type>
inline void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ValueIterator &value, JsonDeserializationErrors *errors)
{
    pull<Type>(reflectable, *value, errors);
    ++value;
}

/*!
 * \brief Pulls the specified member of \a reflectable which has a custom type from the specified object.
 * \remarks It is checked whether the object actually contains the member. If not, the missing member is ignored. So currently all members
 *          are optional.
 */
template <typename Type>
inline void pull(Type &reflectable, const char *name, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value,
    JsonDeserializationErrors *errors)
{
    // find member
    const auto member = value.FindMember(name);
    if (member == value.MemberEnd()) {
        return; // TODO: handle member missing
    }

    // set error context for current member
    const char *previousMember;
    if (errors) {
        previousMember = errors->currentMember;
        errors->currentMember = name;
    }

    // actually pull value for member
    pull<Type>(reflectable, member->value, errors);

    // restore previous error context
    if (errors) {
        errors->currentMember = previousMember;
    }
}

/*!
 * \brief Pulls the \a reflectable which has a custom type from the specified value which is supposed and checked to contain an object.
 */
template <typename Type, Traits::DisableIf<IsBuiltInType<Type>> *>
void pull(Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JsonDeserializationErrors *errors)
{
    if (!value.IsObject()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    pull(reflectable, value.GetObject(), errors);
}

// define functions providing high-level JSON serialization

/*!
 * \brief Serializes the specified \a reflectable which has a custom type or can be mapped to and object.
 */
template <typename Type, Traits::EnableIfAny<IsJsonSerializable<Type>, IsMapOrHash<Type>, IsMultiMapOrHash<Type>> * = nullptr>
RAPIDJSON_NAMESPACE::Document toJsonDocument(const Type &reflectable)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kObjectType);
    push(reflectable, document, document.GetAllocator());
    return document;
}

/*!
 * \brief Serializes the specified \a reflectable which is an integer, float or boolean.
 */
template <typename Type, Traits::EnableIfAny<std::is_integral<Type>, std::is_floating_point<Type>> * = nullptr>
RAPIDJSON_NAMESPACE::Document toJsonDocument(Type reflectable)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kNumberType);
    document.Set(reflectable, document.GetAllocator());
    return document;
}

/*!
 * \brief Serializes the specified \a reflectable which is an std::string.
 */
template <typename Type, Traits::EnableIf<std::is_same<Type, std::string>> * = nullptr>
RAPIDJSON_NAMESPACE::Document toJsonDocument(const std::string &reflectable)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kStringType);
    document.SetString(RAPIDJSON_NAMESPACE::StringRef(reflectable.data(), reflectable.size()), document.GetAllocator());
    return document;
}

/*!
 * \brief Serializes the specified \a reflectable which is a C-string.
 */
template <typename Type, Traits::EnableIf<std::is_same<Type, const char *>> * = nullptr>
RAPIDJSON_NAMESPACE::Document toJsonDocument(const char *reflectable)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kStringType);
    document.SetString(RAPIDJSON_NAMESPACE::StringRef(reflectable), document.GetAllocator());
    return document;
}

/*!
 * \brief Serializes the specified \a reflectable which is an std::string_view.
 */
template <typename Type, Traits::EnableIf<std::is_same<Type, std::string_view>> * = nullptr>
RAPIDJSON_NAMESPACE::Document toJsonDocument(std::string_view reflectable)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kStringType);
    document.SetString(RAPIDJSON_NAMESPACE::StringRef(reflectable.data(), reflectable.size()), document.GetAllocator());
    return document;
}

/*!
 * \brief Serializes the specified \a reflectable which can be mapped to an array.
 */
template <typename Type, Traits::EnableIf<IsArray<Type>> * = nullptr> RAPIDJSON_NAMESPACE::Document toJsonDocument(const Type &reflectable)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kArrayType);
    push(reflectable, document, document.GetAllocator());
    return document;
}

/*!
 * \brief Serializes the specified \a reflectable.
 */
template <typename Type,
    Traits::EnableIfAny<IsJsonSerializable<Type>, IsMapOrHash<Type>, IsMultiMapOrHash<Type>, std::is_integral<Type>, std::is_floating_point<Type>,
        Traits::IsString<Type>, IsArray<Type>> * = nullptr>
RAPIDJSON_NAMESPACE::StringBuffer toJson(const Type &reflectable)
{
    auto document(toJsonDocument(reflectable));
    return serializeJsonDocToString(document);
}

// define functions providing high-level JSON deserialization

/*!
 * \brief Deserializes the specified JSON to \tparam Type which is a custom type or can be mapped to an object.
 */
template <typename Type, Traits::EnableIfAny<IsJsonSerializable<Type>, IsMapOrHash<Type>, IsMultiMapOrHash<Type>> * = nullptr>
Type fromJson(const char *json, std::size_t jsonSize, JsonDeserializationErrors *errors = nullptr)
{
    RAPIDJSON_NAMESPACE::Document doc(parseJsonDocFromString(json, jsonSize));
    if (!doc.IsObject()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(doc.GetType());
        }
        return Type();
    }

    Type res;
    pull<Type>(res, doc.GetObject(), errors);
    return res;
}

/*!
 * \brief Deserializes the specified JSON to \tparam Type which is an integer, float or boolean.
 */
template <typename Type, Traits::EnableIfAny<std::is_integral<Type>, std::is_floating_point<Type>> * = nullptr>
Type fromJson(const char *json, std::size_t jsonSize, JsonDeserializationErrors *errors = nullptr)
{
    RAPIDJSON_NAMESPACE::Document doc(parseJsonDocFromString(json, jsonSize));
    if (!doc.Is<Type>()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(doc.GetType());
        }
        return Type();
    }

    return doc.Get<Type>();
}

/*!
 * \brief Deserializes the specified JSON to \tparam Type which is a std::string.
 */
template <typename Type, Traits::EnableIf<std::is_same<Type, std::string>> * = nullptr>
Type fromJson(const char *json, std::size_t jsonSize, JsonDeserializationErrors *errors = nullptr)
{
    RAPIDJSON_NAMESPACE::Document doc(parseJsonDocFromString(json, jsonSize));
    if (!doc.IsString()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(doc.GetType());
        }
        return Type();
    }

    return doc.GetString();
}

/*!
 * \brief Deserializes the specified JSON to \tparam Type which can be mapped to an array.
 */
template <typename Type, Traits::EnableIf<IsArray<Type>> * = nullptr>
Type fromJson(const char *json, std::size_t jsonSize, JsonDeserializationErrors *errors = nullptr)
{
    RAPIDJSON_NAMESPACE::Document doc(parseJsonDocFromString(json, jsonSize));
    if (!doc.IsArray()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(doc.GetType());
        }
        return Type();
    }

    Type res;
    pull<Type>(res, doc.GetArray(), errors);
    return res;
}

/*!
 * \brief Deserializes the specified JSON from an null-terminated C-string to \tparam Type.
 */
template <typename Type> Type fromJson(const char *json, JsonDeserializationErrors *errors = nullptr)
{
    return fromJson<Type>(json, std::strlen(json), errors);
}

/*!
 * \brief Deserializes the specified JSON from an std::string to \tparam Type.
 */
template <typename Type> Type fromJson(const std::string &json, JsonDeserializationErrors *errors = nullptr)
{
    return fromJson<Type>(json.data(), json.size(), errors);
}

} // namespace JsonReflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_H
