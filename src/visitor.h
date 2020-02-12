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

#define AST_METHOD(METHOD, RET, RETVAL)     \
virtual RET METHOD(ASTNode *node) { return RETVAL; } \
virtual RET METHOD(ASTFunCall *node) { return RETVAL; } \
virtual RET METHOD(ASTProgram *node) { return RETVAL; } \
virtual RET METHOD(ASTArgs *node) { return RETVAL; } \
virtual RET METHOD(ASTBlock *node) { return RETVAL; } \
virtual RET METHOD(ASTIfStmt *node) { return RETVAL; } \
virtual RET METHOD(ASTLiteral *node) { return RETVAL; } \
virtual RET METHOD(ASTConstant *node) { return RETVAL; } \
virtual RET METHOD(ASTLibConstant *node) { return RETVAL; } \
virtual RET METHOD(ASTAddress *node) { return RETVAL; } \
virtual RET METHOD(ASTSubscript *node) { return RETVAL; } \
virtual RET METHOD(ASTEnumConstant *node) { return RETVAL; } \
virtual RET METHOD(ASTStructMember *node) { return RETVAL; } \
virtual RET METHOD(ASTVariable *node) { return RETVAL; } \
virtual RET METHOD(ASTPostfix *node) { return RETVAL; } \
virtual RET METHOD(ASTJudge *node) { return RETVAL; } \
virtual RET METHOD(ASTLoop *node) { return RETVAL; } \
virtual RET METHOD(ASTBrace *node) { return RETVAL; }

struct Visitor {
    virtual void enter(ASTNode *node) {}
    virtual void leave(ASTNode *node) {}
    AST_METHOD(codegen, EValue, nullptr);
    AST_METHOD(codegenLHS, EValue, nullptr);
    AST_METHOD(visit, void, void());
    AST_METHOD(type, Type *, nullptr);

};

#endif //PARSE_E_FILE_VISITOR_H
