#include "SemanticAnalyzer.h"
#include <iostream>
#include <vector>
#include <string_view>
#include <stdexcept>
using namespace std;

// constructor 
    SemanticAnalyzer::SemanticAnalyzer(){
        enter_scope();
    }

// ------ five major functions of SemanticAnalyzer------
    // error declare
        void SemanticAnalyzer::throw_error(const string& msg) {
        	throw runtime_error("Semantic Error: "+msg);
        }
    // Enter scope
        void SemanticAnalyzer::enter_scope() {
        	scopes.push_back({});
        }
    // Exit scope
        void SemanticAnalyzer::exit_scope() {
        	if (scopes.size() <= 1) throw_error("Cannot exit the global scope!");
        	scopes.pop_back();
        }
    // lookup
        Symbol* SemanticAnalyzer::lookup(string_view name) {
        	for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        		auto found = it->find(name);
        		if (found != it->end()) {
        			return &(found->second); // Return memory address of the symbol
        		}
        	}
        	throw runtime_error("Error: variable '" + string(name) + "' doesn't exist");
        }
    // declare
        void SemanticAnalyzer::declare(string_view name, Type type, bool isFunc, FunctionType fType) {
        	auto& currentScope = scopes.back();
        	auto found = currentScope.find(name);
        	if(found!=currentScope.end())throw runtime_error("Error: variable '" + string(name) + "' already exists");
        	currentScope[name]= Symbol{name,type,isFunc,fType};
        }

// --------------- Main Entry Point--------------

    // program node analyze function
        void SemanticAnalyzer::analyze(ProgramNode* program) {
        	program->accept(this);
        }

    // LiteralExpr visit
        void SemanticAnalyzer::visit(LiteralExpr* expr) {
    switch(expr->value.type) {
        case TokenType::INT_LIT:     current_type = Type(TokenType::INT); break;
        case TokenType::FLOAT_LIT:   current_type = Type(TokenType::FLOAT); break;
        case TokenType::DOUBLE_LIT:  current_type = Type(TokenType::DOUBLE); break;
        case TokenType::STRING_LIT:  current_type = Type(TokenType::STRING); break;
        case TokenType::CHAR_LIT:    current_type = Type(TokenType::CHAR); break;
        case TokenType::BOOLEAN_LIT: current_type = Type(TokenType::BOOLEAN); break;
        default: throw_error("Unknown literal type encountered.");
    }
}

    // VariableExpr visit
        void SemanticAnalyzer::visit(VariableExpr* expr) {
        	Symbol* symbol = lookup(expr->name.value);
        	current_type   = symbol->type;
        }
        
   // BinaryExpr visit
