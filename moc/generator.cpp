#include "./generator.h"

#include <c++utilities/application/global.h>
#include <c++utilities/conversion/stringbuilder.h>

//#include <clang/Basic/LangOptions.h>
//#include <clang/Basic/TargetInfo.h>
//#include <clang/Basic/Diagnostic.h>
//#include <clang/Frontend/TextDiagnosticPrinter.h>
//#include <clang/Lex/HeaderSearch.h>
//#include <clang/Lex/Preprocessor.h>
//#include <clang/AST/ASTContext.h>
//#include <clang/AST/ASTConsumer.h>
//#include <clang/Sema/Sema.h>
//#include <clang/Parse/ParseAST.h>

//#include <clang/AST/AST.h>
//#include <clang/AST/RecursiveASTVisitor.h>

//#include <clang/Frontend/FrontendActions.h>
//#include <clang/Frontend/CompilerInstance.h>
//#include <clang/Tooling/Tooling.h>

#include <clang-c/Index.h>

#include <iostream>
#include <memory>

using namespace std;
using namespace ConversionUtilities;

namespace ReflectiveRapidJSON {

ostream &operator<<(ostream &stream, const CXString &str)
{
    stream << clang_getCString(str);
    clang_disposeString(str);
    return stream;
}

struct Struct {
    string ns;
    string name;
    vector<Struct> members;
};

bool generateReflectionCode(const vector<const char *> &sourceFiles, ostream &os)
{
    bool noErrors = true;

    for (const char *sourceFile : sourceFiles) {
        CXIndex index = clang_createIndex(0, 0);
        const char *const args[] = { "-x", "c++" };
        CXTranslationUnit unit = nullptr;
        CXErrorCode parseRes = clang_parseTranslationUnit2(index, sourceFile, args, 2, nullptr, 0, CXTranslationUnit_None, &unit);
        if (!unit && parseRes != CXError_Success) {
            clang_disposeIndex(index);
            throw runtime_error(argsToString("Unable to parse translation unit: ", sourceFile));
        }

        CXCursor cursor = clang_getTranslationUnitCursor(unit);
        clang_visitChildren(cursor,
            [](CXCursor c, CXCursor parent, CXClientData client_data) {
                VAR_UNUSED(parent)
                auto &os = *reinterpret_cast<ostream *>(client_data);
                os << "Cursor kind '" << clang_getCursorKindSpelling(clang_getCursorKind(c)) << "\' ";
                os << clang_getCursorSpelling(c) << '\n';
                os << "type: " << clang_getTypeSpelling(clang_getCursorType(c)) << '\n';
                return CXChildVisit_Recurse;
            },
            &os);
        os.flush();

        for (unsigned int index = 0, diagnosticCount = clang_getNumDiagnostics(unit); index != diagnosticCount; ++index) {
            noErrors = false;
            CXDiagnostic diagnostic = clang_getDiagnostic(unit, index);
            clang_getDiagnosticCategory(diagnostic);

            CXString diagnosticSpelling = clang_formatDiagnostic(
                diagnostic, CXDiagnostic_DisplaySourceLocation | CXDiagnostic_DisplayColumn | CXDiagnostic_DisplaySourceRanges);
            cerr << clang_getCString(diagnosticSpelling) << '\n';
            clang_disposeString(diagnosticSpelling);
            clang_disposeDiagnostic(diagnostic);
        }
        cerr.flush();

        clang_disposeTranslationUnit(unit);
        clang_disposeIndex(index);
    }

    return noErrors;
}

} // namespace ReflectiveRapidJSON
