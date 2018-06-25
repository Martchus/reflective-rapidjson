#ifndef REFLECTIVE_RAPIDJSON_SERIALIZATION_CODE_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_SERIALIZATION_CODE_GENERATOR_H

#include "./codegenerator.h"

#include <llvm/ADT/StringRef.h>

namespace ReflectiveRapidJSON {

std::ostream &operator<<(std::ostream &os, llvm::StringRef str);

/*!
 * \brief The SerializationCodeGenerator class is the common base for (de)serialization
 *        related code generation.
 */
class SerializationCodeGenerator : public CodeGenerator {
public:
    struct RelevantClass {
        explicit RelevantClass(std::string &&qualifiedName, clang::CXXRecordDecl *record);

        std::string qualifiedName;
        clang::CXXRecordDecl *record;
    };

    SerializationCodeGenerator(CodeFactory &factory);

    void addDeclaration(clang::Decl *decl) override;

protected:
    enum class IsRelevant { Yes, No, Maybe };
    IsRelevant isQualifiedNameIfRelevant(clang::CXXRecordDecl *record, const std::string &qualifiedName) const;
    virtual std::string qualifiedNameIfRelevant(clang::CXXRecordDecl *record) const = 0;
    std::vector<RelevantClass> findRelevantClasses() const;
    static std::vector<const RelevantClass *> findRelevantBaseClasses(
        const RelevantClass &relevantClass, const std::vector<RelevantClass> &relevantBases);

protected:
    const char *m_qualifiedNameOfRecords;
    const char *m_qualifiedNameOfAdaptionRecords;

private:
    std::vector<clang::CXXRecordDecl *> m_records;
    std::vector<RelevantClass> m_adaptionRecords;
};

inline SerializationCodeGenerator::RelevantClass::RelevantClass(std::string &&qualifiedName, clang::CXXRecordDecl *record)
    : qualifiedName(qualifiedName)
    , record(record)
{
}

inline SerializationCodeGenerator::SerializationCodeGenerator(CodeFactory &factory)
    : CodeGenerator(factory)
    , m_qualifiedNameOfRecords(nullptr)
    , m_qualifiedNameOfAdaptionRecords(nullptr)
{
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_SERIALIZATION_CODE_GENERATOR_H
