#pragma once

#include <string>
#include <vector>
#include <iostream>

enum TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, 

    MINUS, MINUS_EQUAL, MINUS_MINUS,
    PLUS, PLUS_EQUAL, PLUS_PLUS,
    MUL, MUL_EQUAL,
    DIV, DIV_EQUAL,

    SEMICOLON, SLASH, STAR,

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, NUMBER,

    // Keywords.
    VAR, CONST, IF, ELSE, WHILE, FOR, LOOP, FN,
    BREAK, CONTINUE, TRUE, FALSE,

    EOFILE
};

class Token {
public:
    TokenType type;
    std::string value;
    int line;

    Token(TokenType type, const std::string& value, int line)
    : type(type), value(value), line(line) {}

    void dbPrint();
};