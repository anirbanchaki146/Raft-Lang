#pragma once

#include <memory>
#include <optional>

#include "Util/token.h"

struct LiteralExpr {
    RaftValue val;
};

struct VariableExpr {
    std::string id;
};

struct BinaryExpr;
struct CallExpr;

using Expr = std::variant<
    std::unique_ptr<BinaryExpr>,
    LiteralExpr,
    VariableExpr,
    std::unique_ptr<CallExpr>
>;

struct BinaryExpr {
    TokenType op;
    Expr left;
    Expr right;
};

struct AssignmentStmt {
    std::string id;
    Expr value;
};

struct CallExpr {
    std::string callee;
    std::vector<Expr> arguments;
};

// Statements
struct VarDeclStmt {
    std::string name;
    bool isMutable;
    Expr value;
};

struct ExprStmt {
    Expr expression;
};

struct BreakStmt {};

struct ContinueStmt {};

struct BlockStmt;
struct IfStmt;
struct WhileStmt;
struct FunctionDecl;

struct ReturnStmt {
    Expr value;
};

using Stmt = std::variant<
    VarDeclStmt,
    ExprStmt,
    AssignmentStmt,
    BreakStmt,
    ContinueStmt,
    ReturnStmt,
    std::unique_ptr<WhileStmt>,
    std::unique_ptr<IfStmt>,
    std::unique_ptr<BlockStmt>,
    std::unique_ptr<FunctionDecl>
>;

struct BlockStmt {
    std::vector<Stmt> statements;
};

struct IfStmt {
    Expr conditional;
    Stmt body;
    std::optional<Stmt> elseBranch;
};

struct WhileStmt {
    Expr conditional;
    Stmt body;
};

struct Parameter {
    std::string name;
    std::string type;
};

struct FunctionDecl {
    std::string name;
    std::vector<Parameter> params;
    std::string returnType;
    Stmt body;
};