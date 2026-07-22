#pragma once

#include <unordered_map>
#include <vector>

#include "Parser/parser.h"
#include "AST/AST.h"
#include "Util/token.h"
#include "Resolver/Module.h"

class TypeEnvironment {
public:
    explicit TypeEnvironment(std::shared_ptr<TypeEnvironment> parent = nullptr)
        : parent(std::move(parent)) {}

    void define(const std::string& name, Type type) {
        types[name] = type;
    }

    Type lookup(const std::string& name) {
        if (auto it = types.find(name); it != types.end()) return it->second;
        if (parent) return parent->lookup(name);
        throw std::runtime_error("Undefined variable: " + name);
    }

private:
    std::unordered_map<std::string, Type> types;
    std::shared_ptr<TypeEnvironment> parent;
};

class TypeChecker {
public:
    TypeChecker() : globalTypes(std::make_shared<TypeEnvironment>()), currentTypes(globalTypes) {}

    Type checkExpr(const Expr&);
    Type checkBinaryOp(TokenType, Type, Type);
    Type checkUnaryOp(TokenType, Type);

    void checkStmt(const Stmt&);

private:
    std::shared_ptr<TypeEnvironment> globalTypes;
    std::shared_ptr<TypeEnvironment> currentTypes;

    Type currentExpectedReturn = Type::Void;

    Type typeFromString(const std::string&);
    std::string typeToString(Type);

    bool isNumber(Type type);

    bool isRelational(TokenType op);

    bool isLogical(TokenType op);

    int loop_depth = 0;
};