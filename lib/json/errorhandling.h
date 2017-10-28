#ifndef REFLECTIVE_RAPIDJSON_JSON_ERROR_HANDLING_H
#define REFLECTIVE_RAPIDJSON_JSON_ERROR_HANDLING_H

/*!
 * \file errorhandling.h
 * \brief Contains helper for error handling when deserializing JSON files.
 */

#include <c++utilities/conversion/types.h>
#include <c++utilities/misc/traits.h>

#include <rapidjson/rapidjson.h>

#include <limits>
#include <list>
#include <string>
#include <vector>

namespace ReflectiveRapidJSON {

/*!
 * \brief The JsonDeserializationErrorKind enum specifies which kind of error happend when populating variables from parsing results.
 */
enum class JsonDeserializationErrorKind : byte {
    TypeMismatch,
};

/*!
 * \brief The JsonType enum specifies the JSON data type.
 * \remarks This is currently only used for error handling to propagate expected and actual types in case of a mismatch.
 */
enum class JsonType : byte {
    Null,
    Number,
    Bool,
    String,
    Array,
    Object,
};

template <typename Type,
    Traits::EnableIf<Traits::Not<std::is_same<Type, bool>>, Traits::Any<std::is_integral<Type>, std::is_floating_point<Type>>>...>
constexpr JsonType jsonType()
{
    return JsonType::Number;
}

template <typename Type, Traits::EnableIfAny<std::is_same<Type, bool>>...> constexpr JsonType jsonType()
{
    return JsonType::Bool;
}

template <typename Type, Traits::EnableIfAny<Traits::IsString<Type>, Traits::IsCString<Type>>...> constexpr JsonType jsonType()
{
    return JsonType::String;
}

template <typename Type, Traits::EnableIf<Traits::IsIteratable<Type>, Traits::Not<Traits::IsString<Type>>>...> constexpr JsonType jsonType()
{
    return JsonType::Array;
}

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, Traits::IsString<Type>, Traits::IsCString<Type>,
        Traits::IsIteratable<Type>>...>
constexpr JsonType jsonType()
{
    return JsonType::Object;
}

/*!
 * \brief Maps the type info provided by RapidJSON to JsonType.
 */
constexpr JsonType jsonType(RAPIDJSON_NAMESPACE::Type type)
{
    switch (type) {
    case RAPIDJSON_NAMESPACE::kNullType:
        return JsonType::Null;
    case RAPIDJSON_NAMESPACE::kFalseType:
    case RAPIDJSON_NAMESPACE::kTrueType:
        return JsonType::Bool;
    case RAPIDJSON_NAMESPACE::kObjectType:
        return JsonType::Object;
    case RAPIDJSON_NAMESPACE::kArrayType:
        return JsonType::Array;
    case RAPIDJSON_NAMESPACE::kStringType:
        return JsonType::String;
    case RAPIDJSON_NAMESPACE::kNumberType:
        return JsonType::Number;
    default:
        return JsonType::Null;
    }
}

/*!
 * \brief The JsonDeserializationError struct describes any errors of fromJson() except such caused by invalid JSON.
 */
struct JsonDeserializationError {
    JsonDeserializationError(JsonDeserializationErrorKind kind, JsonType expectedType, JsonType actualType, const char *record,
        const char *member = nullptr, std::size_t index = noIndex);

    /// \brief Which kind of error occured.
    JsonDeserializationErrorKind kind;
    /// \brief The expected type (might not be relevant for all error kinds).
    JsonType expectedType;
    /// \brief The actual type (might not be relevant for all error kinds).
    JsonType actualType;
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
 * \brief Constructs a new JsonDeserializationError.
 * \remarks Supposed to be called by JsonDeserializationErrors::reportTypeMismatch() and similar methods of JsonDeserializationErrors.
 */
inline JsonDeserializationError::JsonDeserializationError(
    JsonDeserializationErrorKind kind, JsonType expectedType, JsonType actualType, const char *record, const char *member, std::size_t index)
    : kind(kind)
    , expectedType(expectedType)
    , actualType(actualType)
    , record(record)
    , member(member)
    , index(index)
{
}

/*!
 * \brief The JsonDeserializationErrors struct can be passed to fromJson() for error handling.
 *
 * When passed to fromJson() and an error occurs, a JsonDeserializationError is added to this object.
 * If throwOn is set, the JsonDeserializationError is additionally thrown making the error fatal.
 *
 * \remarks Errors due to invalid JSON always lead to a RAPIDJSON_NAMESPACE::ParseResult object being thrown. So this
 *          only concerns errors listed in JsonDeserializationErrorKind.
 */
struct JsonDeserializationErrors : public std::vector<JsonDeserializationError> {
    JsonDeserializationErrors();

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
 * \brief Creates an empty JsonDeserializationErrors object with default context and no errors considered fatal.
 */
inline JsonDeserializationErrors::JsonDeserializationErrors()
    : currentRecord("[document]")
    , currentMember(nullptr)
    , currentIndex(JsonDeserializationError::noIndex)
    , throwOn(ThrowOn::None)
{
}

/*!
 * \brief Combines to ThrowOn values.
 */
constexpr JsonDeserializationErrors::ThrowOn operator|(JsonDeserializationErrors::ThrowOn lhs, JsonDeserializationErrors::ThrowOn rhs)
{
    return static_cast<JsonDeserializationErrors::ThrowOn>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
}

/*!
 * \brief Reports a type missmatch between \tparam ExpectedType and \a presentType within the current context.
 */
template <typename ExpectedType> inline void JsonDeserializationErrors::reportTypeMismatch(RAPIDJSON_NAMESPACE::Type presentType)
{
    emplace_back(
        JsonDeserializationErrorKind::TypeMismatch, jsonType<ExpectedType>(), jsonType(presentType), currentRecord, currentMember, currentIndex);
    if (static_cast<unsigned char>(throwOn) & static_cast<unsigned char>(ThrowOn::TypeMismatch)) {
        throw back();
    }
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_H
