//
// Created by Alex on 2020/2/6.
//

#ifndef ECOMPILER_COMPILER_H
#define ECOMPILER_COMPILER_H

#include <set>
#include "visitor.h"
#include "ECodeParser.h"
#include "ast.h"
#include "llvm/IR/IRBuilder.h"
#define _GT(t) L##t
using namespace llvm;
using namespace std;
struct EContext {
    ECode *code = nullptr;
    LLVMContext llvm;
    EContext(ECode *code) : code(code) {}
};
class ECompiler : public Visitor {

public:
    ECode *code;
    ESub *sub;
    IRBuilder<> builder;
    EContext& context;
    static void CreateStruct(EContext &context, EStruct *estruct) {
        std::vector<Type *> elements(estruct->members.size());
        for (int i = 0; i < estruct->members.size(); ++i) {
            elements[i] = GetType(context, estruct->members[i].type);
        }
        auto *value = StructType::get(context.llvm, elements);
        estruct->type = value;
    }
    static void CreateFunction(EContext &context, ESub *sub) {
        std::string name = sub->name.toString();
        if (name == "_启动子程序") {
            name.assign("main");
        }
        std::vector<Type *> pts(sub->params.size());
        for (int i = 0; i < sub->params.size(); ++i) {
            pts[i] = GetType(context, sub->params[i].type);
        }
        auto *functionType = FunctionType::get(GetType(context, sub->type), pts, false);
        Function *function = Function::Create(functionType, Function::ExternalLinkage, name, sub->belong->module);
        sub->value = function;
    }
    static Type *GetType(EContext& context, Key key) {
        std::map<int, Type *> types = {
                {SDT_BYTE,      IntegerType::getVoidTy(context.llvm)},
                {SDT_SHORT,     IntegerType::getInt16Ty(context.llvm)},
                {SDT_INT,       IntegerType::getInt32Ty(context.llvm)},
                {SDT_INT64,     IntegerType::getInt64Ty(context.llvm)},
                {SDT_FLOAT,     IntegerType::getFloatTy(context.llvm)},
                {SDT_DOUBLE,    IntegerType::getDoubleTy(context.llvm)},
                {SDT_BOOL,      IntegerType::getInt32Ty(context.llvm)},
                {SDT_DATE_TIME, IntegerType::getDoubleTy(context.llvm)},
                {SDT_TEXT,      IntegerType::getInt8PtrTy(context.llvm)},
                {SDT_BIN,       IntegerType::getInt8PtrTy(context.llvm)},
                {SDT_SUB_PTR,   IntegerType::getInt8PtrTy(context.llvm)},
                {SDT_STATMENT,  IntegerType::getInt8PtrTy(context.llvm)}
        };
        if (types.count(key.value)) {
            return types[key.value];
        }
        if (key.type == 0) {
            if (key.index == 48) {

            }
            if (key.index == 49) {

            }
        }
        if (key.type == KeyType::KeyType_DataStruct) {

        }
        if (key.type == KeyType::KeyType_Module) {

        }
        return IntegerType::getVoidTy(context.llvm);
    }
    explicit ECompiler(EContext &context, ESub *sub) :
    context(context), builder(context.llvm), sub(sub) {
        code = context.code;
        Function *function = (Function *) sub->value;
        BasicBlock *entry = BasicBlock::Create(context.llvm, "entrypoint", function);
        IRBuilder<> lb(entry);
        int i = 0;
        for (auto &arg : function->args()) {
            auto *alloc = lb.CreateAlloca(arg.getType(), nullptr);
            lb.CreateStore(alloc, &arg);
            sub->params[i].value = alloc;
            i++;
        }
        for (auto &local : sub->locals) {
            local.value = lb.CreateAlloca(getType(local.type), nullptr, local.name.toString());
        }
        builder.SetInsertPoint(entry);
    }
    Type *getType(Key key) {
        return GetType(context, key);
    }
    void visit(ASTFunCall *node) override {
        if (node->key.value == 0) {
            return;
        }
        if (node->lib >= 0) {
            auto &cmd = code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value];
            std::string name((char *) cmd.m_szName);
            if (name == "返回") {
                if (node->args->args.size() < 1) {
                    return;
                }
                Value *value = node->args->args[0]->codegen(this);
                if (!value) {
                    return;
                }
                builder.CreateRet(value);
            }

            if (name == "赋值") {
                if (node->args->args.size() < 2) {
                    return;
                }
                Value *var = node->args->args[0]->codegen(this);
                Value *val = node->args->args[1]->codegen(this);
                builder.CreateStore(var, val);
            }

        }

    }
    void visit(ASTProgram *node) override {
        for (auto &stmt : node->stmts) {
            stmt->accept(this);
        }
    }
    void visit(ASTLiteral *node) override {
        switch (node->value.type) {
            case 0:
                cout << "null";
                break;
            case 1:
                cout << node->value.val_int;
                break;
            case 2:
                cout << node->value.val_bool;
                break;
            case 3:
                cout << "\"" << node->value.val_string << "\"";
                break;
            case 4:
                cout << node->value.val_time;
                break;
            case 5:
                cout << node->value.val_double;
                break;
            default:
                break;
        }
    }

    Value *codegen(ASTFunCall *node) override {
        using Bins = Instruction::BinaryOps;
        static std::map<std::string, Bins> binary = {
                {"相加", Bins::Add},
                {"相减", Bins::Sub},
                {"相乘", Bins::Mul},
                {"相除", Bins::UDiv},
                {"位与", Bins::And},
                {"位或", Bins::Or}
        };
        if (node->lib >= 0) {
            auto &cmd = code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value];
            std::string name((char *) cmd.m_szName);
            if (binary.count(name)) {
                if (node->args->args.size() < 2) {
                    return builder.getInt32(0);
                }
                Value *lhs = node->args->args[0]->codegen(this);
                Value *rhs = node->args->args[1]->codegen(this);
                return builder.CreateBinOp(binary[name], lhs, rhs, "temp");
            }
        }
        if (node->key.type == KeyType_Sub) {
            auto *sub = code->find<ESub>(node->key);
            if (sub) {
                printf("this is sub\n");
                std::vector<Value *> args(node->args->args.size());
                int idx = 0;
                for (auto &arg : node->args->args) {
                    args[idx] = node->args->args[idx]->codegen(this);
                    idx++;
                }
                return builder.CreateCall(sub->value, args);
            }
        }
        return builder.getInt32(0);

    }
    Value *codegen(ASTLiteral *node) override {
        if (node->value.type == 1) {
            return builder.getInt32(node->value.val_int);
        }
        if (node->value.type == 5) {
            return builder.getInt32(node->value.val_double);
        }

        return builder.getInt32(0);
    }
    Value *codegen(ASTVariable *node) override {
        auto *var = code->find<EVar>(node->key);
        if (node->key.type == KeyType_LocalOrParam) {
            if (!var->value) {
                return builder.getInt32(0);
            }
            return builder.CreateLoad(var->value, var->name.toString());
        }
        return builder.getInt32(0);
    }

};


#endif //ECOMPILER_COMPILER_H
