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

using Expr = std::variant<
    std::unique_ptr<BinaryExpr>,
    LiteralExpr,
    VariableExpr
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

using Stmt = std::variant<
    VarDeclStmt,
    ExprStmt,
    AssignmentStmt,
    BreakStmt,
    ContinueStmt,
    std::unique_ptr<WhileStmt>,
    std::unique_ptr<IfStmt>,
    std::unique_ptr<BlockStmt>
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