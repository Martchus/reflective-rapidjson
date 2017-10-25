// this file is based on clangversionabstraction.cpp from https://github.com/woboq/moc-ng

#include "./clangversionabstraction.h"

#include <clang/Lex/Preprocessor.h>

namespace ReflectiveRapidJSON {

clang::FileID createFileIDForMemBuffer(clang::Preprocessor &pp, llvm::MemoryBuffer *buffer, clang::SourceLocation location)
{
#if CLANG_VERSION_MAJOR != 3 || CLANG_VERSION_MINOR > 4
    return pp.getSourceManager().createFileID(maybe_unique(buffer), clang::SrcMgr::C_User, 0, 0, location);
#else
    return pp.getSourceManager().createFileIDForMemBuffer(buffer, clang::SrcMgr::C_User, 0, 0, location);
#endif
}

} // namespace ReflectiveRapidJSON
