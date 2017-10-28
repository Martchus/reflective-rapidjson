#include "./codegenerator.h"

#include <c++utilities/application/global.h>

#include <clang/AST/DeclCXX.h>

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

} // namespace ReflectiveRapidJSON
