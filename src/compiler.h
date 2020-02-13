//
// Created by Alex on 2020/2/6.
//

#ifndef ECOMPILER_COMPILER_H
#define ECOMPILER_COMPILER_H

#include <set>
#include "visitor.h"
#include "test.h"
#include "ECodeParser.h"
#include "ast.h"
#include "llvm/IR/IRBuilder.h"

struct EContext {
    ECode *code = nullptr;
    LLVMContext llvm;
    Module *dlls;
    Value *bool_true = ConstantInt::getTrue(llvm);
    Value *bool_false = ConstantInt::getFalse(llvm);
    std::map<int, Type *> types = {
            {SDT_BYTE,      IntegerType::getInt8Ty(llvm)},
            {SDT_SHORT,     IntegerType::getInt16Ty(llvm)},
            {SDT_INT,       IntegerType::getInt32Ty(llvm)},
            {SDT_INT64,     IntegerType::getInt64Ty(llvm)},
            {SDT_FLOAT,     IntegerType::getFloatTy(llvm)},
            {SDT_DOUBLE,    IntegerType::getDoubleTy(llvm)},
            {SDT_BOOL,      IntegerType::getInt8Ty(llvm)},
            {SDT_DATE_TIME, IntegerType::getDoubleTy(llvm)},
            {SDT_TEXT,      IntegerType::getInt8PtrTy(llvm)},
            {SDT_BIN,       IntegerType::getInt8PtrTy(llvm)},
            {SDT_SUB_PTR,   IntegerType::getInt8PtrTy(llvm)},
            {SDT_STATMENT,  IntegerType::getInt8PtrTy(llvm)},
    };

