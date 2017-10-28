#ifndef REFLECTIVE_RAPIDJSON_CODE_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_CODE_GENERATOR_H

#include <iosfwd>
#include <string>
#include <vector>

namespace clang {
class Decl;
class CXXRecordDecl;
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

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CODE_GENERATOR_H
