#include "Interpreter.h"
#include "AST/AST.h"
#include "Interpreter/Environment.h"

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

RaftValue Interpreter::applyBinOp(TokenType op, const RaftValue& left, const RaftValue& right) {
    // Handles logical expressions
    if (isBool(left) || isBool(right)) {
        if (!isBool(left) || !isBool(right)) {
            throw std::runtime_error("Cannot mix bool and non-bool operands");
        }

        bool l = std::get<bool>(left);
        bool r = std::get<bool>(right);

        switch (op)
        {
        case TokenType::LOG_AND: return (l && r);
        case TokenType::LOG_OR: return (l || r);
        
        default: throw std::runtime_error("Operator not supported for bool");
        }
    }

    // Handles concatenation
    if (isString(left) || isString(right)) {
        if (op != TokenType::PLUS) {
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
            // Arithmatic operators
            case TokenType::PLUS: return l + r;
            case TokenType::MINUS: return l - r;
            case TokenType::MUL: return l * r;
            case TokenType::DIV: return l / r;

            // Relational operators
            case TokenType::EQUAL_EQUAL: return l == r;
            case TokenType::NOT_EQUAL: return l != r;
            case TokenType::LESS_EQUAL: return l <= r;
            case TokenType::LESS: return l < r;
            case TokenType::GREATER_EQUAL: return l >= r;
            case TokenType::GREATER: return l > r;
        }
    } else {
        int64_t l = std::get<int64_t>(left);
        int64_t r = std::get<int64_t>(right);
        switch (op) {
            // Arithmatic operators
            case TokenType::PLUS: return l + r;
            case TokenType::MINUS: return l - r;
            case TokenType::MUL: return l * r;
            case TokenType::DIV:
                if (r == 0) throw std::runtime_error("Division by zero");
                return l / r;

            // Relational operators
            case TokenType::EQUAL_EQUAL: return l == r;
            case TokenType::NOT_EQUAL: return l != r;
            case TokenType::LESS_EQUAL: return l <= r;
            case TokenType::LESS: return l < r;
            case TokenType::GREATER_EQUAL: return l >= r;
            case TokenType::GREATER: return l > r;
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
        [&](VariableExpr expr) -> RaftValue { 
            return currentEnv->lookup(expr.id);
        },
        [&](const std::unique_ptr<BinaryExpr>& expr) -> RaftValue {
            RaftValue left = evaluate(expr->left);
            RaftValue right = evaluate(expr->right);

            return applyBinOp(expr->op, left, right);
        }
    }, expression);
}

void Interpreter::execute(const Stmt& stmt) {
    std::visit(overloaded {
        [&](const VarDeclStmt& s) {
            RaftValue val = evaluate(s.value);
            currentEnv->define(s.name, val, s.isMutable);
        },

        [&](const AssignmentStmt& s) {
            RaftValue val = evaluate(s.value);
            currentEnv->assign(s.id, val);
        },

        [&](const std::unique_ptr<IfStmt>& s) {
            RaftValue val = evaluate(s->conditional);

            if (!isBool(val)) throw std::runtime_error("If condition must be a boolean");
            
            bool condition = std::get<bool>(val);

            if (condition) execute(s->body);
        },

        [&](const std::unique_ptr<WhileStmt>& s) {
            RaftValue condition = evaluate(s->conditional);

            if (!isBool(condition)) throw std::runtime_error("While condition must be a boolean");

            while (std::get<bool>(condition)) { 
                execute(s->body);
                
                // Recheck condition
                condition = evaluate(s->conditional);
            };
        },

        [&](const std::unique_ptr<BlockStmt>& s) {
            execute(s->statements);
        },

        [&](const ExprStmt& s) {
            auto value = evaluate(s.expression);

            // Temporary pretty printer
            std::visit(overloaded{
                [](std::monostate) { std::cout << "nil" << std::endl; },  // or "none", "undefined" — your call on the label
                [](int64_t i)      { std::cout << i << std::endl; },
                [](double d)       { std::cout << d << std::endl; },
                [](const std::string& s) { std::cout << s << std::endl; },
                [](bool b)         { std::cout << (b ? "true\n" : "false\n"); }  // if RaftValue has bool
            }, value);
        }
    }, stmt);
}

void Interpreter::execute(const std::vector<Stmt>& statements) {
    for (const auto& statement: statements) {
        execute(statement);
    }
}