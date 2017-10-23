#ifndef REFLECTIVE_RAPIDJSON_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_GENERATOR_H

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace clang {
class Decl;
class NamedDecl;
class CXXRecordDecl;
class CompilerInstance;
} // namespace clang

namespace ReflectiveRapidJSON {

class CodeFactory;

/*!
 * \brief The CodeGenerator class is the base for generators used by the CodeFactory class.
 */
class CodeGenerator {
public:
    CodeGenerator(CodeFactory &factory);
    virtual ~CodeGenerator();

    virtual void addDeclaration(clang::Decl *decl);

    /// \brief Generates code based on the previously added declarations. The code is written to \a os.
    virtual void generate(std::ostream &os) const = 0;

protected:
    CodeFactory &factory() const;
    static bool inheritsFromInstantiationOf(clang::CXXRecordDecl *record, const char *templateClass);

private:
    CodeFactory &m_factory;
};

inline CodeGenerator::CodeGenerator(CodeFactory &factory)
    : m_factory(factory)
{
}

inline CodeFactory &CodeGenerator::factory() const
{
    return m_factory;
}

/*!
 * \brief The JSONSerializationCodeGenerator class generates code for JSON (de)serialization
 *        of objects inheriting from an instantiation of JSONSerializable.
 */
class JSONSerializationCodeGenerator : public CodeGenerator {
public:
    JSONSerializationCodeGenerator(CodeFactory &factory);

    void addDeclaration(clang::Decl *decl) override;
    void generate(std::ostream &os) const override;

private:
    struct RelevantClass {
        explicit RelevantClass(const std::string &qualifiedName, clang::CXXRecordDecl *record);

        std::string qualifiedName;
        clang::CXXRecordDecl *record;
    };

    std::vector<RelevantClass> m_relevantClasses;
};

inline JSONSerializationCodeGenerator::JSONSerializationCodeGenerator(CodeFactory &factory)
    : CodeGenerator(factory)
{
}

inline JSONSerializationCodeGenerator::RelevantClass::RelevantClass(const std::string &qualifiedName, clang::CXXRecordDecl *record)
    : qualifiedName(qualifiedName)
    , record(record)
{
}

/*!
 * \brief The CodeFactory class produces additional (reflection) code for a specified list of C++ source files.
 * \remarks
 * - The code is written to a specified std::ostream instance.
 * - The CodeFactory class is constituted by its underlying CodeGenerator instances.
 */
class CodeFactory {
public:
    CodeFactory(
        const char *applicationPath, const std::vector<const char *> &sourceFiles, const std::vector<const char *> &clangOptions, std::ostream &os);
    ~CodeFactory();

    const std::vector<std::unique_ptr<CodeGenerator>> &generators() const;
    template <typename GeneratorType> void addGenerator();

    void addDeclaration(clang::Decl *decl);
    bool readAST();
    bool generate() const;
    clang::CompilerInstance *compilerInstance();
    void setCompilerInstance(clang::CompilerInstance *compilerInstance);

private:
    struct ToolInvocation;

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

#endif // REFLECTIVE_RAPIDJSON_GENERATOR_H
