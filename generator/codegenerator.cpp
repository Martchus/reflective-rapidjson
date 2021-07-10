#include "./codegenerator.h"
#include "./codefactory.h"

#include <c++utilities/application/global.h>

#include <clang/AST/DeclCXX.h>
#include <clang/Frontend/CompilerInstance.h>

using namespace std;

namespace ReflectiveRapidJSON {

CodeGenerator::~CodeGenerator()
{
}

/*!
 * \brief Adds the specified \a decl to the code generator. The generator might ignore irrelevant declarations.
 */
void CodeGenerator::addDeclaration(clang::Decl *decl)
{
    CPP_UTILITIES_UNUSED(decl)
}

/*!
 * \brief Lazy initializes the source manager.
 * \remarks This method must be called in generate() when subclassing to make use of isOnlyIncluded().
 */
void CodeGenerator::lazyInitializeSourceManager() const
{
    if (factory().compilerInstance()) {
        m_sourceManager = &factory().compilerInstance()->getSourceManager();
    }
}

/*!
 * \brief Returns whether the specified \a declaration is only included and not part of the actual file.
 */
bool CodeGenerator::isOnlyIncluded(const clang::Decl *declaration) const
{
    return m_sourceManager
        && m_sourceManager->getFileID(m_sourceManager->getExpansionLoc(declaration->getSourceRange().getBegin())) != m_sourceManager->getMainFileID();
}

/*!
 * \brief Returns whether the specified \a record inherits from an instantiation of the specified \a templateClass.
 * \returns Returns the relevant base class if that's the case and otherwise nullptr.
 * \remarks The specified \a record must be defined (not only forward-declared).
 */
clang::CXXBaseSpecifier *CodeGenerator::inheritsFromInstantiationOf(clang::CXXRecordDecl *const record, const char *const templateClass)
{
    for (clang::CXXBaseSpecifier &base : record->bases()) {
        clang::CXXRecordDecl *const baseDecl = base.getType()->getAsCXXRecordDecl();
        if (baseDecl && baseDecl->getQualifiedNameAsString() == templateClass) {
            return &base;
        }
    }
    return nullptr;
}

} // namespace ReflectiveRapidJSON
