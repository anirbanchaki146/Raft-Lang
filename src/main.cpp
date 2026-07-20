#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "Lexer/lexer.h"
#include "Parser/parser.h"
#include "Interpreter/Interpreter.h"
#include "TypeChecker/TypeChecker.h"
#include "Resolver/Resolver.h"

void run(const std::string& input) {
    Lexer lexer(input);
    auto tokens = lexer.scanTokens();

    if (lexer.error()) return;
    
    Parser parser(tokens);

    auto program = parser.parse();

    Resolver resolver;

    resolver.resolveProgram(program);

    TypeChecker checker;

    for (const auto& statement: program)
        checker.checkStmt(statement);

    Interpreter interpreter;

    interpreter.execute(std::move(program));
}

void runPrompt() {
    std::string input;
    
    std::cout <<
    "Raft JIT [v1.0.0]\n"
    "licensed under GPL 3\n"
    "Use exit() to quit\n\n";
    
    auto interpreter = Interpreter();

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

            interpreter.execute(std::move(program));
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