    std::map<string, Type *> names = {
            {"char*", IntegerType::getInt8PtrTy(llvm)},
            {"size", IntegerType::getInt64Ty(llvm)},
            {"char", IntegerType::getInt8Ty(llvm)},
            {"byte", IntegerType::getInt8Ty(llvm)},
            {"short", IntegerType::getInt16Ty(llvm)},
            {"int", IntegerType::getInt32Ty(llvm)},
            {"int64", IntegerType::getInt64Ty(llvm)},
            {"float", IntegerType::getFloatTy(llvm)},
            {"double", IntegerType::getDoubleTy(llvm)},
            {"bool", IntegerType::getInt1Ty(llvm)},
    };
    std::map<string, string> translator;
    explicit EContext(ECode *code) : code(code) {
        dlls = new Module("DllImport", llvm);
        for (auto &estruct : code->structs) {
            CreateStruct(&estruct);
        }
        for (auto &dll : code->dlls) {
            CreateDll(&dll);
        }
        for (auto &module : code->modules) {
            module.module = dlls;//new Module(module.name.toStringRef(), llvm);
            for (auto &sub : module.include) {
                auto *find = code->find<ESub>(sub);
                if (find) {
                    find->belong = &module;
                    CreateFunction(find);
                }
            }
        }
        CreateMain();
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
        std::vector<Type *> elements(estruct->members.size());
        for (int i = 0; i < estruct->members.size(); ++i) {
            estruct->members[i].value.ref = estruct->members[i].isRef() + estruct->members[i].isArray();
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
                    // TODO 数组维度
                    //elements[i] = ArrayType::get(elements[i], value);
                }
            }
        }
        auto *value = StructType::create(llvm, elements, estruct->name.toStringRef());
        //value->setName(estruct->name.toStringRef());
        estruct->type = value;
        std::string &&comment = estruct->comment.toString();
        if (!comment.empty()) {
            if (comment[0] == '#') {
                estruct->attr = json::parse(comment.c_str() + 1);
            }
            if (comment[0] == '@') {
                if (comment == "@String") {
                    types[SDT_TEXT] = value;
                }
                if (comment == "@All") {
                    types[_SDT_ALL] = value;
                }
            }
        }
    }
    void CreateFunction(ESub *sub) {
        std::string &&comment = sub->comment.toString();
        if (!comment.empty()) {
            if (comment[0] == '#') {
                sub->attr = json::parse(comment.c_str() + 1);
            }
            if (comment[0] == '@') {
                translator.emplace(comment.c_str() + 1, sub->name.toString());
            }
        }
        std::string &&name = sub->name.toString();
        if (name == "_启动子程序") {
            name.assign("start_main");
        }
        std::vector<Type *> pts(sub->params.size());
        for (uint32_t i = 0; i < sub->params.size(); ++i) {
            auto &&cmt = sub->params[i].comment.toString();
            sub->params[i].value.ref = sub->params[i].isRef() + sub->params[i].isArray();
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
        sub->retType = getType(sub->type);
        int ref = sub->comment.count('*');
        for (int j = 0; j < ref; ++j) {
            sub->retType = sub->retType->getPointerTo();
        }
        if (sub->attr.count("type")) {
            sub->retType = names[sub->attr["type"]];
        }
        auto *functionType = FunctionType::get(sub->retType, pts, sub->attr.count("va_args"));
        Function *function = Function::Create(functionType, Function::ExternalLinkage, name, sub->belong->module);
        //function->addFnAttr(Attribute::AttrKind::Alignment);
        sub->value = function;
        SetSubSignature(sub);
    }
    void CreateDll(EDllSub *dll) {
        std::string &&comment = dll->comment.toString();
        bool isVarArgs = false;
        if (comment.back() == '.') {
            comment.pop_back();
            isVarArgs = true;
        }
        if (!comment.empty()) {
            if (comment[0] == '@') {
                translator.emplace(comment.c_str() + 1, dll->name.toString());
            }
        }
        std::vector<Type *> pts(dll->params.size());
        for (uint32_t i = 0; i < dll->params.size(); ++i) {
            dll->params[i].value.ref = dll->params[i].isRef() + dll->params[i].isArray();
            auto &&cmt = dll->params[i].comment.toString();
            if (!cmt.empty() && cmt[0] == '#') {
                auto value = json::parse(cmt.c_str() + 1);
                pts[i] = names[value["type"]];
            } else {
                if (dll->params[i].type == SDT_TEXT) {
                    pts[i] = IntegerType::get(llvm, 8);
                } else {
                    pts[i] = getType(dll->params[i].type);
                }
            }
            for (uint32_t j = 0; j < dll->params[i].value.ref; ++j) {
                pts[i] = pts[i]->getPointerTo(0);
            }
        }
        auto *ret = getType(dll->type);
        int ref = dll->comment.count('*');
        for (int k = 0; k < ref; ++k) {
            ret = ret->getPointerTo();
        }
        auto *type = FunctionType::get(ret, pts, isVarArgs);
        auto *func = Function::Create(type, Function::ExternalLinkage, dll->func.toString(), dlls);
        dll->value = func;
    }
    static void SetSubSignature(ESub *sub) {
        sub->signature = sub->name.toString();
        sub->signature.append("(");
        size_t i = 0;
        for (auto &arg : sub->value->args()) {
            i++;
            sub->signature.append(GetTypeString(arg.getType()));
            if (i < sub->value->arg_size()) {
                sub->signature.append(",");
            }
        }
        sub->signature.append(")");
        //printf("func sig -> %s\n", sub->signature.c_str());
    }
    static string GetTypeString(Type *type) {
        if (type == nullptr) {
            return {};
        }
        std::string type_str;
        llvm::raw_string_ostream rso(type_str);
        type->print(rso, false, true);
        return rso.str();
    }
    void CreateMain() {
/*
        auto *i8ptr = IntegerType::getInt8PtrTy(llvm);
        auto *i32 = IntegerType::getInt32Ty(llvm);
        std::vector<Type *> pts = {i8ptr, i8ptr, i8ptr, i32};
        auto *type = FunctionType::get(i32, pts, false);
        Function *func = Function::Create(type, Function::ExternalLinkage, "start_e", dlls);
        auto *entry = BasicBlock::Create(llvm, "entrypoint", func);
        IRBuilder<> builder(entry);
        auto *sub = get<ESub>("_启动子程序", KeyType_Sub);
        Value *ret = builder.CreateCall(sub->value);
        builder.CreateRet(ret);
*/
    }
};
class ECompiler : public Visitor {
public:
    ECode *code;
    ESub *sub;
    IRBuilder<> builder;
    EContext& context;
    BasicBlock *current = nullptr;
    vector<EValue> frees;
    DumpVisitor dumper;
    bool hasReturn = false;

