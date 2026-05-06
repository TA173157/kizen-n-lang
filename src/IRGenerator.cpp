#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include<llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/TargetParser/Triple.h>
#include <llvm/Config/llvm-config.h>
#include <iostream>
#include <memory>
#include <vector>
#include "IRGenerator.h"
using namespace std;
using namespace llvm;

//--------------------------------------------- -------------
// Helper functions
 // Escape characters 
string processEscapes(string input){
    string res;
    for(size_t i=0;i<input.length();++i){
            if(input[i] == '\\' && i+1 <input.length()){
            switch(input[++i]){
                case 'n':res +='\n';break;
                case 't':res +='\t';break;
                case '\"':res +='\"';break;
                default:res +=input[i];break;
            }
        }else{
            res+= input[i];
        }
    }
        return res;
    }
    
// get llvm type
llvm::Type* IRGenerator::getLLVMType(TokenType kind){
    if(kind==TokenType::INT)return llvm::Type::getInt32Ty(*context);
    if(kind==TokenType::FLOAT)return llvm::Type::getFloatTy(*context);
    if(kind==TokenType::DOUBLE)return llvm::Type::getDoubleTy(*context);
    if(kind==TokenType::BOOLEAN)return llvm::Type::getInt1Ty(*context);
    if(kind==TokenType::CHAR)return llvm::Type::getInt8Ty(*context);
    if(kind==TokenType::STRING)return llvm::PointerType::get(*context,0);
    if(kind==TokenType::CLASS)return llvm::PointerType::get(*context,0);
    if(kind==TokenType::VOID)return llvm::Type::getVoidTy(*context);
    return nullptr;
}
    //----------------------------------------------------------
// Constructor logic
IRGenerator::IRGenerator(){
     context = make_unique<LLVMContext>();
        module= make_unique<Module>("myCompiler",*context);
        builder = make_unique<IRBuilder<>>(*context);
        // Add this to the bottom of the constructor!
    std::vector<llvm::Type*> ioArgs;
    ioArgs.push_back(llvm::PointerType::get(*context, 0));
    
    llvm::FunctionType* ioType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), ioArgs, true);
    llvm::Function::Create(ioType, llvm::Function::ExternalLinkage, "printf", module.get());
    llvm::Function::Create(ioType, llvm::Function::ExternalLinkage, "scanf", module.get());
}
// for Currentval
Value* IRGenerator::generate(Node* node){
        node->accept(this);
        return currentVal;
    }

    // Literal Type AST Node;
void IRGenerator::visit(LiteralExpr* expr){
    string lexeme = string(expr->value.value);
    TokenType type = expr->value.type;

    switch(type){
        case TokenType::INT_LIT :{
            int val = stoi(lexeme);
            currentVal = ConstantInt::get(*context,APInt(32,val,true));
            break;
        }
        case TokenType::FLOAT_LIT :{
            double val = stod(lexeme);
            currentVal = ConstantFP::get(*context,APFloat(val));
            break;
        }
        case TokenType::BOOLEAN_LIT :{
            if(lexeme=="true"){
                currentVal = ConstantInt::get(*context,APInt(1,true,false));
            }
            else{
                currentVal = ConstantInt::get(*context,APInt(1,false,false));
            }
            break;
        }
        case TokenType::DOUBLE_LIT :{
            double val = stod(lexeme);
            currentVal = ConstantFP::get(llvm::Type::getDoubleTy(*context), val);
            break;
        }
        case TokenType::CHAR_LIT :{
           char content = lexeme[1];
           currentVal= ConstantInt::get(*context,APInt(8,content,true)); 
            break;
        }
        case TokenType::STRING_LIT :{
            string content= "";
            if(lexeme.length()>=2)content = lexeme.substr(1,lexeme.length()-2);
            currentVal = builder->CreateGlobalString(processEscapes(content),"strtmp");
            break;
        }
        default:
            currentVal = nullptr;
            cerr<<"Unknown Literal Type"<<endl;
    }
}

