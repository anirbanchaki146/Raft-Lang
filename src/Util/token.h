#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <variant>

enum class TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, 

    MINUS, MINUS_EQUAL, MINUS_MINUS,
    PLUS, PLUS_EQUAL, PLUS_PLUS,
    MUL, MUL_EQUAL,
    DIV, DIV_EQUAL,

    SEMICOLON, COLON, ARROW,

    // Logical
    NOT, LOG_AND, LOG_OR, BIT_AND, BIT_OR,

    // Relational
    NOT_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, DOUBLE, INT, BOOL,

    // Keywords.
    LET, VAR, IF, ELSE, WHILE, FOR, LOOP, FN,
    BREAK, CONTINUE, RETURN,

    EOFILE
};

// Every value in Raft is defined as a RaftValue
// This will be extensively used everywhere including the lexer, parser and interpreter
using RaftValue = std::variant<std::monostate, int64_t, double, std::string, bool>;

class Token {
public:
    TokenType type;
    RaftValue value;
    int line;

    Token(TokenType type, RaftValue value = std::monostate{}, int line = 0)
    : type(type), value(std::move(value)), line(line) {}

    void dbPrint() const;
};