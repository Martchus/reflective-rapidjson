#ifndef REFLECTIVE_RAPIDJSON_CODE_BINARY_SERIALIZATION_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_CODE_BINARY_SERIALIZATION_GENERATOR_H

#include "./codegenerator.h"

#include <c++utilities/application/argumentparser.h>

namespace ReflectiveRapidJSON {

/*!
 * \brief The BinarySerializationCodeGenerator class generates code for JSON (de)serialization
 *        of objects inheriting from an instantiation of JsonSerializable.
 */
class BinarySerializationCodeGenerator : public CodeGenerator {
public:
    struct Options {
        Options();
        Options(const Options &other) = delete;
        void appendTo(ApplicationUtilities::Argument *arg);

        ApplicationUtilities::ConfigValueArgument additionalClassesArg;
        ApplicationUtilities::ConfigValueArgument visibilityArg;
    };

private:
    struct RelevantClass {
        explicit RelevantClass(std::string &&qualifiedName, clang::CXXRecordDecl *record);

        std::string qualifiedName;
        clang::CXXRecordDecl *record;
    };

public:
    BinarySerializationCodeGenerator(CodeFactory &factory, const Options &options);

    void addDeclaration(clang::Decl *decl) override;
    void generate(std::ostream &os) const override;

private:
    std::string qualifiedNameIfRelevant(clang::CXXRecordDecl *record) const;
    std::vector<RelevantClass> findRelevantClasses() const;
    static std::vector<const RelevantClass *> findRelevantBaseClasses(
        const RelevantClass &relevantClass, const std::vector<RelevantClass> &relevantBases);

    std::vector<clang::CXXRecordDecl *> m_records;
    std::vector<RelevantClass> m_adaptionRecords;
    const Options &m_options;
};

inline BinarySerializationCodeGenerator::BinarySerializationCodeGenerator(CodeFactory &factory, const Options &options)
    : CodeGenerator(factory)
    , m_options(options)
{
}

inline void BinarySerializationCodeGenerator::Options::appendTo(ApplicationUtilities::Argument *arg)
{
    arg->addSubArgument(&additionalClassesArg);
    arg->addSubArgument(&visibilityArg);
}

inline BinarySerializationCodeGenerator::RelevantClass::RelevantClass(std::string &&qualifiedName, clang::CXXRecordDecl *record)
    : qualifiedName(qualifiedName)
    , record(record)
{
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CODE_BINARY_SERIALIZATION_GENERATOR_H
