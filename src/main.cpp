//
// Created by Alex on 2020/2/6.
//
#include "compiler.h"
#include "test.h"
int main() {
    FileBuffer buffer(R"(C:\Users\Administrator\Desktop\a.e)");
    ECodeParser parser(buffer);
    parser.Parse();
    EContext context(&parser.code);
    for (auto &estruct : parser.code.structs) {
        ECompiler::CreateStruct(context, &estruct);
    }
    for (auto &module : parser.code.modules) {
        module.module = new Module(module.name.toString(), context.llvm);
        for (auto &sub : module.include) {
            auto *find = parser.code.find<ESub>(sub);
            if (find) {
                find->belong = &module;
                ECompiler::CreateFunction(context, find);
            }
        }
    }
    for (auto &module : parser.code.modules) {
        for (auto &sub : module.include) {
            auto *find = parser.code.find<ESub>(sub);
            if (find) {
                DumpVisitor dump(&parser.code, find);
                find->ast->accept(&dump);

                ECompiler compiler(context, find);
                find->ast->accept(&compiler);

            }
        }
        module.module->print(llvm::outs(), nullptr);
    }

    return 0;
}