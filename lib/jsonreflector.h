#ifndef REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_H
#define REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_H

/*!
 * \file jsonreflector.h
 * \brief Contains functions to (de)serialize basic types such as int, double, bool, std::string,
 *        std::vector, ... with RapidJSON.
 */

#include <c++utilities/conversion/types.h>
#include <c++utilities/misc/traits.h>

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <list>
#include <string>
#include <vector>

namespace ReflectiveRapidJSON {

template <typename Type> struct JSONSerializable;

/*!
 * \brief The JSONParseErrorKind enum specifies which kind of error happend when populating variables from parsing results.
 */
enum class JSONParseErrorKind : byte {
    TypeMismatch,
};

/*!
 * \brief The JSONType enum specifies the JSON data type.
 * \remarks This is currently only used for error handling to propagate expected and actual types in case of a mismatch.
 */
enum class JSONType : byte {
    Null,
    Number,
    Bool,
    String,
    Array,
    Object,
};

template <typename Type,
    Traits::EnableIf<Traits::Not<std::is_same<Type, bool>>, Traits::Any<std::is_integral<Type>, std::is_floating_point<Type>>>...>
constexpr JSONType jsonType()
{
    return JSONType::Number;
}

template <typename Type, Traits::EnableIfAny<std::is_same<Type, bool>>...> constexpr JSONType jsonType()
{
    return JSONType::Bool;
}

template <typename Type, Traits::EnableIfAny<Traits::IsString<Type>, Traits::IsCString<Type>>...> constexpr JSONType jsonType()
{
    return JSONType::String;
}

template <typename Type, Traits::EnableIf<Traits::IsIteratable<Type>, Traits::Not<Traits::IsString<Type>>>...> constexpr JSONType jsonType()
{
    return JSONType::Array;
}

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, Traits::IsString<Type>, Traits::IsCString<Type>,
        Traits::IsIteratable<Type>>...>
constexpr JSONType jsonType()
{
    return JSONType::Object;
}

/*!
 * \brief Maps the type info provided by RapidJSON to JSONType.
 */
constexpr JSONType jsonType(RAPIDJSON_NAMESPACE::Type type)
{
    switch (type) {
    case RAPIDJSON_NAMESPACE::kNullType:
        return JSONType::Null;
    case RAPIDJSON_NAMESPACE::kFalseType:
    case RAPIDJSON_NAMESPACE::kTrueType:
        return JSONType::Bool;
    case RAPIDJSON_NAMESPACE::kObjectType:
        return JSONType::Object;
    case RAPIDJSON_NAMESPACE::kArrayType:
        return JSONType::Array;
    case RAPIDJSON_NAMESPACE::kStringType:
        return JSONType::String;
    case RAPIDJSON_NAMESPACE::kNumberType:
        return JSONType::Number;
    default:
        return JSONType::Null;
    }
}

/*!
 * \brief The JSONParseError struct describes any errors of fromJson() except such caused by invalid JSON.
 */
struct JSONParseError {
    JSONParseError(JSONParseErrorKind kind, JSONType expectedType, JSONType actualType, const char *record, const char *member = nullptr,
        std::size_t index = noIndex);

    /// \brief Which kind of error occured.
    JSONParseErrorKind kind;
    /// \brief The expected type (might not be relevant for all error kinds).
    JSONType expectedType;
    /// \brief The actual type (might not be relevant for all error kinds).
    JSONType actualType;
    /// \brief The name of the class or struct which was being processed when the error was ascertained.
    const char *record;
    /// \brief The name of the member which was being processed when the error was ascertained.
    const char *member;
    /// \brief The index in the array which was being processed when the error was ascertained.
    std::size_t index;

    /// \brief Indicates no array was being processed when the error occured.
    static constexpr std::size_t noIndex = std::numeric_limits<std::size_t>::max();
};

/*!
 * \brief Constructs a new JSONParseError.
 * \remarks Supposed to be called by JSONParseErrors::reportTypeMismatch() and similar methods of JSONParseErrors.
 */
inline JSONParseError::JSONParseError(
    JSONParseErrorKind kind, JSONType expectedType, JSONType actualType, const char *record, const char *member, std::size_t index)
    : kind(kind)
    , expectedType(expectedType)
    , actualType(actualType)
    , record(record)
    , member(member)
    , index(index)
{
}

/*!
 * \brief The JSONParseErrors struct can be passed to fromJson() for error handling.
 *
 * When passed to fromJson() and an error occurs, a JSONParseError is added to this object.
 * If throwOn is set, the JSONParseError is additionally thrown making the error fatal.
 *
 * \remarks Errors due to invalid JSON always lead to a RAPIDJSON_NAMESPACE::ParseResult object being thrown. So this
 *          only concerns errors listed in JSONParseErrorKind.
 */
struct JSONParseErrors : public std::vector<JSONParseError> {
    JSONParseErrors();

