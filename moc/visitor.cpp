#include "./visitor.h"
#include "./codefactory.h"

#include <c++utilities/application/global.h>

#include <clang/AST/CXXInheritance.h>

#include <iostream>

using namespace std;

namespace ReflectiveRapidJSON {

/*!
 * \brief Constructs a new Visitor.
 */
Visitor::Visitor(CodeFactory &factory)
    : m_factory(factory)
{
}

/*!
 * \brief Adds any kind of declaration to the factory.
 */
bool Visitor::VisitDecl(clang::Decl *decl)
{
    m_factory.addDeclaration(decl);
    return true;
}

/*!
 * \brief Visits function declarations. Currently not used.
 * \remarks Might be used later to detect functions.
 */
bool ReflectiveRapidJSON::Visitor::VisitFunctionDecl(clang::FunctionDecl *func)
{
    VAR_UNUSED(func)
    return true;
}

/*!
 * \brief Visits statements. Currently not used.
 */
bool ReflectiveRapidJSON::Visitor::VisitStmt(clang::Stmt *st)
{
    VAR_UNUSED(st)
    return true;
}

/*!
 * \brief Visits namespace declarations. Currently not used.
 */
bool Visitor::VisitNamespaceDecl(clang::NamespaceDecl *decl)
{
    VAR_UNUSED(decl)
    return true;
}

/*!
 * \brief Visits classes and class templates. Currently not used.
 */
bool Visitor::VisitCXXRecordDecl(clang::CXXRecordDecl *decl)
{
    VAR_UNUSED(decl)
    return true;
}

} // namespace ReflectiveRapidJSON
