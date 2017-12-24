#include "./jsonserializationcodegenerator.h"

#include "../lib/json/serializable.h"

#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>

#include <iostream>

using namespace std;
using namespace ApplicationUtilities;

namespace ReflectiveRapidJSON {

/*!
 * \brief Initializes the CLI arguments which are specific to the JsonSerializationCodeGenerator.
 * \todo Find a more general approach to pass CLI arguments from main() to the particular code generators.
 */
JsonSerializationCodeGenerator::Options::Options()
    : additionalClassesArg("json-classes", '\0', "specifies additional classes to consider for JSON serialization", { "class-name" })
    , visibilityArg("json-visibility", '\0', "specifies the \"visibility attribute\" for generated functions", { "attribute" })
{
    additionalClassesArg.setRequiredValueCount(Argument::varValueCount);
    additionalClassesArg.setValueCompletionBehavior(ValueCompletionBehavior::None);
    visibilityArg.setPreDefinedCompletionValues("LIB_EXPORT");
}

/*!
 * \brief Adds all class declarations (to the internal member variable m_records).
 * \remarks "AdaptedJsonSerializable" specializations are directly filtered and added to m_adaptionRecords (instead of m_records).
 */
void JsonSerializationCodeGenerator::addDeclaration(clang::Decl *decl)
{
    switch (decl->getKind()) {
    case clang::Decl::Kind::CXXRecord:
    case clang::Decl::Kind::ClassTemplateSpecialization: {
        auto *const record = static_cast<clang::CXXRecordDecl *>(decl);
        // skip forward declarations
        if (!record->hasDefinition()) {
            return;
        }

        // check for template specializations to adapt a 3rd party class/struct
        if (decl->getKind() == clang::Decl::Kind::ClassTemplateSpecialization) {
            auto *const templRecord = static_cast<clang::ClassTemplateSpecializationDecl *>(decl);
            if (templRecord->getQualifiedNameAsString() == AdaptedJsonSerializable<void>::qualifiedName) {
                const clang::TemplateArgumentList &templateArgs = templRecord->getTemplateArgs();
                if (templateArgs.size() != 1 || templateArgs.get(0).getKind() != clang::TemplateArgument::Type) {
                    return; // FIXME: use Clang diagnostics to print warning
                }
                const clang::CXXRecordDecl *templateRecord = templateArgs.get(0).getAsType()->getAsCXXRecordDecl();
                if (!templateRecord) {
                    return; // FIXME: use Clang diagnostics to print warning
                }
                m_adaptionRecords.emplace_back(templateRecord->getQualifiedNameAsString());
                return;
            }
        }

        // add any other records
        m_records.emplace_back(record);
    } break;
    case clang::Decl::Kind::Enum:
        // TODO: add enums
        break;
    default:;
    }
}

/*!
 * \brief Returns the qualified name of the specified \a record if it is considered relevant.
 */
string JsonSerializationCodeGenerator::qualifiedNameIfRelevant(clang::CXXRecordDecl *record) const
{
    // skip all classes which are only included
    if (isOnlyIncluded(record)) {
        return string();
    }

    // consider all classes inheriting from an instantiation of "JsonSerializable" relevant
    if (inheritsFromInstantiationOf(record, JsonSerializable<void>::qualifiedName)) {
        return record->getQualifiedNameAsString();
    }

    // consider all classes for which a specialization of the "AdaptedJsonSerializable" struct is available
    const string qualifiedName(record->getQualifiedNameAsString());
    for (const string &adaptionRecord : m_adaptionRecords) {
        if (adaptionRecord == qualifiedName) {
            return qualifiedName;
        }
    }

    // consider all classes specified via "--additional-classes" argument relevant
    if (!m_options.additionalClassesArg.isPresent()) {
        return string();
    }
    for (const char *className : m_options.additionalClassesArg.values()) {
        if (className == qualifiedName) {
            return qualifiedName;
        }
    }

    return string();
}

/*!
 * \brief Searches the records added via addDeclaration() and returns the relevant ones.
 * \sa Whether a record is relevant is determined using the qualifiedNameIfRelevant() method.
 */
std::vector<JsonSerializationCodeGenerator::RelevantClass> JsonSerializationCodeGenerator::findRelevantClasses() const
{
    std::vector<RelevantClass> relevantClasses;
    for (clang::CXXRecordDecl *record : m_records) {
        string qualifiedName(qualifiedNameIfRelevant(record));
        if (!qualifiedName.empty()) {
            relevantClasses.emplace_back(move(qualifiedName), record);
        }
    }
    return relevantClasses;
}

/*!
 * \brief Returns the relevant base classes of the specified \a relevantClass. All base classes in \a relevantBases are considered relevant.
 */
std::vector<const JsonSerializationCodeGenerator::RelevantClass *> JsonSerializationCodeGenerator::findRelevantBaseClasses(
    const RelevantClass &relevantClass, const std::vector<RelevantClass> &relevantBases)
{
    vector<const RelevantClass *> relevantBaseClasses;
    for (const RelevantClass &otherClass : relevantBases) {
        if (relevantClass.record != otherClass.record && relevantClass.record->isDerivedFrom(otherClass.record)) {
            relevantBaseClasses.push_back(&otherClass);
        }
    }
    return relevantBaseClasses;
}

/*!
 * \brief Prints an LLVM string reference without instantiating a std::string first.
 */
ostream &operator<<(ostream &os, llvm::StringRef str)
{
    return os.write(str.data(), static_cast<streamsize>(str.size()));
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
        for (const clang::FieldDecl *field : relevantClass.record->fields()) {
            if (pushPrivateMembers || field->getAccess() == clang::AS_public) {
                os << "    push(reflectable." << field->getName() << ", \"" << field->getName() << "\", value, allocator);\n";
            }
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
        for (const clang::FieldDecl *field : relevantClass.record->fields()) {
            if (pullPrivateMembers || field->getAccess() == clang::AS_public) {
                os << "    pull(reflectable." << field->getName() << ", \"" << field->getName() << "\", value, errors);\n";
            }
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