// Variable Expression AST Node;
void IRGenerator::visit(VariableExpr* expr){
    string varName = string(expr->name.value);
    AllocaInst* alloca = NamedValues[varName];
    if(!alloca){
        cerr<<"Error: Undefined Variable "<<varName<<endl;
        currentVal= nullptr;
        return;
    }
    currentVal= builder->CreateLoad(alloca->getAllocatedType(),alloca,varName.c_str());
}

//Dereference AST Node;
void IRGenerator::visit(DereferenceExpr* expr){
    expr->operand->accept(this);
    Value* prtVal = currentVal;
    if(!prtVal || !prtVal->getType()->isPointerTy()){
        cerr<<"IR ERROR: Cannot dereference a non-pointer type!"<<endl;
        return;
    }
    llvm::Type* targetType = llvm::Type::getInt32Ty(*context);
    currentVal = builder->CreateLoad(targetType , prtVal,"derefTmp");
}

// Binary expression AST Node;
void IRGenerator::visit(BinaryExpr* expr){
    if (!expr->left || !expr->right) {
        std::cerr << "\n💥 CRASH AVERTED: A math expression is missing its left or right side! Your parser dropped it.\n" << std::endl;
        currentVal = nullptr;
        return;
    }
    
    // 1. Evaluate both sides
    expr->left->accept(this);
    llvm::Value* l = currentVal;
    
    expr->right->accept(this);
    llvm::Value* r = currentVal;
    
    if(!l || !r){
        cerr << "Error: Invalid operands in binary expression.\n";
        currentVal = nullptr;
        return;
    }


    // 2. CHECK FOR FLOATS & TYPE PROMOTION
    bool isFloat = l->getType()->isFloatingPointTy() || r->getType()->isFloatingPointTy();

    if (isFloat) {
        // If one side is an integer, we must UPCAST it to a float to match the other side.
        // SIToFP means "Signed Integer To Floating Point"
        if (l->getType()->isIntegerTy()) {
            l = builder->CreateSIToFP(l, r->getType(), "int_to_float");
        }
        if (r->getType()->isIntegerTy()) {
            r = builder->CreateSIToFP(r, l->getType(), "int_to_float");
        }
    }

    // 3. EMIT THE CORRECT INSTRUCTIONS
    switch (expr->op.type){
        // Arithmetic Operators
        case TokenType::PLUS:
            currentVal = isFloat ? builder->CreateFAdd(l, r, "faddtmp") 
                                 : builder->CreateAdd(l, r, "addtmp");
            break;
        case TokenType::MINUS:
            currentVal = isFloat ? builder->CreateFSub(l, r, "fsubtmp") 
                                 : builder->CreateSub(l, r, "subtmp");
            break;
        case TokenType::MULTI:
            currentVal = isFloat ? builder->CreateFMul(l, r, "fmultmp") 
                                 : builder->CreateMul(l, r, "multmp");
            break;
        case TokenType::DIV:
            // SDiv is "Signed Divide" for ints. FDiv is "Float Divide".
            currentVal = isFloat ? builder->CreateFDiv(l, r, "fdivtmp") 
                                 : builder->CreateSDiv(l, r, "sdivtmp");
            break;

        // Comparison Operators
        case TokenType::LESSER:
            currentVal = isFloat ? builder->CreateFCmpOLT(l, r, "cmptmp") // OLT: Ordered Less Than
                                 : builder->CreateICmpSLT(l, r, "cmptmp");
            break;
        case TokenType::GREATER:
            currentVal = isFloat ? builder->CreateFCmpOGT(l, r, "cmptmp") 
                                 : builder->CreateICmpSGT(l, r, "cmptmp");
            break;
        case TokenType::EQUALS:
            currentVal = isFloat ? builder->CreateFCmpOEQ(l, r, "cmptmp") 
                                 : builder->CreateICmpEQ(l, r, "cmptmp");
            break;
        case TokenType::NOT_EQU:
            currentVal = isFloat ? builder->CreateFCmpONE(l, r, "cmptmp") 
                                 : builder->CreateICmpNE(l, r, "cmptmp");
            break;
        case TokenType::LESSER_EQU:
            currentVal = isFloat ? builder->CreateFCmpOLE(l, r, "cmptmp") 
                                 : builder->CreateICmpSLE(l, r, "cmptmp");
            break;
        case TokenType::GREATER_EQU:
            currentVal = isFloat ? builder->CreateFCmpOGE(l, r, "cmptmp") 
                                 : builder->CreateICmpSGE(l, r, "cmptmp");
            break;

        // Logical Operators (AND/OR are generally integer/boolean only in strict languages)
        case TokenType::AND:
            if (isFloat) cerr << "Warning: Bitwise/Logical AND on floats is not standard.\n";
            currentVal = builder->CreateAnd(l, r, "andtmp");
            break;
        case TokenType::OR:
            if (isFloat) cerr << "Warning: Bitwise/Logical OR on floats is not standard.\n";
            currentVal = builder->CreateOr(l, r, "ortmp");
            break;

        default:
            cerr << "Error: Unknown binary operator '" << string(expr->op.value) << "'\n";
            break;
    }
}

