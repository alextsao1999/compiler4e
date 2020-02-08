//
// Created by 曹顺 on 2019/5/5.
//

#ifndef PARSE_E_FILE_TEST_H
#define PARSE_E_FILE_TEST_H

#include "visitor.h"
#include "ast.h"
#include <iostream>
#include <map>
struct ECode;
struct ESub;
using namespace std;
struct DumpVisitor : Visitor {
    // 当前子程序
    ESub *current;
    ECode *code;
    int indent{0};
    explicit DumpVisitor(ECode *code, ESub *current);

    void visit(ASTProgram *node) override;
    void visit(ASTArgs *node) override;
    void visit(ASTBlock *node) override;
    void visit(ASTFunCall *node) override;
    void visit(ASTIfStmt *node) override;
    void visit(ASTLiteral *node) override;
    void visit(ASTConstant *node) override;
    void visit(ASTLibConstant *node) override;
    void visit(ASTAddress *node) override;
    void visit(ASTSubscript *node) override;
    void visit(ASTEnumConstant *node) override;
    void visit(ASTStructMember *node) override;
    void visit(ASTVariable *node) override;
    void visit(ASTDot *node) override;
    void visit(ASTJudge *node) override;
    void visit(ASTLoop *node) override;
    /**
     * 花括号表达式{xxx, xx ,xxx}
     * @param node
     */
    void visit(ASTBrace *node) override;

    void print_indent();

};


#endif //PARSE_E_FILE_TEST_H
