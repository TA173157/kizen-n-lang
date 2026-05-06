#include "Lexer.h"
#include <cctype>

// Constructor initializes the source string and the keyword map
Lexer::Lexer(std::string_view v) : src(v) {
    keywords = {
        {"if",        TokenType::IF},
        {"else",      TokenType::ELSE},
        {"new",      TokenType::NEW},
        {"while",     TokenType::WHILE},
        {"return",    TokenType::RETURN},
        {"for",       TokenType::FOR},
        {"break",     TokenType::BREAK},
        {"continue",  TokenType::CONTINUE},
        {"float",     TokenType::FLOAT},
        {"double",    TokenType::DOUBLE},
        {"string",    TokenType::STRING},
        {"char",      TokenType::CHAR},
        {"bool",      TokenType::BOOLEAN},
        {"nill",      TokenType::NILL},
        {"void",      TokenType::VOID},
        {"int",       TokenType::INT},
        {"blueprint", TokenType::CLASS},
        {"function", TokenType::FUNCTION},
        {"private",   TokenType::PRIVATE},
        {"public",    TokenType::PUBLIC},
        {"protected", TokenType::PROTECTED},
        {"let",       TokenType::LET},
        {"print",       TokenType::PRINT},
        {"scan",       TokenType::SCAN},
        {"true",      TokenType::BOOLEAN_LIT},
        {"false",     TokenType::BOOLEAN_LIT}
    };
}

char Lexer::peek() {
    return (pos < src.size()) ? src[pos] : '\0';
}

char Lexer::peekNext() {
    return (pos + 1 < src.size()) ? src[pos + 1] : '\0';
}

char Lexer::advance() {
    return src[pos++];
}

