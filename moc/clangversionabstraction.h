// this file is based on clangversionabstraction.h from https://github.com/woboq/moc-ng

#ifndef REFLECTIVE_RAPIDJSON_CLANG_VERSION_ABSTRACTION_H
#define REFLECTIVE_RAPIDJSON_CLANG_VERSION_ABSTRACTION_H

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/Version.h>

#include <memory>

namespace clang {
class Preprocessor;
}

namespace ReflectiveRapidJSON {

clang::FileID createFileIDForMemBuffer(clang::Preprocessor &pp, llvm::MemoryBuffer *buffer, clang::SourceLocation location);

/*!
 * \brief The MaybeUnique class represents either a std::unique_ptr or a raw pointer.
 * \remarks This is used to support Clang < 3.6 which has not been using unique_ptr in many places yet.
 */
template <typename T> struct MaybeUnique {
    T *val;
    operator T *()
    {
        return val;
    }
    template <typename X> operator std::unique_ptr<X>() &&
    {
        return std::unique_ptr<X>(val);
    }
};
template <typename T> MaybeUnique<T> maybe_unique(T *val)
{
    return { val };
}
template <typename T> MaybeUnique<T> maybe_unique(std::unique_ptr<T> val)
{
    return { val.release() };
}

    /*!
 * \def REFLECTIVE_RAPIDJSON_MAYBE_UNIQUE
 * \brief The REFLECTIVE_RAPIDJSON_MAYBE_UNIQUE macro either expands to a std::unique_ptr or a raw pointer depending on the Clang version.
 */

#if CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR <= 5
#define REFLECTIVE_RAPIDJSON_MAYBE_UNIQUE(t) t *
#else
#define REFLECTIVE_RAPIDJSON_MAYBE_UNIQUE(t) std::unique_ptr<t>
#endif

} // namespace ReflectiveRapidJSON

#endif // REFLECTIVE_RAPIDJSON_CLANG_VERSION_ABSTRACTION_H
