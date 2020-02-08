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
using namespace llvm;
using namespace std;
struct EContext {
    ECode *code = nullptr;
    LLVMContext llvm;
    std::map<int, Type *> types = {
            {SDT_BYTE,      IntegerType::getInt8Ty(llvm)},
            {SDT_SHORT,     IntegerType::getInt16Ty(llvm)},
            {SDT_INT,       IntegerType::getInt32Ty(llvm)},
            {SDT_INT64,     IntegerType::getInt64Ty(llvm)},
            {SDT_FLOAT,     IntegerType::getFloatTy(llvm)},
            {SDT_DOUBLE,    IntegerType::getDoubleTy(llvm)},
            {SDT_BOOL,      IntegerType::getInt32Ty(llvm)},
            {SDT_DATE_TIME, IntegerType::getDoubleTy(llvm)},
            {SDT_TEXT,      IntegerType::getInt8PtrTy(llvm)},
            {SDT_BIN,       IntegerType::getInt8PtrTy(llvm)},
            {SDT_SUB_PTR,   IntegerType::getInt8PtrTy(llvm)},
            {SDT_STATMENT,  IntegerType::getInt8PtrTy(llvm)}
    };
    explicit EContext(ECode *code) : code(code) {
        for (auto &estruct : code->structs) {
            CreateStruct(&estruct);
        }
        for (auto &module : code->modules) {
            module.module = new Module(module.name.toStringRef(), llvm);
            for (auto &sub : module.include) {
                auto *find = code->find<ESub>(sub);
                if (find) {
                    find->belong = &module;
                    CreateFunction(find);
                }
            }
        }
    }
    Type *getType(Key key) {
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
            auto *find = code->find<EStruct>(key);
            if (find) {
                return find->type;
            }
        }
        if (key.type == KeyType::KeyType_Module) {

        }
        return IntegerType::getVoidTy(llvm);
    }
    template <typename Type>
    Type *get(const char *name, KeyType type) {
        for (auto begin = code->maps.begin(); begin != code->maps.end(); begin++) {
            if (begin->second->name == name && begin->second->key.type == type) {
                return (Type *) begin->second;
            }
        }
        return nullptr;
    }
    void CreateStruct(EStruct *estruct) {
        if (estruct->type) {
            return;
        }
        std::string &&comment = estruct->comment.toString();
        if (!comment.empty() && comment[0] == '#') {
            estruct->attr = json::parse(comment.c_str() + 1);
        }
        std::vector<Type *> elements(estruct->members.size());
        for (int i = 0; i < estruct->members.size(); ++i) {
            elements[i] = getType(estruct->members[i].type);
            if (elements[i] == nullptr) {
                auto *find = code->find<EStruct>(estruct->members[i].type);
                CreateStruct(find);
                elements[i] = find->type;
            }
            if ((estruct->members[i].property & Property_Ref) == Property_Ref) {
                elements[i] = elements[i]->getPointerTo(0);
            }
            if (!estruct->members[i].dimension.empty()) {
                for (auto &value : estruct->members[i].dimension) {
                    printf("%d, ", value);

                    //elements[i] = ArrayType::get(elements[i], value);
                }
            }
        }
        auto *value = StructType::get(llvm, elements);
        value->setName(estruct->name.toStringRef());
        estruct->type = value;
        if (estruct->name == "String") {
            types[SDT_TEXT] = value;
        }

    }
    void CreateFunction(ESub *sub) {
        std::string &&comment = sub->comment.toString();
        if (!comment.empty() && comment.at(0) == '#') {
            sub->attr = json::parse(comment.c_str() + 1);
        }
        std::string &&name = sub->name.toString();
        if (name == "_启动子程序") {
            name.assign("main");
        }
        std::vector<Type *> pts(sub->params.size());
        for (int i = 0; i < sub->params.size(); ++i) {
            pts[i] = getType(sub->params[i].type);
            if ((sub->params[i].property & Property_Ref) == Property_Ref) {
                pts[i] = pts[i]->getPointerTo(0);
            }
            if ((sub->params[i].property & Property_Array) == Property_Array) {
                pts[i] = pts[i]->getPointerTo(0);
            }
        }
        auto *functionType = FunctionType::get(getType(sub->type), pts, sub->attr.count("va_args"));
        Function *function = Function::Create(functionType, Function::ExternalLinkage, name, sub->belong->module);
        //function->addFnAttr(Attribute::AttrKind::Alignment);
        sub->value = function;
    }

};
class ECompiler : public Visitor {
public:
    ECode *code;
    ESub *sub;
    IRBuilder<> builder;
    EContext& context;
    BasicBlock *current;
    explicit ECompiler(EContext &context, ESub *sub) :
    context(context), builder(context.llvm), sub(sub) {
        code = context.code;
        Function *function = (Function *) sub->value;
        BasicBlock *entry = BasicBlock::Create(context.llvm, "entrypoint", function);
        IRBuilder<> lb(entry);
        int i = 0;
        for (auto &arg : function->args()) {
            auto *alloc = lb.CreateAlloca(arg.getType(), nullptr);
            lb.CreateStore(&arg, alloc);
            sub->params[i].value = alloc;
            i++;
        }
        for (auto &local : sub->locals) {
            Value *array = nullptr;
            if (!local.dimension.empty()) {
                array = lb.getInt32(local.dimension[0]);
            }
            local.value = lb.CreateAlloca(getType(local.type), array, local.name.toString());
        }
        builder.SetInsertPoint(entry);
    }
    Type *getType(Key key) {
        return context.getType(key);
    }
    void push(BasicBlock *block) {
        sub->value->getBasicBlockList().push_back(block);
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
                return;
            }
            if (name == "赋值") {
                if (node->args->args.size() < 2) {
                    return;
                }
                Value *var = node->args->args[0]->codegenLHS(this);
                Value *val = node->args->args[1]->codegen(this);
                builder.CreateStore(val, var);
                return;
            }
        }
        if (node->key.type == KeyType_Sub) {
            node->codegen(this);
        }
    }
    void visit(ASTProgram *node) override {
        for (auto &stmt : node->stmts) {
            stmt->accept(this);
        }
        if (sub->type == 0) {
            builder.CreateRetVoid();
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
    void visit(ASTBlock *node) override {
        for (auto &stmt : node->element) {
            stmt->accept(this);
        }
    }
    void visit(ASTIfStmt *node) override {
        Value *value = node->condition->codegen(this);
        auto *leave = BasicBlock::Create(context.llvm, "leave");
        auto *then_block = BasicBlock::Create(context.llvm, "then");
        auto *else_block = node->else_block ? BasicBlock::Create(context.llvm, "else") : leave;
        builder.CreateCondBr(value, then_block, else_block);
        builder.SetInsertPoint(then_block);
        if (node->then_block) {
            node->then_block->accept(this);
        }
        builder.CreateBr(leave);
        sub->value->getBasicBlockList().push_back(then_block);
        if (node->else_block) {
            builder.SetInsertPoint(else_block);
            node->else_block->accept(this);
            sub->value->getBasicBlockList().push_back(else_block);
            builder.CreateBr(leave);
        }
        sub->value->getBasicBlockList().push_back(leave);
        builder.SetInsertPoint(leave);
    }
    void visit(ASTJudge *node) override {
        Visitor::visit(node);
    }
    void visit(ASTLoop *node) override {
        auto *cond = BasicBlock::Create(context.llvm, "cond");
        auto *loop = BasicBlock::Create(context.llvm, "loop");
        auto *leave = BasicBlock::Create(context.llvm, "leave");
        auto *before = current;
        Value *counter = nullptr;
        current = leave;
        if (node->head->lib >= 0) {
            auto &cmd = code->libraries[node->head->lib].info->m_pBeginCmdInfo[node->head->key.value];
            std::string name((char *) cmd.m_szName);
            if (name == "判断循环首") {
                push(cond);
                builder.SetInsertPoint(cond);
                Value *condition = node->head->args->args[0]->codegen(this);
                builder.CreateCondBr(condition, loop, leave);
            }
            if (name == "计次循环首") {
                counter = node->head->args->args[1]->codegenLHS(this);
                builder.CreateStore(builder.getInt32(0), counter);
                push(cond);
                builder.SetInsertPoint(cond);
                Value *value = node->head->args->args[0]->codegen(this);
                Value *count = builder.CreateLoad(counter);
                Value *cmp = builder.CreateICmp(CmpInst::ICMP_UGT, count, value);
                builder.CreateCondBr(cmp, loop, leave);
            }
            push(loop);
            builder.SetInsertPoint(loop);
            if (node->block)
                node->block->accept(this);
            if (counter) {
                Value *value = builder.CreateAdd(builder.CreateLoad(counter), builder.getInt32(1));
                builder.CreateStore(value, counter);
            }
            builder.CreateBr(cond);
            if (name == "循环判断首") {
                push(cond);
                builder.SetInsertPoint(cond);
                Value *condition = node->tail->args->args[0]->codegen(this);
                builder.CreateCondBr(condition, loop, leave);
            }
            push(leave);
            builder.SetInsertPoint(leave);
        }
        current = before;
    }

    Value *LoadVar(Key key) {
        EVar *var = code->find<EVar>(key);
        if (var == nullptr || var ->value == nullptr) {
            return builder.getInt32(0);
        }
        return builder.CreateLoad(var->value, var->name.toString());
    }
    Value *GetVar(Key key) {
        EVar *var = code->find<EVar>(key);
        if (!var) {
            return nullptr;
        }
        return var->value;
    }
    Value *codegenLHS(ASTDot *node) override {
        Value *var = node->var->codegenLHS(this);
        if (node->field->getType() == ASTStructMember::Type) {
            auto *field = node->field->cast<ASTStructMember>();
            auto *find = context.code->find<EStruct>(field->key);
            for (int i = 0; i < find->members.size(); ++i) {
                if (find->members[i].key == field->member) {
                    return builder.CreateStructGEP(var, i);
                }
            }
        }
        return var;
    }
    Value *codegenLHS(ASTVariable *node) override {
        Value *var = GetVar(node->key);
        if (!var) {
            var = GetVar(node->key.value - 2);
        }
        return var;
    }
    Value *codegen(ASTFunCall *node) override {
        using Bins = Instruction::BinaryOps;
        using Pred = CmpInst::Predicate;
        struct BinPair {
            Bins iOp;
            Bins fOp;
        };

        static std::map<std::string, BinPair> binary = {
                {"相加", {Bins::Add,  Bins::FAdd}},
                {"相减", {Bins::Sub,  Bins::FSub}},
                {"相乘", {Bins::Mul,  Bins::FMul}},
                {"相除", {Bins::SDiv, Bins::FDiv}},
                {"位与", {Bins::And,  Bins::And}},
                {"位或", {Bins::Or,   Bins::Or}}
        };
        #define PRED(P) Pred::ICMP_##P
        static std::map<std::string, Pred> cmps = {
                {"大于", PRED(SGT)},
                {"小于", PRED(SLT)},
                {"等于", PRED(EQ)},
                {"大于等于", PRED(SGE)},
                {"小于等于", PRED(SLE)},
        };
        #undef PRED
        if (node->lib >= 0) {
            auto &cmd = code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value];
            std::string name((char *) cmd.m_szName);
            if (binary.count(name)) {
                if (node->args->args.size() < 2) {
                    return builder.getInt32(0);
                }
                auto ops = binary[name];
                Value *lhs = nullptr;
                for (auto & arg : node->args->args) {
                    Value *value = arg->codegen(this);
                    if (lhs == nullptr) {
                        lhs = value;
                    } else {
                        lhs = builder.CreateBinOp(binary[name].iOp, lhs, value, "temp");
                    }
                }
                return lhs;
            }
            if (cmps.count(name)) {
                if (node->args->args.size() < 2) {
                    return builder.getInt32(0);
                }
                Value *lhs = node->args->args[0]->codegen(this);
                Value *rhs = node->args->args[1]->codegen(this);
                return builder.CreateICmp(cmps[name], lhs, rhs);
            }
        }
        if (node->key.type == KeyType_Sub) {
            auto *find = code->find<ESub>(node->key);
            if (find) {
                std::vector<Value *> args(node->args->args.size());
                int idx = 0;
                for (auto &arg : node->args->args) {
                    args[idx] = node->args->args[idx]->codegen(this);
                    idx++;
                }
                return builder.CreateCall(find->value, args);
            }
        }
        return builder.getInt32(0);
    }
    Value *codegen(ASTLiteral *node) override {
        if (node->value.type == 1) {
            return builder.getInt32(node->value.val_int);
        }
        if (node->value.type == 2) {
            return builder.getInt32(node->value.val_bool);
        }
        if (node->value.type == 3) {
            auto *global = builder.CreateGlobalString(node->value.val_string.toStringRef());
            //auto *data = ConstantDataArray::getString(context.llvm, node->value.val_string.toStringRef(), true);
            //auto *global = new GlobalVariable(*sub->belong->module, data->getType(), true, GlobalValue::PrivateLinkage, data);
            Value *zero = builder.getInt32(0);
            Value *idxs[] = {zero, zero};
            return StringCreate(builder.CreateInBoundsGEP(global, idxs));
        }

        if (node->value.type == 5) {
            return builder.getInt32(node->value.val_double);
        }
        return builder.getInt32(node->value.val_time);
    }
    Value *codegen(ASTVariable *node) override {
        return LoadVar(node->key);
    }
    Value *codegen(ASTDot *node) override {
        Value *value = node->codegenLHS(this);
        return builder.CreateLoad(value);
    }
    Value *StringCreate(Value *ptr) {
        auto *find = context.get<ESub>("String_Create", KeyType_Sub);
        if (find) {
            Value *arg[] = {ptr};
            return builder.CreateCall(find->value, arg);
        }
        //
        return ptr;
    }
};

#endif //ECOMPILER_COMPILER_H
