//===--- UseIsEmptyCheck.h - Use isEmpty() instead of getLength() == 0 ---===//
//
// This check finds patterns like:
//   - AsciiString::getLength() == 0  -> AsciiString::isEmpty()
//   - AsciiString::getLength() > 0   -> !AsciiString::isEmpty()
//   - UnicodeString::getLength() == 0 -> UnicodeString::isEmpty()
//   - UnicodeString::getLength() > 0  -> !UnicodeString::isEmpty()
//   - StringClass::Get_Length() == 0  -> StringClass::Is_Empty()
//   - WideStringClass::Get_Length() == 0 -> WideStringClass::Is_Empty()
//   - AsciiString::compare("") == 0  -> AsciiString::isEmpty()
//   - UnicodeString::compare(L"") == 0 -> UnicodeString::isEmpty()
//   - AsciiString::compare(AsciiString::TheEmptyString) == 0 -> AsciiString::isEmpty()
//   - UnicodeString::compare(UnicodeString::TheEmptyString) == 0 -> UnicodeString::isEmpty()
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_GENERALSGAMECODE_READABILITY_USEISEMPTYCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_GENERALSGAMECODE_READABILITY_USEISEMPTYCHECK_H

#include "clang-tidy/ClangTidyCheck.h"

namespace clang::tidy::generalsgamecode::readability {

/// Finds uses of getLength() == 0 or getLength() > 0 on AsciiString and
/// UnicodeString, and Get_Length() == 0 on StringClass and WideStringClass,
/// and suggests using isEmpty()/Is_Empty() or !isEmpty()/!Is_Empty() instead.
/// Also finds uses of compare("") == 0 or compare(TheEmptyString) == 0 and
/// suggests using isEmpty() instead.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/generals-use-is-empty.html
class UseIsEmptyCheck : public ClangTidyCheck {
public:
  UseIsEmptyCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  bool isLanguageVersionSupported(const LangOptions &LangOpts) const override {
    return LangOpts.CPlusPlus;
  }
};

} // namespace clang::tidy::generalsgamecode::readability

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_GENERALSGAMECODE_READABILITY_USEISEMPTYCHECK_H