    template <typename ExpectedType> void reportTypeMismatch(RAPIDJSON_NAMESPACE::Type presentType);

    /// \brief The name of the class or struct which is currently being processed.
    const char *currentRecord;
    /// \brief The name of the member (in currentRecord) which is currently being processed.
    const char *currentMember;
    /// \brief The index in the array which is currently processed.
    std::size_t currentIndex;
    /// \brief The list of fatal error types in form of flags.
    enum class ThrowOn : unsigned char { None = 0, TypeMismatch = 0x1 } throwOn;
};

/*!
 * \brief Creates an empty JSONParseErrors object with default context and no errors considered fatal.
 */
inline JSONParseErrors::JSONParseErrors()
    : currentRecord("[document]")
    , currentMember(nullptr)
    , currentIndex(JSONParseError::noIndex)
    , throwOn(ThrowOn::None)
{
}

/*!
 * \brief Combines to ThrowOn values.
 */
constexpr JSONParseErrors::ThrowOn operator|(JSONParseErrors::ThrowOn lhs, JSONParseErrors::ThrowOn rhs)
{
    return static_cast<JSONParseErrors::ThrowOn>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
}

/*!
 * \brief Reports a type missmatch between \tparam ExpectedType and \a presentType within the current context.
 */
template <typename ExpectedType> inline void JSONParseErrors::reportTypeMismatch(RAPIDJSON_NAMESPACE::Type presentType)
{
    emplace_back(JSONParseErrorKind::TypeMismatch, jsonType<ExpectedType>(), jsonType(presentType), currentRecord, currentMember, currentIndex);
    if (static_cast<unsigned char>(throwOn) & static_cast<unsigned char>(ThrowOn::TypeMismatch)) {
        throw back();
    }
}

inline RAPIDJSON_NAMESPACE::StringBuffer documentToString(RAPIDJSON_NAMESPACE::Document &document)
{
    RAPIDJSON_NAMESPACE::StringBuffer buffer;
    RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer;
}

inline RAPIDJSON_NAMESPACE::Document parseDocumentFromString(const char *json, std::size_t jsonSize)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kObjectType);
    const RAPIDJSON_NAMESPACE::ParseResult parseRes = document.Parse(json, jsonSize);
    if (parseRes.IsError()) {
        throw parseRes;
    }
    return document;
}

namespace Reflector {

// define functions to "push" values to a RapidJSON array or object

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator);

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator);

template <typename Type, Traits::EnableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>>...>
inline void push(Type reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.PushBack(reflectable, allocator);
}

template <>
inline void push<const char *>(
    const char *reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.PushBack(RAPIDJSON_NAMESPACE::StringRef(reflectable), allocator);
}

template <>
inline void push<std::string>(
    const std::string &reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.PushBack(RAPIDJSON_NAMESPACE::StringRef(reflectable.data(), reflectable.size()), allocator);
}

template <>
inline void push<const char *const &>(
    const char *const &reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.PushBack(RAPIDJSON_NAMESPACE::StringRef(reflectable), allocator);
}

template <typename Type, Traits::EnableIf<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>...>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    RAPIDJSON_NAMESPACE::Value arrayValue(RAPIDJSON_NAMESPACE::kArrayType);
    RAPIDJSON_NAMESPACE::Value::Array array(arrayValue.GetArray());
    array.Reserve(reflectable.size(), allocator);
    for (const auto &item : reflectable) {
        push<decltype(item)>(item, array, allocator);
    }
    value.PushBack(array, allocator);
}

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
inline void push(
    const Type &reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    RAPIDJSON_NAMESPACE::Value objectValue(RAPIDJSON_NAMESPACE::kObjectType);
    RAPIDJSON_NAMESPACE::Value::Object object(objectValue.GetObject());
    push(reflectable, object, allocator);
    value.AddMember(RAPIDJSON_NAMESPACE::StringRef(name), object, allocator);
}

template <typename Type, Traits::EnableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>>...>
inline void push(
    Type reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.AddMember(RAPIDJSON_NAMESPACE::StringRef(name), reflectable, allocator);
}

