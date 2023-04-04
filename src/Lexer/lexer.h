#pragma once

#include <string>
#include <vector>

#include "Util/token.h"

enum class LexerError {
    InvalidToken,
    UnendingString
};

class Lexer {
    std::string source;
    std::vector<Token> tokens;
    size_t index = 0;
    size_t line = 1;

    bool hasError = false;

    void Error(LexerError);

    bool isAtEnd(int);
    char advance();
    char getChar();
    bool match(char);
    char peek();
    char peekNext();

    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlnum(char c);
    bool isKeyword(const std::string&);

    std::string getString();
    std::string getNumber();
    std::string getIdentifier();

    void addToken(TokenType, const std::string&);

public:
    Lexer(const std::string&);
    bool error() const;

    std::vector<Token> scanTokens();
};