//
// Created by 曹顺 on 2019/5/5.
//

#ifndef PARSE_E_FILE_AST_H
#define PARSE_E_FILE_AST_H

#include <vector>
#include <memory>
#include "visitor.h"
#include "FileBuffer.h"
namespace llvm {
    class Value;
}
using namespace llvm;
#define AST_DECL() void accept(Visitor *visitor) override {visitor->enter(this);visitor->visit(this);visitor->leave(this);} \
Value *codegen(Visitor *visitor) override { return visitor->codegen(this); }\
Value *codegenLHS(Visitor *visitor) override { return visitor->codegenLHS(this); }
#define AST_NODE(ast) struct AST##ast;using AST##ast##Ptr = std::shared_ptr<AST##ast>;struct AST##ast : public ASTNode

enum Property : uint16_t {
    Property_Static = 1,
    Property_Ref = 2,
    Property_Nullable = 4,
    Property_Array = 8,
    Property_Public = 256,
};

enum KeyType : uint8_t {
    KeyType_None = 0,
    KeyType_Sub = 4,
    KeyType_Global = 5,
    KeyType_Program = 9,
    KeyType_DllFunc = 10,
    KeyType_ProgramVar = 21,
    KeyType_Control = 22,
    KeyType_Constant = 24,
    KeyType_WindowProgram = 25,
    KeyType_LocalOrParam = 37,
    KeyType_ImageRes = 40,
    KeyType_DataStructureMember = 53,
    KeyType_SoundRes = 56,
    KeyType_DataStruct = 65,
    KeyType_DllFunParam = 69,
    KeyType_Module = 73,
    KeyType_WindowRes = 82,
    KeyType_BuildIn = 128,
};

struct Key {
    union {
        struct {
            uint16_t index;
            uint8_t code; // 不知道这个是干啥的
            KeyType type;
        };
        uint32_t value;
        uint8_t part[4];
    };

    Key(int value) : value(value) {}
    Key() : value(0) {}
    inline bool operator==(const int &rhs) { return value == rhs; }
    inline bool operator==(const Key &rhs) { return value == rhs.value; }
    inline bool operator<(const Key &rhs) { return value < rhs.value; }
    void dump() {
        printf("%u<%d, %d, %d>\n", value, type, code, index);
    }
};

struct EValue {
    int type{0};
    union {
        int val_int{};
        double val_double;
        bool val_bool;
        long long val_time;
        FixedData val_string;
    };
    explicit EValue() : type(0), val_int(0) {}
    explicit EValue(int value) : type(1), val_int(value) {}
    explicit EValue(bool value) : type(2), val_bool(value) {}
    explicit EValue(FixedData value) : type(3), val_string(value) {
        val_string.length--;
    }
    explicit EValue(long long value) : type(4), val_time(value) {}
    explicit EValue(double value) : type(5), val_double(value) {}

};

struct ASTNode {
    ASTNode() = default;
    virtual void accept(Visitor *visitor) {
        visitor->visit(this);
    };
    virtual Value *codegen(Visitor *visitor) = 0;
    virtual Value *codegenLHS(Visitor *visitor) = 0;
    virtual int getType() { return 0; }
    template <typename Class>
    inline Class *cast() {
        if (getType() != Class::Type) {
            return nullptr;
        }
        return (Class *) this;
    }
};
using ASTNodePtr = std::shared_ptr<ASTNode>;
#define AST_TYPE(value) int getType() override { return value; };static constexpr int Type = value;
AST_NODE(Args) {
    std::vector<ASTNodePtr> args;
    inline void AddArg(const ASTNodePtr &node) {
        args.push_back(node);
    }
    AST_DECL();
    AST_TYPE(1);

};

AST_NODE(Block) {
    std::vector<ASTNodePtr> element;
    inline void AddStmt(const ASTNodePtr &node) {
        element.push_back(node);
    }
    AST_DECL();
    AST_TYPE(2);
};

AST_NODE(FunCall) {
    Key key;
    short lib;
    short unknown;
    FixedData object;
    FixedData comment;
    ASTArgsPtr args;
    AST_DECL();
    AST_TYPE(3);

};

AST_NODE(Program) {
    std::vector<ASTNodePtr> stmts;
    inline void AddStmt(const ASTNodePtr &node) {
        stmts.push_back(node);
    }

    AST_DECL();
    AST_TYPE(4);

};

AST_NODE(IfStmt) {
    ASTNodePtr condition;
    ASTBlockPtr then_block;
    ASTBlockPtr else_block;
    AST_DECL();
    AST_TYPE(5);

};

AST_NODE(Literal) {
    EValue value;
    explicit ASTLiteral(const EValue &value) : value(value) {}
    AST_DECL();
    AST_TYPE(6);
};

AST_NODE(Constant) {
    Key key;
    explicit ASTConstant(const int &key) : key(key) {}
    AST_DECL();
    AST_TYPE(7);
};

AST_NODE(LibConstant) {
    short index;
    short member;
    explicit ASTLibConstant(int code) : index((short) (code & 0xff - 1)), member((short) ((code >> 16) - 1)) {}
    AST_DECL();
    AST_TYPE(8);

};

AST_NODE(Address) {
    Key key;
    explicit ASTAddress(const int &key) : key(key) {}
    AST_DECL();
    AST_TYPE(9);

};

AST_NODE(Subscript) {
    ASTNodePtr value;
    explicit ASTSubscript(ASTNodePtr value) : value(std::move(value)) {}
    AST_DECL();
    AST_TYPE(10);

};

AST_NODE(EnumConstant) {
    Key key;
    Key index;
    ASTEnumConstant(int key, int index) : key(key), index(index) {}
    AST_DECL();
    AST_TYPE(11);

};

AST_NODE(StructMember) {
    Key key;
    Key member;
    ASTStructMember(int key, int member) : member(member), key(key) {}
    AST_DECL();
    AST_TYPE(12);

};

AST_NODE(Variable) {
    Key key;
    explicit ASTVariable(Key key) : key(key) {}
    AST_DECL();
    AST_TYPE(13);

};

AST_NODE(Postfix) {
    ASTNodePtr var;
    ASTNodePtr field;
    ASTPostfix(ASTNodePtr var, ASTNodePtr field) : var(std::move(var)), field(std::move(field)) {}
    AST_DECL();
    AST_TYPE(14);

};

AST_NODE(Judge) {
    std::vector<ASTNodePtr> conditions;
    std::vector<ASTNodePtr>  blocks;
    ASTBlockPtr default_block;
    AST_DECL();
    AST_TYPE(15);

};

AST_NODE(Loop) {
    ASTFunCallPtr head;
    ASTBlockPtr block;
    ASTFunCallPtr tail;
    AST_DECL();
    AST_TYPE(16);

};

AST_NODE(Brace) {
    ASTArgsPtr args;
    ASTBrace(ASTArgsPtr args) : args(std::move(args)) {}
    AST_DECL();
    AST_TYPE(17);

};


#endif //PARSE_E_FILE_AST_H
