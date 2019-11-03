#ifndef REFLECTIVE_RAPIDJSON_JSON_ERROR_HANDLING_H
#define REFLECTIVE_RAPIDJSON_JSON_ERROR_HANDLING_H

/*!
 * \file errorhandling.h
 * \brief Contains helper for error handling when deserializing JSON files.
 */

#include <c++utilities/misc/traits.h>

#include <rapidjson/rapidjson.h>

#include <cstdint>
#include <limits>
#include <list>
#include <string>
#include <vector>

namespace ReflectiveRapidJSON {

/*!
 * \brief The JsonDeserializationErrorKind enum specifies which kind of error happend when populating variables from parsing results.
 */
enum class JsonDeserializationErrorKind : std::uint8_t {
    TypeMismatch, /**< The expected type does not match the type actually present in the JSON document. */
    ArraySizeMismatch, /**< The expected array size does not match the actual size of the JSON array. A fixed size is expected when deserializing an std::tuple. */
    ConversionError, /**< The expected type matches the type present in the JSON document, but further conversion of the value failed. */
    UnexpectedDuplicate, /**< The expected type matches the type present in the JSON document, but the value can not be added to the container because it is already present and duplicates are not allowed. */
    InvalidVariantObject, /**< The present object is supposed to represent an std::variant but lacks the index or data member. */
    InvalidVariantIndex, /**< The present variant index is not a number of outside of the expected range. */
};

/*!
 * \brief The JsonType enum specifies the JSON data type.
 * \remarks This is currently only used for error handling to propagate expected and actual types in case of a mismatch.
 */
enum class JsonType : std::uint8_t {
    Null,
    Number,
    Bool,
    String,
    Array,
    Object,
};

// define helper functions which return the JsonType for the C++ type specified as template parameter

template <typename Type,
    Traits::EnableIf<Traits::Not<std::is_same<Type, bool>>, Traits::Any<std::is_integral<Type>, std::is_floating_point<Type>>> * = nullptr>
constexpr JsonType jsonType()
{
    return JsonType::Number;
}

template <typename Type, Traits::EnableIfAny<std::is_same<Type, bool>> * = nullptr> constexpr JsonType jsonType()
{
    return JsonType::Bool;
}

template <typename Type, Traits::EnableIfAny<Traits::IsString<Type>, Traits::IsCString<Type>> * = nullptr> constexpr JsonType jsonType()
{
    return JsonType::String;
}

template <typename Type,
    Traits::EnableIf<Traits::IsIteratable<Type>,
        Traits::Not<Traits::Any<Traits::IsString<Type>, Traits::IsSpecializationOf<Type, std::map>,
            Traits::IsSpecializationOf<Type, std::unordered_map>>>> * = nullptr>
constexpr JsonType jsonType()
{
    return JsonType::Array;
}

template <typename Type,
    Traits::DisableIfAny<std::is_integral<Type>, std::is_floating_point<Type>, Traits::IsString<Type>, Traits::IsCString<Type>,
        Traits::All<Traits::IsIteratable<Type>,
            Traits::Not<Traits::Any<Traits::IsString<Type>, Traits::IsSpecializationOf<Type, std::map>,
                Traits::IsSpecializationOf<Type, std::unordered_map>>>>> * = nullptr>
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
    void reportArraySizeMismatch();
    void reportConversionError(JsonType jsonType);
    void reportUnexpectedDuplicate(JsonType jsonType);

    /// \brief The name of the class or struct which is currently being processed.
    const char *currentRecord;
    /// \brief The name of the member (in currentRecord) which is currently being processed.
    const char *currentMember;
    /// \brief The index in the array which is currently processed.
    std::size_t currentIndex;
    /// \brief The list of fatal error types in form of flags.
    enum class ThrowOn : std::uint8_t {
        None = 0,
        TypeMismatch = 0x1,
        ArraySizeMismatch = 0x2,
        ConversionError = 0x4,
        UnexpectedDuplicate = 0x8
    } throwOn;

private:
    void throwMaybe(ThrowOn on) const;
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
    return static_cast<JsonDeserializationErrors::ThrowOn>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}

/*!
 * \brief Throws the last error if its type is considered critical.
 * \param on Specifies the type of the last error as ThrowOn mask.
 * \remarks Behaviour is undefined if no error is present.
 */
inline void JsonDeserializationErrors::throwMaybe(ThrowOn on) const
{
    if (static_cast<std::uint8_t>(throwOn) & static_cast<std::uint8_t>(on)) {
        throw back();
    }
}

/*!
 * \brief Reports a type mismatch between \tparam ExpectedType and \a presentType within the current context.
 */
template <typename ExpectedType> inline void JsonDeserializationErrors::reportTypeMismatch(RAPIDJSON_NAMESPACE::Type presentType)
{
    emplace_back(
        JsonDeserializationErrorKind::TypeMismatch, jsonType<ExpectedType>(), jsonType(presentType), currentRecord, currentMember, currentIndex);
    throwMaybe(ThrowOn::TypeMismatch);
}

/*!
 * \brief Reports an array size mismatch.
 * \todo Allow specifying expected and actual size.
 * \remarks This can happen when mapping a JSON array to a C++ tuple.
 */
inline void JsonDeserializationErrors::reportArraySizeMismatch()
{
    emplace_back(JsonDeserializationErrorKind::ArraySizeMismatch, JsonType::Array, JsonType::Array, currentRecord, currentMember, currentIndex);
    throwMaybe(ThrowOn::ArraySizeMismatch);
}

/*!
 * \brief Reports a conversion error. An error of that kind occurs when the JSON type matched the expected type, but further conversion of the value has failed.
 * \todo Allow specifying the error message.
 * \remarks This can happen when doing custom mapping (eg. when interpreting a JSON string as time value).
 */
inline void JsonDeserializationErrors::reportConversionError(JsonType jsonType)
{
    emplace_back(JsonDeserializationErrorKind::ConversionError, jsonType, jsonType, currentRecord, currentMember, currentIndex);
    throwMaybe(ThrowOn::ConversionError);
}

/*!
 * \brief Reports an unexpected duplicate. An error of that kind occurs when the JSON type matched the expected type, but the value can not be inserted in the container because it is already present and duplicates are not allowed.
 * \todo Allow specifying the error message.
 * \remarks This can happen when doing custom mapping (eg. when interpreting a JSON string as time value).
 */
inline void JsonDeserializationErrors::reportUnexpectedDuplicate(JsonType jsonType)
{
    emplace_back(JsonDeserializationErrorKind::UnexpectedDuplicate, jsonType, jsonType, currentRecord, currentMember, currentIndex);
    throwMaybe(ThrowOn::UnexpectedDuplicate);
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_REFLECTOR_H