    explicit ECompiler(EContext &context, ESub *sub) :
            context(context), builder(context.llvm), sub(sub), dumper(context.code, sub) {
        code = context.code;
        auto *function = sub->value;
        auto *entry = BasicBlock::Create(context.llvm, "entrypoint", function);
        builder.SetInsertPoint(entry);
        int i = 0;
        for (auto &arg : function->args()) {
            auto *alloc = builder.CreateAlloca(arg.getType(), nullptr);
            //builder.CreateStore(&arg, alloc);
            sub->params[i].value.value = alloc;
            alloc->setName(sub->params[i].name.toStringRef());
            i++;
        }
        i = 0;
        for (auto &arg : function->args()) {
            builder.CreateStore(&arg, sub->params[i].value);
            i++;
        }
        for (auto &local : sub->locals) {
            EValue array = nullptr;
            if (!local.dimension.empty()) {
                array = builder.getInt32(local.dimension[0]);
            }
            auto *type = getType(local.type);
            int ref = local.comment.count('*');
            for (int j = 0; j < ref; ++j) {
                type = type->getPointerTo();
            }
            local.value.value = builder.CreateAlloca(type, array, local.name.toString());
            //LocalValueInit(local.value);
        }
        for (auto &local : sub->locals) {
            LocalValueInit(local.value);
        }

    }

