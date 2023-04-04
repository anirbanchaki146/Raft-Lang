#include "AST/AST.h"
#include "CodeGen/IRCodegenVisitor.h"

llvm::Value* ExprStmt::accept(const IRCodegenVisitor& visitor) {
    return visitor.codegen(*this);
}

llvm::Value* VarDeclStmt::accept(const IRCodegenVisitor& visitor) {
    return visitor.codegen(*this);
}

llvm::Value* NumberExpr::accept(const IRCodegenVisitor& visitor) {
    return visitor.codegen(*this);
}

llvm::Value* StringExpr::accept(const IRCodegenVisitor& visitor) {
    return visitor.codegen(*this);
}

llvm::Value* IdentifierExpr::accept(const IRCodegenVisitor& visitor) {
    return visitor.codegen(*this);
}

llvm::Value* BoolExpr::accept(const IRCodegenVisitor& visitor) {
    return visitor.codegen(*this);
}

llvm::Value* CallExpr::accept(const IRCodegenVisitor& visitor) {
    return visitor.codegen(*this);
}

llvm::Value* BinaryExpr::accept(const IRCodegenVisitor& visitor) {
    return visitor.codegen(*this);
}

llvm::Value* UnaryExpr::accept(const IRCodegenVisitor& visitor) {
    return visitor.codegen(*this);
}

llvm::Value* GroupedExpr::accept(const IRCodegenVisitor& visitor) {
    return visitor.codegen(*this);
}