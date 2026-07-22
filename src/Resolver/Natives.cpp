#include <variant>

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

std::vector<NativeFunctionDef> getAllNativeDefs() {
    std::vector<NativeFunctionDef> defs;

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

    return defs;
}