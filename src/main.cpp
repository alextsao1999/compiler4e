//
// Created by Alex on 2020/2/6.
//
#include "compiler.h"
#include "test.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/Module.h"
#include <llvm/Support/SourceMgr.h>
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include <llvm/Support/MemoryBuffer.h>
#include "llvm/Support/raw_ostream.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
ExecutionEngine *EE = nullptr;
RTDyldMemoryManager *RTDyldMM = nullptr;
void initEE(std::unique_ptr<Module> Owner) {
    string ErrStr;
    if (EE == nullptr) {
        RTDyldMM = new SectionMemoryManager();
        EE = EngineBuilder(std::move(Owner))
                .setEngineKind(EngineKind::JIT)
                .setErrorStr(&ErrStr)
                .setVerifyModules(true)
                .setMCJITMemoryManager(std::unique_ptr<RTDyldMemoryManager>(RTDyldMM))
                .setOptLevel(CodeGenOpt::Default)
                .create();
    } else {
        EE->addModule(std::move(Owner));
    }
    if (ErrStr.length() != 0)
        cerr << "Create Engine Error" << endl << ErrStr << endl;
    EE->finalizeObject();
}
int main() {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

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
        initEE(std::unique_ptr<Module>(module.module));
    }
    auto *func = EE->FindFunctionNamed("main");
    void *addr = EE->getPointerToFunction(func);
    typedef int (*FuncType)();
    auto mainFunc = (FuncType) addr;
    std::cout << "JIT result: " << mainFunc() << std::endl;
    return 0;
}
