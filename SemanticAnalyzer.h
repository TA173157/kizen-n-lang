#pragma once
#include <unordered_map>
#include <vector>
#include <string_view>
#include <string>
#include "AST.h"
#include "Token.h"

using namespace std;

// Keeps track of function return types and parameters
struct FunctionType {
    ::Type returnType;
    vector<::Type> paramTypes;
};

// What we store in the Symbol Table
struct Symbol {
    string_view name;
    ::Type type;
    bool isFunction;
    ::FunctionType funcType;
    unordered_map<string, ::Type> blueprintFields;
};

class SemanticAnalyzer : public Visitor {
private:
    // The Symbol Table Stack (each {} block adds a new map to the vector)
    vector<unordered_map<string_view, Symbol>> scopes;
    
    // Because Visitors return void, we store the evaluated type of expressions here
    ::Type current_type ;
    ::Type current_function_return_type = ::Type(TokenType::ERROR);
    int loop_depth =0;

public:
    SemanticAnalyzer();

    // Scope Management
    void enter_scope();
    void exit_scope();
    void declare(string_view name, ::Type type, bool isFunc = false, ::FunctionType fType = {});
    Symbol* lookup(string_view name);
    void throw_error(const string& msg);

    // The main entry point
    void analyze(ProgramNode* program);

    // --- The Visitor Implementation ---
        // Expression AST nodes 
            void visit(LiteralExpr* expr) override;
            void visit(VariableExpr* expr) override;
            void visit(DereferenceExpr* expr) override;
            void visit(BinaryExpr* expr) override;
            void visit(AssignExpr* expr) override;
            void visit(UnaryExpr* expr) override;
            void visit(MemberExpr* expr) override;
            void visit(ArrayAccessExpr* expr) override;
            void visit(CallExpr* expr) override;
            void visit(InstantiateExpr* expr) override;
            
        // Statement AST nodes 
            void visit(VarDeclStmt* stmt) override;
            void visit(IfStmt* stmt) override;
            void visit(WhileStmt* stmt) override;
            void visit(BlockStmt* stmt) override;
            void visit(ForStmt* stmt) override;
            void visit(ReturnStmt* stmt) override;
            void visit(BreakStmt* stmt) override;
            void visit(ContinueStmt* stmt) override;
            void visit(ProgramNode* node) override;
            void visit(ExprStmt* stmt) override;
            void visit(FunctionStmt* stmt) override;
            void visit(BluePrintExpr* stmt) override;
            void visit(PrintStmt* stmt) override;
            void visit(ScanStmt * stmt) override;
};