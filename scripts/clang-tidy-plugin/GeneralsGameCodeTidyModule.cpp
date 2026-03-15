//===--- GeneralsGameCodeTidyModule.cpp - GeneralsGameCode Tidy Module ---===//
//
// Custom clang-tidy module for GeneralsGameCode
// Provides GeneralsGameCode-specific checks
//
//===----------------------------------------------------------------------===//

#include "GeneralsGameCodeTidyModule.h"
#include "readability/UseIsEmptyCheck.h"
#include "readability/UseThisInsteadOfSingletonCheck.h"
#include "llvm/Support/Registry.h"

namespace clang::tidy::generalsgamecode {

void GeneralsGameCodeTidyModule::addCheckFactories(
    ClangTidyCheckFactories &CheckFactories) {
  CheckFactories.registerCheck<readability::UseIsEmptyCheck>(
      "generals-use-is-empty");
  CheckFactories.registerCheck<readability::UseThisInsteadOfSingletonCheck>(
      "generals-use-this-instead-of-singleton");
}

} // namespace clang::tidy::generalsgamecode

static llvm::Registry<::clang::tidy::ClangTidyModule>::Add<
    ::clang::tidy::generalsgamecode::GeneralsGameCodeTidyModule>
    X("generalsgamecode", "GeneralsGameCode-specific checks");

volatile int GeneralsGameCodeTidyModuleAnchorSource = 0;

