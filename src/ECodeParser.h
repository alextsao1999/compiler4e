//
// Created by 曹顺 on 2019/5/3.
//

#ifndef PARSE_E_FILE_ECODEPARSER_H
#define PARSE_E_FILE_ECODEPARSER_H
#include "nlohmann/json.hpp"
using json = nlohmann::json;
namespace llvm {
    class Value;
    class Module;
    class Type;
    class Function;
}
using namespace llvm;
#define assert(condition, str) \
    do { \
    if(!(condition)) \
        std::cout << str;\
    } while(0);
#include <vector>
#include <map>
#include "windows.h"
#include "FileBuffer.h"
#include "tools.h"
#include "ast.h"
#include "lib2.h"
#include <unordered_map>
#define NOT_REACHED() printf("not reached !! %s:%d\n", __FILE__, __LINE__);
#define make_ptr(p, ...) make_shared<p>(__VA_ARGS__)
using namespace std;

ostream &operator<<(ostream &os, Key &key);
ostream &operator<<(ostream &os, FixedData &data);

struct BasicInfo {
    int type{0};
    FixedData name;
    FixedData description;
    FixedData author;
    FixedData code;
    FixedData address;
    FixedData telephone;
    FixedData fox;
    FixedData email;
    FixedData host;
    FixedData copyright;
    int version[2]{0, 0};
    int create[2]{0, 0};
};
struct EBase {
    Key key;
    FixedData name;
    FixedData comment;
    virtual void dump() {
        printf("%s ", name.toString().c_str());
        key.dump();
    }
};
struct EWindow : public EBase {
    // Key key;
    Key belong;
    // FixedData name;
    // FixedData comment;
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;
    FixedData cursor;
    FixedData mark;
    int visible = 0;
    int bidden = 0;
    int border = 0;
    int bgSize = 0;
    int bgColor = 0;
    int maxBtn = 0;
    int minBtn = 0;
    int ctrlBtn = 0;
    int position = 0;
    int movable = 0;
    int musicTimes = 0;
    int enterFocus = 0;
    int escClose = 0;
    int f1Help = 0;
    int helpMark = 0;
    int showInTaskbar = 0;
    int mov = 0; // 随意移动
    int shape = 0;
    int alwaysTop = 0;
    int alwaysActive = 0;
    FixedData className;
    FixedData title;
    FixedData helpFileName;
    int number{0};
    EWindow() = default;
};

struct ELibConst {
    int lib; // lib index
    int index; // constant index
};

struct EConst : public EBase {
    // Key key;
    short property = 0;
    // FixedData name;
    // FixedData comment;
    FixedData data;
    EValue value;
};

struct EVar : public EBase {
    // Key key;
    Key type;
    uint16_t property = 0;
    // FixedData name;
    // FixedData comment;
    std::vector<int> dimension;
    Value *value = nullptr;
    inline bool isRef() { return (property & Property_Ref) == Property_Ref; }
    inline bool isArray() { return (property & Property_Array) == Property_Array; }
    inline bool isStatic() { return (property & Property_Static) == Property_Static; }
    inline bool isPublic() { return (property & Property_Public) == Property_Public; }
    inline bool isNullable() { return (property & Property_Nullable) == Property_Nullable; }

    void dump() override {
        if (isPublic())
            printf("Public ");
        if (isRef())
            printf("Ref ");
        if (isStatic())
            printf("Static");
        if (isArray())
            printf("[]");
        EBase::dump();
    }
};

struct ELibrary {
    FixedData path;
    HMODULE hModule = nullptr;
    PLIB_INFO info = nullptr;

};

struct EModule : public EBase {
    // Key key;
    int property;
    Key base;
    // FixedData name;
    // FixedData comment;
    std::vector<Key> include;
    std::vector<EVar> vars;
    Module *module = nullptr;
    inline bool has(Key test) {
        for (auto & i : include) {
            if (test.value == i.value) {
                return true;
            }
        }
        return false;
    }
};

struct ESub : public EBase {
    // Key key;
    int property = 0;
    int type = 0;
    // FixedData name;
    // FixedData comment;
    FixedData code[6];
    std::vector<EVar> params;
    std::vector<EVar> locals;
    EModule *belong = nullptr; // 所属模块
    ASTProgramPtr ast = nullptr;
    Function *value = nullptr;
    json attr;
};

struct EStruct : public EBase {
    // Key key;
    int property = 0;
    // FixedData name;
    // FixedData comment;
    std::vector<EVar> members;
    Type *type = nullptr;
    json attr;
};

struct EDllSub : public EBase {
    // Key key;
    int property = 0;
    int type = 0;
    // FixedData name;
    // FixedData comment;
    FixedData lib;
    FixedData func;
    std::vector<EVar> params;
};

struct ECode {
    BasicInfo info; // 源码信息
    std::vector<EWindow> windows; //窗口
    std::vector<EConst> constants; //常量
    std::vector<ELibrary> libraries; //支持库
    std::vector<EModule> modules; // 程序集/类
    std::vector<ESub> subs; // 所有子程序
    std::vector<EVar> globals; // 全局变量
    std::vector<EStruct> structs; // 自定义数据类型
    std::vector<EDllSub> dlls; // dll
    std::unordered_map<int, EBase *> maps;
    template <typename Type>
    inline Type *find(Key key) {
        return (Type *) maps[key.value];
    }
    template <typename Type>
    inline Type *find(int value) {
        return (Type *) maps[value];
    }
    void free() {
        for (auto & librarie : libraries) {
            FreeLibrary(librarie.hModule);
        }
    }
};

static char seg_start[4] = {0x19, 0x73, 0x11, 0x15};

static char sec_start[2] = {0x19, 0x73};

class ECodeParser {
public:
    FileBuffer &_buffer;
    ECode code{};
    int _check{0};
    char *_eLibPath;

    explicit ECodeParser(FileBuffer &buf) : _buffer(buf), _eLibPath(GetLibPath()) {}
    ~ECodeParser() {
        delete[]_eLibPath;
    }
    bool Check(const char *check, size_t length);
    bool Cmp(const char *check, size_t length);
    void SetElibPath(char *path);
    void Parse();
    ECode &GetECode() {
        return code;
    }
private:

    bool CheckSegment(int num);
    void SkipSegment();
    Key ParseKey();
    void ParseCodeSegement();
    void ParseLibrary();
    void ParseModule();
    void ParseInfoSegement(int arg);
    void ParseWindow();
    void ParseResourceSegement();
    void ParseConstant();
    void ParseVariable(std::vector<EVar> &vars);
    void ParseSub();
    void ParseDataStruct();
    void ParseDll();
    void ParseAST();
    EValue ParseValue(FileBuffer &buf, uint8_t type);
    ASTProgramPtr ParseSubCode(FileBuffer &buf);;
    ASTNodePtr ParseLineNode(FileBuffer &buf, uint8_t type);
    ASTIfStmtPtr ParseIf(FileBuffer &buf);
    ASTIfStmtPtr ParseIfTrue(FileBuffer &buf);
    ASTJudgePtr ParseJudge(FileBuffer &buf);
    ASTLoopPtr ParseLoop(FileBuffer &buf);
    ASTFunCallPtr ParseFunCall(FileBuffer &buf);
    ASTNodePtr ParseNode(FileBuffer &buf, uint8_t type);
    ASTArgsPtr ParseArgs(FileBuffer &buf);

};


#endif //PARSE_E_FILE_ECODEPARSER_H
