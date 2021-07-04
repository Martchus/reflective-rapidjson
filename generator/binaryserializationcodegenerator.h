#ifndef REFLECTIVE_RAPIDJSON_CODE_BINARY_SERIALIZATION_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_CODE_BINARY_SERIALIZATION_GENERATOR_H

#include "./serializationcodegenerator.h"

#include <c++utilities/application/argumentparser.h>

namespace ReflectiveRapidJSON {

/*!
 * \brief The BinarySerializationCodeGenerator class generates code for JSON (de)serialization
 *        of objects inheriting from an instantiation of JsonSerializable.
 */
class BinarySerializationCodeGenerator : public SerializationCodeGenerator {
public:
    struct Options {
        Options();
        Options(const Options &other) = delete;
        void appendTo(CppUtilities::Argument *arg);

        CppUtilities::ConfigValueArgument additionalClassesArg;
        CppUtilities::ConfigValueArgument visibilityArg;
    };

    BinarySerializationCodeGenerator(CodeFactory &factory, const Options &options);

    void generate(std::ostream &os) const override;

protected:
    void computeRelevantClass(RelevantClass &possiblyRelevantClass) const override;

    const Options &m_options;
};

inline void BinarySerializationCodeGenerator::Options::appendTo(CppUtilities::Argument *arg)
{
    arg->addSubArgument(&additionalClassesArg);
    arg->addSubArgument(&visibilityArg);
}

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CODE_BINARY_SERIALIZATION_GENERATOR_H
