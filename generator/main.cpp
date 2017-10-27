#include "./codefactory.h"

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
    ConfigValueArgument inputFileArg("input-file", 'i', "specifies the input file", { "path" });
    inputFileArg.setRequired(true);
    ConfigValueArgument outputFileArg("output-file", 'o', "specifies the output file", { "path" });
    Argument generatorsArg("generators", 'g', "specifies the generators (by default all generators are enabled)");
    generatorsArg.setValueNames({ "json" });
    generatorsArg.setPreDefinedCompletionValues("json");
    generatorsArg.setRequiredValueCount(Argument::varValueCount);
    generatorsArg.setCombinable(true);
    ConfigValueArgument clangOptionsArg("clang-opt", 'c', "specifies an argument to be passed to Clang", { "option" });
    HelpArgument helpArg(parser);
    NoColorArgument noColorArg;
    parser.setMainArguments({ &inputFileArg, &outputFileArg, &generatorsArg, &clangOptionsArg, &noColorArg, &helpArg });

    // parse arguments
    parser.parseArgsOrExit(argc, argv);
    if (helpArg.isPresent()) {
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

        // configure code generator
        vector<const char *> defaultClangOptions;
        CodeFactory factory(
            parser.executable(), inputFileArg.values(0), clangOptionsArg.isPresent() ? clangOptionsArg.values(0) : defaultClangOptions, *os);
        // add only specified generators if the --generator argument is present
        if (generatorsArg.isPresent()) {
            // find and construct generators by name
            for (const char *generatorName : generatorsArg.values(0)) {
                if (!strcmp(generatorName, "json")) {
                    factory.addGenerator<JSONSerializationCodeGenerator>();
                } else {
                    cerr << Phrases::Error << "The specified generator \"" << generatorName << "\" does not exist." << Phrases::EndFlush;
                    return -5;
                }
            }
        } else {
            // add default generators
            factory.addGenerator<JSONSerializationCodeGenerator>();
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
