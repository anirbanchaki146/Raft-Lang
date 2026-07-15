#pragma once

#include <unordered_map>

#include "Util/token.h"
#include "AST/AST.h"

class Environment {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr)
        : parent(std::move(parent)) {}

    void define(const std::string& name, RaftValue value) {
        values[name] = std::move(value);
    }

    RaftValue lookup(const std::string& name) {
        auto it = values.find(name);

        if (it != values.end()) return it->second;

        if (parent) return parent->lookup(name);

        throw std::runtime_error("Undefined variable: " + name);
    }

    void assign(const std::string& name, RaftValue value) {
        auto it = values.find(name);

        if (it != values.end()) {
            it->second = std::move(value);
            return;
        }

        if (parent) { parent->assign(name, std::move(value)); return; }
        
        throw std::runtime_error("Undefined variable: " + name);
    }

private:
    std::unordered_map<std::string, RaftValue> values;
    std::shared_ptr<Environment> parent;
};