// Assignment AST Node;
// Assignment AST Node;
void IRGenerator::visit(AssignExpr* expr){
    // 1. Evaluate the value on the right side of the equals sign
    expr->value->accept(this);
    llvm::Value* val = currentVal;
    if(!val) return;

    llvm::Value* targetPtr = nullptr;
    llvm::Type* targetType = nullptr; // <--- NEW: Track the size of the memory box

    // 2a. Is it a standard variable?
    if(auto* varExpr = dynamic_cast<VariableExpr*>(expr->target.get())){
        string varName = string(varExpr->name.value);
        llvm::AllocaInst* alloca = NamedValues[varName];
        if(!alloca){
            cerr << "IR ERROR: Unknown variable '" << varName << "'" << endl;
            currentVal = nullptr;
            return;
        }
        targetPtr = alloca;
        targetType = alloca->getAllocatedType(); // Get exact memory size
    }
    // 2b. Is it a struct field?
    else if (auto* memExpr = dynamic_cast<MemberExpr*>(expr->target.get())) {
        memExpr->object->accept(this);
        llvm::Value* basePtr = currentVal;
        
        std::string structName = memExpr->className; 
        if (!StructTypes[structName]) return;
        std::string propertyName = std::string(memExpr->property);
        int fieldIndex = StructFields[structName][propertyName];
        
        std::vector<llvm::Value*> indices;
        indices.push_back(llvm::ConstantInt::get(*context, llvm::APInt(32, 0, true)));
        indices.push_back(llvm::ConstantInt::get(*context, llvm::APInt(32, fieldIndex, true)));

        targetPtr = builder->CreateInBoundsGEP(StructTypes[structName], basePtr, indices, propertyName + "Ptr");
        targetType = StructTypes[structName]->getElementType(fieldIndex); // Get exact memory size
    }
    else {
        cerr << "IR ERROR: Unrecognized assignment target!" << endl;
        currentVal = nullptr;
        return;
    }

    // --- THE MEMORY CORRUPTION FIX ---
    // Safely squeeze or stretch the value so it fits perfectly into targetType!
    if (targetType && val->getType() != targetType) {
        if (targetType->isFloatTy() && val->getType()->isDoubleTy()) {
            // Squeeze 64-bit down to 32-bit (FPTrunc)
            val = builder->CreateFPTrunc(val, targetType, "downcast_to_float");
        } 
        else if (targetType->isDoubleTy() && val->getType()->isFloatTy()) {
            // Stretch 32-bit up to 64-bit (FPExt)
            val = builder->CreateFPExt(val, targetType, "upcast_to_double");
        } 
        else if (targetType->isFloatingPointTy() && val->getType()->isIntegerTy()) {
            // Convert Int to Float (SIToFP)
            val = builder->CreateSIToFP(val, targetType, "int_to_float");
        }
    }

    // 3. Handle Compound Assignments 
    switch(expr->op.type){
        case TokenType::PLUS_EQU: {
            llvm::Value* curVal = builder->CreateLoad(targetType, targetPtr, "loadtmp");
            val = builder->CreateFAdd(curVal, val, "addtmp"); // Use appropriate add
            break;
        }
        // ... (Keep the rest of your compound assignment logic)
        case TokenType::ASSIGN: {
            break; 
        }
        default: {
            break;
        }
    }

    // 4. Store the final value safely!
    builder->CreateStore(val, targetPtr);
    currentVal = val;
}

