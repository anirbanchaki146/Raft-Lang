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

// This just means is Keyword or Identifier
TokenType isKeywOrIden(const std::string& id) {
    static std::map<std::string, TokenType> kwdList = {
        {"let", TokenType::LET},
        {"var", TokenType::VAR},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"for", TokenType::FOR},
        {"loop", TokenType::LOOP},
        {"fn", TokenType::FN},
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"true", TokenType::BOOL},
        {"false", TokenType::BOOL},
        {"return", TokenType::RETURN},
        {"import", TokenType::IMPORT}
    };

    auto MapItr = kwdList.find(id);

    if (MapItr == kwdList.end())
        return TokenType::IDENTIFIER;

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

NumberType Lexer::getNumber() {
    std::string num_string;
    bool dot = false;
    
    for (;;) {
        num_string += getChar();
         
        if (peek() == '.') {
            if (dot || !isDigit(peekNext())) break;

            advance();
            num_string += '.';
            dot = true;
        }

        if (!isDigit(peek())) {
            break;
        }
        
        advance();
    }
    if (dot) return std::stod(num_string);

    // This is important to prevent unintentional type casts
    return static_cast<int64_t>(std::stoll(num_string)); 
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

void Lexer::addToken(TokenType type, RaftValue value = std::monostate{}) {
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

            case '(': addToken(TokenType::LEFT_PAREN); break;
            case ')': addToken(TokenType::RIGHT_PAREN); break;
            case '{': addToken(TokenType::LEFT_BRACE); break;
            case '}': addToken(TokenType::RIGHT_BRACE); break;
            case ',': addToken(TokenType::COMMA); break;
            case '.': addToken(TokenType::DOT); break;
            case ';': addToken(TokenType::SEMICOLON); break;

            case '+': 
                if (match('=')) {
                    addToken(TokenType::PLUS_EQUAL);
                    break;
                }

                if (match('+')) {
                    addToken(TokenType::PLUS_PLUS);
                    break;
                }
                
                addToken(TokenType::PLUS);
                break;

            case '-': 
                if (match('=')) {
                    addToken(TokenType::MINUS_EQUAL);
                    break;
                }

                if (match('-')) {
                    addToken(TokenType::MINUS_MINUS);
                    break;
                }

                if (match('>')) {
                    addToken(TokenType::ARROW);
                    break;
                }
                
                addToken(TokenType::MINUS);
                break;

            case '*': addToken(match('=')? TokenType::MUL_EQUAL : TokenType::MUL); break;
            case '/':
                if (match('/')) {
                    while (peek() != '\n' && !isAtEnd(index)) advance();
                }
                else {
                    addToken(TokenType::DIV);
                }
                break;

            case '<': addToken(match('=')? TokenType::LESS_EQUAL : TokenType::LESS); break;
            case '>': addToken(match('=')? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
            case '=': addToken(match('=')? TokenType::EQUAL_EQUAL: TokenType::EQUAL); break;
            
            case '&': addToken(match('&')? TokenType::LOG_AND : TokenType::BIT_AND); break;
            case '|': addToken(match('|')? TokenType::LOG_OR : TokenType::BIT_OR); break;
            case '!': addToken(match('=')? TokenType::NOT_EQUAL: TokenType::NOT); break;

            case ':': addToken(TokenType::COLON); break;

            case '\"': addToken(TokenType::STRING, getString()); break;

            default:
                if (isDigit(c)) {
                    auto num = getNumber();

                    if (std::holds_alternative<double>(num)) {
                        addToken(TokenType::DOUBLE, std::get<double>(num));
                        break;
                    }

                    addToken(TokenType::INT, std::get<int64_t>(num));
                    break;
                }
                
                if (isAlpha(c)) {
                    std::string id = getIdentifier();

                    auto type = isKeywOrIden(id);

                    if (type == TokenType::BOOL) {
                        bool val = (id == "true")? true : false;

                        addToken(TokenType::BOOL, val);
                        break;
                    }

                    if (type == TokenType::IDENTIFIER) {
                        addToken(TokenType::IDENTIFIER, id);
                        break;
                    }

                    // Adding a string to a token acting as a keyword is a waste of space
                    addToken(type);
                    break;
                }

                Error(LexerError::InvalidToken);
        }

        advance();
    }

    addToken(TokenType::EOFILE);
    return tokens;
}