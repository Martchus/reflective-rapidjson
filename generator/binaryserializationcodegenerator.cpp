#include "./binaryserializationcodegenerator.h"

#include "../lib/binary/serializable.h"

#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>

#include <iostream>

using namespace std;
using namespace ApplicationUtilities;

namespace ReflectiveRapidJSON {

/*!
 * \brief Initializes the CLI arguments which are specific to the BinarySerializationCodeGenerator.
 * \todo Find a more general approach to pass CLI arguments from main() to the particular code generators.
 */
BinarySerializationCodeGenerator::Options::Options()
    : additionalClassesArg("binary-classes", '\0', "specifies additional classes to consider for binary (de)serialization", { "class-name" })
    , visibilityArg("binary-visibility", '\0', "specifies the \"visibility attribute\" for generated functions", { "attribute" })
{
    additionalClassesArg.setRequiredValueCount(Argument::varValueCount);
    additionalClassesArg.setValueCompletionBehavior(ValueCompletionBehavior::None);
    visibilityArg.setPreDefinedCompletionValues("LIB_EXPORT");
}

/*!
 * \brief Adds all class declarations (to the internal member variable m_records).
 * \remarks "AdaptedBinarySerializable" specializations are directly filtered and added to m_adaptionRecords (instead of m_records).
 */
void BinarySerializationCodeGenerator::addDeclaration(clang::Decl *decl)
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
            auto *const templateSpecializationRecord = static_cast<clang::ClassTemplateSpecializationDecl *>(decl);
            // check whether the name of the template specialization matches
            if (templateSpecializationRecord->getQualifiedNameAsString() == AdaptedBinarySerializable<void>::qualifiedName) {
                // get the template argument of the template specialization (exactly one argument expected)
                const auto &templateArgs = templateSpecializationRecord->getTemplateArgs();
                if (templateArgs.size() != 1 || templateArgs.get(0).getKind() != clang::TemplateArgument::Type) {
                    return; // FIXME: use Clang diagnostics to print warning
                }
                // get the type the template argument refers to (that's the type of the 3rd party class/struct to adapt)
                auto *const templateRecord = templateArgs.get(0).getAsType()->getAsCXXRecordDecl();
                if (!templateRecord) {
                    return; // FIXME: use Clang diagnostics to print warning
                }
                // save the relevant information for the code generation
                m_adaptionRecords.emplace_back(templateRecord->getQualifiedNameAsString(), templateSpecializationRecord);
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
string BinarySerializationCodeGenerator::qualifiedNameIfRelevant(clang::CXXRecordDecl *record) const
{
    // consider all classes for which a specialization of the "AdaptedBinarySerializable" struct is available
    const string qualifiedName(record->getQualifiedNameAsString());
    for (const auto &adaptionRecord : m_adaptionRecords) {
        // skip all adaption records which are only included
        if (isOnlyIncluded(adaptionRecord.record)) {
            continue;
        }
        if (adaptionRecord.qualifiedName == qualifiedName) {
            return qualifiedName;
        }
    }

    // skip all classes which are only included
    if (isOnlyIncluded(record)) {
        return string();
    }

    // consider all classes inheriting from an instantiation of "BinarySerializable" relevant
    if (inheritsFromInstantiationOf(record, BinarySerializable<void>::qualifiedName)) {
        return qualifiedName;
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
std::vector<BinarySerializationCodeGenerator::RelevantClass> BinarySerializationCodeGenerator::findRelevantClasses() const
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
std::vector<const BinarySerializationCodeGenerator::RelevantClass *> BinarySerializationCodeGenerator::findRelevantBaseClasses(
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
 * \brief Generates pull() and push() helper functions in the ReflectiveRapidJSON::BinaryReflector namespace for the relevant classes.
 */
void BinarySerializationCodeGenerator::generate(ostream &os) const
{
    // initialize source manager to make use of isOnlyIncluded() for skipping records which are only included
    lazyInitializeSourceManager();

    // find relevant classes
    const auto relevantClasses = findRelevantClasses();
    if (relevantClasses.empty()) {
        return; // nothing to generate
    }

    // put everything into namespace ReflectiveRapidJSON::BinaryReflector
    os << "namespace ReflectiveRapidJSON {\n"
          "namespace BinaryReflector {\n\n";

    // determine visibility attribute
    const char *visibility = m_options.visibilityArg.firstValue();
    if (!visibility) {
        visibility = "";
    }

    // add push and pull functions for each class, for an example of the resulting
    // output, see ../lib/tests/binaryserializable.cpp
    for (const RelevantClass &relevantClass : relevantClasses) {
        // determine whether private members should be pushed/pulled as well: check whether friend declarations for push/pull present
        // note: the friend declarations we are looking for are expanded from the REFLECTIVE_RAPIDJSON_ENABLE_PRIVATE_MEMBERS macro
        bool writePrivateMembers = false, readPrivateMembers = false;
        for (const clang::FriendDecl *const friendDecl : relevantClass.record->friends()) {
            // get the actual declaration which must be a function
            const clang::NamedDecl *const actualFriendDecl = friendDecl->getFriendDecl();
            if (!actualFriendDecl || actualFriendDecl->getKind() != clang::Decl::Kind::Function) {
                continue;
            }
            // check whether the friend function matches the push/pull helper function
            const string friendName(actualFriendDecl->getQualifiedNameAsString());
            if (friendName == "ReflectiveRapidJSON::BinaryReflector::writeCustomType") {
                writePrivateMembers = true;
            }
            if (friendName == "ReflectiveRapidJSON::BinaryReflector::readCustomType") {
                readPrivateMembers = true;
            }
            if (writePrivateMembers && readPrivateMembers) {
                break;
            }
        }

        // find relevant base classes
        const vector<const RelevantClass *> relevantBases = findRelevantBaseClasses(relevantClass, relevantClasses);

        // print comment
        os << "// define code for (de)serializing " << relevantClass.qualifiedName << " objects\n";

        // print writeCustomType method
        os << "template <> " << visibility << " void writeCustomType<::" << relevantClass.qualifiedName
           << ">(BinarySerializer &serializer, const ::" << relevantClass.qualifiedName << " &customObject)\n{\n"
              "    // write base classes\n";
        for (const RelevantClass *baseClass : relevantBases) {
            os << "    serializer.write(static_cast<const ::" << baseClass->qualifiedName << " &>(customObject));\n";
        }
        os << "    // write members\n";
        for (const clang::FieldDecl *field : relevantClass.record->fields()) {
            if (writePrivateMembers || field->getAccess() == clang::AS_public) {
                os << "    serializer.write(customObject." << field->getName() << ");\n";
            }
        }
        os << "}\n";

        // skip printing the readCustomType method for classes without default constructor because deserializing those is currently not supported
        if (!relevantClass.record->hasDefaultConstructor()) {
            continue;
        }

        // print readCustomType method
        os << "template <> " << visibility << " void readCustomType<::" << relevantClass.qualifiedName
           << ">(BinaryDeserializer &deserializer, ::" << relevantClass.qualifiedName << " &customObject)\n{\n"
              "    // read base classes\n";
        for (const RelevantClass *baseClass : relevantBases) {
            os << "    deserializer.read(static_cast<::" << baseClass->qualifiedName << " &>(customObject));\n";
        }
        os << "    // read members\n";
        for (const clang::FieldDecl *field : relevantClass.record->fields()) {
            // skip const members
            if (field->getType().isConstant(field->getASTContext())) {
                continue;
            }
            if (readPrivateMembers || field->getAccess() == clang::AS_public) {
                os << "    deserializer.read(customObject." << field->getName() << ");\n";
            }
        }
        os << "}\n\n";
    }

    // close namespace ReflectiveRapidJSON::BinaryReflector
    os << "} // namespace BinaryReflector\n"
          "} // namespace ReflectiveRapidJSON\n";
}

} // namespace ReflectiveRapidJSON
