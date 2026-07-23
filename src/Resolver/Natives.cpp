#include <variant>
#include <cmath>

#include "Module.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// Helpers
bool isDouble(const RaftValue& val) {
    return std::holds_alternative<double>(val);
}

bool isString(const RaftValue& val) {
    return std::holds_alternative<std::string>(val);
}

bool isBool(const RaftValue& val) {
    return std::holds_alternative<bool>(val);
}

bool isInt(const RaftValue& val) {
    return std::holds_alternative<int64_t>(val);
}

double asDouble(const RaftValue& val) {
    return std::get<double>(val);
}

std::vector<NativeFunctionDef> getAllNativeDefs() {
    std::vector<NativeFunctionDef> defs;

    // --- std.io ---
    defs.push_back({ "std.io.println", {}, Type::Void,
        [](std::vector<RaftValue>& args) -> RaftValue {
            for (const auto& arg: args) {
                if (isDouble(arg)) std::cout << std::get<double>(arg);

                else if (isBool(arg)) {
                    auto val = std::get<bool>(arg);

                    if (val) std::cout << "true";
                    else std::cout << "false";
                } 
                else if (isString(arg)) std::cout << std::get<std::string>(arg);
                else if (isInt(arg)) std::cout << std::get<int64_t>(arg);
                else std::cout << "None";
            }
            
            std::cout << "\n";

            return RaftValue{std::monostate{}};
        },
        true // is_variadic set to true
    });
    
    defs.push_back({ "std.io.print", {Type::String}, Type::Void,
        [](std::vector<RaftValue>& args) -> RaftValue {
            for (const auto& arg: args) {
                if (isDouble(arg)) std::cout << std::get<double>(arg);

                else if (isBool(arg)) {
                    auto val = std::get<bool>(arg);

                    if (val) std::cout << "true";
                    else std::cout << "false";
                } 
                else if (isString(arg)) std::cout << std::get<std::string>(arg);
                else if (isInt(arg)) std::cout << std::get<int64_t>(arg);
                else std::cout << "None";
            }

            return RaftValue{std::monostate{}};
        },
        true // is_variadic set to true
    });

    defs.push_back({ "std.io.input", {}, Type::String,
        [](std::vector<RaftValue>& args) -> RaftValue {
            std::string line;
            std::getline(std::cin, line);
            return RaftValue{line};
        }}
    );

        // --- std.math ---
    defs.push_back({ "std.math.sqrt", {Type{Type::Double}}, Type{Type::Double}, 
        [](std::vector<RaftValue>& args) -> RaftValue { return std::sqrt(asDouble(args[0])); }});

    defs.push_back({ "std.math.abs", {Type{Type::Double}}, Type{Type::Double}, 
        [](std::vector<RaftValue>& args) -> RaftValue { return std::abs(asDouble(args[0])); }});

    defs.push_back({ "std.math.pow", {Type{Type::Double}, Type{Type::Double}}, Type{Type::Double}, 
        [](std::vector<RaftValue>& args) -> RaftValue {
            return std::pow(asDouble(args[0]), asDouble(args[1]));
        }});

    defs.push_back({ "std.math.min", {Type{Type::Double}, Type{Type::Double}}, Type{Type::Double}, 
        [](std::vector<RaftValue>& args) -> RaftValue {
            return std::min(asDouble(args[0]), asDouble(args[1]));
        }});

    defs.push_back({ "std.math.max", {Type{Type::Double}, Type{Type::Double}}, Type{Type::Double}, 
        [](std::vector<RaftValue>& args) -> RaftValue {
            return std::max(asDouble(args[0]), asDouble(args[1]));
        }});

    // --- std.string ---
    defs.push_back({ "std.string.length", {Type{Type::String}}, Type{Type::Int}, 
        [](std::vector<RaftValue>& args) -> RaftValue {
            return static_cast<int64_t>(std::get<std::string>(args[0]).length());
        }});

    defs.push_back({ "std.string.toUpper", {Type{Type::String}}, Type{Type::String},
        [](std::vector<RaftValue>& args) -> RaftValue {
            std::string s = std::get<std::string>(args[0]);
            for (auto& c : s) c = std::toupper(c);
            return s;
        }});

    defs.push_back({ "std.string.toLower", {Type{Type::String}}, Type{Type::String},
        [](std::vector<RaftValue>& args) -> RaftValue {
            std::string s = std::get<std::string>(args[0]);
            for (auto& c : s) c = std::tolower(c);
            return s;
        }});

    return defs;
}