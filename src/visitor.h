//
// Created by 曹顺 on 2019/5/5.
//

#ifndef PARSE_E_FILE_VISITOR_H
#define PARSE_E_FILE_VISITOR_H
#include <cstdint>
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
struct ASTPostfix;
struct ASTJudge;
struct ASTLoop;
struct ASTBrace;
namespace llvm {
    class Value;
    class Type;
}
using namespace llvm;
//typedef Value *EValue;
struct EValue {
    Value *value;
    uint32_t ref = false;
    EValue (Value *value = nullptr) : value(value) {}
    inline operator Value*() { return value; }
    inline Value *operator->() { return value; }
    inline uint32_t getRef() { return ref; }
};
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
    virtual void visit(ASTPostfix *node) {}
    virtual void visit(ASTJudge *node) {}
    virtual void visit(ASTLoop *node) {}
    virtual void visit(ASTBrace *node) {}
    
    virtual EValue codegenLHS(ASTNode *node) { return nullptr; }
    virtual EValue codegenLHS(ASTFunCall *node) { return nullptr; }
    virtual EValue codegenLHS(ASTPostfix *node) { return nullptr; }
    virtual EValue codegenLHS(ASTVariable *node) { return nullptr; }
    virtual EValue codegenLHS(ASTStructMember *node) { return nullptr; }

    virtual EValue codegen(ASTNode *node) { return nullptr; }
    virtual EValue codegen(ASTFunCall *node) { return nullptr; }
    virtual EValue codegen(ASTProgram *node) { return nullptr; }
    virtual EValue codegen(ASTArgs *node) { return nullptr; }
    virtual EValue codegen(ASTBlock *node) { return nullptr; }
    virtual EValue codegen(ASTIfStmt *node) { return nullptr; }
    virtual EValue codegen(ASTLiteral *node) { return nullptr; }
    virtual EValue codegen(ASTConstant *node) { return nullptr; }
    virtual EValue codegen(ASTLibConstant *node) { return nullptr; }
    virtual EValue codegen(ASTAddress *node) { return nullptr; }
    virtual EValue codegen(ASTSubscript *node) { return nullptr; }
    virtual EValue codegen(ASTEnumConstant *node) { return nullptr; }
    virtual EValue codegen(ASTStructMember *node) { return nullptr; }
    virtual EValue codegen(ASTVariable *node) { return nullptr; }
    virtual EValue codegen(ASTPostfix *node) { return nullptr; }
    virtual EValue codegen(ASTJudge *node) { return nullptr; }
    virtual EValue codegen(ASTLoop *node) { return nullptr; }
    virtual EValue codegen(ASTBrace *node) { return nullptr; }

};


#endif //PARSE_E_FILE_VISITOR_H
