#ifndef REFLECTIVE_RAPIDJSON_CODE_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_CODE_GENERATOR_H

#include <iosfwd>
#include <string>
#include <vector>

namespace clang {
class Attr;
class Decl;
class CXXRecordDecl;
class SourceManager;
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
    void lazyInitializeSourceManager() const;
    bool isOnlyIncluded(const clang::Decl *declaration) const;
    std::string_view readAnnotation(const clang::Attr *annotation) const;
    const clang::SourceManager *sourceManager() const;
    static bool inheritsFromInstantiationOf(clang::CXXRecordDecl *record, const char *templateClass);

private:
    CodeFactory &m_factory;
    mutable const clang::SourceManager *m_sourceManager;
};

inline CodeGenerator::CodeGenerator(CodeFactory &factory)
    : m_factory(factory)
    , m_sourceManager(nullptr)
{
}

inline CodeFactory &CodeGenerator::factory() const
{
    return m_factory;
}

inline const clang::SourceManager *CodeGenerator::sourceManager() const
{
    return m_sourceManager;
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CODE_GENERATOR_H
