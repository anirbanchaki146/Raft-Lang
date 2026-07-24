#pragma once

#include <iostream>
#include <variant>
#include <map>

#include "Parser/parser.h"
#include "AST/AST.h"
#include "Util/token.h"
#include "Interpreter/Environment.h"
#include "TypeChecker/TypeChecker.h"

class Interpreter {
private:
    std::shared_ptr<Environment> globalEnv;
    std::shared_ptr<Environment> currentEnv;

    const FunctionDecl* mainFn = nullptr;

    RaftValue evalBlockExpr(const BlockExpr&);
    RaftValue evaluate(const Expr&);

    bool isDouble(const RaftValue&);
    bool isString(const RaftValue&);
    bool isBool(const RaftValue&);

    double asDouble(const RaftValue&);

    RaftValue applyBinOp(TokenType, const RaftValue&, const RaftValue&);
    RaftValue applyUnaryOp(TokenType, const RaftValue&);

    RaftValue getAugmentedRHS(TokenType, const RaftValue&);

    RaftValue callUserFn(const FunctionDecl*, const std::vector<RaftValue>&);

    void execute(const Stmt&);
    void execute(const std::vector<Stmt>&);
    
public:
    Interpreter() : globalEnv(std::make_shared<Environment>()), currentEnv(globalEnv) {}
    void executeProgram(const std::vector<Stmt>&);
};