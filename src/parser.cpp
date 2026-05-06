#include "parser.h"
#include <iostream>
#include <string_view>
#include <stdexcept>
#include <unordered_set>

// assign set 
static const std::unordered_set<TokenType> assign = {
    TokenType::ASSIGN, TokenType::PLUS_EQU, TokenType::SUB_EQU,
    TokenType::DIV_EQU, TokenType::MULTI_EQU, TokenType::MOD_EQU,
    TokenType::LS_ASSIGN, TokenType::RS_ASSIGN, TokenType::BIT_AND_ASSIGN,
    TokenType::BIT_XOR_ASSIGN, TokenType::BIT_OR_ASSIGN
};

// precendence table for operators
static pair<int, int> get_binding_power(TokenType op) {
    // if (op == TokenType::COMMA) return {1, 2};
    if (assign.find(op) != assign.end()) return {4,3};
    if (op == TokenType::OP_QUESTION ||op == TokenType::OP_COLON) return {6,5};
    if (op == TokenType::OR) return {7,8};
    if (op == TokenType::AND) return {9,10};
    if (op == TokenType::BIT_OR) return {11,12};
    if (op == TokenType::BIT_XOR) return {13,14};
    if (op == TokenType::BIT_AND) return {15,16};
    if (op ==TokenType::EQUALS||op == TokenType::NOT_EQU) return {17,18};
    if (op == TokenType::LESSER_EQU||op == TokenType::GREATER_EQU||op == TokenType::GREATER||op == TokenType::LESSER) return {19,20};
    if (op == TokenType::L_SHIFT||op== TokenType::R_SHIFT) return {21,22};
    if (op == TokenType::PLUS||op == TokenType::MINUS) return {23,24};
    if (op == TokenType::MULTI||op == TokenType::DIV||op == TokenType::MOD) return {25,26};
    if (op == TokenType::TYPECAST) return {28,27};
    if (op == TokenType::PRE_INCREMENT||op == TokenType::PRE_DECREMENT) return {30,29};
    if (op == TokenType::POST_INCREMENT||op == TokenType::POST_DECREMENT) return {31,32};
    return {0, 0};
}

// Constructor
Parser::Parser(vector<Token> t) : tokens(t), pos(0) {}
//------------------------------------------------------------------------------------------------------------------------------------------------------
// HELPER  FUNCTIONS  LOGIC

