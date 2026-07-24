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
        result = evalBlockExpr(*fn->body);
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

RaftValue Interpreter::applyUnaryOp(TokenType op, const RaftValue& operand) {
    switch (op)
    {
    case TokenType::NOT:
        return !std::get<bool>(operand);

    case TokenType::MINUS:
        if (isDouble(operand)) return -(std::get<double>(operand));

        return -(std::get<int64_t>(operand));

    default:
        throw std::runtime_error("Unknown operator");
    }
}

RaftValue Interpreter::evalBlockExpr(const BlockExpr& block) {
    auto previous = currentEnv;
    currentEnv = std::make_shared<Environment>(previous);

    for (const auto& stmt : block.statements) execute(stmt);

    RaftValue result = block.tail.has_value() ? evaluate(**block.tail) : RaftValue{std::monostate{}};

    currentEnv = previous;
    return result;
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
        [&](const std::unique_ptr<UnaryExpr>& expr) -> RaftValue {
            RaftValue operand = evaluate(expr->operand);

            return applyUnaryOp(expr->op, operand);
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
        },

        [&](const std::unique_ptr<IfExpr>& s) {
            RaftValue condition = evaluate(s->condition);

            RaftValue value = std::monostate{};

            if (std::get<bool>(condition)) {
                value = evalBlockExpr(*s->thenBranch);
            } else if (s->elseBranch) {
                value = evalBlockExpr(*s->elseBranch);
            }

            return value;
        },

        [&](const std::unique_ptr<WhileExpr>& s) {
            RaftValue condition = evaluate(s->conditional);

            RaftValue value = std::monostate{};
            while (std::get<bool>(condition)) {
                try {
                    value = evalBlockExpr(*s->body);
                
                    condition = evaluate(s->conditional);
                } catch(BreakException&) {
                    break;
                } catch(ContinueException&) {
                    continue;
                }
            };

            return value;
        },

        [&](const std::unique_ptr<BlockExpr>& s) {
            return evalBlockExpr(*s);
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

        [&](const std::unique_ptr<FunctionDecl>& s) {}, // Resolver has already handled 

        [&](const BreakStmt& s) { throw BreakException{}; },

        [&](const ContinueStmt& s) { throw ContinueException{}; },

        [&](const ReturnStmt& s) { throw ReturnException{ evaluate(s.value) }; },

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

void Interpreter::executeProgram(const std::vector<Stmt>& program) {
    for (const auto& stmt : program) {
        // This loop only checks top level statements
        std::visit(overloaded{
            [&](const VarDeclStmt& s) {
                RaftValue val = evaluate(s.value);
                currentEnv->define(s.name, val, s.isMutable);
            },
            [&](const ImportStmt&) { /* handled by Resolver, nothing to do */ },
            [&](const std::unique_ptr<FunctionDecl>& f) {
                if (f->name == "main") mainFn = f.get();
            },
            [&](const std::unique_ptr<ModuleDecl>& m) { /* registered by Resolver, nothing to do */ },
            [](const auto&) {
                throw std::runtime_error(
                    "Only declarations (let, fn, mod, import) are allowed at the top level — "
                    "executable code must live inside a function");
            }
        }, stmt);
    }

    callUserFn(mainFn, {}); // Existence of main function guaranteed by Resolver
}