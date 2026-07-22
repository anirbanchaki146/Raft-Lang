#include <variant>

#include "TypeChecker/TypeChecker.h"
#include "Resolver/Module.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct ReturnException {
    Type return_type;
};

Type TypeChecker::typeFromString(const std::string& s) {
    if (s == "int") return Type::Int;
    if (s == "double") return Type::Double;
    if (s == "bool") return Type::Bool;
    if (s == "string") return Type::String;
    if (s == "") return Type::Void;

    throw std::runtime_error("Unknown type: " + s);
}

std::string TypeChecker::typeToString(Type t) {
    switch (t) {
        case Type::Int: return "int";
        case Type::Double: return "double";
        case Type::Bool: return "bool";
        case Type::String: return "string";
        case Type::Void: return "void";

        default: return "unknown";
    }
}

bool TypeChecker::isNumber(Type type) {
    return (type == Type::Int || type == Type::Double);
}

bool TypeChecker::isRelational(TokenType op) {
    return (
        op == TokenType::GREATER ||
        op == TokenType::GREATER_EQUAL ||
        op == TokenType::LESS ||
        op == TokenType::LESS_EQUAL ||
        op == TokenType::EQUAL_EQUAL ||
        op == TokenType::NOT_EQUAL
    );
}

bool TypeChecker::isLogical(TokenType op) {
    return (
        op == TokenType::LOG_AND ||
        op == TokenType::LOG_OR
    );
}

Type TypeChecker::checkExpr(const Expr& expr) {
    return std::visit(overloaded {
        [](const LiteralExpr& e) -> Type {
            if (std::holds_alternative<std::string>(e.val)) return Type::String;
            if (std::holds_alternative<int64_t>(e.val)) return Type::Int;
            if (std::holds_alternative<double>(e.val)) return Type::Int;
            if (std::holds_alternative<bool>(e.val)) return Type::Bool;

            throw std::runtime_error("Fatal error: Unknown literal");
        },
        [&](const VariableExpr& e) -> Type {
            return currentTypes->lookup(e.id);
        },
        [&](const std::unique_ptr<BinaryExpr>& e) -> Type {
            Type leftType = checkExpr(e->left);
            Type rightType = checkExpr(e->right);
            
            return checkBinaryOp(e->op, leftType, rightType);
        },
        [&](const std::unique_ptr<UnaryExpr>& e) -> Type {
            Type operandType = checkExpr(e->operand);
            
            return checkUnaryOp(e->op, operandType);
        },
        [&](const std::unique_ptr<CallExpr>& e) -> Type {
            if (!e->resolved) throw std::runtime_error("Internal error: unresolved function");

            const auto& sig = e->resolved->signature;

            if (sig.is_variadic) {
                for (const auto& arg: e->arguments)
                    checkExpr(arg);
                
                return sig.return_type;
            }

            if (e->arguments.size() != sig.params.size())
                throw std::runtime_error("Wrong number of arguments"); // Implement notation

            for (size_t i = 0; i < e->arguments.size(); ++i) {
                Type argType = checkExpr(e->arguments[i]);

                Type expected = sig.params[i];

                if (argType != expected && !(expected == Type::Double && argType == Type::Int)) {
                    throw std::runtime_error(
                        "Argument " + std::to_string(i + 1) + " of call to " + // Implement notation
                        ": expected " + typeToString(expected) + ", got " + typeToString(argType));
                }
            }

            return e->resolved->signature.return_type;
        }
    }, expr);
}

Type TypeChecker::checkBinaryOp(TokenType op, Type left, Type right) {
    if (left == Type::String) {
        if (right != Type::String) throw std::runtime_error("Invalid: RHS must be string");

        if (op != TokenType::PLUS) throw std::runtime_error("Operator not supported for strings");

        return Type::String;
    }

    if (isNumber(left)) {
        if (!isNumber(right)) throw std::runtime_error("Invalid: RHS must be a numeric value");

        if (isRelational(op)) return Type::Bool;
 
        if (left == Type::Double || right == Type::Double) return Type::Double;

        return Type::Int;
    }

    if (left == Type::Bool) {
        if (right != Type::Bool) throw std::runtime_error("Invalid: RHS must be a boolean");

        if (!isLogical(op)) throw std::runtime_error("Logical operators must handle booleans");

        return Type::Bool;
    }


    throw std::runtime_error("Fatal error: Invalid literal in binary operator");
}

