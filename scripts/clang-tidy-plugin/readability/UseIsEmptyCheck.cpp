//===--- UseIsEmptyCheck.cpp - Use isEmpty() instead of getLength() == 0 -===//
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

#include "UseIsEmptyCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang::tidy::generalsgamecode::readability {

void UseIsEmptyCheck::registerMatchers(MatchFinder *Finder) {
  // Matcher for AsciiString/UnicodeString with getLength()
  auto GetLengthCall = cxxMemberCallExpr(
      callee(cxxMethodDecl(hasName("getLength"))),
      on(hasType(hasUnqualifiedDesugaredType(
          recordType(hasDeclaration(cxxRecordDecl(
              hasAnyName("AsciiString", "UnicodeString"))))))));

  // Matcher for StringClass/WideStringClass with Get_Length()
  auto GetLengthCallWWVegas = cxxMemberCallExpr(
      callee(cxxMethodDecl(hasName("Get_Length"))),
      on(hasType(hasUnqualifiedDesugaredType(
          recordType(hasDeclaration(cxxRecordDecl(
              hasAnyName("StringClass", "WideStringClass"))))))));

  // Helper function to add matchers for a given GetLength call matcher
  auto addMatchersForGetLength = [&](const auto &GetLengthMatcher) {
    Finder->addMatcher(
        binaryOperator(
            hasOperatorName("=="),
            hasLHS(ignoringParenImpCasts(GetLengthMatcher.bind("getLengthCall"))),
            hasRHS(integerLiteral(equals(0)).bind("zero")))
            .bind("comparison"),
        this);

    Finder->addMatcher(
        binaryOperator(
            hasOperatorName("!="),
            hasLHS(ignoringParenImpCasts(GetLengthMatcher.bind("getLengthCall"))),
            hasRHS(integerLiteral(equals(0)).bind("zero")))
            .bind("comparison"),
        this);

    Finder->addMatcher(
        binaryOperator(
            hasOperatorName(">"),
            hasLHS(ignoringParenImpCasts(GetLengthMatcher.bind("getLengthCall"))),
            hasRHS(integerLiteral(equals(0)).bind("zero")))
            .bind("comparison"),
        this);

    Finder->addMatcher(
        binaryOperator(
            hasOperatorName("<="),
            hasLHS(ignoringParenImpCasts(GetLengthMatcher.bind("getLengthCall"))),
            hasRHS(integerLiteral(equals(0)).bind("zero")))
            .bind("comparison"),
        this);

    Finder->addMatcher(
        binaryOperator(
            hasOperatorName("=="),
            hasLHS(integerLiteral(equals(0)).bind("zero")),
            hasRHS(ignoringParenImpCasts(GetLengthMatcher.bind("getLengthCall"))))
            .bind("comparison"),
        this);

    Finder->addMatcher(
        binaryOperator(
            hasOperatorName("!="),
            hasLHS(integerLiteral(equals(0)).bind("zero")),
            hasRHS(ignoringParenImpCasts(GetLengthMatcher.bind("getLengthCall"))))
            .bind("comparison"),
        this);
  };

  addMatchersForGetLength(GetLengthCall);
  addMatchersForGetLength(GetLengthCallWWVegas);

  // Matcher for TheEmptyString static member access (AsciiString::TheEmptyString or UnicodeString::TheEmptyString)
  auto TheEmptyStringRef = memberExpr(
      member(hasName("TheEmptyString")),
      hasObjectExpression(hasType(hasUnqualifiedDesugaredType(
          recordType(hasDeclaration(cxxRecordDecl(
              hasAnyName("AsciiString", "UnicodeString"))))))));

  // Matcher for AsciiString/UnicodeString with compare() - we'll check if argument is empty in check()
  auto CompareCall = cxxMemberCallExpr(
      callee(cxxMethodDecl(hasName("compare"))),
      on(hasType(hasUnqualifiedDesugaredType(
          recordType(hasDeclaration(cxxRecordDecl(
              hasAnyName("AsciiString", "UnicodeString"))))))),
      hasArgument(0, anyOf(
          stringLiteral().bind("stringLiteralArg"),
          TheEmptyStringRef.bind("theEmptyStringArg"))));

  // Matcher for AsciiString/UnicodeString with compareNoCase() - we'll check if argument is empty in check()
  auto CompareNoCaseCall = cxxMemberCallExpr(
      callee(cxxMethodDecl(hasName("compareNoCase"))),
      on(hasType(hasUnqualifiedDesugaredType(
          recordType(hasDeclaration(cxxRecordDecl(
              hasAnyName("AsciiString", "UnicodeString"))))))),
      hasArgument(0, anyOf(
          stringLiteral().bind("stringLiteralArg"),
          TheEmptyStringRef.bind("theEmptyStringArg"))));

  // Helper function to add matchers for compare() and compareNoCase() calls
  auto addMatchersForCompare = [&](const auto &CompareMatcher, const char *BindingName) {
    Finder->addMatcher(
        binaryOperator(
            hasOperatorName("=="),
            hasLHS(ignoringParenImpCasts(CompareMatcher.bind(BindingName))),
            hasRHS(integerLiteral(equals(0)).bind("zero")))
            .bind("comparison"),
        this);

    Finder->addMatcher(
        binaryOperator(
            hasOperatorName("!="),
            hasLHS(ignoringParenImpCasts(CompareMatcher.bind(BindingName))),
            hasRHS(integerLiteral(equals(0)).bind("zero")))
            .bind("comparison"),
        this);

    Finder->addMatcher(
        binaryOperator(
            hasOperatorName("=="),
            hasLHS(integerLiteral(equals(0)).bind("zero")),
            hasRHS(ignoringParenImpCasts(CompareMatcher.bind(BindingName))))
            .bind("comparison"),
        this);

    Finder->addMatcher(
        binaryOperator(
            hasOperatorName("!="),
            hasLHS(integerLiteral(equals(0)).bind("zero")),
            hasRHS(ignoringParenImpCasts(CompareMatcher.bind(BindingName))))
            .bind("comparison"),
        this);
  };

  addMatchersForCompare(CompareCall, "compareCall");
  addMatchersForCompare(CompareNoCaseCall, "compareNoCaseCall");
}

void UseIsEmptyCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Comparison = Result.Nodes.getNodeAs<BinaryOperator>("comparison");
  const auto *GetLengthCall =
      Result.Nodes.getNodeAs<CXXMemberCallExpr>("getLengthCall");
  const auto *CompareCall =
      Result.Nodes.getNodeAs<CXXMemberCallExpr>("compareCall");
  const auto *CompareNoCaseCall =
      Result.Nodes.getNodeAs<CXXMemberCallExpr>("compareNoCaseCall");

  if (!Comparison)
    return;

  const CXXMemberCallExpr *MethodCall = GetLengthCall ? GetLengthCall :
                                        (CompareCall ? CompareCall : CompareNoCaseCall);
  if (!MethodCall)
    return;

  // For compare() and compareNoCase() calls, verify the argument is actually empty
  if (CompareCall || CompareNoCaseCall) {
    const auto *StringLit = Result.Nodes.getNodeAs<StringLiteral>("stringLiteralArg");
    const auto *TheEmptyString = Result.Nodes.getNodeAs<MemberExpr>("theEmptyStringArg");

    if (StringLit) {
      // Check if string literal is actually empty ("" or L"")
      StringRef Str = StringLit->getString();
      if (!Str.empty()) {
        return; // Not an empty string literal
      }
    } else if (!TheEmptyString) {
      return; // Not matching empty string pattern
    }
  }

  const Expr *ObjectExpr = MethodCall->getImplicitObjectArgument();
  if (!ObjectExpr)
    return;

  // Determine which method name to use based on the called method
  StringRef MethodName = MethodCall->getMethodDecl()->getName();
  std::string IsEmptyMethodName;
  std::string MethodNameStr;

  if (MethodName == "Get_Length") {
    IsEmptyMethodName = "Is_Empty()";
    MethodNameStr = "Get_Length()";
  } else if (MethodName == "compare") {
    IsEmptyMethodName = "isEmpty()";
    MethodNameStr = "compare()";
  } else if (MethodName == "compareNoCase") {
    IsEmptyMethodName = "isEmpty()";
    MethodNameStr = "compareNoCase()";
  } else {
    IsEmptyMethodName = "isEmpty()";
    MethodNameStr = "getLength()";
  }

  StringRef Operator = Comparison->getOpcodeStr();
  bool ShouldNegate = false;

  if (Operator == "==") {
    ShouldNegate = false;
  } else if (Operator == "!=") {
    ShouldNegate = true;
  } else if (Operator == ">") {
    ShouldNegate = true;
  } else if (Operator == "<=") {
    ShouldNegate = false;
  } else {
    return;
  }

  StringRef ObjectText = Lexer::getSourceText(
      CharSourceRange::getTokenRange(ObjectExpr->getSourceRange()),
      *Result.SourceManager, Result.Context->getLangOpts());

  std::string Replacement;
  if (ShouldNegate) {
    Replacement = "!" + ObjectText.str() + "." + IsEmptyMethodName;
  } else {
    Replacement = ObjectText.str() + "." + IsEmptyMethodName;
  }

  SourceLocation StartLoc = Comparison->getBeginLoc();
  SourceLocation EndLoc = Comparison->getEndLoc();

  diag(Comparison->getBeginLoc(),
       "use %0 instead of comparing %1 with 0")
      << Replacement << MethodNameStr
      << FixItHint::CreateReplacement(
             CharSourceRange::getTokenRange(StartLoc, EndLoc), Replacement);
}

} // namespace clang::tidy::generalsgamecode::readability
