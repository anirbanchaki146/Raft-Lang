#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <map>

#include "Util/token.h"
#include "Lexer/lexer.h"

Lexer::Lexer(const std::string& source) : source(source)
{}

bool Lexer::error() const {
    return hasError;
}

void Lexer::Error(LexerError err) {
    hasError = true;

    switch (err) {
        case LexerError::InvalidToken:
            std::cout << "[Error] Invalid Token\n";
            break;
            
        case LexerError::UnendingString:
            std::cout << "[Error] Unterminating String\n";
            break;
        
        default:
            break;
    }
}

bool Lexer::isAtEnd(int index) {
    return index >= source.length();
}

char Lexer::advance() {
    return source[index++];
}

char Lexer::getChar() {
    if (isAtEnd(index)) return '\0';

    return source[index];
}

bool Lexer::match(char expected) {
    if (isAtEnd(index)) return false;
    if (expected != peek()) return false;

    advance();
    return true;
}

char Lexer::peek() {
    if (isAtEnd(index + 1)) return '\0';

    return source[index + 1];
}

char Lexer::peekNext() {
    if (isAtEnd(index + 2)) return '\0';

    return source[index + 2];
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) {
    return c >= 'a' && c <= 'z'
    || c >= 'A' && c <= 'Z' || c == '_';
}

bool Lexer::isAlnum(char c) {
    return isAlpha(c) || isDigit(c);
}

TokenType getKeyword(const std::string& id) {
    static std::map<std::string, TokenType> kwdList = {
        {"var", VAR},
        {"const", CONST},
        {"if", IF},
        {"else", ELSE},
        {"while", WHILE},
        {"for", FOR},
        {"loop", LOOP},
        {"fn", FN},
        {"break", BREAK},
        {"continue", CONTINUE},
        {"true", TRUE},
        {"false", FALSE}
    };

    auto MapItr = kwdList.find(id);

    if (MapItr == kwdList.end())
        return IDENTIFIER;

    return MapItr->second;
}

std::string Lexer::getString() {
    std::string output;
    bool ending_quote = false; // To detect unending string literals

    for (;;) {
        // Consume the `"`
        advance();

        if (isAtEnd(index)) break;

        char c = getChar();

        if (c == '\"') {
            ending_quote = true;
            break;
        }

        output += c;
    }

    if (!ending_quote) Error(LexerError::UnendingString);

    return output;
}

std::string Lexer::getNumber() {
    std::string output;
    bool dot;
    
    for (;;) {
        output += getChar();
         
        if (peek() == '.') {
            if (dot || !isDigit(peekNext())) break;

            advance();
            output += '.';
            dot = true;
        }

        if (!isDigit(peek())) {
            break;
        }
        
        advance();
    }

    return output;
}

std::string Lexer::getIdentifier() {
    std::string output;

    for (;;) {
        if (!isAlnum(peek())) {
            output += getChar();
            break;
        }
        
        output += getChar();
        advance();
    }

    return output;
}

void Lexer::addToken(TokenType type, const std::string& value = "") {
    tokens.push_back(Token(type, value, line));
}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd(index)) {
        if (hasError) break;

        char c = getChar();

        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                break;

            case '\n':
                line++;
                break;

            case '(': addToken(LEFT_PAREN); break;
            case ')': addToken(RIGHT_PAREN); break;
            case '{': addToken(LEFT_BRACE); break;
            case '}': addToken(RIGHT_BRACE); break;
            case ',': addToken(COMMA); break;
            case '.': addToken(DOT); break;
            case ';': addToken(SEMICOLON); break;

            case '+': 
                if (match('=')) {
                    addToken(PLUS_EQUAL);
                    break;
                }

                if (match('+')) {
                    addToken(PLUS_PLUS);
                    break;
                }
                
                addToken(PLUS);
                break;

            case '-': 
                if (match('=')) {
                    addToken(MINUS_EQUAL);
                    break;
                }

                if (match('-')) {
                    addToken(MINUS_MINUS);
                    break;
                }
                
                addToken(MINUS);
                break;

            case '*': addToken(match('=')? MUL_EQUAL : MUL); break;
            case '/':
                if (match('/')) {
                    while (peek() != '\n' && !isAtEnd(index)) advance();
                }
                else {
                    addToken(DIV);
                }
                break;

            case '<': addToken(match('=')? LESS_EQUAL : LESS); break;
            case '>': addToken(match('=')? GREATER_EQUAL : GREATER); break;
            case '=': addToken(match('=')? EQUAL_EQUAL: EQUAL); break;

            case '\"': addToken(STRING, getString()); break;

            default:
                if (isDigit(c)) {
                    addToken(NUMBER, getNumber());
                    break;
                }
                
                if (isAlpha(c)) {
                    std::string id = getIdentifier();
                    addToken(getKeyword(id), id);
                    
                    break;
                }

                Error(LexerError::InvalidToken);
        }

        advance();
    }

    addToken(EOFILE);
    return tokens;
}