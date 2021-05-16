#ifndef REFLECTIVE_RAPIDJSON_CODE_FACTORY_H
#define REFLECTIVE_RAPIDJSON_CODE_FACTORY_H

#include "./codegenerator.h"

#include <functional>
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
    CodeFactory(std::string_view applicationPath, const std::vector<const char *> &sourceFiles, const std::vector<std::string_view> &clangOptions,
        std::ostream &os);
    ~CodeFactory();

    const std::vector<std::unique_ptr<CodeGenerator>> &generators() const;
    template <typename GeneratorType, typename... Args> void addGenerator(Args &&...args);
    template <typename GeneratorType, typename... Args> auto bindGenerator(Args &&...args);

    bool run();
    clang::CompilerInstance *compilerInstance();
    void setCompilerInstance(clang::CompilerInstance *compilerInstance);
    bool isErrorResilient() const;
    void setErrorResilient(bool errorResilient);

private:
    struct ToolInvocation;

    void addDeclaration(clang::Decl *decl);
    bool generate() const;
    std::vector<std::string> makeClangArgs() const;

    std::string_view m_applicationPath;
    const std::vector<const char *> &m_sourceFiles;
    const std::vector<std::string_view> &m_clangOptions;
    std::ostream &m_os;
    std::vector<std::unique_ptr<CodeGenerator>> m_generators;
    std::unique_ptr<ToolInvocation> m_toolInvocation;
    clang::CompilerInstance *m_compilerInstance;
    bool m_errorResilient;
};

/*!
 * \brief Instantiates a code generator of the specified type and adds it to the current instance.
 * \remarks The specified \a args are forwarded to the generator's constructor.
 */
template <typename GeneratorType, typename... Args> void CodeFactory::addGenerator(Args &&...args)
{
    m_generators.emplace_back(std::make_unique<GeneratorType>(*this, std::forward<Args>(args)...));
}

namespace Detail {
/*!
 * \brief Wraps const references using std::cref() for use with std::bind().
 */
template <typename T> std::reference_wrapper<const T> wrapReferences(const T &val)
{
    return std::cref(val);
}

/*!
 * \brief Wraps mutable references using std::ref() for use with std::bind().
 */
template <typename T> std::reference_wrapper<T> wrapReferences(T &val)
{
    return std::ref(val);
}

/*!
 * \brief Forwards non-references for use with std::bind().
 */
template <typename T> T &&wrapReferences(T &&val)
{
    return std::forward<T>(val);
}
} // namespace Detail

/*!
 * \brief Returns a function which instantiates a code generator of the specified type and adds it to the current instance.
 * \remarks
 * - The specified \a args are forwarded to the generator's constructor.
 * - No copy of \a args passed by reference is made.
 */
template <typename GeneratorType, typename... Args> auto CodeFactory::bindGenerator(Args &&...args)
{
    return std::bind(&CodeFactory::addGenerator<GeneratorType, Args...>, this, Detail::wrapReferences(std::forward<Args>(args)...));
}

/*!
 * \brief Returns the added generators.
 */
inline const std::vector<std::unique_ptr<CodeGenerator>> &CodeFactory::generators() const
{
    return m_generators;
}

/*!
 * \brief Returns the compiler instance.
 * \remarks The is nullptr for a newly constructed factory and should be assigned by the frontend action.
 */
inline clang::CompilerInstance *CodeFactory::compilerInstance()
{
    return m_compilerInstance;
}

/*!
 * \brief Assigns the compiler instance.
 * \remarks The factory does *not* take ownership.
 */
inline void CodeFactory::setCompilerInstance(clang::CompilerInstance *compilerInstance)
{
    m_compilerInstance = compilerInstance;
}

/*!
 * \brief Returns whether most errors will be turned into warnings (by default false).
 */
inline bool CodeFactory::isErrorResilient() const
{
    return m_errorResilient;
}

/*!
 * \brief Sets whether most errors will be turned into warnings (by default false).
 */
inline void CodeFactory::setErrorResilient(bool errorResilient)
{
    m_errorResilient = errorResilient;
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CODE_FACTORY_H
