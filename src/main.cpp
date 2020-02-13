//
// Created by Alex on 2020/2/6.
//
#include "compiler.h"
#include "test.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include <llvm/Support/SourceMgr.h>
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include <llvm/Support/MemoryBuffer.h>
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/IR/LegacyPassManager.h"

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
void CompileFile(const char *file) {
    FileBuffer buffer(file);
    ECodeParser parser(buffer);
    parser.Parse();
    EContext context(&parser.code);
    for (auto &module : parser.code.modules) {
        for (auto &sub : module.include) {
            auto *find = parser.code.find<ESub>(sub);
            if (find) {
                if (find->attr.count("extern")) {
                    break;
                }
                ECompiler compiler(context, find);
                find->ast->accept(&compiler);
            }
        }
    }
    for (auto &module : parser.code.modules) {
        module.module->print(llvm::outs(), nullptr);
        //initEE(std::unique_ptr<Module>(module.module));
    }
    if (EE) {
        auto *func = EE->FindFunctionNamed("main");
        void *addr = EE->getPointerToFunction(func);
        typedef int (*FuncType)();
        auto mainFunc = (FuncType) addr;
        std::cout << "JIT result: " << mainFunc() << std::endl;
    }

    std::string errors;
    std::error_code code;

    std::string file_ir(file);
    file_ir.append(".ll");
    raw_fd_ostream output_ir(file_ir, code);
    output_ir << *context.dlls;

    //printf("%s", sys::getDefaultTargetTriple().c_str());

    const Target *target = TargetRegistry::lookupTarget(
            sys::getDefaultTargetTriple(), errors);
    TargetOptions targetOptions;
    TargetMachine *targetMachine = target->createTargetMachine(
            "i386-w64-windows-gnu", sys::getHostCPUName(), "",
            targetOptions, {});
    DataLayout layout = targetMachine->createDataLayout();
    context.dlls->setDataLayout(layout);

    legacy::PassManager passManager;

    std::string file_obj(file);
    file_obj.append(".o");
    ToolOutputFile output(file_obj, code, sys::fs::OF_None);

    targetMachine->addPassesToEmitFile(passManager, output.os(), nullptr, TargetMachine::CGFT_ObjectFile);
    passManager.run(*context.dlls);
    output.keep();

}
int main(int count, const char **argv) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

/*
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmPrinters();
    InitializeAllAsmParsers();
*/
    for (int i = 1; i < count; ++i) {
        CompileFile(argv[i]);
        std::error_code code;
        std::string file(argv[i]);
        file.append("compile.bat");
        raw_fd_ostream output(file, code);
        output << "@echo off\n";
        output << "cd /d %~dp0\n";
        output << "gcc runtime.c -c -o runtime.o\n";
        output << "gcc ";
        output << argv[i] << ".o runtime.o -o out.exe\n";

    }

    return 0;
}