template <>
inline void push<std::string>(const std::string &reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value,
    RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.AddMember(RAPIDJSON_NAMESPACE::StringRef(name), RAPIDJSON_NAMESPACE::StringRef(reflectable.data(), reflectable.size()), allocator);
}

template <>
inline void push<const char *>(
    const char *reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.AddMember(RAPIDJSON_NAMESPACE::StringRef(name), RAPIDJSON_NAMESPACE::StringRef(reflectable), allocator);
}

template <>
inline void push<const char *const &>(const char *const &reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value,
    RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    value.AddMember(RAPIDJSON_NAMESPACE::StringRef(name), RAPIDJSON_NAMESPACE::StringRef(reflectable), allocator);
}

template <typename Type,
    Traits::EnableIf<Traits::IsIteratable<Type>, Traits::Not<Traits::HasSize<Type>>,
        Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>...>
void push(
    const Type &reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    RAPIDJSON_NAMESPACE::Value arrayValue(RAPIDJSON_NAMESPACE::kArrayType);
    RAPIDJSON_NAMESPACE::Value::Array array(arrayValue.GetArray());
    for (const auto &item : reflectable) {
        push(item, array, allocator);
    }
    value.AddMember(RAPIDJSON_NAMESPACE::StringRef(name), array, allocator);
}

template <typename Type,
    Traits::EnableIf<Traits::IsIteratable<Type>, Traits::HasSize<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>...>
void push(
    const Type &reflectable, const char *name, RAPIDJSON_NAMESPACE::Value::Object &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    RAPIDJSON_NAMESPACE::Value arrayValue(RAPIDJSON_NAMESPACE::kArrayType);
    RAPIDJSON_NAMESPACE::Value::Array array(arrayValue.GetArray());
    array.Reserve(reflectable.size(), allocator);
    for (const auto &item : reflectable) {
        push(item, array, allocator);
    }
    value.AddMember(RAPIDJSON_NAMESPACE::StringRef(name), array, allocator);
}

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void push(const Type &reflectable, RAPIDJSON_NAMESPACE::Value::Array &value, RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)
{
    RAPIDJSON_NAMESPACE::Value objectValue(RAPIDJSON_NAMESPACE::kObjectType);
    RAPIDJSON_NAMESPACE::Value::Object object(objectValue.GetObject());
    push(reflectable, object, allocator);
    value.PushBack(objectValue, allocator);
}

// define functions to "pull" values from a RapidJSON array or object

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void pull(Type &reflectable, RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ValueIterator &value, JSONParseErrors *errors);

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void pull(Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value, JSONParseErrors *errors);

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>,
        Traits::All<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>>...>
void pull(Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JSONParseErrors *errors)
{
    if (!value.IsObject()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    pull<Type>(reflectable, value.GetObject(), errors);
}

template <typename Type, Traits::EnableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>>...>
inline void pull(Type &reflectable, RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ValueIterator &value, JSONParseErrors *errors)
{
    if (!value->Is<Type>()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value->GetType());
        }
        return;
    }
    reflectable = value->Get<Type>();
    ++value;
}

template <typename Type, Traits::EnableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, std::is_pointer<Type>>...>
inline void pull(Type &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JSONParseErrors *errors)
{
    if (!value.Is<Type>()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    reflectable = value.Get<Type>();
}

template <>
inline void pull<std::string>(
    std::string &reflectable, RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ValueIterator &value, JSONParseErrors *errors)
{
    if (!value->IsString()) {
        if (errors) {
            errors->reportTypeMismatch<std::string>(value->GetType());
        }
        return;
    }
    reflectable = value->GetString();
    ++value;
}

template <>
inline void pull<std::string>(
    std::string &reflectable, const RAPIDJSON_NAMESPACE::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JSONParseErrors *errors)
{
    if (!value.IsString()) {
        if (errors) {
            errors->reportTypeMismatch<std::string>(value.GetType());
        }
        return;
    }
    reflectable = value.GetString();
}

template <typename Type, Traits::EnableIf<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>...>
void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstArray array, JSONParseErrors *errors);

template <typename Type,
    Traits::EnableIf<Traits::IsIteratable<Type>, Traits::Not<Traits::IsReservable<Type>>,
        Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>...>
void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ValueIterator &value, JSONParseErrors *errors)
{
    if (!value->IsArray()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value->GetType());
        }
        return;
    }
    pull(reflectable, value->GetArray(), errors);
    ++value;
}

template <typename Type,
    Traits::EnableIf<Traits::IsIteratable<Type>, Traits::IsReservable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>...>
