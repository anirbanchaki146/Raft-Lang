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