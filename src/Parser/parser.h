#pragma once

#include <stdexcept>
#include <vector>
#include <initializer_list>
#include <memory>
#include "Util/token.h"
#include "AST/AST.h"

// For error handling
class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& what)
    : std::runtime_error(what) {}
};

class Parser {
    std::vector<Token> tokens;
    size_t index = 0;

    bool isAtEnd();

    Token advance();
    Token peek();
    Token previous();
    Token expect(TokenType, const std::string&);

    bool check(TokenType);
    bool match(const std::initializer_list<TokenType>&);

    std::unique_ptr<Expr> parsePrimary();

    std::vector<std::unique_ptr<Expr>> parseArgs();
    std::unique_ptr<Expr> parseCall();
    
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parseFactor();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseExpression();

    std::unique_ptr<Stmt> parseExprStmt();
    std::unique_ptr<Stmt> parseVarDeclStmt();
    std::unique_ptr<Stmt> parseStmt();

public:
    Parser(const std::vector<Token>&);

    std::vector<std::unique_ptr<Stmt>> parse();
};