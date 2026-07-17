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
            // Temporary arrangement for println. This will be later addressed through FFI
            if (expr->callee == "println") {
                for (const auto& arg : expr->arguments) {
                    auto val = evaluate(arg);

                    std::visit(overloaded{
                        [](std::monostate) { std::cout << "nil"; },  
                        [](int64_t i)      { std::cout << i; },
                        [](double d)       { std::cout << d; },
                        [](const std::string& s) { std::cout << s; },
                        [](bool b)         { std::cout << (b ? "true" : "false"); } 
                    }, val);
                }

                std::cout << "\n";
                return std::monostate {};
            }

            auto it = functions.find(expr->callee);

            if (it == functions.end())
                throw std::runtime_error("Undefined function: " + expr->callee);

            const FunctionDecl* fn = it->second;

            if (expr->arguments.size() != fn->params.size())
                throw std::runtime_error("Wrong number of arguments to " + expr->callee);

            std::vector<RaftValue> argVals;
            for (const auto& arg : expr->arguments) argVals.push_back(evaluate(arg));

            auto callEnv = std::make_shared<Environment>(globalEnv);

            for (size_t i = 0; i < fn->params.size(); ++i) {
                callEnv->define(fn->params[i].name, argVals[i], true);
            }

            auto previous = currentEnv;
            currentEnv = callEnv;

            RaftValue result{std::monostate{}};
            try {
                execute(fn->body);
            } catch (ReturnException& ret) {
                result = ret.value;
            }

            currentEnv = previous;
            return result;
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

        [&](const std::unique_ptr<FunctionDecl>& s) {
            functions[s->name] = s.get();
        },

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
        }
    }, stmt);
}

void Interpreter::execute(const std::vector<Stmt>& statements) {
    for (const auto& statement: statements) {
        execute(statement);
    }
}