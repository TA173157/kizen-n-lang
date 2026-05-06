#pragma once
#include "AST.h"  
#include "Token.h"  
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <map>
#include <string>
using namespace std;
using namespace llvm;

class IRGenerator: public Visitor{
    public:
    // 3 main llvm header files
    unique_ptr<LLVMContext>context;
    unique_ptr<Module>module;
    unique_ptr<IRBuilder<>>builder;

    // Symbol table
    map<string,AllocaInst*> NamedValues;
    // store the llvm type of stuct
    map<string,llvm::StructType*>StructTypes;
    // store the index of each field
    map<string,map<string,int>>StructFields;
    
    // for break statements
    vector<BasicBlock*> LoopIncBlocks;  // Where to go for 'continue'
    vector<BasicBlock*> LoopExitBlocks; // Where to go for 'break
    // temp val for passing values up the tree
    Value* currentVal = nullptr;

    //constructor
    IRGenerator();
    
    Value* generate(Node* node);
    llvm::Type* getLLVMType(TokenType kind);
    void generateObjectFile(const std::string& outFilename);
    // overriding all the visit methods
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
            void visit(ScanStmt* stmt) override;
};