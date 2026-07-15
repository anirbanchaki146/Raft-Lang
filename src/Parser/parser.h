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

    Token consume();
    Token current();
    Token previous();
    Token peek();

    Token expect(TokenType, const std::string&);
    Token expect(const std::initializer_list<TokenType>&, const std::string&);

    bool match(const std::initializer_list<TokenType>&);
    bool match(TokenType);

    bool match_peek(const std::initializer_list<TokenType>&);
    bool match_peek(TokenType);

    Expr parsePrimary();
    Expr parseTerm();
    Expr parseExpression();

    Stmt parseLetStmt();
    Stmt parseStmt();

public:
    Parser(const std::vector<Token>&);

    std::vector<Stmt> parse();
};