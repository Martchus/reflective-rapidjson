#include "./jsonserializationcodegenerator.h"

#include "../lib/json/serializable.h"

#include <clang/AST/DeclCXX.h>

#include <iostream>

using namespace std;

namespace ReflectiveRapidJSON {

/*!
 * \brief Prints an LLVM string reference without instantiating a std::string first.
 */
ostream &operator<<(ostream &os, llvm::StringRef str)
{
    os.write(str.data(), static_cast<streamsize>(str.size()));
    return os;
}

void JsonSerializationCodeGenerator::addDeclaration(clang::Decl *decl)
{
    switch (decl->getKind()) {
    case clang::Decl::Kind::CXXRecord: {
        auto *const record = static_cast<clang::CXXRecordDecl *>(decl);
        // skip forward declarations
        if (!record->hasDefinition()) {
            return;
        }
        // add classes derived from any instantiation of "ReflectiveRapidJSON::JsonSerializable"
        if (inheritsFromInstantiationOf(record, JsonSerializable<void>::qualifiedName)) {
            m_relevantClasses.emplace_back(record->getQualifiedNameAsString(), record);
        }
        break;
    }
    case clang::Decl::Kind::Enum:
        // TODO: add enums
        break;
    default:;
    }
}

void JsonSerializationCodeGenerator::generate(ostream &os) const
{
    if (m_relevantClasses.empty()) {
        return;
    }

    // put everything into namespace ReflectiveRapidJSON::JsonReflector
    os << "namespace ReflectiveRapidJSON {\n"
          "namespace JsonReflector {\n\n";

    // add push and pull functions for each class, for an example of the resulting
    // output, see ../lib/tests/jsonserializable.cpp (code under comment "pretend serialization code...")
    for (const RelevantClass &relevantClass : m_relevantClasses) {
        // write comment
        os << "// define code for (de)serializing " << relevantClass.qualifiedName << " objects\n";

        // find relevant base classes
        const vector<const RelevantClass *> relevantBases = findRelevantBaseClasses(relevantClass);

        // print push method
        os << "template <> inline void push<::" << relevantClass.qualifiedName << ">(const ::" << relevantClass.qualifiedName
           << " &reflectable, ::RAPIDJSON_NAMESPACE::Value::Object &value, ::RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)\n{\n"
              "    // push base classes\n";
        for (const RelevantClass *baseClass : relevantBases) {
            os << "    push(static_cast<const ::" << baseClass->qualifiedName << " &>(reflectable), value, allocator);\n";
        }
        os << "    // push members\n";
        for (const clang::FieldDecl *field : relevantClass.record->fields()) {
            os << "    push(reflectable." << field->getName() << ", \"" << field->getName() << "\", value, allocator);\n";
        }
        os << "}\n";

        // print pull method
        os << "template <> inline void pull<::" << relevantClass.qualifiedName << ">(::" << relevantClass.qualifiedName
           << " &reflectable, const ::RAPIDJSON_NAMESPACE::GenericValue<::RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value, "
              "JsonDeserializationErrors "
              "*errors)\n{\n"
              "    // pull base classes\n";
        for (const RelevantClass *baseClass : relevantBases) {
            os << "    pull(static_cast<::" << baseClass->qualifiedName << " &>(reflectable), value, errors);\n";
        }
        os << "    // set error context for current record\n"
              "    const char *previousRecord;\n"
              "    if (errors) {\n"
              "        previousRecord = errors->currentRecord;\n"
              "        errors->currentRecord = \""
           << relevantClass.qualifiedName
           << "\";\n"
              "    }\n"
              "    // pull members\n";
        for (const clang::FieldDecl *field : relevantClass.record->fields()) {
            os << "    pull(reflectable." << field->getName() << ", \"" << field->getName() << "\", value, errors);\n";
        }
        os << "    // restore error context for previous record\n"
              "    if (errors) {\n"
              "        errors->currentRecord = previousRecord;\n"
              "    }\n";
        os << "}\n\n";
    }

    // close namespace ReflectiveRapidJSON::JsonReflector
    os << "} // namespace JsonReflector\n"
          "} // namespace ReflectiveRapidJSON\n";
}

std::vector<const JsonSerializationCodeGenerator::RelevantClass *> JsonSerializationCodeGenerator::findRelevantBaseClasses(
    const RelevantClass &relevantClass) const
{
    vector<const RelevantClass *> relevantBaseClasses;
    for (const RelevantClass &otherClass : m_relevantClasses) {
        if (relevantClass.record != otherClass.record && relevantClass.record->isDerivedFrom(otherClass.record)) {
            relevantBaseClasses.push_back(&otherClass);
        }
    }
    return relevantBaseClasses;
}

} // namespace ReflectiveRapidJSON
