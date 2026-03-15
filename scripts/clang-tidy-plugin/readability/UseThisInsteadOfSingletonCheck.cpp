#include "UseThisInsteadOfSingletonCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace clang::tidy::generalsgamecode::readability {


static const CXXMethodDecl *getEnclosingMethod(ASTContext *Context,
                                                const Stmt *S) {
  const CXXMethodDecl *Method = nullptr;
  
  auto Parents = Context->getParents(*S);
  while (!Parents.empty()) {
    if (const auto *M = Parents[0].get<CXXMethodDecl>()) {
      if (!M->isStatic()) {
        Method = M;
        break;
      }
    }
    Parents = Context->getParents(Parents[0]);
  }
  
  return Method;
}

static bool typesMatch(const QualType &SingletonType,
                       const CXXRecordDecl *EnclosingClass) {
  if (!EnclosingClass) {
    return false;
  }

  const Type *TypePtr = SingletonType.getTypePtrOrNull();
  if (!TypePtr) {
    return false;
  }

  if (const PointerType *PtrType = TypePtr->getAs<PointerType>()) {
    QualType PointeeType = PtrType->getPointeeType();
    if (const RecordType *RecordTy = PointeeType->getAs<RecordType>()) {
      if (const CXXRecordDecl *RecordDecl =
              dyn_cast<CXXRecordDecl>(RecordTy->getDecl())) {
        return RecordDecl->getCanonicalDecl() ==
               EnclosingClass->getCanonicalDecl();
      }
    }
  }

  return false;
}

void UseThisInsteadOfSingletonCheck::registerMatchers(MatchFinder *Finder) {
  auto MemberExprMatcher = memberExpr(
      hasObjectExpression(ignoringParenImpCasts(declRefExpr(to(varDecl(anyOf(hasGlobalStorage(), hasExternalFormalLinkage())).bind("singletonVar"))))),
      hasDeclaration(fieldDecl()),
      unless(hasAncestor(cxxMemberCallExpr())))
      .bind("memberExpr");

  auto MemberCallMatcher = cxxMemberCallExpr(
      on(ignoringParenImpCasts(declRefExpr(to(varDecl(anyOf(hasGlobalStorage(), hasExternalFormalLinkage())).bind("singletonVarCall"))))))
      .bind("memberCall");

  Finder->addMatcher(MemberExprMatcher, this);
  Finder->addMatcher(MemberCallMatcher, this);
}

void UseThisInsteadOfSingletonCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *MemExpr = Result.Nodes.getNodeAs<clang::MemberExpr>("memberExpr");
  const auto *MemberCall = Result.Nodes.getNodeAs<CXXMemberCallExpr>("memberCall");
  
  if (!MemExpr && !MemberCall) {
    return;
  }

  const Stmt *TargetStmt = nullptr;
  const VarDecl *FoundSingletonVar = nullptr;
  bool IsCall = false;

  if (MemberCall) {
    IsCall = true;
    TargetStmt = MemberCall;
    FoundSingletonVar = Result.Nodes.getNodeAs<VarDecl>("singletonVarCall");
    if (!FoundSingletonVar) {
      const Expr *ImplicitObject = MemberCall->getImplicitObjectArgument();
      if (ImplicitObject) {
        ImplicitObject = ImplicitObject->IgnoreParenImpCasts();
        if (const DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(ImplicitObject)) {
          FoundSingletonVar = dyn_cast<VarDecl>(DRE->getDecl());
        }
      }
    }
  } else if (MemExpr) {
    TargetStmt = MemExpr;
    FoundSingletonVar = Result.Nodes.getNodeAs<VarDecl>("singletonVar");
  }

  if (!TargetStmt || !FoundSingletonVar) {
    return;
  }

  StringRef SingletonName = FoundSingletonVar->getName();
  if (!SingletonName.starts_with("The") || SingletonName.size() <= 3 ||
      (SingletonName[3] < 'A' || SingletonName[3] > 'Z')) {
    return;
  }

  const ASTContext *Context = Result.Context;

  const CXXMethodDecl *EnclosingMethod = getEnclosingMethod(
      const_cast<ASTContext *>(Context), TargetStmt);
  if (!EnclosingMethod) {
    return;
  }

  const CXXRecordDecl *EnclosingClass = EnclosingMethod->getParent();
  if (!EnclosingClass) {
    return;
  }

  QualType SingletonType = FoundSingletonVar->getType();
  if (!typesMatch(SingletonType, EnclosingClass)) {
    return;
  }

  const ValueDecl *Member = nullptr;
  StringRef MemberName;
  SourceLocation StartLoc, EndLoc;

  if (IsCall && MemberCall) {
    const CXXMethodDecl *Method = MemberCall->getMethodDecl();
    if (!Method) {
      return;
    }
    if (Method->isStatic()) {
      return;
    }
    if (EnclosingMethod->isConst() && !Method->isConst()) {
      return;
    }
    Member = Method;
    MemberName = Method->getName();
    StartLoc = MemberCall->getBeginLoc();
    EndLoc = MemberCall->getEndLoc();
  } else if (MemExpr) {
    Member = MemExpr->getMemberDecl();
    if (!Member) {
      return;
    }
    MemberName = Member->getName();
    StartLoc = MemExpr->getBeginLoc();
    EndLoc = MemExpr->getEndLoc();
  } else {
    return;
  }

  SourceManager &SM = *Result.SourceManager;

  std::string Replacement = std::string(MemberName);
  
  if (IsCall && MemberCall) {
    SourceLocation RParenLoc = MemberCall->getRParenLoc();
    const Expr *Callee = MemberCall->getCallee();
    
    if (RParenLoc.isValid() && Callee) {
      SourceLocation CalleeEnd = Lexer::getLocForEndOfToken(
          Callee->getEndLoc(), 0, SM, Result.Context->getLangOpts());
      
      if (CalleeEnd.isValid() && CalleeEnd < RParenLoc) {
        SourceLocation ArgsStart = CalleeEnd;
        SourceLocation ArgsEnd = Lexer::getLocForEndOfToken(
            RParenLoc, 0, SM, Result.Context->getLangOpts());
        
        if (ArgsStart.isValid() && ArgsEnd.isValid() && ArgsStart < ArgsEnd) {
          StringRef ArgsText = Lexer::getSourceText(
              CharSourceRange::getCharRange(ArgsStart, ArgsEnd), SM,
              Result.Context->getLangOpts());
          ArgsText = ArgsText.ltrim();
          if (!ArgsText.empty()) {
            Replacement += ArgsText.str();
          } else {
            Replacement += "()";
          }
        } else {
          Replacement += "()";
        }
      } else {
        SourceLocation CallEnd = Lexer::getLocForEndOfToken(
            MemberCall->getEndLoc(), 0, SM, Result.Context->getLangOpts());
        if (CalleeEnd.isValid() && CallEnd.isValid() && CalleeEnd < CallEnd) {
          StringRef ArgsText = Lexer::getSourceText(
              CharSourceRange::getCharRange(CalleeEnd, CallEnd), SM,
              Result.Context->getLangOpts());
          ArgsText = ArgsText.ltrim();
          if (!ArgsText.empty()) {
            Replacement += ArgsText.str();
          } else {
            Replacement += "()";
          }
        } else {
          Replacement += "()";
        }
      }
    } else {
      Replacement += "()";
    }
  }

  diag(StartLoc, "use '%0' instead of '%1->%2' when inside a member function")
      << Replacement << FoundSingletonVar->getName() << MemberName
      << FixItHint::CreateReplacement(
             CharSourceRange::getTokenRange(StartLoc, EndLoc), Replacement);
}

} // namespace clang::tidy::generalsgamecode::readability

