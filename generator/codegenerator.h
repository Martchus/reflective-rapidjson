#ifndef REFLECTIVE_RAPIDJSON_CODE_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_CODE_GENERATOR_H

#include <iosfwd>
#include <string>
#include <vector>

namespace clang {
class Decl;
class NamedDecl;
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

/*!
 * \brief The JSONSerializationCodeGenerator class generates code for JSON (de)serialization
 *        of objects inheriting from an instantiation of JsonSerializable.
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

    std::vector<const RelevantClass *> findRelevantBaseClasses(const RelevantClass &relevantClass) const;

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

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CODE_GENERATOR_H
