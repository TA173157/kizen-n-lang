#pragma once
#include <string_view>
#include <unordered_map> // for checking keywords 
#include <cstddef> // ADD THIS FOR size_t!
#include "Token.h" // We must include this so the Lexer knows what a Token is

class Lexer {
private:
    std::string_view src;
    size_t pos = 0;
    int currentline = 0;
    std::unordered_map<std::string_view, TokenType> keywords;

    // Helper functions
    char peek();
    char peekNext();
    char advance();

public:
    // Constructor
    Lexer(std::string_view v);

    // The main lexing function
    Token getNextToken();
};