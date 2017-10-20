#ifndef REFLECTIVE_RAPIDJSON_FRONTEND_ACTION_H
#define REFLECTIVE_RAPIDJSON_FRONTEND_ACTION_H
#include "./clangversionabstraction.h"

#include <clang/Frontend/FrontendAction.h>

namespace ReflectiveRapidJSON {

class CodeFactory;

/*!
 * \brief The FrontendAction class instantiates the AST-Consumer (Consumer class). An instance is passed to clang::tooling::ToolInvocation.
 */
class FrontendAction : public clang::ASTFrontendAction {
public:
    FrontendAction(CodeFactory &factory);
    bool hasCodeCompletionSupport() const override;

protected:
    REFLECTIVE_RAPIDJSON_MAYBE_UNIQUE(clang::ASTConsumer)
    CreateASTConsumer(clang::CompilerInstance &compilerInstance, llvm::StringRef inputFile) override;

private:
    CodeFactory &m_factory;
};

inline FrontendAction::FrontendAction(CodeFactory &factory)
    : m_factory(factory)
{
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_FRONTEND_ACTION_H