Token Parser::peek() {
    return (pos < tokens.size()) ? tokens[pos] : Token{TokenType::EF, "",tokens[pos-1].line};
}
Token Parser::consume() {
    if (pos >= tokens.size()) throw_error("Unexpected end of input");
    return tokens[pos++];
}
bool Parser::is_at_end() {
    return peek().type == TokenType::EF;
}
void Parser::throw_error(string_view msg) {
    throw runtime_error("Parse Error: " + string(msg));
}
void Parser::consume_if(TokenType type, string_view error_msg) {
    if (peek().type == type) {
        consume();
    } else {
        Token badToken = peek();

        throw_error("[Line " + std::to_string(badToken.line) + "] Parse Error: " + string(error_msg) + 
        " (Found '" + string(badToken.value) + "')");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
// recursive type Parse
Type Parser::parse_type(){
    Token t = consume();
    Type currentType;
    
    //base Type
    if (t.type == TokenType::INT || t.type == TokenType::FLOAT || t.type == TokenType::DOUBLE ||
        t.type == TokenType::CHAR || t.type == TokenType::STRING || t.type == TokenType::BOOLEAN ||
        t.type == TokenType::VOID) {
        currentType = Type(t.type);
    }
    else if (t.type == TokenType::VAL) { 
        currentType = Type(TokenType::CLASS, string(t.value)); // this is for blueprint  
    } 
    else {
        throw_error("Expected a valid type (e.g., int, float, or a Blueprint name)");
    }
    
    while (!is_at_end()) {
        if (peek().type == TokenType::MULTI) { 
            consume();
            currentType = Type(TokenType::POINTER, new Type(currentType));
        } else if (peek().type == TokenType::L_SQUB) { 
            consume();
            int size = -1;
            if (peek().type == TokenType::INT_LIT) {
                Token sizeToken = consume();
                size = std::stoi(std::string(sizeToken.value)); 
                if (size <= 0) {
                    throw_error("Array size must be greater than 0!");
                }
            }
            consume_if(TokenType::R_SQUB, "Expected ']' after '[' to define an array type");
            currentType = Type(TokenType::ARRAY, new Type(currentType),size);
        } else {
            break; 
        }
    }

    return currentType;
}

// parse_statement
unique_ptr<Stmt> Parser::parse_statement() {
    Token t = peek();
    if(t.type==TokenType::RETURN) return parse_return();
    if(t.type==TokenType::BREAK) return parse_break();
    if(t.type==TokenType::CONTINUE) return parse_continue();
    if(t.type==TokenType::IF)return parse_if();
    if(t.type==TokenType::WHILE)return parse_while();
    if(t.type==TokenType::FOR)return parse_for();
    if(t.type==TokenType::L_CURLB)return parse_block();
    if(t.type==TokenType::ELSE)throw_error("Unexpected 'else' without 'if'");
    if(t.type==TokenType::LET)return parse_variable_declaration();
    if(t.type==TokenType::FUNCTION)return parse_function();
    if(t.type==TokenType::CLASS)return parse_blueprint();
    if(t.type==TokenType::PRINT)return parse_print();
    if(t.type==TokenType::SCAN)return parse_scan();
    
    auto expr = parse_expression(0);
    consume_if(TokenType::SEMICOLON,"Expected ';' here"); 
    return make_unique<ExprStmt>(std::move(expr));
}

// parse_parameter
Parameter Parser::parse_parameter() {
    if(peek().type!=TokenType::LET)throw_error("Expected LET here");
    consume();
    if(peek().type!=TokenType::VAL)throw_error("Expected VAL here");
    string_view name = consume().value;
    consume_if(TokenType::OP_COLON, "Expected ':' here");
    
    Type type = parse_type();
    return Parameter(name,type);
}

// program_parse
unique_ptr<ProgramNode> Parser::parse_program() {
    vector<unique_ptr<Stmt>> stmts;
    while (!is_at_end()) {
        stmts.push_back(parse_statement());
    }
    return make_unique<ProgramNode>(std::move(stmts));
}

// block_parse
unique_ptr<Stmt> Parser::parse_block() {
    consume_if(TokenType::L_CURLB,"Expected '{' here");
    vector<unique_ptr<Stmt>> statements;
    while (peek().type != TokenType::R_CURLB && !is_at_end()) {
        statements.push_back(parse_statement());
    }
    consume_if(TokenType::R_CURLB,"Missing '}' here");
    return make_unique<BlockStmt>(std::move(statements));
}

// if_parse
unique_ptr<Stmt> Parser::parse_if() {
    consume(); 
    consume_if(TokenType::L_PAREN," 'if' without '(' ");
    auto condition = parse_expression(0);
    consume_if(TokenType::R_PAREN," 'if' without ')' ");
    auto thenBranch = parse_statement();
    unique_ptr<Stmt> elseBranch = nullptr;
    if (peek().type==TokenType::ELSE) {
        consume();
        elseBranch = parse_statement();
    }
    return make_unique<IfStmt>(std::move(condition),std::move(thenBranch),std::move(elseBranch));
}

// while_parse
unique_ptr<Stmt> Parser::parse_while() {
    consume(); 
    consume_if(TokenType::L_PAREN," 'while' without '(' ");
    auto condition = parse_expression(0);
    consume_if(TokenType::R_PAREN," 'while' without ')' ");
    auto body = parse_statement();
    return make_unique<WhileStmt>(std::move(condition),std::move(body));
}

// for_parse
unique_ptr<Stmt> Parser::parse_for() {
    consume(); 
    consume_if(TokenType::L_PAREN," 'for' without '(' ");
    unique_ptr<Stmt> init = parse_statement();
    auto condition = parse_expression(0);
    consume_if(TokenType::SEMICOLON," 'for' without ';' ");
    auto increment = parse_expression(0);
    consume_if(TokenType::R_PAREN," 'while' without ')' ");
    auto body = parse_statement();
    return make_unique<ForStmt>(std::move(init), std::move(condition), std::move(increment), std::move(body));
}

// return parse
unique_ptr<Stmt> Parser::parse_return() {
    consume();
    unique_ptr<Expr> value = nullptr;
    if (peek().type != TokenType::SEMICOLON) {
        value = parse_expression(0);
    } 
    consume_if(TokenType::SEMICOLON, "Expected ';'after return");
    return make_unique<ReturnStmt>(std::move(value));
}

// break_parse
unique_ptr<Stmt> Parser::parse_break() {
    consume();
    consume_if(TokenType::SEMICOLON," 'for' without ';' ");
    return make_unique<BreakStmt>();
}

// continue_parse
unique_ptr<Stmt> Parser::parse_continue() {
    consume();
    consume_if(TokenType::SEMICOLON," 'for' without ';' ");
    return make_unique<ContinueStmt>();
}

// variable declare parse
unique_ptr<Stmt> Parser::parse_variable_declaration() {
    consume(); 
    Token nameToken = consume();
    if (nameToken.type != TokenType::VAL) throw_error("Expected identifier name");
    consume_if(TokenType::OP_COLON, "Expected ':' here");
    Type type = parse_type();
    consume_if(TokenType::ASSIGN,"Expected '='");
    auto initializer = parse_expression(0);
    consume_if(TokenType::SEMICOLON," 'for' without ';' ");
    return make_unique<VarDeclStmt>(nameToken.value, type, std::move(initializer));
}

//parse funciton
unique_ptr<Stmt> Parser::parse_function() {
    consume();
    Token name = consume();
    consume_if(TokenType::L_PAREN," Expected '(' here ");
    vector<Parameter> args;
    if (peek().type != TokenType::R_PAREN) {
        while (true) {
            args.push_back(parse_parameter());
            if (peek().type == TokenType::COMMA) {
                consume();
            } else if (peek().type == TokenType::R_PAREN) {
                break;
            } else {
                throw_error("Expected ',' or ')' in argument list");
            }
        }
    }
    consume_if(TokenType::R_PAREN,"Expected ')' here");
    consume_if(TokenType::OP_COLON,"Expected ':' here");
    Type func_type = parse_type();
    if(peek().type!=TokenType::L_CURLB)throw_error("Expected '{' here");
    auto body = parse_block();
    return make_unique<FunctionStmt>(name.value,std::move(args),func_type,std::move(body)); 
}

//parse blueprint
unique_ptr<Stmt> Parser::parse_blueprint() {
    consume(); // eat blueprint 
    string_view name;
    if(peek().type==TokenType::VAL) name = consume().value;
    else throw_error("Expected An identifier here");
    consume_if(TokenType::L_CURLB,"Expected '{' here");
    
    vector<unique_ptr<Stmt>> feild;
    while(peek().type!=TokenType::R_CURLB&& peek().type!=TokenType::EF){
        auto stmt = parse_statement();
        feild.push_back(std::move(stmt));
    }
    consume_if(TokenType::R_CURLB,"Expected '}' here");
    return make_unique<BluePrintExpr>(name, std::move(feild));
}

//print parse
unique_ptr<Stmt> Parser::parse_print() {
    consume(); // Eat 'print'
    consume_if(TokenType::L_PAREN, "Expected '(' after print");
    auto expr = parse_expression(0);
    consume_if(TokenType::R_PAREN, "Expected ')' after print expression");
    consume_if(TokenType::SEMICOLON, "Expected ';' after print statement");
    return make_unique<PrintStmt>(std::move(expr));
}

// scan parse
unique_ptr<Stmt> Parser::parse_scan() {
    consume(); // Eat 'scan'
    consume_if(TokenType::L_PAREN, "Expected '(' after scan");
    Token varName = consume();
    if (varName.type != TokenType::VAL) throw_error("Expected variable name inside scan()");
    consume_if(TokenType::R_PAREN, "Expected ')' after scan variable");
    consume_if(TokenType::SEMICOLON, "Expected ';' after scan statement");
    return make_unique<ScanStmt>(varName);
}

// parse expression
unique_ptr<Expr> Parser::parse_expression(int min_bp) {
    Token t = consume();
    unique_ptr<Expr> lhs;

    if(t.type==TokenType::MULTI) { 
        auto rhs = parse_expression(29);
        lhs = make_unique<DereferenceExpr>(std::move(rhs));
    }
    else if(t.type == TokenType::MINUS ||t.type == TokenType::NOT ||t.type == TokenType::PRE_INCREMENT ||t.type == TokenType::PRE_DECREMENT) {
        auto [l,r]= get_binding_power(t.type);
        auto rhs = parse_expression(r);
        lhs = make_unique<UnaryExpr>(t,std::move(rhs),false);
    }
    else if (t.type == TokenType::INT_LIT || t.type == TokenType::DOUBLE_LIT || t.type == TokenType::FLOAT_LIT || t.type == TokenType::CHAR_LIT || t.type == TokenType::STRING_LIT || t.type == TokenType::BOOLEAN_LIT) {
        lhs = make_unique<LiteralExpr>(t);
    }
    else if (t.type == TokenType::L_PAREN) {
        lhs = parse_expression(0);
        if (peek().type != TokenType::R_PAREN) throw_error("Expected )");
        consume();
    }
    else if (t.type == TokenType::VAL) {
        lhs = make_unique<VariableExpr>(t);
    }
    else if(t.type == TokenType::NEW){
        Token className = consume();
        if(className.type != TokenType::VAL){
            throw_error("Expected Blueprint name after 'new'");
        }
        consume_if(TokenType::L_PAREN, "Expected '(' after Blueprint name");
        consume_if(TokenType::R_PAREN, "Expected ')' to complete instantiation");
        
        lhs = make_unique<InstantiateExpr>(className);
    }
    else {
        throw_error("Unexpected token: " + string(t.value) + ". Expected an expression.");
    }
    
    while (true) {
        Token op = peek();
        
        if (op.type == TokenType::POST_INCREMENT || op.type == TokenType::POST_DECREMENT) {
            consume();
            lhs = make_unique<UnaryExpr>(op, std::move(lhs), true);
            continue;
        }
        if (peek().type == TokenType::DOT || peek().type==TokenType::ARROW) {
            Token op = consume();
            Token name = consume();
            if (name.type != TokenType::VAL) throw_error("Expected property name after '.'");
            lhs = make_unique<MemberExpr>(std::move(lhs), name.value, op);
            continue;
        }
        if (peek().type == TokenType::L_SQUB) {
            consume();
            auto index = parse_expression(0);
            consume_if(TokenType::R_SQUB, "Expected ']'");
            lhs = make_unique<ArrayAccessExpr>(std::move(lhs), std::move(index));
            continue;
        }
        if (peek().type == TokenType::L_PAREN) {
            consume(); 
            vector<unique_ptr<Expr>> args;
            if (peek().type != TokenType::R_PAREN) {
                while (true) {
                    args.push_back(parse_expression(0));
                    if (peek().type == TokenType::COMMA) {
                        consume();
                    } else {
                        break;
                    }
                }
            }
            consume_if(TokenType::R_PAREN, "Expected ')'");
            lhs = make_unique<CallExpr>(std::move(lhs), std::move(args));
            continue;
        }
        
        if (op.type == TokenType::R_PAREN || op.type == TokenType::EF || op.type ==TokenType::SEMICOLON || op.type == TokenType::R_CURLB || op.type == TokenType::R_SQUB||op.type == TokenType::COMMA) break;
        
        auto [l_bp, r_bp] = get_binding_power(op.type);
        if (l_bp == 0 || l_bp < min_bp) break;
        consume(); 
        
        auto it = assign.find(op.type);
        if (it != assign.end()) {
            if (!lhs->isAssignable()) throw_error("Left-hand side is not assignable");
            auto rhs = parse_expression(r_bp);
            lhs = make_unique<AssignExpr>(std::move(lhs), op,std::move(rhs));
            continue;
        }
        
        unique_ptr<Expr> rhs = parse_expression(r_bp);
        lhs = make_unique<BinaryExpr>(std::move(lhs), op, std::move(rhs));
    }
    return lhs;
}