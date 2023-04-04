#include <vector>
#include <initializer_list>
#include <memory>

#include "Util/token.h"
#include "AST/AST.h"
#include "Parser/parser.h"

Parser::Parser(const std::vector<Token>& tokens)
: tokens(tokens) {}

bool Parser::isAtEnd() {
    return peek().type == EOFILE;
}

Token Parser::advance() {
    if (!isAtEnd()) index++;
    return previous();
}

Token Parser::peek() {
    return tokens[index];
}

Token Parser::previous() {
    return tokens[index - 1];
}

Token Parser::expect(TokenType type, const std::string& err) {
    if (check(type)) return advance();

    throw ParseError(err);
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(const std::initializer_list<TokenType>& types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }

    return false;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match({STRING}))
        return std::make_unique<StringExpr>(previous().value);

    if (match({NUMBER}))
        return std::make_unique<NumberExpr>(std::stof(previous().value));

    if (match({IDENTIFIER}))
        return std::make_unique<IdentifierExpr>(previous().value);

    if (match({TRUE})) return std::make_unique<BoolExpr>(true);
    if (match({FALSE})) return std::make_unique<BoolExpr>(false);

    if (match({LEFT_PAREN})) {
        auto expr = parseExpression();

        expect(RIGHT_PAREN, "Non-terminated delimiter");
        
        return std::make_unique<GroupedExpr>(std::move(expr));
    }
    
    throw ParseError("Expected expression.");
}

std::vector<std::unique_ptr<Expr>> Parser::parseArgs() {
    std::vector<std::unique_ptr<Expr>> Args;

    if (!check(LEFT_PAREN)) {
        do {
            Args.push_back(std::move(parseExpression()));
        } while (match({COMMA}));
    }

    expect(RIGHT_PAREN, "Expected a ')'");

    return Args;
}

std::unique_ptr<Expr> Parser::parseCall() {
    std::unique_ptr<Expr> expr = parsePrimary();

    if (previous().type != IDENTIFIER) return expr;

    if (match({LEFT_PAREN})) {
        expr = std::make_unique<CallExpr>(previous().value, parseArgs());
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match({BANG, MINUS})) {
        TokenType Op = previous().type;
        auto right = parseUnary();

        return std::make_unique<UnaryExpr>(std::move(right), Op);
    }

    return parseCall();
}

std::unique_ptr<Expr> Parser::parseFactor() {
    auto expr = parseUnary();

    while (match({MUL, DIV})) {
        TokenType Op = previous().type;
        auto right = parseUnary();

        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(right), Op);
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseTerm() {
    auto expr = parseFactor();

    while (match({PLUS, MINUS})) {
        TokenType Op = previous().type;
        auto right = parseFactor();

        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(right), Op);
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto expr = parseTerm();

    while (match({GREATER, LESS, GREATER_EQUAL, LESS_EQUAL})) {
        TokenType Op = previous().type;
        auto right = parseTerm();

        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(right), Op);
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto expr = parseComparison();

    while (match({EQUAL_EQUAL, BANG_EQUAL})) {
        TokenType Op = previous().type;
        auto right = parseComparison();

        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(right), Op);
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseEquality();
}

std::unique_ptr<Stmt> Parser::parseExprStmt() {
    std::unique_ptr<Expr> expr = parseExpression();
    expect(SEMICOLON, "Expected a semi-colon");

    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseVarDeclStmt() {
    Token id = expect(IDENTIFIER, "Expected an identifier");

    if (!match({EQUAL})) expect(SEMICOLON, "Expected a semi-colon");

    std::unique_ptr<Expr> expr = parseExpression();

    expect(SEMICOLON, "Expected a semi-colon");

    return std::make_unique<VarDeclStmt>(id.value, std::move(expr));
}



std::unique_ptr<Stmt> Parser::parseStmt() {
    if (match({VAR, CONST})) return parseVarDeclStmt();

    return parseExprStmt();
}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!isAtEnd()) {
        statements.push_back(
            std::move(parseStmt())
        );
    }

    return statements;
}