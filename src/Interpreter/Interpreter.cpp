#include "Interpreter.h"
#include "AST/AST.h"
#include "Interpreter/Environment.h"

#include <variant>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// Helpers for loop control flow
struct BreakException {};
struct ContinueException {};

// Helper for function return
struct ReturnException { RaftValue value; };

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

RaftValue Interpreter::callUserFn(const FunctionDecl* fn, const std::vector<RaftValue>& args) {
    auto previous = currentEnv;
    currentEnv = std::make_shared<Environment>(globalEnv);

    if (fn->params.size() != args.size())
        throw std::runtime_error("Number of Arguments in call does not match with function declaration");

    for (size_t i = 0; i < fn->params.size(); i++)
        currentEnv->define(fn->params[i].name, args[i]);

    RaftValue result{std::monostate{}};
    try {
        execute(fn->body);
    } catch (ReturnException& ret) {
        result = ret.value;
    }

    currentEnv = previous;
    return result;
}

RaftValue Interpreter::applyBinOp(TokenType op, const RaftValue& left, const RaftValue& right) {
    // Handles logical expressions
    if (isBool(left) && isBool(right)) {
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
    if (isString(left) && isString(right)) {
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
        },
        [&](const std::unique_ptr<CallExpr>& expr) -> RaftValue {
            std::vector<RaftValue> argVals;

            for (auto& arg : expr->arguments) argVals.push_back(evaluate(arg));

            if (expr->resolved->native_def) return expr->resolved->native_def->impl(argVals);
            return callUserFn(expr->resolved->decl, argVals);
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
            RaftValue condition = evaluate(s->conditional);

            if (std::get<bool>(condition)) {
                execute(s->body);
            } else if (s->elseBranch) {
                execute(*s->elseBranch);
            }
        },

        [&](const std::unique_ptr<WhileStmt>& s) {
            RaftValue condition = evaluate(s->conditional);

            while (std::get<bool>(condition)) {
                try {
                    execute(s->body);
                
                    condition = evaluate(s->conditional);
                } catch(BreakException&) {
                    break;
                } catch(ContinueException&) {
                    continue;
                }
            };
        },

        [&](const std::unique_ptr<FunctionDecl>& s) {}, // Resolver has already handled 

        [&](const BreakStmt& s) { throw BreakException{}; },

        [&](const ContinueStmt& s) { throw ContinueException{}; },

        [&](const ReturnStmt& s) { throw ReturnException{ evaluate(s.value) }; },

        [&](const std::unique_ptr<BlockStmt>& s) {
            auto previous = currentEnv;
            currentEnv = std::make_shared<Environment>(previous);

            execute(s->statements);

            currentEnv = previous;
        },

        [&](const ExprStmt& s) {
            auto value = evaluate(s.expression);
        },

        [&](const ImportStmt& s) {
            // Implement later
        },

        [&](const std::unique_ptr<ModuleDecl>& s) {
            // Implement later
        }
    }, stmt);
}

void Interpreter::execute(const std::vector<Stmt>& statements) {
    for (const auto& statement: statements) {
        execute(statement);
    }
}