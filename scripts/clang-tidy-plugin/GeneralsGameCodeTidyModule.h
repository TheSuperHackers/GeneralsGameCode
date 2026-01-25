//===--- GeneralsGameCodeTidyModule.h - GeneralsGameCode Tidy Module -----===//
//
// Custom clang-tidy module for GeneralsGameCode
// Provides checks for custom types like AsciiString and UnicodeString
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_GENERALSGAMECODE_GENERALSGAMECODETIDYMODULE_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_GENERALSGAMECODE_GENERALSGAMECODETIDYMODULE_H

#include "clang-tidy/ClangTidyModule.h"

namespace clang::tidy::generalsgamecode {

/// This module is for GeneralsGameCode-specific checks.
class GeneralsGameCodeTidyModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override;
};

} // namespace clang::tidy::generalsgamecode

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_GENERALSGAMECODE_GENERALSGAMECODETIDYMODULE_H

