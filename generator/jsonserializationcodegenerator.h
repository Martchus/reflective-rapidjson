#ifndef REFLECTIVE_RAPIDJSON_CODE_JSON_SERIALIZATION_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_CODE_JSON_SERIALIZATION_GENERATOR_H

#include "./codegenerator.h"

#include <c++utilities/application/argumentparser.h>

namespace ReflectiveRapidJSON {

/*!
 * \brief The JSONSerializationCodeGenerator class generates code for JSON (de)serialization
 *        of objects inheriting from an instantiation of JsonSerializable.
 */
class JsonSerializationCodeGenerator : public CodeGenerator {
public:
    struct Options {
        Options();

        ApplicationUtilities::Argument additionalClassesArg;
    };

private:
    struct RelevantClass {
        explicit RelevantClass(std::string &&qualifiedName, clang::CXXRecordDecl *record);

        std::string qualifiedName;
        clang::CXXRecordDecl *record;
    };

public:
    JsonSerializationCodeGenerator(CodeFactory &factory, const Options &options);

    void addDeclaration(clang::Decl *decl) override;
    void generate(std::ostream &os) const override;
    std::string qualifiedNameIfRelevant(clang::CXXRecordDecl *record) const;

private:
    std::vector<const RelevantClass *> findRelevantBaseClasses(const RelevantClass &relevantClass) const;

    std::vector<RelevantClass> m_relevantClasses;
    const Options &m_options;
};

inline JsonSerializationCodeGenerator::JsonSerializationCodeGenerator(CodeFactory &factory, const Options &options)
    : CodeGenerator(factory)
    , m_options(options)
{
}

inline JsonSerializationCodeGenerator::RelevantClass::RelevantClass(std::string &&qualifiedName, clang::CXXRecordDecl *record)
    : qualifiedName(qualifiedName)
    , record(record)
{
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CODE_JSON_SERIALIZATION_GENERATOR_H
