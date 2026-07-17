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
    
    std::unordered_map<std::string, const FunctionDecl*> functions;

    std::vector<Stmt> statements;

    RaftValue evaluate(const Expr&);

    bool isDouble(const RaftValue&);
    bool isString(const RaftValue&);
    bool isBool(const RaftValue&);

    double asDouble(const RaftValue&);
    RaftValue applyBinOp(TokenType, const RaftValue&, const RaftValue&);
    
public:
    Interpreter() : globalEnv(std::make_shared<Environment>()), currentEnv(globalEnv) {}

    void execute(const Stmt& stmt);
    void execute(const std::vector<Stmt>& stmts);
};