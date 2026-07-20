#include <variant>

#include "Module.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::vector<NativeFunctionDef> getAllNativeDefs() {
    std::vector<NativeFunctionDef> defs;

    defs.push_back({ "std.io.println", {Type::String}, Type::Void,
        [](std::vector<RaftValue>& args) -> RaftValue {
            for (const auto& arg: args)
                std::cout << std::get<std::string>(arg);
            
            std::cout << "\n";

            return RaftValue{std::monostate{}};
        }
    });
    
    defs.push_back({ "std.io.print", {Type::String}, Type::Void,
        [](std::vector<RaftValue>& args) -> RaftValue {
            for (const auto& arg: args)
                std::cout << std::get<std::string>(arg);

            return RaftValue{std::monostate{}};
        }
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