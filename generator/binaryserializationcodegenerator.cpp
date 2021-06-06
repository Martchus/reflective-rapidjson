#include "./binaryserializationcodegenerator.h"

#include "../lib/binary/serializable.h"

#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <llvm/ADT/APInt.h>

#include <iostream>

using namespace std;
using namespace CppUtilities;

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
    visibilityArg.setPreDefinedCompletionValues("CPP_UTILITIES_GENERIC_LIB_EXPORT");
}

BinarySerializationCodeGenerator::BinarySerializationCodeGenerator(CodeFactory &factory, const Options &options)
    : SerializationCodeGenerator(factory)
    , m_options(options)
{
    m_qualifiedNameOfRecords = BinarySerializable<void>::qualifiedName;
    m_qualifiedNameOfAdaptionRecords = AdaptedBinarySerializable<void>::qualifiedName;
}

/*!
 * \brief Returns the qualified name of the specified \a record if it is considered relevant.
 */
string BinarySerializationCodeGenerator::qualifiedNameIfRelevant(clang::CXXRecordDecl *record) const
{
    const string qualifiedName(record->getQualifiedNameAsString());
    switch (isQualifiedNameIfRelevant(record, qualifiedName)) {
    case IsRelevant::Yes:
        return qualifiedName;
    case IsRelevant::No:
        return string();
    default:;
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

/// \brief The RetrieveIntegerLiteralFromDeclaratorDecl struct is used to traverse a variable declaration to get the integer value.
struct RetrieveIntegerLiteralFromDeclaratorDecl : public clang::RecursiveASTVisitor<RetrieveIntegerLiteralFromDeclaratorDecl> {
    explicit RetrieveIntegerLiteralFromDeclaratorDecl(const clang::ASTContext &ctx);
    bool VisitStmt(clang::Stmt *st);
    const clang::ASTContext &ctx;
    std::uint64_t res;
    bool success;
};

/// \brief Constructs a new instance for the specified AST context.
RetrieveIntegerLiteralFromDeclaratorDecl::RetrieveIntegerLiteralFromDeclaratorDecl(const clang::ASTContext &ctx)
    : ctx(ctx)
    , res(0)
    , success(false)
{
}

/// \brief Reads the integer value of \a st for integer literals.
bool RetrieveIntegerLiteralFromDeclaratorDecl::VisitStmt(clang::Stmt *st)
{
    if (st->getStmtClass() != clang::Stmt::IntegerLiteralClass) {
        return true;
    }
    const auto *const integerLiteral = static_cast<const clang::IntegerLiteral *>(st);
    auto evaluation = clang::Expr::EvalResult();
    integerLiteral->EvaluateAsInt(evaluation, ctx, clang::Expr::SE_NoSideEffects, true);
    if (!evaluation.Val.isInt()) {
        return true;
    }
    const auto &asInt = evaluation.Val.getInt();
    if (asInt.getActiveBits() > 64) {
        return true;
    }
    res = asInt.getZExtValue();
    success = true;
    return false;
}

/// \brief The MemberTracking struct is an internal helper for BinarySerializationCodeGenerator::generate().
struct MemberTracking {
    bool membersWritten = false, withinCondition = false;
    BinaryVersion asOfVersion = BinaryVersion(), lastAsOfVersion = BinaryVersion();
    BinaryVersion untilVersion = BinaryVersion(), lastUntilVersion = BinaryVersion();

    bool checkForVersionMarker(clang::Decl *decl);
    void concludeCondition(std::ostream &os);
    void writeVersionCondition(std::ostream &os);
    void writeExtraPadding(std::ostream &os);
};

/*!
 * \brief Returns whether \a delc is a static member variable and processes special static member variables
 *        for versioning.
 */
bool MemberTracking::checkForVersionMarker(clang::Decl *decl)
{
    if (decl->getKind() != clang::Decl::Kind::Var) {
        return false;
    }
    auto *const declarator = static_cast<clang::DeclaratorDecl *>(decl);
    const auto declarationName = declarator->getName();
    const auto isAsOfVersion = declarationName.startswith("rrjAsOfVersion");
    if (isAsOfVersion || declarationName.startswith("rrjUntilVersion")) {
        auto v = RetrieveIntegerLiteralFromDeclaratorDecl(declarator->getASTContext());
        v.TraverseDecl(declarator);
        if (v.success) {
            if (isAsOfVersion) {
                asOfVersion = v.res;
                if (asOfVersion > untilVersion) {
                    untilVersion = 0;
                }
            } else {
                untilVersion = v.res;
                if (untilVersion < asOfVersion) {
                    asOfVersion = 0;
                }
            }
        }
    }
    return true;
}

/*!
 * \brief Concludes an unfinished version condition if-block.
 */
void MemberTracking::concludeCondition(std::ostream &os)
{
    if (withinCondition) {
        os << "    }\n";
    }
}

/*!
 * \brief Starts a new version condition if-block if versioning parameters have changed.
 */
void MemberTracking::writeVersionCondition(std::ostream &os)
{
    if (asOfVersion == lastAsOfVersion && untilVersion == lastUntilVersion) {
        return;
    }
    concludeCondition(os);
    lastAsOfVersion = asOfVersion;
    lastUntilVersion = untilVersion;
    if ((withinCondition = asOfVersion || untilVersion)) {
        os << "    if (";
        if (asOfVersion) {
            os << "version >= " << asOfVersion;
            if (untilVersion) {
                os << " && ";
            }
        }
        if (untilVersion) {
            os << "version <= " << untilVersion;
        }
        os << ") {\n";
    }
}

/*!
 * \brief Writes extra padding (if within a version condition).
 */
void MemberTracking::writeExtraPadding(std::ostream &os)
{
    if (withinCondition) {
        os << "    ";
    }
}

/*!
 * \brief Generates pull() and push() helper functions in the ReflectiveRapidJSON::BinaryReflector namespace for the relevant classes.
 */
void BinarySerializationCodeGenerator::generate(std::ostream &os) const
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
           << ">(BinarySerializer &serializer, const ::" << relevantClass.qualifiedName
           << " &customObject, BinaryVersion version)\n{\n"
              "    // write base classes\n";
        for (const RelevantClass *baseClass : relevantBases) {
            os << "    serializer.write(static_cast<const ::" << baseClass->qualifiedName << " &>(customObject), version);\n";
        }
        os << "    // write members\n";
        auto mt = MemberTracking();
        for (clang::Decl *const decl : relevantClass.record->decls()) {
            // check static member variables for version markers
            if (mt.checkForVersionMarker(decl))  {
                continue;
            }

            // skip all further declarations but fields
            if (decl->getKind() != clang::Decl::Kind::Field) {
                continue;
            }

            // skip const members
            const auto *const field = static_cast<const clang::FieldDecl *>(decl);
            if (field->getType().isConstant(field->getASTContext())) {
                continue;
            }

            // skip private members conditionally
            if (!writePrivateMembers && field->getAccess() != clang::AS_public) {
                continue;
            }

            // write version markers
            mt.writeVersionCondition(os);
            mt.writeExtraPadding(os);

            // write actual code for serialization
            os << "    serializer.write(customObject." << field->getName() << ", version);\n";
            mt.membersWritten = true;
        }
        mt.concludeCondition(os);
        if (relevantBases.empty() && !mt.membersWritten) {
            os << "    (void)serializer;\n    (void)customObject;\n";
        }
        os << "}\n";

        // skip printing the readCustomType method for classes without default constructor because deserializing those is currently not supported
        if (!relevantClass.record->hasDefaultConstructor()) {
            continue;
        }

        // print readCustomType method
        mt = MemberTracking();
        os << "template <> " << visibility << " void readCustomType<::" << relevantClass.qualifiedName
           << ">(BinaryDeserializer &deserializer, ::" << relevantClass.qualifiedName
           << " &customObject)\n{\n"
              "    // read base classes\n";
        for (const RelevantClass *baseClass : relevantBases) {
            os << "    deserializer.read(static_cast<::" << baseClass->qualifiedName << " &>(customObject));\n";
        }
        os << "    // read members\n";
        for (clang::Decl *const decl : relevantClass.record->decls()) {
            // check static member variables for version markers
            if (mt.checkForVersionMarker(decl))  {
                continue;
            }

            // skip all further declarations but fields
            if (decl->getKind() != clang::Decl::Kind::Field) {
                continue;
            }

            // skip const members
            const auto *const field = static_cast<const clang::FieldDecl *>(decl);
            if (field->getType().isConstant(field->getASTContext())) {
                continue;
            }

            // write version markers
            mt.writeVersionCondition(os);
            mt.writeExtraPadding(os);

            if (readPrivateMembers || field->getAccess() == clang::AS_public) {
                os << "    deserializer.read(customObject." << field->getName() << ");\n";
                mt.membersWritten = true;
            }
        }
        mt.concludeCondition(os);
        if (relevantBases.empty() && !mt.membersWritten) {
            os << "    (void)deserializer;\n    (void)customObject;\n";
        }
        os << "}\n\n";
    }

    // close namespace ReflectiveRapidJSON::BinaryReflector
    os << "} // namespace BinaryReflector\n"
          "} // namespace ReflectiveRapidJSON\n";
}

} // namespace ReflectiveRapidJSON