    Type *getType(Key key) {
        return context.getType(key);
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
        EValue value = node->condition->codegen(this);
        auto *leave = BasicBlock::Create(context.llvm, "leave");
        auto *then_block = BasicBlock::Create(context.llvm, "then");
        auto *else_block = node->else_block ? BasicBlock::Create(context.llvm, "else") : leave;
        builder.CreateCondBr(value, then_block, else_block);
        PushBasicBlock(then_block);
        builder.SetInsertPoint(then_block);
        if (node->then_block) {
            current = then_block;
            node->then_block->accept(this);
        }
        builder.CreateBr(leave);

        if (node->else_block) {
            current = else_block;
            PushBasicBlock(else_block);
            builder.SetInsertPoint(else_block);
            node->else_block->accept(this);
            builder.CreateBr(leave);
        }
        PushBasicBlock(leave);
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
        EValue counter = nullptr;
        current = leave;
        if (node->head->lib >= 0) {
            auto &cmd = code->libraries[node->head->lib].info->m_pBeginCmdInfo[node->head->key.value];
            std::string name((char *) cmd.m_szName);
            if (name == "判断循环首") {
                builder.CreateBr(cond);
                PushBasicBlock(cond);
                builder.SetInsertPoint(cond);
                EValue condition = node->head->args->args[0]->codegen(this);
                builder.CreateCondBr(condition, loop, leave);
            }
            if (name == "计次循环首") {
                counter = node->head->args->args[1]->codegenLHS(this);
                builder.CreateStore(builder.getInt32(0), counter);
                builder.CreateBr(cond);
                PushBasicBlock(cond);
                builder.SetInsertPoint(cond);
                EValue value = node->head->args->args[0]->codegen(this);
                EValue count = builder.CreateLoad(counter);
                EValue cmp = builder.CreateICmp(CmpInst::ICMP_SLT, count, value);
                builder.CreateCondBr(cmp, loop, leave);
            }
            PushBasicBlock(loop);
            builder.SetInsertPoint(loop);
            if (node->block)
                node->block->accept(this);
            if (counter) {
                EValue value = builder.CreateAdd(builder.CreateLoad(counter), builder.getInt32(1));
                builder.CreateStore(value, counter);
            }
            builder.CreateBr(cond);
            if (name == "循环判断首") {
                PushBasicBlock(cond);
                builder.SetInsertPoint(cond);
                EValue condition = node->tail->args->args[0]->codegen(this);
                builder.CreateCondBr(condition, loop, leave);
            }
            PushBasicBlock(leave);
            builder.SetInsertPoint(leave);
        }
        current = before;
    }
    void visit(ASTFunCall *node) override {
        if (node->key.value == 0) {
            return;
        }
        if (node->lib >= 0) {
            auto &cmd = code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value];
            std::string name((char *) cmd.m_szName);
            if (name == "返回") {
                if (node->args->args.empty()) {
                    Return();
                } else {
                    EValue value = node->args->args[0]->codegen(this);
                    Return(value);
                }
                return;
            }
            if (name == "赋值") {
                if (node->args->args.size() < 2) {
                    return;
                }
                Type *var_type = node->args->args[0]->type(this);
                Type *val_type = node->args->args[1]->type(this);
                if (var_type == builder.getInt8PtrTy()) {
                    if (val_type == builder.getInt8PtrTy()) {
                        EValue var = node->args->args[0]->codegenLHS(this);
                        EValue val = node->args->args[1]->codegenLHS(this);
                        if (val == nullptr) {
                            val = node->args->args[1]->codegen(this);
                        }
                        builder.CreateStore(val, var);
                        return;
                    }
                }
                EValue var = node->args->args[0]->codegenLHS(this);
                EValue val = node->args->args[1]->codegen(this);
                if (var_type == getType(SDT_TEXT)) {
                    StringCopy(var, val);
                    return;
                }
                builder.CreateStore(TypeCast(val, var_type), var);
                return;
            }
        }
        node->codegen(this);
    }

    EValue codegenLHS(ASTPostfix *node) override {
        EValue var = node->var->codegenLHS(this);
        if (node->field->getType() == ASTStructMember::Type) {
            auto *field = node->field->cast<ASTStructMember>();
            auto *find = context.code->find<EStruct>(field->key);
            for (int i = 0; i < find->members.size(); ++i) {
                if (find->members[i].key == field->member) {
                    EValue value = find->members[i].value;
                    value.value = builder.CreateStructGEP(var, i, {"addr.", find->members[i].name.toStringRef()});
                    return value;
                }
            }
        }
        if (node->field->getType() == ASTSubscript::Type) {
            auto *field = node->field->cast<ASTSubscript>();
            EValue value = field->value->codegen(this);
            return builder.CreateGEP(var, value, "addr");
        }
        return var;
    }
    EValue codegenLHS(ASTVariable *node) override {
        EVar *var = code->find<EVar>(node->key);
        if (!var) {
            if (!sub->params.empty()) {
                var = &sub->params.front();
            }
            if (!sub->locals.empty()) {
                var = &sub->locals.front();
            }
            ASSERT(var, "cannot find var\n");
        }
        EValue value = var->value;
        for (uint32_t i = 0; i < value.getRef(); ++i) {
            value.value = builder.CreateLoad(value.value);
        }
        return value;
    }
    EValue codegen(ASTFunCall *node) override {
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
/*
                node->accept(&dumper);
                printf("  ");
*/
                EValue lhs = nullptr;
                for (auto & arg : node->args->args) {
                    Value *value = nullptr;
                    auto *type = arg->type(this);
                    value = arg->codegen(this);
                    if (type == builder.getInt8PtrTy()) {
                        value = TypeCast(value, getType(SDT_TEXT)->getPointerTo());
                    }
                    if (lhs == nullptr) {
                        lhs = value;
                        continue;
                    }
                    type = value->getType();
                    if (type == getType(SDT_INT)) {
                        lhs = builder.CreateBinOp(ops.iOp, lhs, value, "temp");
                    } else if (type == getType(SDT_FLOAT)) {
                        lhs = builder.CreateBinOp(ops.fOp, lhs, value, "temp");
                    } else if (type == getType(SDT_TEXT)->getPointerTo()) {
                        lhs = StringAdd(lhs, value);
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
                EValue lhs = node->args->args[0]->codegen(this);
                EValue rhs = node->args->args[1]->codegen(this);
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
            if (context.translator.count(name)) {
                return CreateCall(context.translator[name].c_str(), node->args.get());
            }
        }
        if (node->key.type == KeyType_Sub) {
            auto *find = code->find<ESub>(node->key);
            if (find) {
                return CreateCall(find->value, node->args.get());
            }
        }
        if (node->key.type == KeyType_DllFunc) {
            auto *find = code->find<EDllSub>(node->key);
            if (find) {
                return CreateCall(find->value, node->args.get(), true);
            }
        }
        return builder.getInt32(0);
    }
    EValue codegen(ASTLiteral *node) override {
        if (node->value.type == 1) {
            return builder.getInt32(node->value.val_int);
        }
        if (node->value.type == 2) {
            return node->value.val_bool ? context.bool_true : context.bool_false;
        }
        if (node->value.type == 3) {
            auto *global = builder.CreateGlobalString(node->value.val_string.toStringRef());
            //auto *data = ConstantDataArray::getString(context.llvm, node->value.val_string.toStringRef(), true);
            //auto *global = new GlobalVariable(*sub->belong->module, data->getType(), true, GlobalValue::PrivateLinkage, data);
            Value *zero = builder.getInt32(0);
            Value *idxs[] = {zero, zero};
            return builder.CreateInBoundsGEP(global, idxs);
        }
        if (node->value.type == 5) {
            return builder.getInt32(node->value.val_double);
        }
        return builder.getInt32(node->value.val_time);
    }
    EValue codegen(ASTVariable *node) override {
        EValue addr = node->codegenLHS(this);
        if (node->type(this)->isStructTy()) {
            return addr;
        }
        if (node->type(this)->isPointerTy()) {
            return addr;
        }
        return builder.CreateLoad(addr);
    }
    EValue codegen(ASTPostfix *node) override {
        EValue value = node->codegenLHS(this);
        return builder.CreateLoad(value.value);
    }

    Type *type(ASTFunCall *node) override {
        if (node->lib >= 0) {
            auto &cmd = code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value];
            std::string name((char *) cmd.m_szName);
            if (cmd.m_dtRetValType) {
                return getType(cmd.m_dtRetValType);
            }
            return node->args->args[0]->type(this);
        }
        if (node->key.type == KeyType_Sub) {
            auto *find = code->find<ESub>(node->key);
            if (find) {
                return find->retType;
            }
        }
        if (node->key.type == KeyType_DllFunc) {
            auto *find = code->find<EDllSub>(node->key);
            if (find) {
                return getType(find->type);
            }
        }

        return builder.getVoidTy();
    }
    Type *type(ASTLiteral *node) override {
        if (node->value.type == 3) {
            return builder.getInt8PtrTy();;
        }
        return builder.getInt32Ty();
    }
    Type *type(ASTAddress *node) override {
        return Visitor::type(node);
    }
    Type *type(ASTVariable *node) override {
        EVar *var = code->find<EVar>(node->key);
        if (!var) {
            for (auto &local : sub->locals) {
                if ((local.key.value | node->key.value) == node->key.value) {
                    var = &local;
                }
            }
            ASSERT(var, "Cannot find the local!");
        }
        if (var->value.getRef()) {
            //return var->value->getType(); // 引用类型返回 指针类型
        }
        return var->value->getType()->getPointerElementType();
    }
    Type *type(ASTPostfix *node) override {
        if (node->field->getType() == ASTStructMember::Type) {
            auto *field = node->field->cast<ASTStructMember>();
            auto *find = context.code->find<EStruct>(field->key);
            for (int i = 0; i < find->members.size(); ++i) {
                if (find->members[i].key == field->member) {
                    return find->type->getStructElementType(i);
                }
            }
        }
        if (node->field->getType() == ASTSubscript::Type) {
            return node->var->type(this)->getPointerElementType();
        }
        return nullptr;
    }

    template<typename ...Args>
    EValue CallFunction(const char *name, Args ... args) {
        Value *arg_list[] = {args...};
        for (auto &dll : code->dlls) {
            if (dll.name == name) {
                return PushFree(builder.CreateCall(dll.value, arg_list));
            }
        }
        for (auto &fun : code->subs) {
            if (fun.name == name) {
                return PushFree(builder.CreateCall(fun.value, arg_list));
            }
        }
        return builder.getInt32(0);
    }
    EValue CreateCall(const char *name, ASTArgs *args) {
        for (auto &dll : code->dlls) {
            if (dll.name == name) {
                return CreateCall(dll.value, args, true);
            }
        }
        for (auto &fun : code->subs) {
            if (fun.name == name) {
                return CreateCall(fun.value, args);
            }
        }
        return builder.getInt32(0);
    }
    EValue CreateCall(Function *func, vector<EVar> &params, ASTArgs *args) {
        if (func) {
            vector<Value *> arg_list(args->args.size());
            int index = 0;
            for (auto &arg : args->args) {
                Type *arg_type = (func->arg_begin() + index)->getType();
                arg_list[index] = TypeCast(arg->codegen(this), arg_type);
                index++;
            }
            return PushFree(builder.CreateCall(func, arg_list));
        }
        return builder.getInt32(0);
    }
    EValue CreateCall(Function *func, ASTArgs *args, bool isDll = false) {
        if (func) {
            vector<Value *> arg_list(args->args.size());
            int index = 0;
            size_t arg_size = func->arg_size();
            for (auto &arg : args->args) {
                if (index < arg_size) {
                    Type *arg_type = (func->arg_begin() + index)->getType();
                    arg_list[index] = TypeCast(arg->codegen(this), arg_type);
                } else {
                    arg_list[index] = arg->codegen(this);
                }
                if (isDll) {
                    if (arg_list[index]->getType() == getType(SDT_TEXT)->getPointerTo()) {
                        arg_list[index] = TypeCast(arg_list[index], builder.getInt8PtrTy());
                    }
                }
                index++;
            }
            return PushFree(builder.CreateCall(func, arg_list));
        }
        return builder.getInt32(0);
    }
    Value *TypeCast(EValue value, Type *cast_to) {
        Type *type = value->getType();
        if (type == cast_to) {
            return value;
        }
        if (type == cast_to->getPointerTo()) {
            return builder.CreateLoad(value);
        }
        if (type == getType(SDT_TEXT)->getPointerTo()) {
            if (cast_to == builder.getInt8PtrTy()) {
                return CallFunction("String_CStr", value);
            }
            if (cast_to == getType(SDT_TEXT)) {
                return builder.CreateLoad(value);
            }
        }
        if (type == builder.getInt8PtrTy()) {
            if (cast_to == getType(SDT_TEXT)) {
                return builder.CreateLoad(StringCreate(value));
            }
            if (cast_to == getType(SDT_TEXT)->getPointerTo()) {
                return StringCreate(value);
            }
        }
        uint32_t bits = type->getIntegerBitWidth();
        uint32_t to_bits = cast_to->getIntegerBitWidth();
        if (bits < to_bits) {
            value.value = builder.CreateZExt(value, cast_to);
        }
        if (bits > to_bits) {
            value.value = builder.CreateTrunc(value, cast_to);
        }
        if (bits == to_bits) {
            value.value = builder.CreateBitCast(value, cast_to);
        }
        return value.value;
    }
    EValue StringCreate(EValue ptr) {
        return CallFunction("String_Create", ptr);
    }
    void StringCopy(EValue lhs, EValue rhs) {
        if (rhs->getType() == builder.getInt8PtrTy()) {
            rhs = StringCreate(rhs);
        }
        CallFunction("String_Copy", lhs, rhs);
    }
    EValue StringAdd(EValue lhs, EValue rhs) {
        return CallFunction("String_Add", lhs, rhs);
    }
    EValue StringFree(EValue value) {
        return CallFunction("String_Free", value);
    }
    void LocalValueInit(EValue value) {
        Type *type = value->getType()->getPointerElementType();
        if (type == getType(SDT_INT)) {
            builder.CreateStore(builder.getInt32(0), value);
        }
        if (type == getType(SDT_TEXT)) {
            CallFunction("String_Init", value);
        }
    }
    EValue PushFree(EValue value) {
        Type *type = value->getType();
        if (type == getType(SDT_TEXT)) {
            Value *alloc = builder.CreateAlloca(type, nullptr, "string_temp");
            builder.CreateStore(value, alloc);
            frees.push_back(alloc);
            return alloc;
        }
        return value;
    }
    void PushBasicBlock(BasicBlock *block) {
        sub->value->getBasicBlockList().push_back(block);
    }
    void Return(EValue value = nullptr) {
        for (auto &tofree : frees) {
            if (tofree != value) {
                StringFree(tofree);
            }
        }
        for (auto &local : sub->locals) { //释放局部变量
            if (local.type == SDT_TEXT) {
                StringFree(local.value);
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
            Value *ret = TypeCast(value, sub->retType);

            builder.CreateRet(ret);

        }

    }
};

#endif //ECOMPILER_COMPILER_H
