#pragma once

#include <unordered_map>

#include "Util/token.h"
#include "AST/AST.h"

struct RaftVariable {
    RaftValue value;
    bool isMutable;
    std::string declaredType;
};

class Environment {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr)
        : parent(std::move(parent)) {}

    void define(const std::string& name, RaftValue value, bool is_mutable = false, std::string declared_type = "") {
        values[name] = std::move(
            RaftVariable {
                std::move(value),
                is_mutable,
                std::move(declared_type)
            }
        );
    }

    RaftValue lookup(const std::string& name) {
        auto it = values.find(name);

        if (it != values.end()) return it->second.value;

        if (parent) return parent->lookup(name);

        throw std::runtime_error("Undefined variable: " + name);
    }

    void assign(const std::string& name, RaftValue value) {
        auto it = values.find(name);

        if (it != values.end()) {
            if (!it->second.isMutable) throw std::runtime_error(name + " is not a mutable value");

            it->second.value = std::move(value);
            return;
        }

        if (parent) { parent->assign(name, std::move(value)); return; }
        
        throw std::runtime_error("Undefined variable: " + name);
    }

private:
    std::unordered_map<std::string, RaftVariable> values;
    std::shared_ptr<Environment> parent;
};