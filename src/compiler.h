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
    std::map<string, Type *> names = {
            {"char*", IntegerType::getInt8PtrTy(llvm)},
            {"char", IntegerType::getInt8Ty(llvm)},
            {"byte", IntegerType::getInt8Ty(llvm)},
            {"short", IntegerType::getInt16Ty(llvm)},
            {"int", IntegerType::getInt32Ty(llvm)},
            {"int64", IntegerType::getInt64Ty(llvm)},
            {"float", IntegerType::getFloatTy(llvm)},
            {"double", IntegerType::getDoubleTy(llvm)},
            {"bool", IntegerType::getInt1PtrTy(llvm)},
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
            } else {
                if (!code->structs.empty()) {
                    return code->structs[0].type;
                }
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
            auto &&cmt = sub->params[i].comment.toString();
            if (!cmt.empty() && cmt[0] == '#') {
                auto value = json::parse(cmt.c_str() + 1);
                pts[i] = names[value["type"]];
            } else {
                pts[i] = getType(sub->params[i].type);
                if (sub->params[i].isRef()) {
                    pts[i] = pts[i]->getPointerTo(0);
                }
                if (sub->params[i].isArray()) {
                    pts[i] = pts[i]->getPointerTo(0);
                }
            }
        }
        auto *returnType = getType(sub->type);
        if (sub->attr.count("type")) {
            returnType = names[sub->attr["type"]];
        }
        auto *functionType = FunctionType::get(returnType, pts, sub->attr.count("va_args"));
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
    BasicBlock *current = nullptr;
    vector<Value *> frees;
    bool hasReturn = false;
    map<string, string> translator = {
            {"取文本长度", "String_Length"},
    };
    explicit ECompiler(EContext &context, ESub *sub) :
    context(context), builder(context.llvm), sub(sub) {
        code = context.code;
        auto *function = (Function *) sub->value;
        auto *entry = BasicBlock::Create(context.llvm, "entrypoint", function);
        IRBuilder<> lb(entry);
        int i = 0;
        for (auto &arg : function->args()) {
            auto *alloc = lb.CreateAlloca(arg.getType(), nullptr);
            lb.CreateStore(&arg, alloc);
            alloc->setName(sub->params[i].name.toStringRef());
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
                Return(value);
                return;
            }
            if (name == "赋值") {
                if (node->args->args.size() < 2) {
                    return;
                }
                Value *var = node->args->args[0]->codegenLHS(this);
                Value *val = node->args->args[1]->codegen(this);
                int var_bits = var->getType()->getPointerElementType()->getIntegerBitWidth();
                int val_bits = val->getType()->getIntegerBitWidth();
                if (var_bits > val_bits) {
                    val = builder.CreateZExt(val, var->getType()->getPointerElementType());
                }
                if (var_bits < val_bits) {
                    val = builder.CreateTrunc(val, var->getType()->getPointerElementType());
                }
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
        if (!hasReturn) {
            Return();
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
        auto *before = current;
        Value *value = node->condition->codegen(this);
        auto *leave = BasicBlock::Create(context.llvm, "leave");
        auto *then_block = BasicBlock::Create(context.llvm, "then");
        auto *else_block = node->else_block ? BasicBlock::Create(context.llvm, "else") : leave;
        builder.CreateCondBr(value, then_block, else_block);
        builder.SetInsertPoint(then_block);
        if (node->then_block) {
            current = then_block;
            node->then_block->accept(this);
        }
        builder.CreateBr(leave);
        sub->value->getBasicBlockList().push_back(then_block);
        if (node->else_block) {
            current = else_block;
            builder.SetInsertPoint(else_block);
            node->else_block->accept(this);
            sub->value->getBasicBlockList().push_back(else_block);
            builder.CreateBr(leave);
        }
        sub->value->getBasicBlockList().push_back(leave);
        builder.SetInsertPoint(leave);
        current = before;
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
                builder.CreateBr(cond);
                PushBasicBlock(cond);
                builder.SetInsertPoint(cond);
                Value *condition = node->head->args->args[0]->codegen(this);
                builder.CreateCondBr(condition, loop, leave);
            }
            if (name == "计次循环首") {
                counter = node->head->args->args[1]->codegenLHS(this);
                builder.CreateStore(builder.getInt32(0), counter);
                builder.CreateBr(cond);
                PushBasicBlock(cond);
                builder.SetInsertPoint(cond);
                Value *value = node->head->args->args[0]->codegen(this);
                Value *count = builder.CreateLoad(counter);
                Value *cmp = builder.CreateICmp(CmpInst::ICMP_SLT, count, value);
                builder.CreateCondBr(cmp, loop, leave);
            }
            PushBasicBlock(loop);
            builder.SetInsertPoint(loop);
            if (node->block)
                node->block->accept(this);
            if (counter) {
                Value *value = builder.CreateAdd(builder.CreateLoad(counter), builder.getInt32(1));
                builder.CreateStore(value, counter);
            }
            builder.CreateBr(cond);
            if (name == "循环判断首") {
                PushBasicBlock(cond);
                builder.SetInsertPoint(cond);
                Value *condition = node->tail->args->args[0]->codegen(this);
                builder.CreateCondBr(condition, loop, leave);
            }
            PushBasicBlock(leave);
            builder.SetInsertPoint(leave);
        }
        current = before;
    }

    Value *codegenLHS(ASTPostfix *node) override {
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
        if (node->field->getType() == ASTSubscript::Type) {
            auto *field = node->field->cast<ASTSubscript>();
            Value *value = field->value->codegen(this);
            return builder.CreateGEP(var, value);
        }
        return var;
    }
    Value *codegenLHS(ASTVariable *node) override {
        EVar *var = code->find<EVar>(node->key);
        if (!var) {
            var = code->find<EVar>(node->key.value - 2);
        }
        Value *value = var->value;
        if (value) {
            if (var->isRef()) {
                value = builder.CreateLoad(value);
            }
            if (var->isArray()) {
                value = builder.CreateLoad(value);
            }
        }
        return value;
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
                {"不等于", PRED(NE)},
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
                    if (arg->getType() == ASTVariable::Type) {

                    }
                    Value *value = arg->codegen(this);
                    if (lhs == nullptr) {
                        lhs = value;
                    } else {
                        auto *type = lhs->getType();
                        if (type == getType(SDT_INT)) {
                            lhs = builder.CreateBinOp(binary[name].iOp, lhs, value, "temp");
                        } else if (type == getType(SDT_FLOAT)) {
                            lhs = builder.CreateBinOp(binary[name].fOp, lhs, value, "temp");
                        } else if (type == getType(SDT_TEXT)) {
                            lhs = StringAdd(lhs, value);
                        } else {
                            lhs = builder.CreateBinOp(binary[name].iOp, lhs, value, "temp");
                        }
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
                uint32_t lhs_bits = lhs->getType()->getIntegerBitWidth();
                uint32_t rhs_bits = rhs->getType()->getIntegerBitWidth();
                if (lhs_bits > rhs_bits) {
                    rhs = builder.CreateZExt(rhs, lhs->getType());
                }
                if (lhs_bits < rhs_bits) {
                    lhs = builder.CreateZExt(lhs, rhs->getType());
                }
                return builder.CreateICmp(cmps[name], lhs, rhs, "cmp_temp");
            }
            if (translator.count(name)) {
                return CallFunctionArgs(translator[name].c_str(), node->args);
            }
        }
        if (node->key.type == KeyType_Sub) {
            auto *find = code->find<ESub>(node->key);
            if (find) {
                std::vector<Value *> args(node->args->args.size());
                int idx = 0;
                for (auto &arg : node->args->args) {
                    if (find->params[idx].isRef()) {
                        args[idx] = arg->codegenLHS(this);
                    } else {
                        args[idx] = arg->codegen(this);
                    }
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
        EVar *var = code->find<EVar>(node->key);
        if (var == nullptr || var ->value == nullptr) {
            return builder.getInt32(0);
        }
        Value *value = var->value;
        if (var->type == SDT_TEXT) {
            value = CallFunction("String_Get", value);
        } else {
            value = builder.CreateLoad(value, var->name.toString());
        }
        return value;
    }
    Value *codegen(ASTPostfix *node) override {
        Value *value = node->codegenLHS(this);
        return builder.CreateLoad(value);
    }

    template<typename ...Args>
    Value *CallFunction(const char *name, Args ... args) {
        auto *find = context.get<ESub>(name, KeyType_Sub);
        if (find) {
            Value *arg[] = {args...};
            return builder.CreateCall(find->value, arg);
        }
        return builder.getInt32(0);
    }
    Value *CallFunctionArgs(const char *name, ASTArgsPtr args) {
        auto *find = context.get<ESub>(name, KeyType_Sub);
        if (find) {
            vector<Value *> arg_list(args->args.size());
            int index = 0;
            for (auto &arg : args->args) {
                if (find->params[index].isRef()) {
                    arg_list[index] = arg->codegenLHS(this);
                } else {
                    arg_list[index] = arg->codegen(this);
                }
                index++;
            }
            return builder.CreateCall(find->value, arg_list);
        }
        return builder.getInt32(0);
    }
    Value *StringCreate(Value *ptr) {
        return PushFree(CallFunction("String_Create", ptr));
    }
    Value *StringAdd(Value *lhs, Value *rhs) {
        return PushFree(CallFunction("String_Add", lhs, rhs));
    }
    Value *StringFree(Value *value) {
        return CallFunction("String_Free", value);
    }
    Value *PushFree(Value *value) {
        frees.push_back(value);
        return value;
    }
    void PushBasicBlock(BasicBlock *block) {
        sub->value->getBasicBlockList().push_back(block);
    }
    void Return(Value *value = nullptr) {
        for (auto &tofree : frees) {
            if (tofree != value) {
                StringFree(tofree);
            }
        }
        if (current == nullptr) {
            hasReturn = true;
        }
        if (value == nullptr) {
            if (sub->type == 0) {
                builder.CreateRetVoid();
            }
        } else {
            builder.CreateRet(value);
        }


    }
};

#endif //ECOMPILER_COMPILER_H
