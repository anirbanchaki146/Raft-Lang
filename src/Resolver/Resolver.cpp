#include <sstream>

#include "Resolver.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

Type Resolver::typeFromString(const std::string& s) {
    if (s == "int") return Type::Int;
    if (s == "double") return Type::Double;
    if (s == "bool") return Type::Bool;
    if (s == "string") return Type::String;
    if (s == "") return Type::Void;

    throw std::runtime_error("Unknown type: " + s);
}

std::string Resolver::typeToString(Type t) {
    switch (t) {
        case Type::Int: return "int";
        case Type::Double: return "double";
        case Type::Bool: return "bool";
        case Type::String: return "string";
        case Type::Void: return "void";

        default: return "unknown";
    }
}

std::vector<std::string> Resolver::splitByDot(const std::string& s) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string part;
    while (std::getline(ss, part, '.')) parts.push_back(part);
    return parts;
}

std::string Resolver::joinWithDots(const std::vector<std::string>& v) {
    auto result = std::string();

    for (size_t i = 0; i < v.size(); ++i) {
        result.append(v[i]);

        if (i != (v.size() - 1)) result.append(".");
    }

    return result;
}

void Resolver::registerNativeModules() {
    for (const auto& def : nativeDefs) {
        std::vector<std::string> parts = splitByDot(def.qualifiedName);
        std::string funcName = parts.back();
        parts.pop_back();   // remaining parts are the module path, e.g. {"std", "io"}
        bool variadic = def.is_variadic; // For variadic functions (only for native functions)

        Module* mod = &root;
        for (const auto& segment : parts) {
            auto it = mod->submodules.find(segment);
            if (it == mod->submodules.end()) {
                auto newMod = std::make_unique<Module>();
                newMod->name = segment;
                newMod->parent = mod;
                mod = mod->submodules.emplace(segment, std::move(newMod)).first->second.get();
            } else {
                mod = it->second.get();
            }
        }

        FunctionInfo info;
        info.native_def = &def;
        info.signature = FunctionSig{ def.paramTypes, def.returnType, variadic };
        mod->functions[funcName] = info;
    }
}

void Resolver::registerStmt(Stmt& stmt, Module* currentScope) {
    std::visit(overloaded {
        [&](std::unique_ptr<FunctionDecl>& fn) {
            FunctionInfo info;
            info.decl = fn.get();
            std::vector<Type> paramTypes;
            for (auto& p : fn->params) paramTypes.push_back(typeFromString(p.type));
            Type returnType = fn->returnType.empty() ? Type::Void : typeFromString(fn->returnType);
            info.signature = FunctionSig{ paramTypes, returnType };
            currentScope->functions[fn->name] = info;
        },

        [&](std::unique_ptr<ModuleDecl>& mod) {
            auto newMod = std::make_unique<Module>();
            newMod->name = mod->name;
            newMod->parent = currentScope;
            Module* modPtr = newMod.get();
            currentScope->submodules[mod->name] = std::move(newMod);

            for (auto& inner : mod->body) {
                registerStmt(inner, modPtr);
            }
        },
        [](auto&) { /* everything else ignored during registration */ }
    }, stmt);
}

void Resolver::registerUserDeclarations(std::vector<Stmt>& program) {
    for (auto& stmt : program) {
        registerStmt(stmt, &root);
    }
}

const FunctionInfo* Resolver::tryResolveFrom(const std::vector<std::string>& nameParts, Module* scope) {
    Module* mod = scope;

    for (size_t i = 0; i + 1 < nameParts.size(); ++i) {
        auto subIt = mod->submodules.find(nameParts[i]);
        if (subIt != mod->submodules.end()) { mod = subIt->second.get(); continue; }

        auto aliasIt = mod->aliases.find(nameParts[i]);
        if (aliasIt != mod->aliases.end()) { mod = aliasIt->second; continue; }

        return nullptr;   // this segment doesn't exist from `scope` — give up, don't climb
    }

    auto it = mod->functions.find(nameParts.back());
    if (it == mod->functions.end()) return nullptr;
    return &it->second;
}

const FunctionInfo* Resolver::resolvePath(const std::vector<std::string>& nameParts, Module* currentScope) {
    for (Module* scope = currentScope; scope != nullptr; scope = scope->parent) {
        if (const FunctionInfo* found = tryResolveFrom(nameParts, scope)) {
            return found;
        }
    }

    throw std::runtime_error("Cannot find : " + joinWithDots(nameParts));
}

void Resolver::resolveExpr(const Expr& expr, Module* currentScope) {
    std::visit(overloaded{
        [&](const std::unique_ptr<CallExpr>& e) {
            e->resolved = resolvePath(e->name_parts, currentScope);
            for (auto& arg : e->arguments) resolveExpr(arg, currentScope);
        },
        [&](const std::unique_ptr<BinaryExpr>& e) {
            resolveExpr(e->left, currentScope);
            resolveExpr(e->right, currentScope);
        },
        [](const auto&) { /* literals, variables — nothing to resolve */ }
    }, expr);
}

void Resolver::resolveStmt(Stmt& stmt, Module* currentScope) {
    std::visit(overloaded{
        [&](VarDeclStmt& s) { resolveExpr(s.value, currentScope); },
        [&](ExprStmt& s) { resolveExpr(s.expression, currentScope); },
        [&](ReturnStmt& s) { resolveExpr(s.value, currentScope); },
        [&](std::unique_ptr<IfStmt>& s) {
            resolveExpr(s->conditional, currentScope);
            resolveStmt(*s->elseBranch, currentScope);
            if (s->elseBranch) resolveStmt(*s->elseBranch, currentScope);
        },
        [&](std::unique_ptr<WhileStmt>& s) {
            resolveExpr(s->conditional, currentScope);
            resolveStmt(s->body, currentScope);
        },
        [&](std::unique_ptr<BlockStmt>& s) {
            for (auto& inner : s->statements) resolveStmt(inner, currentScope);
        },
        [&](std::unique_ptr<FunctionDecl>& s) {
            resolveStmt(s->body, currentScope);
        },
        [&](std::unique_ptr<ModuleDecl>& s) {
            Module* modPtr = currentScope->submodules[s->name].get();
            for (auto& inner : s->body) resolveStmt(inner, modPtr);
        },
        [&](ImportStmt& s) {
            if (s.wild_card) {
                Module* mod = &root;
                for (const auto& segment : s.path) {
                    auto it = mod->submodules.find(segment);
                    if (it == mod->submodules.end())
                        throw std::runtime_error("Unknown module: " + segment);

                    mod = it->second.get();
                }

                for (auto& [name, info] : mod->functions) {
                    root.functions[name] = info;
                }

                return;
            } 

            bool is_module = true;
            Module* mod = &root;
            for (const auto& segment: s.path) {
                auto it = mod->submodules.find(segment);
                if (it != mod->submodules.end()) {
                    mod = it->second.get();
                    continue;
                }
                
                is_module = false;
                break;
            }

            if (is_module) {
                std::string alias = s.path.back();
                root.aliases[alias] = mod;
            } else {
                const FunctionInfo* function = resolvePath(s.path, &root);
                std::string alias = s.path.back();
                root.functions[alias] = *function;
            }
        },
        [](auto&) { /* BreakStmt, ContinueStmt */ }
    }, stmt);
}

void Resolver::resolveProgram(std::vector<Stmt>& program) {
    registerNativeModules();
    registerUserDeclarations(program);

    for (auto& stmt : program) {
        resolveStmt(stmt, &root);
    }
}
