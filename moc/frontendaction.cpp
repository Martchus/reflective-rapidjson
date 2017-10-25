#include "./frontendaction.h"
#include "./codefactory.h"
#include "./consumer.h"

#include <c++utilities/application/global.h>

using namespace std;

namespace ReflectiveRapidJSON {

bool FrontendAction::hasCodeCompletionSupport() const
{
    return true;
}

REFLECTIVE_RAPIDJSON_MAYBE_UNIQUE(clang::ASTConsumer)
FrontendAction::CreateASTConsumer(clang::CompilerInstance &compilerInstance, llvm::StringRef inputFile)
{
    VAR_UNUSED(inputFile)

    // propagate compiler instance to factory
    m_factory.setCompilerInstance(&compilerInstance);

    // configure frontent, preporocessor and language options
    clang::FrontendOptions &frontendOpts = compilerInstance.getFrontendOpts();
    clang::Preprocessor &pp = compilerInstance.getPreprocessor();
    clang::LangOptions &lngOpts = compilerInstance.getLangOpts();
    frontendOpts.SkipFunctionBodies = true;
    pp.enableIncrementalProcessing(true);
    pp.SetSuppressIncludeNotFoundError(true);
    lngOpts.DelayedTemplateParsing = true;

    // enable all extensions
    lngOpts.MicrosoftExt = true;
    lngOpts.DollarIdents = true;
    lngOpts.CPlusPlus11 = true;
#if CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR <= 5
    lngOpts.CPlusPlus1y = true;
#else
    lngOpts.CPlusPlus14 = true;
#endif
    lngOpts.GNUMode = true;

    // turn some errors into warnings
    compilerInstance.getDiagnostics().setClient(
        new DiagConsumer(std::unique_ptr<clang::DiagnosticConsumer>(compilerInstance.getDiagnostics().takeClient())));

    return maybe_unique(new Consumer(m_factory, compilerInstance));
}
} // namespace ReflectiveRapidJSON
