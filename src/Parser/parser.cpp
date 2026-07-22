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

Expr Parser::parseLogic() {
    Expr left = parseComparison();

    while (match({ TokenType::LOG_AND, TokenType::LOG_OR })) {
        auto op = consume().type;

        Expr right = parseComparison();
        left = std::make_unique<BinaryExpr>(
            op,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

Expr Parser::parseComparison() {
    Expr left = parseExpression();

    while (match(
        {
            TokenType::GREATER,
            TokenType::GREATER_EQUAL,
            TokenType::LESS,
            TokenType::LESS_EQUAL,
            TokenType::EQUAL_EQUAL,
            TokenType::NOT_EQUAL
        }
    )) {
        auto op = consume().type;

        Expr right = parseExpression();
        left = std::make_unique<BinaryExpr>(
            op,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

Expr Parser::parseExpression() {
    Expr left = parseTerm();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        auto op = consume().type;

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
    Expr left = parseUnary();

    while (match({TokenType::MUL, TokenType::DIV})) {
        auto op = consume().type;

        Expr right = parseUnary();
        left = std::make_unique<BinaryExpr>(
            op,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

Expr Parser::parseUnary() {
    while (match({TokenType::MINUS, TokenType::NOT})) {
        auto op = consume().type;

        Expr right = parsePrimary();

        return std::make_unique<UnaryExpr>(
            op,
            std::move(right)
        );
    }

    return parsePrimary();
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

    if (match(TokenType::BOOL)) {
        Token tok = consume();
        return LiteralExpr{ std::get<bool>(tok.value) };
    }

    if (match(TokenType::IDENTIFIER)) {
        std::vector<std::string> name_parts;
        name_parts.push_back(std::get<std::string>(consume().value));

        while (match(TokenType::DOT)) {
            consume();
            auto tok = expect(TokenType::IDENTIFIER, "Expected identifier after dot");
            name_parts.push_back(std::get<std::string>(tok.value));
        }

        if (match(TokenType::LEFT_PAREN)) {
            consume();
            std::vector<Expr> args;
            
            while (!match(TokenType::RIGHT_PAREN)) {
                args.push_back(parseLogic());

                if (match(TokenType::COMMA)) {
                    consume();
                    continue;
                }

                break;
            }

            expect(TokenType::RIGHT_PAREN, "Expected ')' after parameters");

            return std::make_unique<CallExpr> ( name_parts, std::move(args) );
        }

        if (name_parts.size() > 1) throw ParseError("Modules do not (yet) support variables");
        return VariableExpr{ name_parts[0] };
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        consume();
        Expr inner = parseLogic();
        expect(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return inner;
    }

    throw ParseError("Expected expression");
}

Stmt Parser::parseLetStmt() {
    bool mut = false;
    consume(); // Consumes the let keyword

    if (match({TokenType::VAR})) {
        mut = true;
        consume();
    }

    Token id = expect(TokenType::IDENTIFIER, "Expected an identifier");

    std::string annotated_type = "";
    if (match(TokenType::COLON)) {
        consume();
        auto annotation = expect(TokenType::IDENTIFIER, "Expected type annotation after colon");

        annotated_type = std::get<std::string>(annotation.value);
    }

    if (!match({TokenType::EQUAL})) {
        expect(TokenType::SEMICOLON, "Expected a semi-colon");

        return VarDeclStmt {std::get<std::string>(id.value), mut, LiteralExpr {std::monostate()}};
    }

    consume(); // Consumes the equal

    Expr expr = parseLogic();

    expect(TokenType::SEMICOLON, "Expected a semi-colon");

    return VarDeclStmt{ std::get<std::string>(id.value), mut, std::move(expr), annotated_type };
}

Stmt Parser::parseAssignment() {
    Token id = consume();

    auto op = consume().type; // Consumes the equal

    Expr expr = parseLogic();

    expect(TokenType::SEMICOLON, "Expected a semi-colon");

    return AssignmentStmt { std::get<std::string>(id.value), std::move(expr), op};
}

Stmt Parser::parseStmt() {
    if (match(TokenType::IMPORT)) {
        return parseImportStmt();
    }

    if (match(TokenType::MOD)) {
        return parseModuleDecl();
    }

    if (match(TokenType::LET)) {
        return parseLetStmt();
    }

    if (match(TokenType::IF)) {
        return parseIfStmt();
    }

    if (match(TokenType::WHILE)) {
        return parseWhileStmt();
    }

    if (match(TokenType::FN)) {
        return parseFnDecl();
    }

    if (match(TokenType::BREAK)) {
        consume();

        expect(TokenType::SEMICOLON, "Expected a semi-colon");

        return BreakStmt {};
    }

    if (match(TokenType::CONTINUE)) {
        consume();

        expect(TokenType::SEMICOLON, "Expected a semi-colon");

        return ContinueStmt {};
    }

    if (match(TokenType::RETURN)) {
        consume();

        auto expr = parseLogic();

        expect(TokenType::SEMICOLON, "Expected a semi-colon");

        return ReturnStmt { std::move(expr) };
    }

    if (match(TokenType::LEFT_BRACE)) {
        return parseBlock();
    }
    
    if (match(TokenType::IDENTIFIER) && match_peek(TokenType::EQUAL)) {
        return parseAssignment();
    }

    auto expr = parseLogic();

    expect(TokenType::SEMICOLON, "Expected a semi-colon");

    return ExprStmt{ std::move(expr) };
}

Stmt Parser::parseBlock() {
    expect(TokenType::LEFT_BRACE, "Expected an opening brace");

    std::vector<Stmt> body;

    while (!match(TokenType::RIGHT_BRACE) &&  !isAtEnd()) {
        body.push_back(parseStmt());
    }

    expect(TokenType::RIGHT_BRACE, "Expected a closing brace");

    return std::make_unique<BlockStmt>( std::move(body) );
}

Stmt Parser::parseIfStmt() {
    consume(); // Consume if
    
    auto expr = parseLogic();
    auto body = parseBlock();

    std::optional<Stmt> elseBranch = std::nullopt;
    if (match(TokenType::ELSE)) {
        consume();

        if (match(TokenType::IF)) {
            elseBranch = parseIfStmt();
        } else {
            elseBranch = parseBlock();
        }
    }

    return std::make_unique<IfStmt>( std::move(expr), std::move(body), std::move(elseBranch) );
}

Stmt Parser::parseWhileStmt() {
    consume(); // Consume while
    
    auto expr = parseLogic();
    auto body = parseBlock();

    return std::make_unique<WhileStmt>( std::move(expr), std::move(body) );
}

Stmt Parser::parseFnDecl() {
    consume();  // consume fn
    
    Token nameToken = expect(TokenType::IDENTIFIER, "Expected function name");
    
    auto name = std::get<std::string>(nameToken.value);
    
    expect(TokenType::LEFT_PAREN, "Expected '(' after function name");
    
    std::vector<Parameter> params;
    
    if (!match(TokenType::RIGHT_PAREN)) {
        do {
            Token paramName = expect(TokenType::IDENTIFIER, "Expected parameter name");
            
            expect(TokenType::COLON, "Expected ':' after parameter name");
            
            Token paramType = expect(TokenType::IDENTIFIER, "Expected parameter type");
            
            params.push_back(Parameter{ std::get<std::string>(paramName.value), std::get<std::string>(paramType.value) });
        } while (match(TokenType::COMMA) && (consume(), true));
    }
    
    expect(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
    
    std::string returnType;
    if (match(TokenType::ARROW)) {
        consume();
        
        auto idToken = expect(TokenType::IDENTIFIER, "Expected return type");
        returnType = std::get<std::string>(idToken.value);
    }
    
    Stmt body = parseBlock();

    return std::make_unique<FunctionDecl>(FunctionDecl{
        name, std::move(params), returnType, std::move(body)
    });
}

Stmt Parser::parseImportStmt() {
    consume(); // Consume import

    std::vector<std::string> path;
    path.push_back(std::get<std::string>(expect(TokenType::IDENTIFIER, "Expected module name").value));

    while (match(TokenType::DOT)) {
        consume();

        if (match(TokenType::MUL)) {
            consume();
            expect(TokenType::SEMICOLON, "Expected ';' after import");
            return ImportStmt{ std::move(path), true };
        }

        path.push_back(std::get<std::string>(expect(TokenType::IDENTIFIER, "Expected identifier after .").value));
    }

    expect(TokenType::SEMICOLON, "Expected ';' after import");
    return ImportStmt{ std::move(path), false };
}

Stmt Parser::parseModuleDecl() {
    consume(); // Consume module

    auto mod_name = std::get<std::string>(expect(TokenType::IDENTIFIER, "Expected module name").value);

    expect(TokenType::LEFT_BRACE, "Expected an opening brace");

    std::vector<Stmt> body;

    while (!match(TokenType::RIGHT_BRACE) &&  !isAtEnd()) {
        body.push_back(parseStmt());
    }

    expect(TokenType::RIGHT_BRACE, "Expected a closing brace");

    return std::make_unique<ModuleDecl>(mod_name, std::move(body));
}

std::vector<Stmt> Parser::parse() {
    std::vector<Stmt> statements;

    while (!isAtEnd()) {
        statements.push_back(
            std::move(parseStmt())
        );
    }

    return statements;
}