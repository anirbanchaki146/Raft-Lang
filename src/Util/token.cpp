#include <string>
#include <vector>
#include <iostream>

#include "Util/token.h"

constexpr std::string_view to_string(TokenType token) {
    switch (token) {
        case TokenType::LEFT_PAREN:    return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN:   return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE:    return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE:   return "RIGHT_BRACE";
        case TokenType::COMMA:         return "COMMA";
        case TokenType::DOT:           return "DOT";

        case TokenType::MINUS:         return "MINUS";
        case TokenType::MINUS_EQUAL:   return "MINUS_EQUAL";
        case TokenType::MINUS_MINUS:   return "MINUS_MINUS";
        case TokenType::PLUS:          return "PLUS";
        case TokenType::PLUS_EQUAL:    return "PLUS_EQUAL";
        case TokenType::PLUS_PLUS:     return "PLUS_PLUS";
        case TokenType::MUL:           return "MUL";
        case TokenType::MUL_EQUAL:     return "MUL_EQUAL";
        case TokenType::DIV:           return "DIV";
        case TokenType::DIV_EQUAL:     return "DIV_EQUAL";

        case TokenType::SEMICOLON:     return "SEMICOLON";
        case TokenType::SLASH:         return "SLASH";
        case TokenType::STAR:          return "STAR";

        case TokenType::BANG:          return "BANG";
        case TokenType::BANG_EQUAL:    return "BANG_EQUAL";
        case TokenType::EQUAL:         return "EQUAL";
        case TokenType::EQUAL_EQUAL:   return "EQUAL_EQUAL";
        case TokenType::GREATER:       return "GREATER";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::LESS:          return "LESS";
        case TokenType::LESS_EQUAL:    return "LESS_EQUAL";

        case TokenType::IDENTIFIER:    return "IDENTIFIER";
        case TokenType::STRING:        return "STRING";
        case TokenType::INT:        return "INT";
        case TokenType::DOUBLE:        return "DOUBLE";

        case TokenType::LET:           return "LET";
        case TokenType::VAR:         return "VAR";
        case TokenType::IF:            return "IF";
        case TokenType::ELSE:          return "ELSE";
        case TokenType::WHILE:         return "WHILE";
        case TokenType::FOR:           return "FOR";
        case TokenType::LOOP:          return "LOOP";
        case TokenType::FN:            return "FN";
        case TokenType::BREAK:         return "BREAK";
        case TokenType::CONTINUE:      return "CONTINUE";
        case TokenType::TRUE:          return "TRUE";
        case TokenType::FALSE:         return "FALSE";

        case TokenType::EOFILE:        return "EOFILE";
        
        default:                       return "UNKNOWN_TOKEN";
    }
}

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };

void Token::dbPrint() const {
    std::visit(overload {
        [this](std::monostate) { std::cout << "{" << to_string(this->type) << ", " << this->line << "}\n"; },
        [this](int val) { std::cout << "{" << to_string(this->type) << ", " << val << ", " << this->line << "}\n"; },
        [this](const std::string& s) { std::cout << "{" << to_string(this->type) << ", " << s << ", " << this->line << "}\n"; }
    }, value);
}