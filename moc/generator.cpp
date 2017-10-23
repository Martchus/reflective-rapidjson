#include "./generator.h"
#include "./frontendaction.h"

#include "../lib/jsonserializable.h"

#include <c++utilities/application/global.h>
#include <c++utilities/conversion/stringbuilder.h>

#include <clang/AST/DeclCXX.h>
#include <clang/Basic/FileManager.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#include <iostream>
#include <memory>

using namespace std;
using namespace ConversionUtilities;

namespace ReflectiveRapidJSON {

/*!
 * \brief Prints an LLVM string reference without instantiating a std::string first.
 */
ostream &operator<<(ostream &os, llvm::StringRef str)
{
    os.write(str.data(), static_cast<streamsize>(str.size()));
    return os;
}

CodeGenerator::~CodeGenerator()
{
}

/*!
 * \brief Adds the specified \a decl to the code generator. The generator might ignore irrelevant declarations.
 */
void CodeGenerator::addDeclaration(clang::Decl *decl)
{
    VAR_UNUSED(decl)
}

/*!
 * \brief Returns whether the specified \a record inherits from an instantiation of the specified \a templateClass.
 * \remarks The specified \a record must be defined (not only forward-declared).
 */
bool CodeGenerator::inheritsFromInstantiationOf(clang::CXXRecordDecl *const record, const char *const templateClass)
{
    for (const clang::CXXBaseSpecifier &base : record->bases()) {
        const clang::CXXRecordDecl *const baseDecl = base.getType()->getAsCXXRecordDecl();
        if (baseDecl && baseDecl->getQualifiedNameAsString() == templateClass) {
            return true;
        }
    }
    return false;
}

void JSONSerializationCodeGenerator::addDeclaration(clang::Decl *decl)
{
    switch (decl->getKind()) {
    case clang::Decl::Kind::CXXRecord: {
        auto *const record = static_cast<clang::CXXRecordDecl *>(decl);
        // skip forward declarations
        if (!record->hasDefinition()) {
            return;
        }
        // add classes derived from any instantiation of "ReflectiveRapidJSON::JSONSerializable"
        if (inheritsFromInstantiationOf(record, JSONSerializable<void>::qualifiedName)) {
            m_relevantClasses.emplace_back(record->getQualifiedNameAsString(), record);
        }
        break;
    }
    case clang::Decl::Kind::Enum:
        // TODO: add enums
        break;
    default:;
    }
}

void JSONSerializationCodeGenerator::generate(ostream &os) const
{
    if (m_relevantClasses.empty()) {
        return;
    }

    // put everything into namespace ReflectiveRapidJSON::Reflector
    os << "namespace ReflectiveRapidJSON {\n"
          "namespace Reflector {\n\n";

    // add push and pull functions for each class, for an example of the resulting
    // output, see ../lib/tests/jsonserializable.cpp (code under comment "pretend serialization code...")
    for (const RelevantClass &relevantClass : m_relevantClasses) {
        // print push method
        os << "template <> inline void push<::" << relevantClass.qualifiedName << ">(const ::" << relevantClass.qualifiedName
           << " &reflectable, ::RAPIDJSON_NAMESPACE::Value::Object &value, ::RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator)\n{\n";
        for (const clang::FieldDecl *field : relevantClass.record->fields()) {
            os << "    push(reflectable." << field->getName() << ", \"" << field->getName() << "\", value, allocator);\n";
        }
        os << "}\n";

        // print pull method
        os << "template <> inline void pull<::" << relevantClass.qualifiedName << ">(::" << relevantClass.qualifiedName
           << " &reflectable, const ::RAPIDJSON_NAMESPACE::GenericValue<::RAPIDJSON_NAMESPACE::UTF8<char>>::ConstObject &value)\n{\n";
        for (const clang::FieldDecl *field : relevantClass.record->fields()) {
            os << "    pull(reflectable." << field->getName() << ", \"" << field->getName() << "\", value);\n";
        }
        os << "}\n\n";
    }

    // close namespace ReflectiveRapidJSON::Reflector
    os << "} // namespace Reflector\n"
          "} // namespace ReflectiveRapidJSON\n";
}

struct CodeFactory::ToolInvocation {
    ToolInvocation(CodeFactory &factory);

    clang::FileManager fileManager;
    clang::tooling::ToolInvocation invocation;
};

CodeFactory::ToolInvocation::ToolInvocation(CodeFactory &factory)
    : fileManager({ "." })
    , invocation(factory.makeClangArgs(), new FrontendAction(factory), &fileManager)
{
    fileManager.Retain();
}

CodeFactory::CodeFactory(
    const char *applicationPath, const std::vector<const char *> &sourceFiles, const std::vector<const char *> &clangOptions, std::ostream &os)
    : m_applicationPath(applicationPath)
    , m_sourceFiles(sourceFiles)
    , m_clangOptions(clangOptions)
    , m_os(os)
    , m_compilerInstance(nullptr)
{
}

CodeFactory::~CodeFactory()
{
}

/*!
 * \brief Constructs arguments for the Clang tool invocation.
 */
std::vector<string> CodeFactory::makeClangArgs() const
{
    static const initializer_list<const char *> flags
        = { m_applicationPath, "-x", "c++", "-fPIE", "-fPIC", "-Wno-microsoft", "-Wno-pragma-once-outside-header", "-std=c++14", "-fsyntax-only" };
    vector<string> clangArgs;
    clangArgs.reserve(flags.size() + m_clangOptions.size() + m_sourceFiles.size());
    clangArgs.insert(clangArgs.end(), flags.begin(), flags.end());
    clangArgs.insert(clangArgs.end(), m_clangOptions.cbegin(), m_clangOptions.cend());
    clangArgs.insert(clangArgs.end(), m_sourceFiles.cbegin(), m_sourceFiles.cend());
    return clangArgs;
}

/*!
 * \brief Adds the specified \a decl to all underlying code generators. The generators might ignore irrelevant declarations.
 * \remarks Supposed to be called by assigned generators inside readAST().
 */
void CodeFactory::addDeclaration(clang::Decl *decl)
{
    for (const auto &generator : m_generators) {
        generator->addDeclaration(decl);
    }
}

/*!
 * \brief Reads (relevent) AST elements using Clang.
 */
bool CodeFactory::readAST()
{
    // lazy initialize Clang tool invocation
    if (!m_toolInvocation) {
        m_toolInvocation = make_unique<ToolInvocation>(*this);
    }
    // run Clang
    return m_toolInvocation->invocation.run();
}

/*!
 * \brief Generates code based on the AST elements which have been read by invoking readAST().
 */
bool CodeFactory::generate() const
{
    for (const auto &generator : m_generators) {
        generator->generate(m_os);
    }
    return true;
}

} // namespace ReflectiveRapidJSON
