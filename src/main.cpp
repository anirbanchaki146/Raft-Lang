#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "Lexer/lexer.h"
#include "Parser/parser.h"
#include "Interpreter/Interpreter.h"
#include "TypeChecker/TypeChecker.h"
#include "Resolver/Resolver.h"

namespace fs = std::filesystem;

std::vector<Stmt> parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    Lexer lexer;
    auto tokens = lexer.scanTokens(buffer.str());
    if (lexer.error()) {
        throw std::runtime_error("Lexing failed in file: " + filePath);
    }

    Parser parser(tokens);
    return parser.parse();
}

// "math.rft" --> mod math
std::string moduleNameFromFilename(const fs::path& path) {
    std::string stem = path.stem().string();
    return stem;
}

// Finds every sibling .rft file
std::vector<Stmt> loadSiblingModules(const std::string& entryFilePath) {
    std::vector<Stmt> moduleStmts;

    fs::path entryPath = fs::absolute(entryFilePath);
    fs::path dir = entryPath.parent_path();

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".rft") continue;
        if (fs::equivalent(entry.path(), entryPath)) continue;   // skip the entry file itself

        std::string modName = moduleNameFromFilename(entry.path());
        std::vector<Stmt> body = parseFile(entry.path().string());

        moduleStmts.push_back(
            std::make_unique<ModuleDecl>(ModuleDecl{ modName, std::move(body) })
        );
    }

    return moduleStmts;
}

void run(const std::string& entryFilePath, const std::string& entrySource) {
    Lexer lexer;
    auto tokens = lexer.scanTokens(entrySource);
    if (lexer.error()) return;

    Parser parser(tokens);
    auto entryProgram = parser.parse();

    // Discover and parse sibling files as modules.
    auto moduleStmts = loadSiblingModules(entryFilePath);

    std::vector<Stmt> program;
    program.reserve(moduleStmts.size() + entryProgram.size());
    for (auto& m : moduleStmts) program.push_back(std::move(m));
    for (auto& s : entryProgram) program.push_back(std::move(s));

    Resolver resolver;
    resolver.resolveProgram(program);

    TypeChecker checker;
    for (const auto& statement : program)
        checker.checkStmt(statement);

    Interpreter interpreter;
    interpreter.executeProgram(std::move(program));
}

void runFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        throw std::runtime_error("Could not open file for reading");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    try {
        run(filePath, buffer.str());
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        runFile(argv[1]);
        return 0;
    }
    
    std::cout << "No root file provided. Kindly provide a filename to be considered root.\n";
    return 0;
}