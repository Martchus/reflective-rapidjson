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
    CPP_UTILITIES_UNUSED(inputFile)

    // propagate compiler instance to factory
    m_factory.setCompilerInstance(&compilerInstance);

    // turn some errors into warnings
    compilerInstance.getDiagnostics().setClient(
        new DiagConsumer(std::unique_ptr<clang::DiagnosticConsumer>(compilerInstance.getDiagnostics().takeClient()), m_factory.isErrorResilient()));

    return maybe_unique(new Consumer(m_factory, compilerInstance));
}
} // namespace ReflectiveRapidJSON
