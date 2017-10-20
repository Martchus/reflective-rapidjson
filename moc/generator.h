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
} // namespace clang

namespace ReflectiveRapidJSON {

/*!
 * \brief The CodeGenerator class is the base for generators used by the CodeFactory class.
 */
class CodeGenerator {
public:
    CodeGenerator();
    virtual ~CodeGenerator();

    virtual void addDeclaration(clang::Decl *decl);

    /// \brief Generates code based on the previously added declarations. The code is written to \a os.
    virtual void generate(std::ostream &os) const = 0;

protected:
    static bool inheritsFromInstantiationOf(clang::CXXRecordDecl *record, const char *templateClass);
};

inline CodeGenerator::CodeGenerator()
{
}

/*!
 * \brief The JSONSerializationCodeGenerator class generates code for JSON (de)serialization.
 */
class JSONSerializationCodeGenerator : public CodeGenerator {
public:
    JSONSerializationCodeGenerator();

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

inline JSONSerializationCodeGenerator::JSONSerializationCodeGenerator()
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
    CodeFactory(const char *applicationPath, const std::vector<const char *> &sourceFiles, std::ostream &os);
    ~CodeFactory();

    void addDeclaration(clang::Decl *decl);
    bool readAST();
    bool generate() const;

private:
    struct ToolInvocation;

    std::vector<std::string> makeClangArgs() const;

    const char *const m_applicationPath;
    const std::vector<const char *> &m_sourceFiles;
    std::ostream &m_os;
    std::vector<std::unique_ptr<CodeGenerator>> m_generators;
    std::unique_ptr<ToolInvocation> m_toolInvocation;
};

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_GENERATOR_H
