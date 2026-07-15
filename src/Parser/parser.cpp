#include <vector>
#include <initializer_list>
#include <memory>

#include "Util/token.h"
#include "AST/AST.h"
#include "Parser/parser.h"

Parser::Parser(const std::vector<Token>& tokens)
: tokens(tokens) {}

// matchs if current token is EOF
bool Parser::isAtEnd() {
    return current().type == TokenType::EOFILE;
}

// Consumes and returns current token moving the pointer forward
Token Parser::consume() {
    if (!isAtEnd()) index++;
    return previous();
}

// Looks at the upcoming token
Token Parser::peek() {
    if (!isAtEnd()) return tokens[index + 1];

    return Token(TokenType::EOFILE);
}

// Returns current token
Token Parser::current() {
    return tokens[index];
}

// Returns previous token
Token Parser::previous() {
    return tokens[index - 1];
}

// matchs if current token is type and consumes it, moving forward
// In the event the current token is not what is expected, an error is raised
Token Parser::expect(TokenType type, const std::string& err) {
    if (match(type)) return consume();

    throw ParseError(err);
}

// Same function as above but for matching multiple types at once
Token Parser::expect(const std::initializer_list<TokenType>& types, const std::string& err) {
    for (auto type : types) {
        if (match(type)) {
            return consume();
        }
    }

    throw ParseError(err);
}

// Convenience function to match a single type
bool Parser::match(TokenType type) {
    if (isAtEnd()) return false;
    return (current().type == type);
}

// Convenience function to match multiple types
bool Parser::match(const std::initializer_list<TokenType>& types) {
    for (auto type : types) {
        if (match(type)) {
            return true;
        }
    }

    return false;
}

// Convenience function to match multiple types for the next (upcoming) token
bool Parser::match_peek(const std::initializer_list<TokenType>& types) {
    auto peek_type = peek().type;
    for (auto type : types) {
        if (peek_type == type) {
            return true;
        }
    }

    return false;
}

bool Parser::match_peek(TokenType type) {
    auto peek_type = peek().type;
    
    return (peek_type == type);
}

Expr Parser::parseExpression() {
    Expr left = parseTerm();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        char op = match(TokenType::PLUS) ? '+' : '-';
        consume();
        Expr right = parseTerm();
        left = std::make_unique<BinaryExpr>(
            op,
            std::move(left),
            std::move(right)
        );
    }
    return left;
}

Expr Parser::parseTerm() {
    Expr left = parsePrimary();

    while (match({TokenType::MUL, TokenType::DIV})) {
        char op = match(TokenType::MUL) ? '*' : '/';
        consume();
        Expr right = parsePrimary();
        left = std::make_unique<BinaryExpr>(
            op,
            std::move(left),
            std::move(right)
        );
    }
    return left;
}

Expr Parser::parsePrimary() {
    if (match(TokenType::DOUBLE)) {
        Token tok = consume();
        return LiteralExpr{ std::get<double>(tok.value) };
    }

    if (match(TokenType::INT)) {
        Token tok = consume();
        return LiteralExpr{ std::get<int64_t>(tok.value) };
    }

    if (match(TokenType::STRING)) {
        Token tok = consume();
        return LiteralExpr{ std::get<std::string>(tok.value) };
    }

    if (match(TokenType::IDENTIFIER)) {
        Token tok = consume();
        return VariableExpr{ std::get<std::string>(tok.value) };
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        consume();
        Expr inner = parseExpression();
        expect(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return inner;
    }

    throw ParseError("Expected expression");
}

Expr Parser::parseLetStmt() {
    consume(); // Consumes the let keyword

    if (match({TokenType::VAR})) {
        consume(); // For the time being, lets not care about mutability
    }

    Token id = expect(TokenType::IDENTIFIER, "Expected an identifier");

    if (!match({TokenType::EQUAL})) expect(TokenType::SEMICOLON, "Expected a semi-colon");

    Expr expr = parseExpression();

    expect(TokenType::SEMICOLON, "Expected a semi-colon");

    return std::make_unique<AssignmentExpr>(std::get<std::string>(id.value), std::move(expr));
}



Expr Parser::parseStmt() {
    if (match({TokenType::LET})) {
        return parseLetStmt();
    }

    auto expr = parseExpression();
    expect(TokenType::SEMICOLON, "Expected a semi-colon");

    return expr;
}

std::vector<Expr> Parser::parse() {
    std::vector<Expr> statements;

    while (!isAtEnd()) {
        statements.push_back(
            std::move(parseStmt())
        );
    }

    return statements;
}