//Unary AST Node;
void IRGenerator::visit(UnaryExpr* expr) {
    // 1. Handle simple MINUS and NOT (These don't modify memory)
    if (expr->op.type == TokenType::MINUS || expr->op.type == TokenType::NOT) {
        expr->operand->accept(this);
        llvm::Value* operandval = currentVal;
        if (!operandval) return;
        
        if (expr->op.type == TokenType::MINUS) {
            currentVal = builder->CreateNeg(operandval, "negtmp");
        } else {
            currentVal = builder->CreateNot(operandval, "nottmp");
        }
        return;
    }

    // 2. Handle ++ and -- (Read-Modify-Write)
    llvm::Value* targetPtr = nullptr;
    llvm::Type* expectedType = llvm::Type::getInt32Ty(*context); // Assuming int for now

    // We must find the pointer, similar to how Assignment works
    if (auto* varExpr = dynamic_cast<VariableExpr*>(expr->operand.get())) {
        string varName = string(varExpr->name.value);
        targetPtr = NamedValues[varName];
        if (!targetPtr) {
            cerr << "IR ERROR: Unknown variable '" << varName << "' for unary operation!" << endl;
            return;
        }
    } 
    else if (auto* memExpr = dynamic_cast<MemberExpr*>(expr->operand.get())) {
        memExpr->object->accept(this);
        llvm::Value* basePtr = currentVal;
        
        string structName = "Player"; // Hardcoded for now as in your other methods
        string propertyName = string(memExpr->property);
        int fieldIndex = StructFields[structName][propertyName];
        
        std::vector<llvm::Value*> indices;
        indices.push_back(llvm::ConstantInt::get(*context, llvm::APInt(32, 0, true)));
        indices.push_back(llvm::ConstantInt::get(*context, llvm::APInt(32, fieldIndex, true)));

        targetPtr = builder->CreateInBoundsGEP(StructTypes[structName], basePtr, indices, propertyName + "Ptr");
    } 
    else {
        cerr << "IR ERROR: Invalid operand for ++ or --" << endl;
        return;
    }

    if (!targetPtr) return;

    // 3. READ
    llvm::Value* loadedVal = builder->CreateLoad(expectedType, targetPtr, "loadtmp");
    llvm::Value* one = llvm::ConstantInt::get(*context, llvm::APInt(32, 1, true));
    llvm::Value* newVal = nullptr;

    // 4. MODIFY
    if (expr->op.type == TokenType::PRE_INCREMENT || expr->op.type == TokenType::POST_INCREMENT) {
        newVal = builder->CreateAdd(loadedVal, one, "inctmp");
    } else {
        newVal = builder->CreateSub(loadedVal, one, "dectmp");
    }

    // 5. WRITE
    builder->CreateStore(newVal, targetPtr);

    // 6. RETURN CORRECT VALUE (Pre vs Post)
    if (expr->op.type == TokenType::PRE_INCREMENT || expr->op.type == TokenType::PRE_DECREMENT) {
        currentVal = newVal;     // ++x returns the updated value
    } else {
        currentVal = loadedVal;  // x++ returns the old value
    }
}

//Member AST Node;
void IRGenerator::visit(MemberExpr* expr){
    expr->object->accept(this);
    Value* basePtr= currentVal;
    if(!basePtr)return ;
    
    // hardcoded for now 
    string structName = expr->className;
    if (!StructTypes[structName]) {
    std::cerr << "\n💥 CRASH AVERTED: StructTypes[\"Player\"] is NULL! Your compiler never visited the Blueprint!\n" << std::endl;
    currentVal = nullptr;
    return; 
}
    string propertyName = string(expr->property);
    int fieldIndex = StructFields[structName][propertyName];
    
    std::vector<llvm::Value*> indices;
    indices.push_back(llvm::ConstantInt::get(*context, llvm::APInt(32, 0, true)));
    indices.push_back(llvm::ConstantInt::get(*context, llvm::APInt(32, fieldIndex, true)));

    llvm::Value* fieldPtr = builder->CreateInBoundsGEP(StructTypes[structName], basePtr, indices, propertyName + "Ptr");
    llvm::Type* filedType= StructTypes[structName]->getElementType(fieldIndex);
    currentVal = builder->CreateLoad(filedType,fieldPtr,propertyName+"Val");
}

