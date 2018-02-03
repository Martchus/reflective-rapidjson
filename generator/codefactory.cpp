#include "./codefactory.h"
#include "./frontendaction.h"

#include <clang/Basic/FileManager.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#include <memory>

using namespace std;

namespace ReflectiveRapidJSON {

struct CodeFactory::ToolInvocation {
    ToolInvocation(CodeFactory &factory);

    clang::FileManager fileManager;
    clang::tooling::ToolInvocation invocation;
};

CodeFactory::ToolInvocation::ToolInvocation(CodeFactory &factory)
    : fileManager({ "." })
    , invocation(factory.makeClangArgs(), new FrontendAction(factory), &fileManager)
{
    fileManager.Retain();
}

/*!
 * \brief Constructs a new instance.
 * \remarks The specified arguments are not copied and must remain valid for the live-time of the code factory.
 */
CodeFactory::CodeFactory(
    const char *applicationPath, const std::vector<const char *> &sourceFiles, const std::vector<string> &clangOptions, std::ostream &os)
    : m_applicationPath(applicationPath)
    , m_sourceFiles(sourceFiles)
    , m_clangOptions(clangOptions)
    , m_os(os)
    , m_compilerInstance(nullptr)
    , m_errorResilient(true)
{
}

CodeFactory::~CodeFactory()
{
}

/*!
 * \brief Constructs arguments for the Clang tool invocation.
 */
std::vector<string> CodeFactory::makeClangArgs() const
{
    static const initializer_list<const char *> flags
        = { m_applicationPath, "-x", "c++", "-Wno-pragma-once-outside-header", "-std=c++14", "-fsyntax-only" };
    vector<string> clangArgs;
    clangArgs.reserve(flags.size() + m_clangOptions.size() + m_sourceFiles.size());
    clangArgs.insert(clangArgs.end(), flags.begin(), flags.end());
    clangArgs.insert(clangArgs.end(), m_clangOptions.cbegin(), m_clangOptions.cend());
    clangArgs.insert(clangArgs.end(), m_sourceFiles.cbegin(), m_sourceFiles.cend());
    return clangArgs;
}

/*!
 * \brief Adds the specified \a decl to all underlying code generators. The generators might ignore irrelevant declarations.
 * \remarks Supposed to be called by assigned generators inside readAST().
 */
void CodeFactory::addDeclaration(clang::Decl *decl)
{
    for (const auto &generator : m_generators) {
        generator->addDeclaration(decl);
    }
}

/*!
 * \brief Generates code based on the added declarations.
 */
bool CodeFactory::generate() const
{
    for (const auto &generator : m_generators) {
        generator->generate(m_os);
    }
    return true;
}

/*!
 * \brief Reads (relevent) AST elements using Clang and generates code.
 */
bool CodeFactory::run()
{
    // lazy initialize Clang tool invocation
    if (!m_toolInvocation) {
        m_toolInvocation = make_unique<ToolInvocation>(*this);
    }
    // run Clang
    return m_toolInvocation->invocation.run();
}

} // namespace ReflectiveRapidJSON
