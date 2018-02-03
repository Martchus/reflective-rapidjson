#include "./consumer.h"
#include "./codefactory.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclGroup.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Basic/MacroBuilder.h>
#include <clang/Sema/Sema.h>

using namespace std;

namespace ReflectiveRapidJSON {

bool Consumer::shouldParseDecl(clang::Decl *declaration)
{
    const clang::SourceManager &sourceManager = m_compilerInstance.getSourceManager();
    return sourceManager.getFileID(sourceManager.getExpansionLoc(declaration->getSourceRange().getBegin())) == sourceManager.getMainFileID();
}

bool Consumer::HandleTopLevelDecl(clang::DeclGroupRef groupRefDecl)
{
    for (clang::Decl *decl : groupRefDecl) {
        if (clang::NamespaceDecl *namespaceDecl = llvm::dyn_cast<clang::NamespaceDecl>(decl)) {
            if (!shouldParseDecl(namespaceDecl)) {
                continue;
            }
        }
    }
    return clang::ASTConsumer::HandleTopLevelDecl(groupRefDecl);
}

void Consumer::HandleTranslationUnit(clang::ASTContext &context)
{
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    m_factory.generate();
}

void DiagConsumer::BeginSourceFile(const clang::LangOptions &langOpts, const clang::Preprocessor *pp)
{
    m_proxy->BeginSourceFile(langOpts, pp);
}

void DiagConsumer::clear()
{
    m_proxy->clear();
}

void DiagConsumer::EndSourceFile()
{
    m_proxy->EndSourceFile();
}

void DiagConsumer::finish()
{
    m_proxy->finish();
}

/*!
 * \brief Turns most errors into warnings so the code generator can even work when parsing incomplete headers.
 */
void DiagConsumer::HandleDiagnostic(clang::DiagnosticsEngine::Level diagLevel, const clang::Diagnostic &info)
{
    bool shouldReset = false;
    if (m_errorResilient) {
        const auto diagId = info.getID();
        const auto category = info.getDiags()->getDiagnosticIDs()->getCategoryNumberForDiag(diagId);

        if (diagLevel >= clang::DiagnosticsEngine::Error) {
            if (category == 2 /* 2 means "Semantic Issue" */) {
                if (!m_realErrorCount) {
                    shouldReset = true;
                }
                diagLevel = clang::DiagnosticsEngine::Warning;
            } else {
                ++m_realErrorCount;
            }
        }
    }

    DiagnosticConsumer::HandleDiagnostic(diagLevel, info);
    m_proxy->HandleDiagnostic(diagLevel, info);

    if (shouldReset) {
        // FIXME: is there another way to ignore errors?
        const_cast<clang::DiagnosticsEngine *>(info.getDiags())->Reset();
    }
}

} // namespace ReflectiveRapidJSON
