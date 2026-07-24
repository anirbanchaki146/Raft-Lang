#pragma once

#include "Module.h"
#include "AST/AST.h"

class Resolver {
public:
    void resolveProgram(std::vector<Stmt>& program);

private:
    std::vector<NativeFunctionDef> nativeDefs = getAllNativeDefs();
    std::unordered_map<std::string, FunctionSig> functionSigs;
    
    Module root;

    Type typeFromString(const std::string&);
    std::string typeToString(Type);

    std::string joinWithDots(const std::vector<std::string>&);

    void registerNativeModules();
    void registerUserDeclarations(std::vector<Stmt>& program);
    void registerStmt(Stmt& stmt, Module* currentScope);

    void resolveStmt(Stmt& stmt, Module* currentScope);
    void resolveExpr(const Expr& expr, Module* currentScope);

    void resolveBlockExpr(BlockExpr&, Module* currentScope);

    const FunctionInfo* tryResolveFrom(const std::vector<std::string>&, Module*);
    const FunctionInfo* resolvePath(const std::vector<std::string>& nameParts, Module* currentScope);

    static std::vector<std::string> splitByDot(const std::string& s);
};