#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Token.h"
#include "AST.h"

using namespace std;

// parser class 
class Parser {
private:
    vector<Token> tokens;
    size_t pos = 0;

    // Helper functions
    Token peek();
    Token consume();
    bool is_at_end();
    void throw_error( string_view msg);
    void consume_if(TokenType type, string_view error_msg);

    ::Type parse_type();
    
    // Parsing methods
    unique_ptr<Stmt> parse_statement();
    Parameter parse_parameter();
    unique_ptr<Stmt> parse_block();
    unique_ptr<Stmt> parse_if();
    unique_ptr<Stmt> parse_while();
    unique_ptr<Stmt> parse_for();
    unique_ptr<Stmt> parse_return();
    unique_ptr<Stmt> parse_break();
    unique_ptr<Stmt> parse_continue();
    unique_ptr<Stmt> parse_variable_declaration();
    unique_ptr<Stmt> parse_function();
    unique_ptr<Stmt> parse_blueprint(); 
    unique_ptr<Stmt> parse_print(); 
    unique_ptr<Stmt> parse_scan(); 
    unique_ptr<Expr> parse_expression(int min_bp = 0);

public:
    // constructor
    Parser(vector<Token> t);
    unique_ptr<ProgramNode> parse_program();
};