#include "./generator.h"

#include "resources/config.h"

#include <c++utilities/application/argumentparser.h>
#include <c++utilities/application/commandlineutils.h>
#include <c++utilities/application/failure.h>
#include <c++utilities/io/ansiescapecodes.h>
#include <c++utilities/io/catchiofailure.h>
#include <c++utilities/io/misc.h>

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
    HelpArgument helpArg(parser);
    NoColorArgument noColorArg;
    parser.setMainArguments({ &inputFilesArg, &outputFileArg, &noColorArg, &helpArg });

    // parse arguments
    parser.parseArgsOrExit(argc, argv);
    if (!helpArg.isPresent() && !inputFilesArg.isPresent()) {
        cerr << Phrases::Error << "No input file specified." << Phrases::EndFlush;
        return -2;
    }

    // setup output stream
    try {
        ostream *os;
        ofstream outputFile;
        if (outputFileArg.isPresent()) {
            outputFile.exceptions(ios_base::badbit | ios_base::failbit);
            outputFile.open(outputFileArg.values(0).front(), ios_base::out | ios_base::binary);
            os = &outputFile;
        } else {
            os = &cout;
        }

        // process input files
        return generateReflectionCode(inputFilesArg.values(0), *os) ? 0 : 1;
    } catch (...) {
        catchIoFailure();
        cerr << Phrases::Error << "An IO error occured." << Phrases::EndFlush;
        return -3;
    }

    return 0;
}
