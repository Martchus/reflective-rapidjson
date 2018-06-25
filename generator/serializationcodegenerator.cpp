#include "./serializationcodegenerator.h"

#include <c++utilities/application/global.h>

#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>

using namespace std;

namespace ReflectiveRapidJSON {

/*!
 * \brief Prints an LLVM string reference without instantiating a std::string first.
 */
ostream &operator<<(ostream &os, llvm::StringRef str)
{
    return os.write(str.data(), static_cast<streamsize>(str.size()));
}

/*!
 * \brief Adds all class declarations (to the internal member variable m_records).
 * \remarks "AdaptedXXXSerializable" specializations are directly filtered and added to m_adaptionRecords (instead of m_records).
 */
void SerializationCodeGenerator::addDeclaration(clang::Decl *decl)
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
        if (m_qualifiedNameOfAdaptionRecords && decl->getKind() == clang::Decl::Kind::ClassTemplateSpecialization) {
            auto *const templateSpecializationRecord = static_cast<clang::ClassTemplateSpecializationDecl *>(decl);
            // check whether the name of the template specialization matches
            if (templateSpecializationRecord->getQualifiedNameAsString() == m_qualifiedNameOfAdaptionRecords) {
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

SerializationCodeGenerator::IsRelevant SerializationCodeGenerator::isQualifiedNameIfRelevant(
    clang::CXXRecordDecl *record, const std::string &qualifiedName) const
{
    // consider all classes for which a specialization of the "AdaptedJsonSerializable" struct is available
    for (const auto &adaptionRecord : m_adaptionRecords) {
        // skip all adaption records which are only included
        if (isOnlyIncluded(adaptionRecord.record)) {
            continue;
        }
        if (adaptionRecord.qualifiedName == qualifiedName) {
            return IsRelevant::Yes;
        }
    }

    // skip all classes which are only included
    if (isOnlyIncluded(record)) {
        return IsRelevant::No;
    }

    // consider all classes inheriting from an instantiation of "JsonSerializable" relevant
    if (inheritsFromInstantiationOf(record, m_qualifiedNameOfRecords)) {
        return IsRelevant::Yes;
    }

    return IsRelevant::Maybe;
}

std::vector<SerializationCodeGenerator::RelevantClass> SerializationCodeGenerator::findRelevantClasses() const
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

std::vector<const SerializationCodeGenerator::RelevantClass *> SerializationCodeGenerator::findRelevantBaseClasses(
    const SerializationCodeGenerator::RelevantClass &relevantClass, const std::vector<SerializationCodeGenerator::RelevantClass> &relevantBases)
{
    vector<const RelevantClass *> relevantBaseClasses;
    for (const RelevantClass &otherClass : relevantBases) {
        if (relevantClass.record != otherClass.record && relevantClass.record->isDerivedFrom(otherClass.record)) {
            relevantBaseClasses.push_back(&otherClass);
        }
    }
    return relevantBaseClasses;
}

} // namespace ReflectiveRapidJSON
