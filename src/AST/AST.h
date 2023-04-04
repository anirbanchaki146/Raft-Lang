#pragma once

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "Util/token.h"

class IRCodegenVisitor;

class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    virtual llvm::Value* accept(const IRCodegenVisitor&) = 0;
};

class Expr : public ASTNode {
public:
    virtual ~Expr() = default;
    
    virtual llvm::Value* accept(const IRCodegenVisitor&) = 0;
};

class Stmt : public ASTNode {
public:
    virtual ~Stmt() = default;
    
    virtual llvm::Value* accept(const IRCodegenVisitor&) = 0;
};

class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;

    ExprStmt(std::unique_ptr<Expr> expr)
    : expr(std::move(expr)) {}
    
    llvm::Value* accept(const IRCodegenVisitor&) override;
};

class VarDeclStmt : public Stmt {
public:
    std::string name;
    std::unique_ptr<Expr> expr;

    VarDeclStmt(const std::string& name, std::unique_ptr<Expr> expr)
    : name(name), expr(std::move(expr)) {}
    
    llvm::Value* accept(const IRCodegenVisitor&) override;
};

class NumberExpr : public Expr {
public:
    float Value;

    NumberExpr(float Value) : Value(Value) {}
    llvm::Value* accept(const IRCodegenVisitor&) override;
};

class StringExpr : public Expr {
public:
    std::string Value;

    StringExpr(const std::string& Value)
    : Value(Value) {}

    llvm::Value* accept(const IRCodegenVisitor&) override;
};

class IdentifierExpr : public Expr {
public:
    std::string Value;

    IdentifierExpr(const std::string& Value)
    : Value(Value) {}

    llvm::Value* accept(const IRCodegenVisitor&) override;
};

class BoolExpr : public Expr {
public:
    bool Value;

    BoolExpr(bool Value) : Value(Value) {}

    llvm::Value* accept(const IRCodegenVisitor&) override;
};

class CallExpr : public Expr {
public:
    std::string Name;
    std::vector<std::unique_ptr<Expr>> Args;

    CallExpr(const std::string& Name, std::vector<std::unique_ptr<Expr>> Args)
    : Name(Name), Args(std::move(Args)) {}

    llvm::Value* accept(const IRCodegenVisitor&) override;
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> LHS;
    std::unique_ptr<Expr> RHS;
    TokenType Op;

    BinaryExpr(std::unique_ptr<Expr> LHS, std::unique_ptr<Expr> RHS, 
    TokenType Op)
    : LHS(std::move(LHS)), RHS(std::move(RHS)), Op(Op) {}
    
    llvm::Value* accept(const IRCodegenVisitor&) override;
};

class UnaryExpr : public Expr {
public:
    TokenType Op;
    std::unique_ptr<Expr> RHS;

    UnaryExpr(std::unique_ptr<Expr> RHS, TokenType Op)
    : RHS(std::move(RHS)), Op(Op) {}
    
    llvm::Value* accept(const IRCodegenVisitor&) override;
};

class GroupedExpr : public Expr {
public:
    std::unique_ptr<Expr> Expression;

    GroupedExpr(std::unique_ptr<Expr> Expression) 
    : Expression(std::move(Expression)) {}
    
    llvm::Value* accept(const IRCodegenVisitor&) override;
};

