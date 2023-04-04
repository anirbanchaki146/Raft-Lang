#include "CodeGen/IRCodegenVisitor.h"

IRCodegenVisitor::IRCodegenVisitor () {
    Context = std::make_unique<llvm::LLVMContext>();
    Builder = std::unique_ptr<llvm::IRBuilder<>>(new llvm::IRBuilder<>(*Context));
    Module = std::make_unique<llvm::Module>("Module", *Context);
}

void IRCodegenVisitor::dumpIRCodegen() {
    Module->print(llvm::outs(), nullptr);
}

llvm::Value* IRCodegenVisitor::codegen(const NumberExpr& expr) const {
    return llvm::ConstantFP::get(*Context, llvm::APFloat(expr.Value));
}

llvm::Value* IRCodegenVisitor::codegen(const StringExpr& expr) const {
    return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(const IdentifierExpr& expr) const {
    auto itr = SymbolTable.find(expr.Value);

    if (itr == SymbolTable.end())
        throw std::runtime_error("Unknown Variable Name");

    return itr->second;
}

llvm::Value* IRCodegenVisitor::codegen(const BoolExpr& expr) const {
    return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(const CallExpr& expr) const {
    llvm::Function* CallF = Module->getFunction(expr.Name);

    if (!CallF) throw std::runtime_error("CallF is Null");

    if (CallF->arg_size() != expr.Args.size())
        throw std::runtime_error("Incorrect number of arguments passed");

    std::vector<llvm::Value*> ArgsV;

    for (int i = 0; i != expr.Args.size(); i++) {
        ArgsV.push_back(expr.Args[i]->accept(*this));
    }

    return Builder->CreateCall(CallF, ArgsV, "calltmp");
}

llvm::Value* IRCodegenVisitor::codegen(const GroupedExpr& expr) const {
    return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(const UnaryExpr& expr) const {
    return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(const BinaryExpr& expr) const {
    llvm::Value* L = expr.LHS->accept(*this);
    llvm::Value* R = expr.RHS->accept(*this);

    if (!L || !R) throw std::runtime_error("L or R is Null");

    if (!Builder) throw std::runtime_error("Builder is null");

    switch (expr.Op) {
        case PLUS:
            return Builder->CreateFAdd(L, R, "addtmp");
        case MINUS:
            return Builder->CreateFSub(L, R, "subtmp");
        case MUL:
            return Builder->CreateFMul(L, R, "multmp");
        case DIV:
            return Builder->CreateFDiv(L, R, "divtmp");
        default:
            throw std::runtime_error("Invaild Op");
    }   
}

llvm::Value* IRCodegenVisitor::codegen(const ExprStmt& stmt) const {
    llvm::Value* Val = stmt.expr->accept(*this);
    return Val;
}

llvm::Value* IRCodegenVisitor::codegen(const VarDeclStmt& stmt) const {
    llvm::Value* Val = stmt.expr->accept(*this);
    return Val;
}