// Array Access AST Node;
void IRGenerator::visit(ArrayAccessExpr* expr){
    expr->array->accept(this);
    Value* arrayPtr = currentVal;
    //index
    expr->index->accept(this);
    Value* indexVal = currentVal;

    if(!arrayPtr|| !indexVal)return; 
    vector<Value*> indices;
    indices.push_back(ConstantInt::get(*context,APInt(32,0,true)));
    llvm::Type* elementType = llvm::Type::getInt32Ty(*context);
    llvm::Value* elementPtr = builder->CreateInBoundsGEP(elementType,arrayPtr,indices,"arrayIdxPtr");
    currentVal = builder->CreateLoad(elementType,elementPtr,"arrayElement");
}

// calling function AST Node;
void IRGenerator::visit(CallExpr* expr){
    auto* calleeVar = dynamic_cast<VariableExpr*>(expr->callee.get());
    if(!calleeVar){
        cerr<<"IR ERROR: Callee is not a valid function name!"<<endl;
        currentVal= nullptr;
        return ;
    }
    string calleeName = string(calleeVar->name.value);
    Function* calleef= module->getFunction(calleeName);
    
    if(!calleef){
        cerr<<"IR ERROR: Unkown function referenced: "<<calleeName<<endl;
        currentVal = nullptr;
        return;
    }
    if (calleef->arg_size() != expr->args.size()) {
        std::cerr << "IR Error: Incorrect number of arguments passed to " << calleeName << std::endl;
        currentVal = nullptr;
        return;
    }
    
    vector<Value*> argsV;
    for(unsigned i = 0; i < expr->args.size(); ++i){
        expr->args[i]->accept(this);
        if(!currentVal){
            cerr<<"IR ERROR: Failed to evaluate argument "<<i<<endl;
            return ;
        }
        
        llvm::Value* argVal = currentVal;
        
        // Get the exact type LLVM expects for this parameter!
        llvm::Type* expectedType = calleef->getFunctionType()->getParamType(i);
        llvm::Type* actualType = argVal->getType();

        // --- AUTOMATIC LLVM UPCASTING FOR ARGUMENTS ---
        if (expectedType != actualType) {
            // Float -> Double (FPExt)
            if (expectedType->isDoubleTy() && actualType->isFloatTy()) {
                argVal = builder->CreateFPExt(argVal, expectedType, "upcast_to_double");
            }
            // Int -> Float/Double (SIToFP)
            else if (expectedType->isFloatingPointTy() && actualType->isIntegerTy()) {
                argVal = builder->CreateSIToFP(argVal, expectedType, "int_to_float_arg");
            }
        }
        
        argsV.push_back(argVal);
    }
    
    // NOTE: This MUST be outside the loop!
    currentVal = builder->CreateCall(calleef, argsV, "calltmp");
}

// Instantiate the struct;
void IRGenerator::visit(InstantiateExpr* expr){
    string className = string(expr->className.value);
    StructType* structType = StructTypes[className];
    if (!structType) {
        std::cerr << "IR Error: Unknown Blueprint '" << className << "'" << std::endl;
        currentVal = nullptr;
        return;
    }
    currentVal = builder->CreateAlloca(structType,nullptr,"new"+ className);
}
            
        // Statement AST nodes 

// Variable Declare AST Node;
void IRGenerator::visit(VarDeclStmt* stmt){
    string varName = string(stmt->name);
    llvm::Type* llvmType= getLLVMType(stmt->type.base);
    if(!llvmType){
        cerr<<"IR ERROR: Unkown type for variable '"<< varName<<"'"<<endl;
        return ;
    }
    AllocaInst* alloca = builder->CreateAlloca(llvmType,nullptr,varName);
    NamedValues[varName] = alloca;
    if(stmt->initializer){
        stmt->initializer->accept(this);
        builder->CreateStore(currentVal,alloca);
    }
}

