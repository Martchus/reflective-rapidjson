#include "./generator.h"

#include "resources/config.h"

#include <c++utilities/application/argumentparser.h>
#include <c++utilities/application/commandlineutils.h>
#include <c++utilities/application/failure.h>
#include <c++utilities/io/ansiescapecodes.h>
#include <c++utilities/io/catchiofailure.h>
#include <c++utilities/io/misc.h>

#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;
using namespace ApplicationUtilities;
using namespace EscapeCodes;
using namespace IoUtilities;
using namespace ReflectiveRapidJSON;

int main(int argc, char *argv[])
{
    SET_APPLICATION_INFO;
    CMD_UTILS_CONVERT_ARGS_TO_UTF8;

    // setup argument parser
    ArgumentParser parser;
    Argument inputFilesArg("input-files", 'i', "specifies the input files");
    inputFilesArg.setValueNames({ "path" });
    inputFilesArg.setRequiredValueCount(Argument::varValueCount);
    Argument outputFileArg("output-file", 'o', "specifies the output file");
    outputFileArg.setValueNames({ "path" });
    outputFileArg.setRequiredValueCount(1);
    outputFileArg.setCombinable(true);
    Argument generatorsArgs("generators", 'g', "specifies the generators (by default all generators are enabled)");
    generatorsArgs.setValueNames({ "json" });
    generatorsArgs.setRequiredValueCount(Argument::varValueCount);
    generatorsArgs.setCombinable(true);
    HelpArgument helpArg(parser);
    NoColorArgument noColorArg;
    parser.setMainArguments({ &inputFilesArg, &outputFileArg, &noColorArg, &helpArg });

    // parse arguments
    parser.parseArgsOrExit(argc, argv);
    if (helpArg.isPresent()) {
        return 0;
    }
    if (!inputFilesArg.isPresent()) {
        cerr << Phrases::Error << "No input file specified." << Phrases::EndFlush;
        return -1;
    }

    // setup output stream
    ostream *os = nullptr;
    try {
        ofstream outputFile;
        if (outputFileArg.isPresent()) {
            outputFile.exceptions(ios_base::badbit | ios_base::failbit);
            outputFile.open(outputFileArg.values(0).front(), ios_base::out | ios_base::binary);
            os = &outputFile;
        } else {
            os = &cout;
        }

        // configure code generator
        CodeFactory factory(parser.executable(), inputFilesArg.values(0), *os);
        auto &generators(factory.generators());
        // add only specified generators if the --generator argument is present
        if (generatorsArgs.isPresent()) {
            // find and construct generators by name
            for (vector<const char *> generatorName : generatorsArgs.values(0)) {
                if (!strcmp(generatorName, "json")) {
                    generators.emplace_back(std::make_unique<JSONSerializationCodeGenerator>());
                } else {
                    cerr << Phrases::Error << "The specified generator \"" << generatorName << "\" does not exist." << Phrases::EndFlush;
                    return -5;
                }
            }
        } else {
            // add default generators
            generators.emplace_back(std::make_unique<JSONSerializationCodeGenerator>());
        }

        // read AST elements from input files
        if (!factory.readAST()) {
            cerr << Phrases::Error << "Errors occured when parsing the input files." << Phrases::EndFlush;
            return -2;
        }

        // run the code generator
        if (!factory.generate()) {
            cerr << Phrases::Error << "Errors occured when during code generation." << Phrases::EndFlush;
            return -3;
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
