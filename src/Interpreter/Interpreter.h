#pragma once

#include <iostream>
#include <variant>
#include <map>

#include "Parser/parser.h"
#include "AST/AST.h"
#include "Util/token.h"

class Interpreter {
private:
    std::map<std::string, RaftValue> Environment;
    std::vector<Expr> statements;

    RaftValue evaluate(const Expr&);

    bool isDouble(const RaftValue&);
    bool isString(const RaftValue&);
    bool isBool(const RaftValue&);

    double asDouble(const RaftValue&);
    RaftValue applyBinOp(char, const RaftValue&, const RaftValue&);
public:
    Interpreter(std::vector<Expr> stmts) : statements(std::move(stmts)) {}

    void execute();
};