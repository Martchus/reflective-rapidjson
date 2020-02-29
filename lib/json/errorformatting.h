#ifndef REFLECTIVE_RAPIDJSON_JSON_ERROR_FORMATTING_H
#define REFLECTIVE_RAPIDJSON_JSON_ERROR_FORMATTING_H

/*!
 * \file errorformatting.h
 * \brief Contains helper functions to format errors when deserializing JSON files.
 */

#include "./errorhandling.h"

#include <c++utilities/conversion/stringbuilder.h>

#include <string_view>

namespace ReflectiveRapidJSON {

inline std::string_view jsonTypeToString(JsonType jsonType)
{
    switch (jsonType) {
    case ReflectiveRapidJSON::JsonType::Null:
        return "null";
    case ReflectiveRapidJSON::JsonType::Number:
        return "number";
    case ReflectiveRapidJSON::JsonType::Bool:
        return "bool";
    case ReflectiveRapidJSON::JsonType::String:
        return "string";
    case ReflectiveRapidJSON::JsonType::Array:
        return "array";
    case ReflectiveRapidJSON::JsonType::Object:
        return "object";
    default:
        return "?";
    }
}

inline std::string formatJsonDeserializationError(const JsonDeserializationError &error)
{
    using namespace CppUtilities;
    std::string_view errorKind;
    std::string additionalInfo;
    switch (error.kind) {
    case JsonDeserializationErrorKind::TypeMismatch:
        errorKind = "type mismatch";
        additionalInfo = ": expected \"" % jsonTypeToString(error.expectedType) % "\", got \"" % jsonTypeToString(error.actualType) + '\"';
        break;
    case JsonDeserializationErrorKind::ArraySizeMismatch:
        errorKind = "array size mismatch";
        break;
    case JsonDeserializationErrorKind::ConversionError:
        errorKind = "conversion error";
        break;
    case JsonDeserializationErrorKind::UnexpectedDuplicate:
        errorKind = "unexpected duplicate";
        break;
    case JsonDeserializationErrorKind::InvalidVariantObject:
        errorKind = "invalid variant object";
        break;
    case JsonDeserializationErrorKind::InvalidVariantIndex:
        errorKind = "invalid variant index";
        break;
    default:
        errorKind = "semantic error";
    }
    if (error.record && error.member) {
        return errorKind % " within record \"" % error.record % "\" and member \"" % error.member % '\"' + additionalInfo;
    } else if (error.record && error.index != JsonDeserializationError::noIndex) {
        return errorKind % " within record \"" % error.record % "\" and index \"" % error.index % '\"' + additionalInfo;
    } else if (error.record) {
        return errorKind % " within record \"" % error.record % '\"' + additionalInfo;
    } else {
        return errorKind % " in document" + additionalInfo;
    }
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_JSON_ERROR_FORMATTING_H
