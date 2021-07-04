#ifndef REFLECTIVE_RAPIDJSON_CODE_JSON_SERIALIZATION_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_CODE_JSON_SERIALIZATION_GENERATOR_H

#include "./serializationcodegenerator.h"

#include <c++utilities/application/argumentparser.h>

namespace ReflectiveRapidJSON {

/*!
 * \brief The JsonSerializationCodeGenerator class generates code for JSON (de)serialization
 *        of objects inheriting from an instantiation of JsonSerializable.
 */
class JsonSerializationCodeGenerator : public SerializationCodeGenerator {
public:
    struct Options {
        Options();
        Options(const Options &other) = delete;
        void appendTo(CppUtilities::Argument *arg);

        CppUtilities::ConfigValueArgument additionalClassesArg;
        CppUtilities::ConfigValueArgument visibilityArg;
    };

    JsonSerializationCodeGenerator(CodeFactory &factory, const Options &options);

    void generate(std::ostream &os) const override;

protected:
    void computeRelevantClass(RelevantClass &possiblyRelevantClass) const override;

    const Options &m_options;
};

inline void JsonSerializationCodeGenerator::Options::appendTo(CppUtilities::Argument *arg)
{
    arg->addSubArgument(&additionalClassesArg);
    arg->addSubArgument(&visibilityArg);
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CODE_JSON_SERIALIZATION_GENERATOR_H
