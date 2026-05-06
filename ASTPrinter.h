#pragma once
#include "AST.h"
#include <iostream>
#include <string>

using namespace std;

class ASTPrinter : public Visitor {
private:
    int indent_level = 0;

    // Helper to print spaces for the tree structure
    void printIndent() {
        for (int i = 0; i < indent_level; i++) cout << "  | ";
    }

public:
    void visit(PrintStmt* stmt) override {
        printIndent(); cout << "PrintStmt\n";
        indent_level++;
        stmt->value->accept(this);
        indent_level--;
    }

    void visit(ScanStmt* stmt) override {
        printIndent(); cout << "ScanStmt (target: " << string(stmt->variableName.value) << ")\n";
    }
    void print(ProgramNode* node) {
        cout << "\n=== ABSTRACT SYNTAX TREE ===\n";
        node->accept(this);
        cout << "============================\n\n";
    }

    // --- Statements ---
    void visit(ProgramNode* node) override {
        cout << "Program\n";
        indent_level++;
        for (auto& stmt : node->statements) stmt->accept(this);
        indent_level--;
    }

    void visit(VarDeclStmt* stmt) override {
        printIndent(); 
        cout << "VarDecl (name: " << stmt->name << ")\n";
        indent_level++;
        stmt->initializer->accept(this);
        indent_level--;
    }

    void visit(BlockStmt* stmt) override {
        printIndent(); cout << "Block\n";
        indent_level++;
        for (auto& s : stmt->statements) s->accept(this);
        indent_level--;
    }

    void visit(ExprStmt* stmt) override {
        printIndent(); cout << "ExprStmt\n";
        indent_level++;
        stmt->expr->accept(this);
        indent_level--;
    }

    void visit(IfStmt* stmt) override {
        printIndent(); cout << "IfStmt\n";
        indent_level++;
        printIndent(); cout << "[Condition]\n";
        stmt->condition->accept(this);
        printIndent(); cout << "[Then]\n";
        stmt->thenBranch->accept(this);
        if (stmt->elseBranch) {
            printIndent(); cout << "[Else]\n";
            stmt->elseBranch->accept(this);
        }
        indent_level--;
    }

    void visit(FunctionStmt* stmt) override {
        printIndent(); 
        cout << "FunctionDecl (name: " << stmt->name << ")\n";
        indent_level++;
        stmt->body->accept(this);
        indent_level--;
    }

    void visit(ReturnStmt* stmt) override {
        printIndent(); cout << "ReturnStmt\n";
        indent_level++;
        if (stmt->value) stmt->value->accept(this);
        indent_level--;
    }

    // --- Expressions ---
    void visit(BinaryExpr* expr) override {
        printIndent(); cout << "BinaryExpr (op: " << expr->op.value << ")\n";
        indent_level++;
        expr->left->accept(this);
        expr->right->accept(this);
        indent_level--;
    }

    void visit(LiteralExpr* expr) override {
        printIndent(); cout << "Literal (" << expr->value.value << ")\n";
    }

    void visit(VariableExpr* expr) override {
        printIndent(); cout << "Variable (" << expr->name.value << ")\n";
    }

    void visit(AssignExpr* expr) override {
        printIndent(); cout << "AssignExpr\n";
        indent_level++;
        expr->target->accept(this);
        expr->value->accept(this);
        indent_level--;
    }
    
    void visit(CallExpr* expr) override {
        printIndent(); cout << "CallExpr\n";
        indent_level++;
        expr->callee->accept(this);
        for(auto& arg : expr->args) arg->accept(this);
        indent_level--;
    }
    // Print Blueprint Instantiation
    void visit(InstantiateExpr* expr)override {
        cout << "InstantiateExpr( Blueprint: " << string(expr->className.value) << " )";
    }

    // Unused nodes for now (Stubs to prevent errors)
    void visit(WhileStmt* stmt) override {}
    void visit(ForStmt* stmt) override {}
    void visit(BreakStmt* stmt) override {}
    void visit(ContinueStmt* stmt) override {}
    void visit(BluePrintExpr* stmt) override {}
    void visit(DereferenceExpr* expr) override {}
    void visit(UnaryExpr* expr) override {}
    void visit(MemberExpr* expr) override {}
    void visit(ArrayAccessExpr* expr) override {}
};




