#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

#include "Util/token.h"
#include "AST/AST.h"

using NativeFunction = std::function<RaftValue(std::vector<RaftValue>&)>;

enum class Type {
    Int,
    Double,
    Bool,
    String,
    Void,
    Unknown
};

struct FunctionSig {
    std::vector<Type> params;
    Type return_type;
    bool is_variadic = false; // This is specially for functions that can take multiple arguments irrespective of types (println)
};

struct NativeFunctionDef {
    std::string qualifiedName;
    std::vector<Type> paramTypes;
    Type returnType;
    NativeFunction impl;
    bool is_variadic = false;
};

struct FunctionDecl;

struct FunctionInfo {
    const FunctionDecl* decl = nullptr;         // For user-defined functions
    const NativeFunctionDef* native_def = nullptr;  // For natively defined functions
    FunctionSig signature;
};

struct Module {
    std::string name;
    std::unordered_map<std::string, FunctionInfo> functions;
    std::unordered_map<std::string, std::unique_ptr<Module>> submodules;
    std::unordered_map<std::string, Module*> aliases;
    Module* parent = nullptr;
};

std::vector<NativeFunctionDef> getAllNativeDefs();