#pragma once
#include <string_view>
#include <string>
//TokenType
enum class TokenType {
	// OPERATOR
	PLUS, MINUS, DIV, MULTI, MOD, VAL, ERROR, OP_QUESTION,OP_COLON,

	// KEYWORDS
	IF, ELSE, WHILE, FOR, INT, FLOAT, DOUBLE, STRING, CHAR, BOOLEAN, RETURN, CONTINUE,
	BREAK, NILL, VOID,LET, FUNCTION,NEW,

	// LITERALS (Values)
	INT_LIT, FLOAT_LIT, DOUBLE_LIT, STRING_LIT, CHAR_LIT, BOOLEAN_LIT,

	// CONDITIONERS
	EQUALS, ASSIGN, PLUS_EQU, SUB_EQU, DIV_EQU, MULTI_EQU, MOD_EQU, AND, OR, NOT,POINTER,ARRAY,
	GREATER, LESSER, GREATER_EQU, LESSER_EQU, NOT_EQU, POST_INCREMENT, POST_DECREMENT, ARROW,PRE_INCREMENT, PRE_DECREMENT,

	//COMPOUND ASSIGNMENT OPERATORS
	LS_ASSIGN, RS_ASSIGN,BIT_AND_ASSIGN,BIT_XOR_ASSIGN,BIT_OR_ASSIGN,

	//BITWISE OPERATORS
	BIT_OR,BIT_XOR,BIT_AND,L_SHIFT, R_SHIFT,

	//OOPM
	CLASS, MEMBER,PUBLIC, PROTECTED, PRIVATE,

	// BRACKETS
	L_PAREN, L_SQUB, L_CURLB, R_PAREN, R_SQUB, R_CURLB,

	// PUNCTUATIONS
	COMMENT, SEMICOLON, DOT, COMMA,

	//TYPECAST
	TYPECAST,Block, EXPR_STMT,

	// END OF FILE
	EF,
	//I/O
	PRINT,SCAN
};

struct Token {
	TokenType type;
	std::string_view value;
	int line;

	Token(TokenType t,std::string_view v,int l):type(t),value(v),line(l){}
};

//Type 
struct Type; // concept of the TokenType is described here

struct Type {
    TokenType base;
    Type* innerType = nullptr; // check type inside array or pointer
    std::string structName = "";    // name of blueprint or struct
    int arraySize = -1; // for checking  size of array
    // Helper constructors
    Type() : base(TokenType::ERROR) {}
    Type(TokenType b) : base(b) {}
    Type(TokenType b, Type* inner,int size=-1) : base(b), innerType(inner),arraySize(size) {}       //for array
    Type(TokenType b, std::string name) : base(b), structName(name) {}  // for blueprint

    // check if Type object is exactly same;
    bool operator==(const Type& other) const {
        if (base != other.base) return false;
        if (structName != other.structName) return false;
        if (arraySize != other.arraySize) return false;
        // If both have inner types, compare those too (recursively!)
        if (innerType && other.innerType) {
            return *innerType == *other.innerType;
        }
        return innerType == other.innerType; 
    }
    
    bool operator!=(const Type& other) const {
        return !(*this == other);
    }
};