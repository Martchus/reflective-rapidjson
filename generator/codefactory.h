#ifndef REFLECTIVE_RAPIDJSON_CODE_FACTORY_H
#define REFLECTIVE_RAPIDJSON_CODE_FACTORY_H

#include "./codegenerator.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace clang {
class CompilerInstance;
} // namespace clang

namespace ReflectiveRapidJSON {

class Consumer;
class Visitor;

/*!
 * \brief The CodeFactory class produces additional (reflection) code for a specified list of C++ source files.
 * \remarks
 * - The code is written to a specified std::ostream instance.
 * - The CodeFactory class is constituted by its underlying CodeGenerator instances.
 */
class CodeFactory {
    friend class Consumer;
    friend class Visitor;

public:
    CodeFactory(
        const char *applicationPath, const std::vector<const char *> &sourceFiles, const std::vector<const char *> &clangOptions, std::ostream &os);
    ~CodeFactory();

    const std::vector<std::unique_ptr<CodeGenerator>> &generators() const;
    template <typename GeneratorType> void addGenerator();

    bool run();
    clang::CompilerInstance *compilerInstance();
    void setCompilerInstance(clang::CompilerInstance *compilerInstance);

private:
    struct ToolInvocation;

    void addDeclaration(clang::Decl *decl);
    bool generate() const;
    std::vector<std::string> makeClangArgs() const;

    const char *const m_applicationPath;
    const std::vector<const char *> &m_sourceFiles;
    const std::vector<const char *> &m_clangOptions;
    std::ostream &m_os;
    std::vector<std::unique_ptr<CodeGenerator>> m_generators;
    std::unique_ptr<ToolInvocation> m_toolInvocation;
    clang::CompilerInstance *m_compilerInstance;
};

template <typename GeneratorType> void CodeFactory::addGenerator()
{
    m_generators.emplace_back(std::make_unique<GeneratorType>(*this));
}

inline const std::vector<std::unique_ptr<CodeGenerator>> &CodeFactory::generators() const
{
    return m_generators;
}

inline clang::CompilerInstance *CodeFactory::compilerInstance()
{
    return m_compilerInstance;
}

inline void CodeFactory::setCompilerInstance(clang::CompilerInstance *compilerInstance)
{
    m_compilerInstance = compilerInstance;
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CODE_FACTORY_H
