#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "Lexer/lexer.h"
#include "Parser/parser.h"
#include "CodeGen/IRCodegenVisitor.h"

void run(const std::string& input) {
    IRCodegenVisitor CG_visitor;

    Lexer lexer(input);
    auto tokens = lexer.scanTokens();

    if (lexer.error()) return;
    
    Parser parser(tokens);

    auto program = parser.parse();

    for (const auto& statement: program)
        statement->accept(CG_visitor);
}

void runPrompt() {
    IRCodegenVisitor CG_visitor;
    std::string input;
    
    std::cout <<
    "Raft JIT [v1.0.0]\n"
    "licensed under GPL 3\n"
    "Use help() for more information\n\n";

    for (;;) {
        std::cout << "Raft> ";

        std::getline(std::cin, input);

        if (input == "exit()") return;

        try {
            Lexer lexer(input);
            auto tokens = lexer.scanTokens();

            if (lexer.error()) return;
            
            Parser parser(tokens);

            auto program = parser.parse();

            for (const auto& statement: program) {
                statement->accept(CG_visitor)->print(llvm::errs());
                std::cout << '\n';
            }
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
    }
}

void runFile(const std::string& filePath) {
    std::ifstream file(filePath);

    if (!file) {
        throw std::runtime_error("Could not open file for reading");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    
    try {
        run(buffer.str());
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        runFile(argv[1]);
        return 0;
    }

    runPrompt();
    return 0;
}