#include "Interpreter.h"
#include "AST/AST.h"

#include <variant>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

bool Interpreter::isDouble(const RaftValue& val) {
    return std::holds_alternative<double>(val);
}

bool Interpreter::isString(const RaftValue& val) {
    return std::holds_alternative<std::string>(val);
}

bool Interpreter::isBool(const RaftValue& val) {
    return std::holds_alternative<bool>(val);
}

double Interpreter::asDouble(const RaftValue& val) {
    if (auto* i = std::get_if<int64_t>(&val)) return static_cast<double>(*i);
    if (auto* d = std::get_if<double>(&val))  return *d;

    throw std::runtime_error("Expected a numeric value");
}

RaftValue Interpreter::applyBinOp(char op, const RaftValue& left, const RaftValue& right) {
    // Handles concatenation
    if (isString(left) || isString(right)) {
        if (op != '+') {
            throw std::runtime_error("Operator not supported for strings");
        }
        if (!isString(left) || !isString(right)) {
            throw std::runtime_error("Cannot mix string and non-string operands");
        }

        return std::get<std::string>(left) + std::get<std::string>(right);
    }

    // Handles numerical results
    bool resultIsDouble = isDouble(left) || isDouble(right);

    if (resultIsDouble) {
        double l = asDouble(left);
        double r = asDouble(right);

        switch (op) {
            case '+': return l + r;
            case '-': return l - r;
            case '*': return l * r;
            case '/': return l / r;
        }
    } else {
        int64_t l = std::get<int64_t>(left);
        int64_t r = std::get<int64_t>(right);
        switch (op) {
            case '+': return l + r;
            case '-': return l - r;
            case '*': return l * r;
            case '/':
                if (r == 0) throw std::runtime_error("Division by zero");
                return l / r;
        }
    }
    
    throw std::runtime_error("Unknown operator");
}

RaftValue Interpreter::evaluate(const Expr& expression) {
    return std::visit(overloaded {
        [&](LiteralExpr expr) -> RaftValue {
            if (isString(expr.val)) return std::get<std::string>(expr.val);
            if (isDouble(expr.val)) return std::get<double>(expr.val);
            if (isBool(expr.val)) return std::get<bool>(expr.val);
            
            return std::get<int64_t>(expr.val);
        },
        [](const std::unique_ptr<AssignmentExpr>& expr) -> RaftValue { return 0.0; },
        [](VariableExpr expr) -> RaftValue { return 0.0; },
        [&](const std::unique_ptr<BinaryExpr>& expr) -> RaftValue {
            RaftValue left = evaluate(expr->left);
            RaftValue right = evaluate(expr->right);

            return applyBinOp(expr->op, left, right);
        }
    }, expression);
}

void Interpreter::execute() {
    for (const auto& statement: statements) {
        auto value = evaluate(statement);
        
        // Temporary pretty printer
        std::visit(overloaded{
            [](std::monostate) { std::cout << "nil" << std::endl; },  // or "none", "undefined" — your call on the label
            [](int64_t i)      { std::cout << i << std::endl; },
            [](double d)       { std::cout << d << std::endl; },
            [](const std::string& s) { std::cout << s << std::endl; },
            [](bool b)         { std::cout << (b ? "true\n" : "false\n"); }  // if RaftValue has bool
        }, value);
    }
}