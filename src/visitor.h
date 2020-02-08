//
// Created by 曹顺 on 2019/5/5.
//

#ifndef PARSE_E_FILE_VISITOR_H
#define PARSE_E_FILE_VISITOR_H

#include "llvm/IR/Value.h"
struct ASTNode;
struct ASTFunCall;
struct ASTProgram;
struct ASTArgs;
struct ASTBlock;
struct ASTIfStmt;
struct ASTLiteral;
struct ASTConstant;
struct ASTLibConstant;
struct ASTAddress;
struct ASTSubscript;
struct ASTEnumConstant;
struct ASTStructMember;
struct ASTVariable;
struct ASTDot;
struct ASTJudge;
struct ASTLoop;
struct ASTBrace;
namespace llvm {
    class Value;
}
using namespace llvm;
struct Visitor {
    virtual void enter(ASTNode *node) {}
    virtual void leave(ASTNode *node) {}
    virtual void visit(ASTNode *node) {}
    virtual void visit(ASTFunCall *node) {}
    virtual void visit(ASTProgram *node) {}
    virtual void visit(ASTArgs *node) {}
    virtual void visit(ASTBlock *node) {}
    virtual void visit(ASTIfStmt *node) {}
    virtual void visit(ASTLiteral *node) {}
    virtual void visit(ASTConstant *node) {}
    virtual void visit(ASTLibConstant *node) {}
    virtual void visit(ASTAddress *node) {}
    virtual void visit(ASTSubscript *node) {}
    virtual void visit(ASTEnumConstant *node) {}
    virtual void visit(ASTStructMember *node) {}
    virtual void visit(ASTVariable *node) {}
    virtual void visit(ASTDot *node) {}
    virtual void visit(ASTJudge *node) {}
    virtual void visit(ASTLoop *node) {}
    virtual void visit(ASTBrace *node) {}
    
    virtual Value *codegenLHS(ASTNode *node) { return nullptr; }
    virtual Value *codegenLHS(ASTFunCall *node) { return nullptr; }
    virtual Value *codegenLHS(ASTDot *node) { return nullptr; }
    virtual Value *codegenLHS(ASTVariable *node) { return nullptr; }
    virtual Value *codegenLHS(ASTStructMember *node) { return nullptr; }

    virtual Value *codegen(ASTNode *node) { return nullptr; }
    virtual Value *codegen(ASTFunCall *node) { return nullptr; }
    virtual Value *codegen(ASTProgram *node) { return nullptr; }
    virtual Value *codegen(ASTArgs *node) { return nullptr; }
    virtual Value *codegen(ASTBlock *node) { return nullptr; }
    virtual Value *codegen(ASTIfStmt *node) { return nullptr; }
    virtual Value *codegen(ASTLiteral *node) { return nullptr; }
    virtual Value *codegen(ASTConstant *node) { return nullptr; }
    virtual Value *codegen(ASTLibConstant *node) { return nullptr; }
    virtual Value *codegen(ASTAddress *node) { return nullptr; }
    virtual Value *codegen(ASTSubscript *node) { return nullptr; }
    virtual Value *codegen(ASTEnumConstant *node) { return nullptr; }
    virtual Value *codegen(ASTStructMember *node) { return nullptr; }
    virtual Value *codegen(ASTVariable *node) { return nullptr; }
    virtual Value *codegen(ASTDot *node) { return nullptr; }
    virtual Value *codegen(ASTJudge *node) { return nullptr; }
    virtual Value *codegen(ASTLoop *node) { return nullptr; }
    virtual Value *codegen(ASTBrace *node) { return nullptr; }

};


#endif //PARSE_E_FILE_VISITOR_H
