#pragma once

#include <memory>
#include <optional>

#include "Util/token.h"
#include "Resolver/Module.h"

struct LiteralExpr {
    RaftValue val;
};

struct VariableExpr {
    std::string id;
};

struct BinaryExpr;
struct UnaryExpr;

struct IndexExpr;
struct ArrayExpr;

struct IfExpr;
struct BlockExpr;
struct WhileExpr;

struct CallExpr;

using Expr = std::variant<
    LiteralExpr,
    VariableExpr,
    std::unique_ptr<BinaryExpr>,
    std::unique_ptr<UnaryExpr>,
    std::unique_ptr<CallExpr>,
    
    std::unique_ptr<IfExpr>,
    std::unique_ptr<BlockExpr>,
    std::unique_ptr<WhileExpr>
>;

struct BinaryExpr {
    TokenType op;
    Expr left;
    Expr right;
};

struct UnaryExpr {
    TokenType op;
    Expr operand;
};

struct AssignmentStmt {
    std::string id;
    Expr value;
    TokenType op;
};

struct FunctionInfo;

struct CallExpr {
    std::vector<std::string> name_parts;
    std::vector<Expr> arguments;
    // This below is for the resolver.
    mutable const FunctionInfo* resolved = nullptr;
};

// Statements
struct VarDeclStmt {
    std::string name;
    bool isMutable;
    Expr value;

    std::string annotated_type;
};

struct ExprStmt {
    Expr expression;
};

struct BreakStmt {};

struct ContinueStmt {};
struct FunctionDecl;

struct ReturnStmt {
    Expr value;
};

struct ImportStmt {
    std::vector<std::string> path;
    bool wild_card;
};

struct ModuleDecl;

using Stmt = std::variant<
    VarDeclStmt,
    ExprStmt,
    AssignmentStmt,
    BreakStmt,
    ContinueStmt,
    ReturnStmt,
    ImportStmt,
    std::unique_ptr<ModuleDecl>,
    std::unique_ptr<FunctionDecl>
>;

struct BlockStmt {
    std::vector<Stmt> statements;
};

struct Parameter {
    std::string name;
    std::string type;
};

struct FunctionDecl {
    std::string name;
    std::vector<Parameter> params;
    std::string returnType;
    std::unique_ptr<BlockExpr> body;
};

struct ModuleDecl {
    std::string name;
    std::vector<Stmt> body;
};


struct BlockExpr {
    std::vector<Stmt> statements;
    std::optional<std::unique_ptr<Expr>> tail;
};

struct IfExpr {
    Expr condition;
    std::unique_ptr<BlockExpr> thenBranch;
    std::unique_ptr<BlockExpr> elseBranch;
};

struct WhileExpr {
    Expr conditional;
    std::unique_ptr<BlockExpr> body;
};
