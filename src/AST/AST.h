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
struct AssignmentExpr;

using Expr = std::variant<
    std::unique_ptr<BinaryExpr>,
    std::unique_ptr<AssignmentExpr>,
    LiteralExpr,
    VariableExpr
>;

struct BinaryExpr {
    char op;
    Expr left;
    Expr right;
};

struct AssignmentExpr {
    std::string id;
    Expr val;
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
    ExprStmt
    // std::unique_ptr<BlockStmt>
>;

/*
struct BlockStmt {
    std::vector<Stmt> statements;
};
*/