void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ValueIterator &value, JSONParseErrors *errors)
{
    if (!value->IsArray()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value->GetType());
        }
        return;
    }
    auto array = value->GetArray();
    reflectable.reserve(array.Size());
    pull(reflectable, array, errors);
    ++value;
}

template <typename Type,
    Traits::EnableIf<Traits::IsIteratable<Type>, Traits::Not<Traits::IsReservable<Type>>,
        Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>...>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JSONParseErrors *errors)
{
    if (!value.IsArray()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(value.GetType());
        }
        return;
    }
    pull(reflectable, value.GetArray(), errors);
}

template <typename Type,
    Traits::EnableIf<Traits::IsIteratable<Type>, Traits::IsReservable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>...>
void pull(Type &reflectable, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>> &value, JSONParseErrors *errors)
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

template <typename Type, Traits::EnableIf<Traits::IsIteratable<Type>, Traits::Not<Traits::IsSpecializationOf<Type, std::basic_string>>>...>
void pull(Type &reflectable, rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstArray array, JSONParseErrors *errors)
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
        reflectable.emplace_back();
        pull(reflectable.back(), item, errors);
        ++index;
    }

    // clear error context
    if (errors) {
        errors->currentIndex = JSONParseError::noIndex;
    }
}

template <typename Type>
inline void pull(
    Type &reflectable, const char *name, const rapidjson::GenericValue<RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value, JSONParseErrors *errors)
{
    // find member
    auto member = value.FindMember(name);
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
    pull<Type>(reflectable, value.FindMember(name)->value, errors);

    // restore previous error context
    if (errors) {
        errors->currentMember = previousMember;
    }
}

// define functions providing high-level JSON serialization

template <typename Type, Traits::EnableIfAny<std::is_base_of<JSONSerializable<Type>, Type>>...>
RAPIDJSON_NAMESPACE::StringBuffer toJson(const Type &reflectable)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kObjectType);
    RAPIDJSON_NAMESPACE::Document::Object object(document.GetObject());
    push(reflectable, object, document.GetAllocator());
    return documentToString(document);
}

template <typename Type, Traits::EnableIfAny<std::is_integral<Type>, std::is_floating_point<Type>>...>
RAPIDJSON_NAMESPACE::StringBuffer toJson(Type reflectable)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kNumberType);
    document.Set(reflectable, document.GetAllocator());
    return documentToString(document);
}

template <typename Type, Traits::EnableIfAny<std::is_same<Type, std::string>>...>
RAPIDJSON_NAMESPACE::StringBuffer toJson(const std::string &reflectable)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kStringType);
    document.SetString(RAPIDJSON_NAMESPACE::StringRef(reflectable.data(), reflectable.size()), document.GetAllocator());
    return documentToString(document);
}

template <typename Type, Traits::EnableIfAny<std::is_same<Type, const char *>>...> RAPIDJSON_NAMESPACE::StringBuffer toJson(const char *reflectable)
{
    RAPIDJSON_NAMESPACE::Document document(RAPIDJSON_NAMESPACE::kStringType);
    document.SetString(RAPIDJSON_NAMESPACE::StringRef(reflectable), document.GetAllocator());
    return documentToString(document);
}

// define functions providing high-level JSON deserialization

template <typename Type, Traits::EnableIfAny<std::is_base_of<JSONSerializable<Type>, Type>>...>
Type fromJson(const char *json, std::size_t jsonSize, JSONParseErrors *errors = nullptr)
{
    RAPIDJSON_NAMESPACE::Document doc(parseDocumentFromString(json, jsonSize));
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

template <typename Type, Traits::EnableIfAny<std::is_integral<Type>, std::is_floating_point<Type>>...>
Type fromJson(const char *json, std::size_t jsonSize, JSONParseErrors *errors)
{
    RAPIDJSON_NAMESPACE::Document doc(parseDocumentFromString(json, jsonSize));
    if (!doc.Is<Type>()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(doc.GetType());
        }
        return Type();
    }

    return doc.Get<Type>();
}

template <typename Type, Traits::EnableIfAny<std::is_same<Type, std::string>>...>
Type fromJson(const char *json, std::size_t jsonSize, JSONParseErrors *errors)
{
    RAPIDJSON_NAMESPACE::Document doc(parseDocumentFromString(json, jsonSize));
    if (!doc.IsString()) {
        if (errors) {
            errors->reportTypeMismatch<Type>(doc.GetType());
        }
        return Type();
    }

    return doc.GetString();
}

template <typename Type> Type fromJson(const std::string &json)
{
    return fromJson<Type>(json.data(), json.size());
}

} // namespace Reflector
} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_H
