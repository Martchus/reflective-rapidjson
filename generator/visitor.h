#ifndef REFLECTIVE_RAPIDJSON_VISITOR_H
#define REFLECTIVE_RAPIDJSON_VISITOR_H

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>

namespace ReflectiveRapidJSON {

class CodeFactory;

/*!
 * \brief The Visitor class is used to traverse the elements of a translation unit. For this purpose, it is instantiated by the Consumer class.
 */
class Visitor : public clang::RecursiveASTVisitor<Visitor> {
public:
    explicit Visitor(CodeFactory &factory);
    bool VisitDecl(clang::Decl *decl);
    bool VisitFunctionDecl(clang::FunctionDecl *func);
    bool VisitStmt(clang::Stmt *st);
    bool VisitNamespaceDecl(clang::NamespaceDecl *decl);
    bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl);

private:
    CodeFactory &m_factory;
};

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_VISITOR_H
