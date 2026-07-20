#pragma once

#include "Module.h"
#include "AST/AST.h"

class Resolver {
public:
    void resolveProgram(std::vector<Stmt>& program);

private:
    std::vector<NativeFunctionDef> nativeDefs = getAllNativeDefs();
    
    Module root;

    Type typeFromString(const std::string&);
    std::string typeToString(Type);

    void registerNativeModules();
    void registerUserDeclarations(std::vector<Stmt>& program);
    void registerStmt(Stmt& stmt, Module* currentScope);

    void resolveStmt(Stmt& stmt, Module* currentScope);
    void resolveExpr(const Expr& expr, Module* currentScope);

    const FunctionInfo* resolvePath(const std::vector<std::string>& nameParts, Module* currentScope);
    static std::vector<std::string> splitByDot(const std::string& s);
};