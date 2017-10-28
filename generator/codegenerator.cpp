#include "./codegenerator.h"

#include "../lib/jsonserializable.h"

#include <c++utilities/application/global.h>

#include <clang/AST/DeclCXX.h>

#include <iostream>
#include <memory>

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

CodeGenerator::~CodeGenerator()
{
}

/*!
 * \brief Adds the specified \a decl to the code generator. The generator might ignore irrelevant declarations.
 */
void CodeGenerator::addDeclaration(clang::Decl *decl)
{
    VAR_UNUSED(decl)
}

/*!
 * \brief Returns whether the specified \a record inherits from an instantiation of the specified \a templateClass.
 * \remarks The specified \a record must be defined (not only forward-declared).
 */
bool CodeGenerator::inheritsFromInstantiationOf(clang::CXXRecordDecl *const record, const char *const templateClass)
{
    for (const clang::CXXBaseSpecifier &base : record->bases()) {
        const clang::CXXRecordDecl *const baseDecl = base.getType()->getAsCXXRecordDecl();
        if (baseDecl && baseDecl->getQualifiedNameAsString() == templateClass) {
            return true;
        }
    }
    return false;
}

void JSONSerializationCodeGenerator::addDeclaration(clang::Decl *decl)
{
    switch (decl->getKind()) {
    case clang::Decl::Kind::CXXRecord: {
        auto *const record = static_cast<clang::CXXRecordDecl *>(decl);
        // skip forward declarations
        if (!record->hasDefinition()) {
            return;
        }
        // add classes derived from any instantiation of "ReflectiveRapidJSON::JSONSerializable"
        if (inheritsFromInstantiationOf(record, JSONSerializable<void>::qualifiedName)) {
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

void JSONSerializationCodeGenerator::generate(ostream &os) const
{
    if (m_relevantClasses.empty()) {
        return;
    }

    // put everything into namespace ReflectiveRapidJSON::Reflector
    os << "namespace ReflectiveRapidJSON {\n"
          "namespace Reflector {\n\n";

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
           << " &reflectable, const ::RAPIDJSON_NAMESPACE::GenericValue<::RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value, JSONParseErrors "
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

    // close namespace ReflectiveRapidJSON::Reflector
    os << "} // namespace Reflector\n"
          "} // namespace ReflectiveRapidJSON\n";
}

std::vector<const JSONSerializationCodeGenerator::RelevantClass *> JSONSerializationCodeGenerator::findRelevantBaseClasses(
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