//If AST Node;
//If AST Node;
void IRGenerator::visit(IfStmt* stmt){
    stmt->condition->accept(this);
    Value* cond = currentVal; 
    cond = builder->CreateICmpNE(cond,llvm::ConstantInt::get(cond->getType(),0),"ifcond"); 

    Function* theFunction = builder->GetInsertBlock()->getParent();

    BasicBlock* thenBB= BasicBlock::Create(*context,"then",theFunction); 
    BasicBlock* elseBB= BasicBlock::Create(*context,"else");
    BasicBlock* mergeBB= BasicBlock::Create(*context,"ifcont");
     
    builder->CreateCondBr(cond,thenBB,elseBB); 
    
    // --- THEN BLOCK ---
    builder->SetInsertPoint(thenBB);
    stmt->thenBranch->accept(this);
    // FIX: Only branch if there isn't already a return/break!
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB); 
    }
    
    // --- ELSE BLOCK ---
    theFunction->insert(theFunction->end(),elseBB); 
    builder->SetInsertPoint(elseBB);
    if(stmt->elseBranch){
        stmt->elseBranch->accept(this);
    }
    // FIX: Only branch if there isn't already a return/break!
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB);
    }

    // --- MERGE BLOCK ---
    theFunction->insert(theFunction->end(),mergeBB);
    builder->SetInsertPoint(mergeBB);
}

// While AST Node;
// While AST Node;
void IRGenerator::visit(WhileStmt* stmt) {
    Function* theFunction = builder->GetInsertBlock()->getParent();

    BasicBlock* condBB = BasicBlock::Create(*context, "while.cond", theFunction);
    BasicBlock* bodyBB = BasicBlock::Create(*context, "while.body", theFunction);
    BasicBlock* endBB = BasicBlock::Create(*context, "while.end", theFunction);

    builder->CreateBr(condBB);
    builder->SetInsertPoint(condBB);

    stmt->condition->accept(this);
    llvm::Value* cond = currentVal; 
    cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(cond->getType(), 0), "whilecond");
    builder->CreateCondBr(cond, bodyBB, endBB);

    LoopIncBlocks.push_back(condBB);
    LoopExitBlocks.push_back(endBB);
    
    builder->SetInsertPoint(bodyBB);
    if(stmt->body) stmt->body->accept(this);

    LoopIncBlocks.pop_back();
    LoopExitBlocks.pop_back();
    
    // FIX: Only loop back if the body didn't 'break' or 'return'!
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(condBB);
    }

    builder->SetInsertPoint(endBB);
}

// For Loop AST Node;
// For Loop AST Node;
void IRGenerator::visit(ForStmt* stmt) {
    if(stmt->init) stmt->init->accept(this);

    Function* theFunction = builder->GetInsertBlock()->getParent();

    BasicBlock* condbb = BasicBlock::Create(*context, "for.cond", theFunction);
    BasicBlock* bodybb = BasicBlock::Create(*context, "for.body", theFunction);
    BasicBlock* incbb = BasicBlock::Create(*context, "for.inc", theFunction);
    BasicBlock* exitbb = BasicBlock::Create(*context, "for.exit", theFunction);

    builder->CreateBr(condbb);
    builder->SetInsertPoint(condbb);

    if(stmt->condition) {
        stmt->condition->accept(this);
        llvm::Value* cond = currentVal;
        cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(cond->getType(), 0), "forcond");
        builder->CreateCondBr(cond, bodybb, exitbb);
    } else {
        builder->CreateBr(bodybb);
    }

    LoopIncBlocks.push_back(incbb);
    LoopExitBlocks.push_back(exitbb);
    
    builder->SetInsertPoint(bodybb);
    if(stmt->body) stmt->body->accept(this);
    
    LoopIncBlocks.pop_back();
    LoopExitBlocks.pop_back();
    
    // FIX: Only jump to increment if no break/return happened!
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(incbb);
    }
    
    builder->SetInsertPoint(incbb);
    if(stmt->increment) stmt->increment->accept(this);
    
    // FIX: Same here for the loop back
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(condbb);
    }

    builder->SetInsertPoint(exitbb);
}

//Block AST Node;
void IRGenerator::visit(BlockStmt* stmt){
for(auto& s : stmt->statements){
    if(s) s->accept(this);
}
}

