#pragma once
#include <string_view>
#include <vector>
#include <memory>
#include "Token.h" 

using namespace std;

// Forward Declaration of Nodes
struct LiteralExpr;
struct VariableExpr;
struct DereferenceExpr;
struct BinaryExpr;
struct AssignExpr;
struct UnaryExpr;
struct MemberExpr;
struct ArrayAccessExpr;
struct CallExpr;
struct VarDeclStmt;
struct IfStmt;
struct WhileStmt;
struct BlockStmt;
struct ForStmt;
struct ReturnStmt;
struct BreakStmt;
struct ContinueStmt;
struct ProgramNode;
struct ExprStmt;
struct FunctionStmt;
struct BluePrintExpr;
struct InstantiateExpr;
struct PrintStmt;
struct ScanStmt;

//  THE VISITOR INTERFACE
class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void visit(LiteralExpr* expr) = 0;
    virtual void visit(VariableExpr* expr) = 0;
    virtual void visit(DereferenceExpr* expr) = 0;
    virtual void visit(BinaryExpr* expr) = 0;
    virtual void visit(AssignExpr* expr) = 0;
    virtual void visit(UnaryExpr* expr) = 0;
    virtual void visit(MemberExpr* expr) = 0;
    virtual void visit(ArrayAccessExpr* expr) = 0;
    virtual void visit(CallExpr* expr) = 0;
    virtual void visit(VarDeclStmt* stmt) = 0;
    virtual void visit(IfStmt* stmt) = 0;
    virtual void visit(WhileStmt* stmt) = 0;
    virtual void visit(BlockStmt* stmt) = 0;
    virtual void visit(ForStmt* stmt) = 0;
    virtual void visit(ReturnStmt* stmt) = 0;
    virtual void visit(BreakStmt* stmt) = 0;
    virtual void visit(ContinueStmt* stmt) = 0;
    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(ExprStmt* stmt) = 0;
    virtual void visit(FunctionStmt* stmt) = 0;
    virtual void visit(BluePrintExpr* stmt) = 0;
    virtual void visit(struct InstantiateExpr* expr) = 0;
    virtual void visit(PrintStmt* stmt)=0;
    virtual void visit(ScanStmt* stmt)=0;
};

//  BASE NODES
struct Node {
    virtual ~Node() = default;
    virtual void accept(Visitor* visitor) = 0; 
};

struct Expr : public Node {
    virtual ~Expr() = default;
    virtual bool isAssignable() const {
        return false;
    }
};

struct Stmt : public Node {
    virtual ~Stmt() = default;
};