Type TypeChecker::checkUnaryOp(TokenType op, Type operand) {
    if (op == TokenType::MINUS) {
        if (!isNumber(operand)) throw std::runtime_error("Unary negation can only be used on numerical types");

        return operand;
    }

    if (op == TokenType::NOT) {
        if (operand != Type::Bool) throw std::runtime_error("Logical not operator is not supported for non booleans");

        return Type::Bool;
    }


    throw std::runtime_error("Fatal error: Invalid operator");
}

void TypeChecker::checkStmt(const Stmt& stmt) {
    std::visit(overloaded{
        [&](const VarDeclStmt& s) {
            Type initType = checkExpr(s.value);
            Type annotatedType = typeFromString(s.annotated_type);

            if ((annotatedType != Type::Void) && (initType != annotatedType)) {
                if (initType == Type::Int && annotatedType == Type::Double) {
                    currentTypes->define(s.name, Type::Double);
                    return;
                }

                throw std::runtime_error("Declaration type is not compatible with annotated type");
            }

            currentTypes->define(s.name, initType);
        },

        [&](const AssignmentStmt& s) {
            Type expected = currentTypes->lookup(s.id);
            Type actual = checkExpr(s.value);

            if (actual != expected) {
                if (!(expected == Type::Double && actual == Type::Int)) {
                    throw std::runtime_error(
                        "Assignment type mismatch: \'" + s.id + "\' defined as " + typeToString(expected) +
                        " but being assigned to " + typeToString(actual));
                }
            }
        },

        [&](const ExprStmt& s) {
            checkExpr(s.expression);
        },
        
        [&](const std::unique_ptr<FunctionDecl>& s) {
            auto previous = currentTypes;

            currentTypes = std::make_shared<TypeEnvironment>(globalTypes);

            for (const auto& param : s->params) {
                currentTypes->define(param.name, typeFromString(param.type));
            }  

            auto previousExpectedReturn = currentExpectedReturn;
            currentExpectedReturn = typeFromString(s->returnType);
            
            // Note to Self: Implemented this seperately instead of just passing body to checkStmt
            // because we want to capture the globalEnv instead of the previous environment
            for (const auto& statement: std::get<std::unique_ptr<BlockStmt>>(s->body)->statements) {
                checkStmt(statement);
            }

            currentExpectedReturn = previousExpectedReturn;
            currentTypes = previous;
        },

        [&](const ReturnStmt& s) {
            Type actual = checkExpr(s.value);

            if (actual != currentExpectedReturn) {
                if (!(currentExpectedReturn == Type::Double && actual == Type::Int)) {
                    throw std::runtime_error(
                        "Return type mismatch: function declaration specifies returning " + typeToString(currentExpectedReturn) +
                        " but body returns " + typeToString(actual));
                }
            }
        },

        [&](const BreakStmt& s) {
            if (loop_depth == 0) throw std::runtime_error("Used break outside a loop");
        },

        [&](const ContinueStmt& s) {
            if (loop_depth == 0) throw std::runtime_error("Used break outside a loop");
        },

        [&](const std::unique_ptr<BlockStmt>& s) {
            auto previous = currentTypes;
            currentTypes = std::make_shared<TypeEnvironment>(previous);

            for (const auto& statement: s->statements) {
                checkStmt(statement);
            }

            currentTypes = previous;
        },

        [&](const std::unique_ptr<IfStmt>& s) {
            Type cond = checkExpr(s->conditional);

            if (cond != Type::Bool) {
                throw std::runtime_error("Condition for an if statement must be a bool");
            }

            checkStmt(s->body);
        },

        [&](const std::unique_ptr<WhileStmt>& s) {
            Type cond = checkExpr(s->conditional);

            if (cond != Type::Bool) {
                throw std::runtime_error("Condition for a while loop must be a bool");
            }

            loop_depth++;
            checkStmt(s->body);
            loop_depth--;
        },

        [&](const std::unique_ptr<ModuleDecl>& s) {
            for (auto const& stmt: s->body)
                checkStmt(stmt);
        },

        [&](const ImportStmt& s) {
            // Nothing to do here as resolver has already handled
        }

    }, stmt);
}