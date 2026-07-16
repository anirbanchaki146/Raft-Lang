#pragma once

#include <memory>
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

// struct BlockStmt;

using Stmt = std::variant<
    VarDeclStmt,
    ExprStmt,
    AssignmentStmt
    // std::unique_ptr<BlockStmt>
>;

/*
struct BlockStmt {
    std::vector<Stmt> statements;
};
*/