// Nodes with visitor accept method
    // Parameter
    struct Parameter {
        string_view name;
        ::Type type; 
        Parameter(string_view n, ::Type t) : name(n), type(t) {}
    };
    
    // Literal Node
    struct LiteralExpr : public Expr {
        Token value;
        LiteralExpr(Token v) : value(v) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Variable Node
    struct VariableExpr : public Expr {
        Token name;
        VariableExpr(Token v) : name(v) {}
        bool isAssignable() const override {
            return true;
        }
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Print Node
    struct PrintStmt : public Stmt {
        unique_ptr<Expr> value;
        PrintStmt(unique_ptr<Expr> v) : value(std::move(v)) {}
        void accept(Visitor* v) override { v->visit(this); }
    };

    // Scan Node
    struct ScanStmt : public Stmt {
        Token variableName;
        ScanStmt(Token v) : variableName(v) {}
        void accept(Visitor* v) override { v->visit(this); }
    };

    //Dereference Node
    struct DereferenceExpr : public Expr {
        unique_ptr<Expr> operand;
        ::Type type = ::Type(TokenType::POINTER); 
        DereferenceExpr(unique_ptr<Expr> op):operand(std::move(op)) {}
        bool isAssignable() const override {
            return true;
        }
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Binary Node
    struct BinaryExpr : public Expr {
        unique_ptr<Expr> left;
        Token op;
        unique_ptr<Expr> right;
        BinaryExpr(unique_ptr<Expr> l, Token o, unique_ptr<Expr> r) : left(std::move(l)), op(o), right(std::move(r)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Assignment Node
    struct AssignExpr : public Expr {
        unique_ptr<Expr> target;
        Token op;
        unique_ptr<Expr> value;
        AssignExpr(unique_ptr<Expr> t,Token o, unique_ptr<Expr> v) : target(std::move(t)),op(o), value(std::move(v)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Unary Node
    struct UnaryExpr: public Expr {
        Token op;
        unique_ptr<Expr> operand;
        bool isPostfix;
        UnaryExpr(Token o,unique_ptr<Expr>opera,bool t=false):op(o),operand(std::move(opera)),isPostfix(t) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Member Node
    struct MemberExpr : public Expr {
        unique_ptr<Expr> object;
        string_view property;
        Token op;
        string className;
        MemberExpr(unique_ptr<Expr> obj, string_view prop, Token o) : object(std::move(obj)), property(prop), op(o) {}
        bool isAssignable() const override {
            return true;
        }
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Array Node
    struct ArrayAccessExpr : public Expr {
        unique_ptr<Expr> array;
        unique_ptr<Expr> index;
        ArrayAccessExpr(unique_ptr<Expr> arr, unique_ptr<Expr> idx) : array(std::move(arr)), index(std::move(idx)) {}
        bool isAssignable() const override {
            return true;
        }
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // function call Node
    struct CallExpr : public Expr {
        unique_ptr<Expr> callee;
        vector<unique_ptr<Expr>> args;
        CallExpr(unique_ptr<Expr> c, vector<unique_ptr<Expr>> a) : callee(std::move(c)), args(std::move(a)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Variable Declare Node
    struct VarDeclStmt : public Stmt {
        string_view name;
        ::Type type; 
        unique_ptr<Expr> initializer;
        VarDeclStmt(string_view n, ::Type t, unique_ptr<Expr> init) : name(n), type(t), initializer(std::move(init)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // If Node
    struct IfStmt : public Stmt {
        unique_ptr<Expr> condition;
        unique_ptr<Stmt> thenBranch;
        unique_ptr<Stmt> elseBranch;
        TokenType getType() const {
            return TokenType::IF;
        }
        IfStmt(unique_ptr<Expr>cond,unique_ptr<Stmt>then,unique_ptr<Stmt>elseb): condition(std::move(cond)),thenBranch(std::move(then)),elseBranch(std::move(elseb)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // While Node
    struct WhileStmt : public Stmt {
        unique_ptr<Expr> condition;
        unique_ptr<Stmt> body;
        TokenType getType() const {
            return TokenType::WHILE;
        }
        WhileStmt(std::unique_ptr<Expr> cond, unique_ptr<Stmt> b) : condition(std::move(cond)), body(std::move(b)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Block Node
    struct BlockStmt : public Stmt {
        vector<unique_ptr<Stmt>> statements;
        TokenType getType() const  {
            return TokenType::Block;
        }
        BlockStmt(vector<unique_ptr<Stmt>> stmts) : statements(std::move(stmts)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // For Node
    struct ForStmt : public Stmt {
        unique_ptr<Stmt> init;
        unique_ptr<Expr> condition;
        unique_ptr<Expr> increment;
        unique_ptr<Stmt> body;
        TokenType getType() const  {
            return TokenType::FOR;
        }
        ForStmt(unique_ptr<Stmt> in, unique_ptr<Expr> cond, unique_ptr<Expr> inc, unique_ptr<Stmt> b) : init(std::move(in)), condition(std::move(cond)), increment(std::move(inc)), body(std::move(b)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Return Node
    struct ReturnStmt : public Stmt {
        unique_ptr<Expr> value;
        TokenType getType() const  {
            return TokenType::RETURN;
        }
        ReturnStmt(unique_ptr<Expr>v): value(std::move(v)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Break Node
    struct BreakStmt : public Stmt {
        TokenType getType() const  {
            return TokenType::BREAK;
        }
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Continue Node
    struct ContinueStmt : public Stmt {
        TokenType getType() const {
            return TokenType::CONTINUE;
        }
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Program Node
    struct ProgramNode : public Node {
        vector<unique_ptr<Stmt>> statements;
        ProgramNode(vector<unique_ptr<Stmt>> stmts) : statements(std::move(stmts)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Expression to Statement Node
    struct ExprStmt : public Stmt {
        unique_ptr<Expr> expr;
        ExprStmt(unique_ptr<Expr> e) : expr(std::move(e)) {}
        TokenType getType() const  {
            return TokenType::EXPR_STMT;
        }
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Function Declare Node
    struct FunctionStmt : public Stmt {
        string_view name;
        vector<Parameter> params;
        ::Type type; 
        unique_ptr<Stmt> body;
        FunctionStmt(string_view n, vector<Parameter> p, ::Type t, unique_ptr<Stmt> b) : name(n), params(std::move(p)),type(t), body(std::move(b)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Blueprint Node
    struct BluePrintExpr : public Stmt {
        string_view name;
        vector<unique_ptr<Stmt>> feild;
        BluePrintExpr(string_view n, vector<unique_ptr<Stmt>> f) : name(n), feild(std::move(f)) {}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    // Blueprint initializer
    struct InstantiateExpr : public Expr{
        Token className;
        InstantiateExpr(Token n):className(n){}
        void accept(Visitor* v) override {
            v->visit(this);
        }
    };
    
    
    