// Return Node;
void IRGenerator::visit(ReturnStmt* stmt){
    if(stmt->value){
    stmt->value->accept(this);
    Value* val = currentVal;
    builder->CreateRet(val);
    }else builder->CreateRetVoid();
}

// Break AST Node;
void IRGenerator::visit(BreakStmt* stmt){
    if(LoopExitBlocks.empty()){
        cerr<<"IR ERROR: 'break' used outside of a loop!"<<endl;
        return ;
    }
    builder->CreateBr(LoopExitBlocks.back());
}

// Exit AST Node;
void IRGenerator::visit(ContinueStmt* stmt){
if(LoopIncBlocks.empty()){
    cerr<<"IR ERROR: 'continue' used outside of a loop!"<<endl;
    return ;
}
builder->CreateBr(LoopIncBlocks.back());
}

//Program AST Node;
void IRGenerator::visit(ProgramNode* node){
    for(auto& stmt: node->statements) {if(stmt)stmt->accept(this);}
}

// Expression statment AST Node;
void IRGenerator::visit(ExprStmt* stmt){
    if(stmt->expr)stmt->expr->accept(this);
    else {
        cerr<<"IR ERROR: Exprstmt has a missing expression"<<endl;
    }
}

// Function AST Node;
void IRGenerator::visit(FunctionStmt* stmt){
    llvm::Type* returnType = getLLVMType(stmt->type.base);

    vector<llvm::Type*>argTypes;
    for(auto& param:stmt->params){
        argTypes.push_back(getLLVMType(param.type.base));
    }

    //creating function blueprint
    FunctionType* funcType = FunctionType::get(returnType,argTypes,false);

    // creating the function 
    Function* theFunction = Function::Create(funcType,Function::ExternalLinkage,string(stmt->name),module.get());
    
    // creating the first room
    BasicBlock* entrybb = BasicBlock::Create(*context,"entry",theFunction);
    builder->SetInsertPoint(entrybb);

    // describing the variables 
    NamedValues.clear();
    // take the incoming arguments and put them in memory
    unsigned idx =0;
    for(auto& arg : theFunction->args()){
        string argName = string(stmt->params[idx].name);
        arg.setName(argName);
        // creating alloca for the argument
        AllocaInst* alloca = builder->CreateAlloca(argTypes[idx],nullptr,argName);
        builder->CreateStore(&arg,alloca);

        // register it in symbol table;
        NamedValues[argName]= alloca;
        idx++;
    }
    // the body is generated here;
    if(stmt->body)stmt->body->accept(this);
    if(returnType->isVoidTy())builder->CreateRetVoid();
    verifyFunction(*theFunction);
}

// Blueprint AST Node;
void IRGenerator::visit(BluePrintExpr* stmt){
    string structName = string(stmt->name);
    vector<llvm::Type*>bodyTypes;
    map<string,int> fields;
    int currentIndex=0;

    for(auto& fieldNode: stmt->feild){
        auto* varDecl = dynamic_cast<VarDeclStmt*>(fieldNode.get());
        if (!varDecl) continue;
        llvm::Type* fieldType = nullptr;
        if(varDecl->type.base==TokenType::INT)fieldType = llvm::Type::getInt32Ty(*context);
        else if(varDecl->type.base==TokenType::BOOLEAN)fieldType = llvm::Type::getInt1Ty(*context);
        else if(varDecl->type.base==TokenType::FLOAT)fieldType = llvm::Type::getFloatTy(*context);
        else if(varDecl->type.base==TokenType::CHAR)fieldType = llvm::Type::getInt8Ty(*context);
        else if(varDecl->type.base==TokenType::STRING)fieldType = llvm::PointerType::get(*context,0);

        bodyTypes.push_back(fieldType);
        fields[string(varDecl->name)]=currentIndex++;
    }
    StructType* structType = StructType::create(*context,bodyTypes,structName);
    StructTypes[structName]= structType;
    StructFields[structName]= fields;
}

