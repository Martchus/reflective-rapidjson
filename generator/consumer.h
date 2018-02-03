#ifndef REFLECTIVE_RAPIDJSON_CONSUMER_H
#define REFLECTIVE_RAPIDJSON_CONSUMER_H

#include "./visitor.h"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Decl.h>
#include <clang/Basic/DiagnosticIDs.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/LexDiagnostic.h>

namespace ReflectiveRapidJSON {

class CodeFactory;

/*!
 * \brief The Consumer class is passed to FrontendAction for handling occurrences of different elements of the file.
 *
 * These elements consist of top-level declarations, namespace definitions and most importantly the whole translation unit.
 * If the translations unit has occurred, that means nested elements (eg. classes) have been read completely.
 * In this case, the Consumer class will trigger traversing the translation unit using a Visitor instance.
 */
class Consumer : public clang::ASTConsumer {
public:
    Consumer(CodeFactory &factory, clang::CompilerInstance &compilerInstance);

    bool HandleTopLevelDecl(clang::DeclGroupRef groupRefDecl) override;
    void HandleTranslationUnit(clang::ASTContext &context) override;

    bool shouldParseDecl(clang::Decl *declaration);

private:
    void handleNamespaceDefinition(clang::NamespaceDecl *nsDecl);

    CodeFactory &m_factory;
    clang::CompilerInstance &m_compilerInstance;
    Visitor m_visitor;
};

inline Consumer::Consumer(CodeFactory &factory, clang::CompilerInstance &compilerInstance)
    : m_factory(factory)
    , m_compilerInstance(compilerInstance)
    , m_visitor(factory)
{
}

/*!
 * \brief The DiagConsumer class allows turning most errors into warnings.
 * \remarks This class is based on MocDiagConsumer from https://github.com/woboq/moc-ng.
 */
class DiagConsumer : public clang::DiagnosticConsumer {
public:
    DiagConsumer(std::unique_ptr<DiagnosticConsumer> previous, bool errorResilient);
    unsigned int realErrorCount() const;

    void BeginSourceFile(const clang::LangOptions &langOpts, const clang::Preprocessor *pp = nullptr) override;
    void clear() override;
    void EndSourceFile() override;
    void finish() override;
    void HandleDiagnostic(clang::DiagnosticsEngine::Level diagLevel, const clang::Diagnostic &info) override;

private:
    std::unique_ptr<DiagnosticConsumer> m_proxy;
    unsigned int m_realErrorCount;
    bool m_errorResilient;
};

inline DiagConsumer::DiagConsumer(std::unique_ptr<clang::DiagnosticConsumer> previous, bool errorResilient)
    : m_proxy(std::move(previous))
    , m_realErrorCount(0)
    , m_errorResilient(errorResilient)
{
}

/*!
 * \brief Returns the number of errors which actually would have been occurred if we would not turn most errors into warnings.
 */
inline unsigned int DiagConsumer::realErrorCount() const
{
    return m_realErrorCount;
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CONSUMER_H