void SemanticAnalyzer::visit(BinaryExpr* expr) {
    expr->left->accept(this);
    Type leftType = current_type; // Upgraded to Type

    expr->right->accept(this);
    Type rightType = current_type; // Upgraded to Type

    if (leftType != rightType) { // Uses our custom == and != operators!
        throw_error("Type mismatch: Cannot perform operation between different types.");
    }
    
    TokenType op = expr->op.type;

    // math operator
    if (op == TokenType::PLUS || op == TokenType::MINUS ||
        op == TokenType::MULTI || op == TokenType::DIV || op == TokenType::MOD) {
        
        // check for booleans or strings
        if (leftType.base != TokenType::INT && leftType.base != TokenType::FLOAT && leftType.base != TokenType::DOUBLE) {
            throw_error("Math operations are only allowed on numbers.");
        }
        current_type = leftType;
    }
    // comparsion types gives boolean 
    else if (op == TokenType::GREATER || op == TokenType::LESSER ||
             op == TokenType::GREATER_EQU || op == TokenType::LESSER_EQU ||
             op == TokenType::EQUALS || op == TokenType::NOT_EQU) {
        current_type = Type(TokenType::BOOLEAN);
    }
    // both sides MUST be booleans for logical operators
    else if (op == TokenType::AND || op == TokenType::OR) {
        if (leftType.base != TokenType::BOOLEAN) {
            throw_error("Logical operators (&&, ||) require boolean expressions.");
        }
        current_type = Type(TokenType::BOOLEAN);
    }
    else {
        throw_error("Unrecognized binary operator in semantic analysis.");
    }
}
    
    // ProgramNode visit    
        void SemanticAnalyzer::visit(ProgramNode* node){
            for(auto& stmt: node->statements){
                stmt->accept(this);
            }
        } 
        
    // ExprStmt visit
        void SemanticAnalyzer::visit(ExprStmt* stmt){
            stmt->expr->accept(this);
        }
        
    // Block visit    
        void SemanticAnalyzer::visit(BlockStmt* stmt){
            enter_scope();
            for(auto& s:stmt->statements){
                s->accept(this);
            }
            exit_scope();
        }
        
    // variable Declare visit    
       void SemanticAnalyzer::visit(VarDeclStmt* stmt) {
            stmt->initializer->accept(this);
            Type valueType = current_type;

            if (stmt->type != valueType) {
                throw_error("Type mismatch in variable declaration: '" + string(stmt->name) + "'");
            }
            declare(stmt->name, stmt->type);
        }
    
    // If visit        
            void SemanticAnalyzer::visit(IfStmt* stmt) {
            stmt->condition->accept(this);
            if (current_type.base != TokenType::BOOLEAN) {
                throw_error("If condition must evaluate to a boolean type.");
            }
            stmt->thenBranch->accept(this);
            if (stmt->elseBranch) {
                stmt->elseBranch->accept(this);
            }
        }
        
    // while visit    
        void SemanticAnalyzer::visit(WhileStmt* stmt) {
            stmt->condition->accept(this);
            if (current_type.base != TokenType::BOOLEAN) {
                throw_error("While condition must evaluate to a boolean type.");
            }
            loop_depth++;
            stmt->body->accept(this);
            loop_depth--;
        }
    
    // for visit  
        void SemanticAnalyzer::visit(ForStmt* stmt) {
            enter_scope(); 
            stmt->init->accept(this);
            
            stmt->condition->accept(this);
            if (current_type.base != TokenType::BOOLEAN) {
                throw_error("For loop condition must evaluate to a boolean type.");
            }
            
            stmt->increment->accept(this);
            loop_depth++;
            stmt->body->accept(this);
            loop_depth--;
            
            exit_scope();
        }
        
    // Return visit 
        void SemanticAnalyzer::visit(ReturnStmt* stmt) {
            if (stmt->value) {
                stmt->value->accept(this);
            } else {
                current_type = Type(TokenType::VOID);
            }
        }
      
    // Break and Continue  
        void SemanticAnalyzer::visit(BreakStmt* stmt) { if(loop_depth == 0){
            throw_error("'break' statement can only be used inside a loop!");
        } }
        void SemanticAnalyzer::visit(ContinueStmt* stmt) { if(loop_depth == 0){
            throw_error("'continue' statement can only be used inside a loop!");
        }}
        
    // Function visit
        void SemanticAnalyzer::visit(FunctionStmt* stmt) {
            FunctionType fType;
            fType.returnType = stmt->type;
            for (auto& p : stmt->params) fType.paramTypes.push_back(p.type);
            
            declare(stmt->name, stmt->type, true, fType);
            enter_scope();
            for (auto& p : stmt->params) {
                declare(p.name, p.type);
            }
            stmt->body->accept(this);
            exit_scope();
        }
    
    // Blueprint visit    
        void SemanticAnalyzer::visit(BluePrintExpr* stmt) {
            Symbol blueprintSym;
            blueprintSym.name = stmt->name;
            blueprintSym.type = Type(TokenType::CLASS, string(stmt->name));
            blueprintSym.isFunction = false;
            for(auto& fieldStmt: stmt->feild){
                VarDeclStmt* varDecl = dynamic_cast<VarDeclStmt*>(fieldStmt.get());
                if(varDecl){
                    blueprintSym.blueprintFields[string(varDecl->name)]= varDecl->type;
                }
            }
            auto & currentScope = scopes.back();
            currentScope[stmt->name] = blueprintSym;
        }
        
        // ---------------------------------------------------------
        // STEP 4 PREVIEW: ADVANCED EXPRESSIONS (To prevent compile errors)
        // ---------------------------------------------------------
   
    // Assignment visit  
        void SemanticAnalyzer::visit(AssignExpr* expr) {
            expr->target->accept(this);
            Type targetType = current_type;
            expr->value->accept(this);
            Type valueType = current_type;
            
            if (targetType != valueType) throw_error("Type mismatch in assignment!");
            current_type = targetType;
        }
    
    // Unary visit    
void SemanticAnalyzer::visit(UnaryExpr* expr) {
    // 1. Check what type the operand is
    expr->operand->accept(this);

    // 2. Apply rules based on the specific operator
    if (expr->op.type == TokenType::NOT) {
        if (current_type.base != TokenType::BOOLEAN) {
            throw_error("Unary '!' requires a boolean operand.");
        }
        current_type = Type(TokenType::BOOLEAN);
    } 
    else {
        // For -, ++, --
        if (current_type.base != TokenType::INT && current_type.base != TokenType::FLOAT && current_type.base != TokenType::DOUBLE) {
            throw_error("Unary math operations (-, ++, --) require numbers.");
        }
        // current_type remains the number type
    }
}
        
        // Shells for complex data structures (pointers, arrays, classes)