void IRGenerator::visit(PrintStmt* stmt) {
    // 1. Evaluate the expression
    stmt->value->accept(this);
    llvm::Value* val = currentVal;
    if (!val) return;

    llvm::Value* formatStr = nullptr;

    // 2. Ask LLVM what type this value is!
    if (val->getType()->isIntegerTy(32)) {
        formatStr = builder->CreateGlobalString("%d\n", "printInt");
    } 
    else if (val->getType()->isPointerTy()) {
        formatStr = builder->CreateGlobalString("%s\n", "printStr");
    } 
    else if (val->getType()->isFloatingPointTy()) {
        // C's printf function ALWAYS expects a double for %f. 
        // If it's a 32-bit float, we must upcast it just for the print!
        if (val->getType()->isFloatTy()) {
            val = builder->CreateFPExt(val, llvm::Type::getDoubleTy(*context), "upcast_for_printf");
        }
        formatStr = builder->CreateGlobalString("%f\n", "printFloat");
    }
    else {
        std::cerr << "IR ERROR: print() doesn't know how to print this type!" << std::endl;
        return;
    }
    
    // 3. Call printf with the correct format string
    llvm::Function* printfFunc = module->getFunction("printf");
    builder->CreateCall(printfFunc, {formatStr, val}, "printfCall");
}

void IRGenerator::visit(ScanStmt* stmt) {
    string varName = string(stmt->variableName.value);
    llvm::AllocaInst* alloca = NamedValues[varName];
    
    if(!alloca) {
        cerr << "IR ERROR: Variable " << varName << " not found for scanning!" << endl;
        return;
    }

    // What kind of memory did we allocate for this variable?
    llvm::Type* varType = alloca->getAllocatedType();
    llvm::Value* formatStr = nullptr;

    if (varType->isIntegerTy(32)) {
        formatStr = builder->CreateGlobalString("%d", "scanInt");
    } 
    else if (varType->isFloatTy()) {
        formatStr = builder->CreateGlobalString("%f", "scanFloat");
    } 
    else if (varType->isDoubleTy()) {
        // scanf uses %lf (long float) specifically for memory addresses of doubles
        formatStr = builder->CreateGlobalString("%lf", "scanDouble"); 
    } 
    else {
        cerr << "IR ERROR: scan() doesn't support this type yet!" << endl;
        return;
    }

    llvm::Function* scanfFunc = module->getFunction("scanf");
    builder->CreateCall(scanfFunc, {formatStr, alloca}, "scanfCall");
}
void IRGenerator::generateObjectFile(const std::string& outFilename) {
    // Initialize the target registry
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // 1. Get the raw string
    std::string targetTripleStr = llvm::sys::getDefaultTargetTriple();
    
    // 2. Parse it into an official LLVM Triple object
    llvm::Triple targetTriple(targetTripleStr); 

    // setTargetTriple signature changed in newer LLVM versions.
#if LLVM_VERSION_MAJOR >= 21
    module->setTargetTriple(targetTriple);
#else
    module->setTargetTriple(targetTripleStr);
#endif

    std::string error;
    // The lookup target still wants the string
    auto target = llvm::TargetRegistry::lookupTarget(targetTripleStr, error);
    if (!target) {
        std::cerr << error;
        return;
    }

    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    
    // LLVM API changed across versions:
    // - older versions expect target triple as StringRef
    // - newer versions accept llvm::Triple
#if LLVM_VERSION_MAJOR >= 18
    auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, llvm::Reloc::PIC_);
#else
    auto targetMachine = target->createTargetMachine(targetTripleStr, CPU, features, opt, llvm::Reloc::PIC_);
#endif

    module->setDataLayout(targetMachine->createDataLayout());

    std::error_code EC;
    llvm::raw_fd_ostream dest(outFilename, EC, llvm::sys::fs::OF_None);
    if (EC) {
        std::cerr << "Could not open file: " << EC.message();
        return;
    }

    llvm::legacy::PassManager pass;
#if LLVM_VERSION_MAJOR >= 18
    auto fileType = llvm::CodeGenFileType::ObjectFile;
#else
    auto fileType = llvm::CGFT_ObjectFile;
#endif
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        std::cerr << "TargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*module);
    dest.flush();
    
    std::cout << "[6/6] Generating Native Object File (" << outFilename << ")...\n";
    std::cout << "✅ [SUCCESS] Compiled to native machine code: " << outFilename << "\n";
}