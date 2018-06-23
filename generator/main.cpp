#include "./codefactory.h"
#include "./jsonserializationcodegenerator.h"
#include "./binaryserializationcodegenerator.h"

#include "resources/config.h"

#include <c++utilities/application/argumentparser.h>
#include <c++utilities/application/commandlineutils.h>
#include <c++utilities/application/failure.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/ansiescapecodes.h>
#include <c++utilities/io/catchiofailure.h>
#include <c++utilities/io/misc.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace std;
using namespace ApplicationUtilities;
using namespace ConversionUtilities;
using namespace EscapeCodes;
using namespace IoUtilities;
using namespace ReflectiveRapidJSON;

int main(int argc, char *argv[])
{
    SET_APPLICATION_INFO;
    CMD_UTILS_CONVERT_ARGS_TO_UTF8;

    // setup argument parser
    ArgumentParser parser;
    OperationArgument generateArg("generate", '\0', "runs the code generator");
    generateArg.setImplicit(true);
    ConfigValueArgument inputFileArg("input-file", '\0', "specifies the input file", { "path" });
    inputFileArg.setRequired(true);
    ConfigValueArgument outputFileArg("output-file", '\0', "specifies the output file", { "path" });
    Argument generatorsArg("generators", '\0', "specifies the generators (by default all generators are enabled)");
    generatorsArg.setValueNames({ "json" });
    generatorsArg.setPreDefinedCompletionValues("json");
    generatorsArg.setRequiredValueCount(Argument::varValueCount);
    generatorsArg.setCombinable(true);
    ConfigValueArgument clangOptionsArg("clang-opt", '\0', "specifies arguments/options to be passed to Clang", { "option" });
    clangOptionsArg.setRequiredValueCount(Argument::varValueCount);
    ConfigValueArgument errorResilientArg("error-resilient", '\0', "turns most errors into warnings");
    HelpArgument helpArg(parser);
    NoColorArgument noColorArg;
    generateArg.setSubArguments({ &inputFileArg, &outputFileArg, &generatorsArg, &clangOptionsArg, &errorResilientArg });
    JsonSerializationCodeGenerator::Options jsonOptions;
    jsonOptions.appendTo(&generateArg);
    BinarySerializationCodeGenerator::Options binaryOptions;
    binaryOptions.appendTo(&generateArg);
    parser.setMainArguments({ &generateArg, &noColorArg, &helpArg });

    // parse arguments
    parser.parseArgsOrExit(argc, argv);
    if (helpArg.isPresent() || !generateArg.isPresent()) {
        return 0;
    }

    // setup output stream
    ostream *os = nullptr;
    try {
        ofstream outputFile;
        if (outputFileArg.isPresent()) {
            outputFile.exceptions(ios_base::badbit | ios_base::failbit);
            outputFile.open(outputFileArg.values(0).front(), ios_base::out | ios_base::trunc | ios_base::binary);
            os = &outputFile;
        } else {
            os = &cout;
        }

        // compose options passed to the clang tool invocation
        vector<string> clangOptions;
        if (clangOptionsArg.isPresent()) {
            // add additional options specified via CLI argument
            for (const auto *const value : clangOptionsArg.values(0)) {
                // split options by ";" - not nice but this eases using CMake generator expressions
                const auto splittedValues(splitString<vector<string>>(value, ";", EmptyPartsTreat::Omit));
                clangOptions.reserve(clangOptions.size() + splittedValues.size());
                for (const auto &splittedValue : splittedValues) {
                    clangOptions.emplace_back(move(splittedValue));
                }
            }
        }

        // instantiate the code factory and add generators to it
        CodeFactory factory(parser.executable(), inputFileArg.values(0), clangOptions, *os);
        factory.setErrorResilient(errorResilientArg.isPresent());
        // add specified generators if the --generator argument is present; otherwise add default generators
        if (generatorsArg.isPresent()) {
            // define mapping of generator names to generator constructors (add new generators here!)
            // clang-format off
            const std::unordered_map<std::string, std::function<void()>> generatorsByName{
                { "json", factory.bindGenerator<JsonSerializationCodeGenerator, const JsonSerializationCodeGenerator::Options &>(jsonOptions) },
                { "binary", factory.bindGenerator<BinarySerializationCodeGenerator, const BinarySerializationCodeGenerator::Options &>(binaryOptions) },
            };
            // clang-format on

            // find and construct generators by name
            for (const char *generatorName : generatorsArg.values(0)) {
                try {
                    generatorsByName.at(generatorName)();
                } catch (const out_of_range &) {
                    cerr << Phrases::Error << "The specified generator \"" << generatorName << "\" does not exist." << Phrases::End;
                    cerr << "Available generators:";
                    for (const auto &generators : generatorsByName) {
                        cerr << ' ' << generators.first;
                    }
                    cerr << endl;
                    return -5;
                }
            }
        } else {
            // add default generators
            factory.addGenerator<JsonSerializationCodeGenerator>(jsonOptions);
        }

        // read AST elements from input files and run the code generator
        if (!factory.run()) {
            cerr << Phrases::Error << "Errors occured." << Phrases::EndFlush;
            return -2;
        }

    } catch (...) {
        catchIoFailure();
        const char *errorMessage;
        if (os) {
            errorMessage = os->fail() || os->bad() ? "An IO error occured when writing to the output stream." : "An IO error occured.";
        } else {
            errorMessage = "An IO error when opening output stream.";
        }
        cerr << Phrases::Error << errorMessage << Phrases::EndFlush;
        return -4;
    }

    return 0;
}
