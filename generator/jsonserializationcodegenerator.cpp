#include "./jsonserializationcodegenerator.h"

#include "../lib/json/serializable.h"

#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>

#include <iostream>

using namespace std;
using namespace CppUtilities;

namespace ReflectiveRapidJSON {

/*!
 * \brief Initializes the CLI arguments which are specific to the JsonSerializationCodeGenerator.
 * \todo Find a more general approach to pass CLI arguments from main() to the particular code generators.
 */
JsonSerializationCodeGenerator::Options::Options()
    : additionalClassesArg("json-classes", '\0', "specifies additional classes to consider for JSON (de)serialization", { "class-name" })
    , visibilityArg("json-visibility", '\0', "specifies the \"visibility attribute\" for generated functions", { "attribute" })
{
    additionalClassesArg.setRequiredValueCount(Argument::varValueCount);
    additionalClassesArg.setValueCompletionBehavior(ValueCompletionBehavior::None);
    visibilityArg.setPreDefinedCompletionValues("CPP_UTILITIES_GENERIC_LIB_EXPORT");
}

JsonSerializationCodeGenerator::JsonSerializationCodeGenerator(CodeFactory &factory, const Options &options)
    : SerializationCodeGenerator(factory)
    , m_options(options)
{
    m_qualifiedNameOfRecords = JsonSerializable<void>::qualifiedName;
    m_qualifiedNameOfAdaptionRecords = AdaptedJsonSerializable<void>::qualifiedName;
}

/*!
 * \brief Checks whether \a possiblyRelevantClass is actually relevant.
 */
void JsonSerializationCodeGenerator::computeRelevantClass(RelevantClass &possiblyRelevantClass) const
{
    SerializationCodeGenerator::computeRelevantClass(possiblyRelevantClass);
    if (possiblyRelevantClass.isRelevant != IsRelevant::Maybe) {
        return;
    }

    // consider all classes specified via "--additional-classes" argument relevant
    if (!m_options.additionalClassesArg.isPresent()) {
        return;
    }
    for (const char *const className : m_options.additionalClassesArg.values()) {
        if (className == possiblyRelevantClass.qualifiedName) {
            possiblyRelevantClass.isRelevant = IsRelevant::Yes;
            return;
        }
    }
}

/*!
 * \brief Generates pull() and push() helper functions in the ReflectiveRapidJSON::JsonReflector namespace for the relevant classes.
 */
void JsonSerializationCodeGenerator::generate(ostream &os) const
{
    // initialize source manager to make use of isOnlyIncluded() for skipping records which are only included
    lazyInitializeSourceManager();

    // find relevant classes
    const auto relevantClasses = findRelevantClasses();
    if (relevantClasses.empty()) {
        return; // nothing to generate
    }

    // put everything into namespace ReflectiveRapidJSON::JsonReflector
    os << "namespace ReflectiveRapidJSON {\n"
          "namespace JsonReflector {\n\n";

    // determine visibility attribute
    const char *visibility = m_options.visibilityArg.firstValue();
    if (!visibility) {
        visibility = "";
    }

    // add push and pull functions for each class, for an example of the resulting
    // output, see ../lib/tests/jsonserializable.cpp (code under comment "pretend serialization code...")
    for (const RelevantClass &relevantClass : relevantClasses) {
        // determine whether private members should be pushed/pulled as well: check whether friend declarations for push/pull present
        // note: the friend declarations we are looking for are expanded from the REFLECTIVE_RAPIDJSON_ENABLE_PRIVATE_MEMBERS macro
        bool pushPrivateMembers = false, pullPrivateMembers = false;
        for (const clang::FriendDecl *const friendDecl : relevantClass.record->friends()) {
            // get the actual declaration which must be a function
            const clang::NamedDecl *const actualFriendDecl = friendDecl->getFriendDecl();
            if (!actualFriendDecl || actualFriendDecl->getKind() != clang::Decl::Kind::Function) {
                continue;
            }
            // check whether the friend function matches the push/pull helper function
            const string friendName(actualFriendDecl->getQualifiedNameAsString());
            if (friendName == "ReflectiveRapidJSON::JsonReflector::push") {
                pushPrivateMembers = true;
            }
            if (friendName == "ReflectiveRapidJSON::JsonReflector::pull") {
                pullPrivateMembers = true;
            }
            if (pushPrivateMembers && pullPrivateMembers) {
                break;
            }
        }

        // find relevant base classes
        const vector<const RelevantClass *> relevantBases = findRelevantBaseClasses(relevantClass, relevantClasses);

        // print comment
        os << "// define code for (de)serializing " << relevantClass.qualifiedName << " objects\n";

        // print push method
        os << "template <> " << visibility << " void push<::" << relevantClass.qualifiedName << ">(const ::" << relevantClass.qualifiedName
           << " &reflectable, ::RAPIDJSON_NAMESPACE::Value::Object &value, ::RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)\n{\n"
              "    // push base classes\n";
        for (const RelevantClass *baseClass : relevantBases) {
            os << "    push(static_cast<const ::" << baseClass->qualifiedName << " &>(reflectable), value, allocator);\n";
        }
        os << "    // push members\n";
        auto pushWritten = false;
        for (const clang::FieldDecl *field : relevantClass.record->fields()) {
            if (pushPrivateMembers || field->getAccess() == clang::AS_public) {
                os << "    push(reflectable." << field->getName() << ", \"" << field->getName() << "\", value, allocator);\n";
                pushWritten = true;
            }
        }
        if (relevantBases.empty() && !pushWritten) {
            os << "    (void)reflectable;\n    (void)value;\n";
        }
        os << "}\n";

        // skip printing the pull method for classes without default constructor because deserializing those is currently not supported
        if (!relevantClass.record->hasDefaultConstructor()) {
            continue;
        }

        // print pull method
        os << "template <> " << visibility << " void pull<::" << relevantClass.qualifiedName << ">(::" << relevantClass.qualifiedName
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
        auto pullWritten = false;
        for (const clang::FieldDecl *field : relevantClass.record->fields()) {
            // skip const members
            if (field->getType().isConstant(field->getASTContext())) {
                continue;
            }
            if (pullPrivateMembers || field->getAccess() == clang::AS_public) {
                os << "    pull(reflectable." << field->getName() << ", \"" << field->getName() << "\", value, errors);\n";
                pullWritten = true;
            }
        }
        if (relevantBases.empty() && !pullWritten) {
            os << "    (void)reflectable;\n    (void)value;\n";
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

} // namespace ReflectiveRapidJSON
