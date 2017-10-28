#ifndef REFLECTIVE_RAPIDJSON_CODE_JSON_SERIALIZATION_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_CODE_JSON_SERIALIZATION_GENERATOR_H

#include "./codegenerator.h"

namespace ReflectiveRapidJSON {

/*!
 * \brief The JSONSerializationCodeGenerator class generates code for JSON (de)serialization
 *        of objects inheriting from an instantiation of JsonSerializable.
 */
class JsonSerializationCodeGenerator : public CodeGenerator {
public:
    JsonSerializationCodeGenerator(CodeFactory &factory);

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

inline JsonSerializationCodeGenerator::JsonSerializationCodeGenerator(CodeFactory &factory)
    : CodeGenerator(factory)
{
}

inline JsonSerializationCodeGenerator::RelevantClass::RelevantClass(const std::string &qualifiedName, clang::CXXRecordDecl *record)
    : qualifiedName(qualifiedName)
    , record(record)
{
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CODE_JSON_SERIALIZATION_GENERATOR_H
