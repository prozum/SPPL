#include "LLVMCodeGenerator.h"

using namespace std;
using namespace llvm;
using namespace codegen;

LLVMCodeGenerator::LLVMCodeGenerator(parser::Driver &Drv)
        : CodeGenerator(Drv),
          Builder(getGlobalContext()),
          Machine(EngineBuilder().selectTarget()),
          DataLayout(Machine->createDataLayout()),
          Module(std::make_unique<llvm::Module>("SpplModule", getGlobalContext())),
          Ctx(getGlobalContext()){

    Int1 = llvm::Type::getInt1Ty(Ctx);
    Int8 = llvm::Type::getInt8Ty(Ctx);
    Int32 = llvm::Type::getInt32Ty(Ctx);
    Int64 = llvm::Type::getInt64Ty(Ctx);
    Double = llvm::Type::getDoubleTy(Ctx);
}

void LLVMCodeGenerator::visit(common::Program &node) {
    for (auto &func : node.Decls) {
        func->accept(*this);
    }

    *Drv.Out << ModuleString();
}

string LLVMCodeGenerator::ModuleString() {
    string ModuleStr;
    raw_string_ostream out(ModuleStr);
    out << *Module.get();
    return out.str();
}