Token Lexer::getNextToken() {
    while (pos < src.size() && isspace(src[pos])) {
        if(src[pos]=='\n')currentline++;
        pos++;
    }
    if (pos >= src.size()) return {TokenType::EF, "",currentline};

    size_t start = pos;
    int state = 0;

    while (true) {
        char c = peek();
        switch (state) {
        case 0:
            if (isalpha(c)) {    // Alphabet
                state = 1;
                advance();
            }
            else if (c == '"') {    // String LITERALS
                state = 3;
                advance();
            }
            else if (c == '\'') {  // Char LITERALS
                state = 4;
                advance();
            }
            else if (isdigit(c)) {  // Number LITERALS
                state = 2;
                advance();
            }
            else if (c == '=') {     // ASSIGN
                advance();
                if (peek() == '=') {
                    advance();
                    return {TokenType::EQUALS, "==",currentline};
                }
                return {TokenType::ASSIGN, "=",currentline};
            }
            else if (c == '*') {      // MULTIPLY
                advance();
                if (peek() == '=') {
                    advance();
                    return {TokenType::MULTI_EQU, "*=",currentline};
                }
                return {TokenType::MULTI, "*",currentline};
            }
            else if (c == '/') {            // Divide
                advance();
                if (peek() == '=') {
                    advance();
                    return {TokenType::DIV_EQU, "/=",currentline};
                }
                if (peek() == '/') {
                    advance();
                    while(peek()!='\n' && peek()!='\0') advance();
                    return getNextToken();
                }
                return {TokenType::DIV, "/",currentline};
            }
            else if (c == '>') {
                advance();                       // Greater
                if (peek() == '=') {
                    advance();
                    return {TokenType::GREATER_EQU, ">=",currentline};
                }
                else if (peek() == '>') {
                    advance();
                    if (peek() == '=') {
                        advance();
                        return {TokenType::RS_ASSIGN, ">>=",currentline};
                    }
                    return {TokenType::R_SHIFT, ">>",currentline};
                }
                return {TokenType::GREATER, ">",currentline};
            }
            else if (c == '<') {                // Lesser
                advance();
                if (peek() == '=') {
                    advance();
                    return {TokenType::LESSER_EQU, "<=",currentline};
                }
                else if (peek() == '<') {
                    advance();
                    if (peek() == '=') {
                        advance();
                        return {TokenType::LS_ASSIGN, "<<=",currentline};
                    }
                    return {TokenType::L_SHIFT, "<<",currentline};
                }
                return {TokenType::LESSER, "<",currentline};
            }
            else if (c == '!') {              // Not
                advance();
                if (peek() == '=') {
                    advance();
                    return {TokenType::NOT_EQU, "!=",currentline};
                }
                return {TokenType::NOT, "!",currentline};
            }
            else if (c == '+') {             // Plus
                advance();
                if (peek() == '=') {
                    advance();
                    return {TokenType::PLUS_EQU, "+=",currentline};
                }
                else if(peek()=='+'){
                    advance();
                    int prevIdex = start-1;
                    while(prevIdex>=0 && isspace(src[prevIdex]))prevIdex--;
                    if(prevIdex>= 0 &&(isalnum(src[prevIdex]) || src[prevIdex]==')'|| src[prevIdex]==']')){
                        return {TokenType::POST_INCREMENT,"++",currentline};
                    }
                    return {TokenType::PRE_INCREMENT,"++",currentline};
                }
                return {TokenType::PLUS, "+",currentline};
            }
            else if (c == '-') {               // Minus
                advance();
                if (peek() == '=') {
                    advance();
                    return {TokenType::SUB_EQU, "-=",currentline};
                }
                else if(peek()=='-'){
                    advance();
                    int prevIdx = start -1;
                    while(prevIdx>=0 && isspace(src[prevIdx])) prevIdx--;
                    if(prevIdx>=0 &&( isalnum(src[prevIdx]) || src[prevIdx]==')'|| src[prevIdx]==']')){
                        return{TokenType::POST_DECREMENT,"--",currentline};
                    }
                    return {TokenType::PRE_DECREMENT,"--",currentline};
                }
                else if (peek() == '>') {
                    advance();
                    return {TokenType::ARROW, "->",currentline};
                }
                return {TokenType::MINUS, "-",currentline};
            }
            else if (c == ';') {               // Semicolon
                advance();
                return {TokenType::SEMICOLON, ";",currentline};
            }
            else if (c == '(') {               // Left parentheses
                advance();
                return {TokenType::L_PAREN, "(",currentline};
            }
            else if (c == ')') {               // Right parentheses
                advance();
                return {TokenType::R_PAREN, ")",currentline};
            }
            else if (c == '{') {               // Left block
                advance();
                return {TokenType::L_CURLB, "{",currentline};
            }
            else if (c == '}') {               // Right block
                advance();
                return {TokenType::R_CURLB, "}",currentline};
            }
            else if (c == ',') {              // Comma
                advance();
                return {TokenType::COMMA, ",",currentline};
            }
            else if (c == '?') {              // Ternary
                advance();
                return {TokenType::OP_QUESTION, "?",currentline};
            }
            else if (c == ':') {              // Colon
                advance();
                return {TokenType::OP_COLON, ":",currentline};
            }
            else if (c == '|') {                // OR
                advance();
                if (peek() == '|') return {TokenType::OR, "||",currentline};
                else if (peek() == '=') return {TokenType::BIT_OR_ASSIGN, "|=",currentline};
                return {TokenType::BIT_OR, "|",currentline};
            }
            else if (c == '^') {                // XOR
                advance();
                if (peek() == '=') return {TokenType::BIT_XOR_ASSIGN, "^=",currentline};
                return {TokenType::BIT_XOR, "^",currentline};
            }
            else if (c == '&') {                // AND
                advance();
                if (peek() == '&') {advance();return {TokenType::AND, "&&",currentline};}
                if (peek() == '=') {advance();return {TokenType::BIT_AND_ASSIGN, "&=",currentline};}
                return {TokenType::BIT_AND, "&",currentline};
            }
            else if (c == '[') {                // L_SQUB
                advance();
                return {TokenType::L_SQUB, "[",currentline};
            }
            else if (c == ']') {                // R_SQUB
                advance();
                return {TokenType::R_SQUB, "]",currentline};
            }
            else if (c == '.') {                // DOT
                advance();
                return {TokenType::DOT, ".",currentline}; 
            }
            else {
                advance();
                return {TokenType::ERROR, "Unknown char",currentline};
            }
            break;

        case 1: {
            while (isalnum(peek())) advance();
            std::string_view text = src.substr(start, pos - start);
            auto it = keywords.find(text);
            if (it != keywords.end()) return {it->second, text,currentline};
            return {TokenType::VAL, text,currentline};
        }
        case 2: {
            bool isFP = false;
            while (isdigit(peek())) advance();
            if (peek() == '.' && isdigit(peekNext())) {
                isFP = true;
                advance();
                while (isdigit(peek())) advance();
            }
            if (peek() == 'f' || peek() == 'F') {
                advance();
                return {TokenType::FLOAT_LIT, src.substr(start, pos - start-1),currentline};
            }
            return {isFP ? TokenType::DOUBLE_LIT : TokenType::INT_LIT, src.substr(start, pos - start),currentline};
        }
        case 3: { // String literals
            while (peek() != '"' && peek() != '\0') advance();
            if (peek() == '\0') return {TokenType::ERROR, "Unterminated string",currentline};
            advance();
            return {TokenType::STRING_LIT, src.substr(start, pos - start),currentline};
        }
        case 4: { // Char literals
            if (peek() != '\'' && peek() != '\0') advance();
            if (peek() == '\'') {
                advance();
                return {TokenType::CHAR_LIT, src.substr(start, pos - start),currentline};
            }
            return {TokenType::ERROR, "Unterminated char",currentline};
        }
        }
    }
}
