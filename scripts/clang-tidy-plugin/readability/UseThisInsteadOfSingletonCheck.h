#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_GENERALSGAMECODE_READABILITY_USETHISINSTEADOFSINGLETONCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_GENERALSGAMECODE_READABILITY_USETHISINSTEADOFSINGLETONCHECK_H

#include "clang-tidy/ClangTidyCheck.h"

namespace clang::tidy::generalsgamecode::readability {

class UseThisInsteadOfSingletonCheck : public ClangTidyCheck {
public:
  UseThisInsteadOfSingletonCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  bool isLanguageVersionSupported(const LangOptions &LangOpts) const override {
    return LangOpts.CPlusPlus;
  }
};

} // namespace clang::tidy::generalsgamecode::readability

#endif