// Call function visit
void SemanticAnalyzer::visit(CallExpr* expr) { 
    // We must cast callee to VariableExpr to get the name string
    VariableExpr* varCallee = dynamic_cast<VariableExpr*>(expr->callee.get());
    if (!varCallee) {
        throw_error("Indirect function calls (e.g. from arrays/pointers) are not yet supported!");
    }

    Symbol* sym = lookup(varCallee->name.value);
    if (!sym || !sym->isFunction) {
        throw_error("'" + string(varCallee->name.value) + "' is not a function!");
    }

    FunctionType fType = sym->funcType; 
    if(expr->args.size() != fType.paramTypes.size()) {
        throw_error("Incorrect Number of arguments passed to Function: " + string(sym->name));
    }

    for(size_t i = 0; i < expr->args.size(); i++){
        expr->args[i]->accept(this); 
        
        Type argType = current_type;
        Type expectedParamType = fType.paramTypes[i];

        if(argType != expectedParamType) {
            // --- ALLOW IMPLICIT UPCASTING (TYPE PROMOTION) ---
            bool isValidPromotion = false;

            // 1. Float can upgrade to Double
            if (expectedParamType.base == TokenType::DOUBLE && argType.base == TokenType::FLOAT) isValidPromotion = true;
            
            // 2. Int can upgrade to Float or Double
            if (expectedParamType.base == TokenType::FLOAT && argType.base == TokenType::INT) isValidPromotion = true;
            if (expectedParamType.base == TokenType::DOUBLE && argType.base == TokenType::INT) isValidPromotion = true;

            // If it's not a safe promotion, throw your custom error!
            if (!isValidPromotion) {
                throw_error("Type Mismatch in parameters of Function: " + string(sym->name));
            }
        }
    }
    
    current_type = fType.returnType; // Set expression result to function's return type
}

// Dereference visit
void SemanticAnalyzer::visit(DereferenceExpr* expr) { 
    expr->operand->accept(this);
    
    if(current_type.base != TokenType::POINTER) {
        throw_error("Type Mismatch: Cannot dereference a non-pointer type!");
    }
    
    if (current_type.innerType != nullptr) {
        current_type = *(current_type.innerType); // Peel off one pointer layer!
    } else {
        throw_error("Pointer is missing inner type definition!");
    }
}

// Member visit
void SemanticAnalyzer::visit(MemberExpr* expr) { 
    expr->object->accept(this); 
    
    // We use CLASS as the token type for Blueprints
    if (current_type.base != TokenType::CLASS) {  
        throw_error("Cannot access a member of a non-blueprint type!");
    }
     expr->className= current_type.structName;
    Symbol* blueprintDef = lookup(current_type.structName);
    if(!blueprintDef){
        throw_error("Blueprint '"+current_type.structName+"' doesn't exist!");
    }
    string propName = string(expr->property);
    if(blueprintDef->blueprintFields.find(propName) == blueprintDef->blueprintFields.end()){
        throw_error("Blueprint '"+ current_type.structName+ "' has no property named '"+ propName+"'!");
    }
    current_type = blueprintDef->blueprintFields[propName];
}

// Array visit
void SemanticAnalyzer::visit(ArrayAccessExpr* expr) { 
    expr->index->accept(this); 
    if (current_type.base != TokenType::INT) {
        throw_error("Array index must be an integer!");
    }
    
    expr->array->accept(this); 
    if (current_type.base != TokenType::ARRAY) {
        throw_error("Cannot use [] on a non-array type!");
    }
    
    if (current_type.innerType != nullptr) {
        current_type = *(current_type.innerType); // Peel off one array layer!
    } else {
        throw_error("Array is missing its inner type definition!");
    }
}

// blueprint creation visit
void SemanticAnalyzer::visit(InstantiateExpr * expr){
    Symbol * blueprintDef = lookup(expr->className.value);
    if(!blueprintDef || blueprintDef->type.base!=TokenType::CLASS){
        throw_error("Cannot instantiate '"+ string(expr->className.value)+"' because it's not a Blueprint!");
    }
    current_type= Type(TokenType::CLASS , string(expr->className.value));
}

void SemanticAnalyzer::visit(PrintStmt* stmt) {
    stmt->value->accept(this); // Evaluates the expression
    // Optional: You could add checks here to ensure they aren't printing a raw Blueprint object!
}

void SemanticAnalyzer::visit(ScanStmt* stmt) {
    Symbol* sym = lookup(stmt->variableName.value);
    if (!sym) {
        throw_error("Cannot scan into undefined variable: " + string(stmt->variableName.value));
    }
}






