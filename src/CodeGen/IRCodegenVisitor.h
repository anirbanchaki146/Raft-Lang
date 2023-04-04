#pragma once

#include <map>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "Lexer/lexer.h"
#include "Parser/parser.h"
#include "AST/AST.h"

class IRCodegenVisitor {
    std::unique_ptr<llvm::LLVMContext> Context;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::unique_ptr<llvm::Module> Module;

    std::map<std::string, llvm::Value*> SymbolTable;

public:
    IRCodegenVisitor();

    void dumpIRCodegen();

    llvm::Value* codegen(const NumberExpr&) const;
    llvm::Value* codegen(const StringExpr&) const;
    llvm::Value* codegen(const IdentifierExpr&) const;
    llvm::Value* codegen(const BoolExpr&) const;
    llvm::Value* codegen(const CallExpr&) const;
    llvm::Value* codegen(const GroupedExpr&) const;
    llvm::Value* codegen(const UnaryExpr&) const;
    llvm::Value* codegen(const BinaryExpr&) const;

    llvm::Value* codegen(const ExprStmt&) const;
    llvm::Value* codegen(const VarDeclStmt&) const;
};