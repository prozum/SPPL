#include "LLVMCodeGen.h"

using namespace std;
using namespace codegen;
using namespace llvm;

void LLVMCodeGen::visit(common::FloatExpr &Node) {
    CurVal = ConstantFP::get(getLLVMType(Node.RetTy), Node.Val);
}

void LLVMCodeGen::visit(common::IntExpr &Node) {
    CurVal = ConstantInt::get(getLLVMType(Node.RetTy), (uint64_t)Node.Val);
}

void LLVMCodeGen::visit(common::BoolExpr &Node) {
    CurVal = ConstantInt::get(getLLVMType(Node.RetTy), (uint64_t)Node.Val);
}

void LLVMCodeGen::visit(common::StringExpr &Node) {
    Value *NextListNode = nullptr;

    if (Concat) {
        NextListNode = CurVal;
        Concat = false;
    }

    for (auto Char = Node.Val.rbegin(); Char < Node.Val.rend(); ++Char) {
        CurVal = ConstantInt::get(Int, (uint64_t)*Char);
        NextListNode = createListNode(Node.RetTy, CurVal, NextListNode, Builder.GetInsertBlock(), true);
    }

    CurVal = NextListNode;
}

void LLVMCodeGen::visit(common::CharExpr &Node) {
    CurVal = ConstantInt::get(getLLVMType(Node.RetTy), (uint64_t)Node.Val);
}

void LLVMCodeGen::visit(common::IdExpr &Node) {
    // Identifier value
    CurVal = IdVals[Node.Val];
    if (CurVal)
        return;

    // Internal function
    CurVal = Module->getFunction(Node.Val);
    if (CurVal)
        return;

    // External function
    if (Drv.Global.Decls.count(Node.Val))
        CurVal = llvm::Function::Create(getLLVMFuncType(Drv.Global.Decls[Node.Val]),
                                        llvm::Function::ExternalLinkage,
                                        Node.Val, Module.get());
    if (CurVal)
        return;

    assert(0 && "Something went wrong in the TypeChecker");
}

void LLVMCodeGen::visit(common::ParExpr &Node) {
    Node.Child->accept(*this);
}

void LLVMCodeGen::visit(common::TupleExpr &Node) {
    auto TupleType = getLLVMTupleType(Node.RetTy);
    auto TuplePtrType = getLLVMType(Node.RetTy);

    if (Node.Const) {
        std::vector<llvm::Constant *> TupleVal;
        for (auto &Element : Node.Elements) {
            Element->accept(*this);
            TupleVal.push_back(static_cast<Constant *>(CurVal));
        }

        auto ConstVal = ConstantStruct::get(TupleType, TupleVal);
        CurVal = new GlobalVariable(*Module.get(), ConstVal->getType(), true,
                                    GlobalVariable::ExternalLinkage, ConstVal);
        return;
    }

    auto TupleMalloc = createMalloc(TupleType, CurCaseBlock);
    auto Tuple = Builder.CreateBitCast(TupleMalloc, TuplePtrType);

    for (size_t i = 0; i < Node.Elements.size(); ++i) {
        Node.Elements[i]->accept(*this);

        auto GEP = Builder.CreateStructGEP(TupleType, Tuple, i);
        Builder.CreateStore(CurVal, GEP);
    }

    CurVal = Tuple;
}

void LLVMCodeGen::visit(common::ListExpr &Node) {
    Value *NextListNode = nullptr;

    if (Concat) {
        NextListNode = CurVal;
        Concat = false;
    }

    for (auto Element = Node.Elements.rbegin(); Element < Node.Elements.rend(); ++Element) {
        (*Element)->accept(*this);
        NextListNode = createListNode(Node.RetTy, CurVal, NextListNode, Builder.GetInsertBlock(), Node.Const);
    }

    // Empty list
    if (!NextListNode)
        NextListNode = ConstantPointerNull::get(PointerType::getUnqual(getLLVMListType(Node.RetTy)));

    CurVal = NextListNode;
}

void LLVMCodeGen::visit(common::CallExpr &Node) {
    Node.Callee->accept(*this);
    auto Callee = CurVal;

    std::vector<Value *> Args;
    for (auto &Arg : Node.Args) {
        Arg->accept(*this);
        Args.push_back(CurVal);
    }

    auto Call = Builder.CreateCall(Callee, Args);

    // Set tail recursion state
    Call->setCallingConv(CallingConv::Fast);
    Call->setTailCall(TailRec);

    CurVal = Call;
}



void LLVMCodeGen::visit(common::DoExpr &Node) {
    for (auto &Expr : Node.Exprs) {
        Expr->accept(*this);
    }
}

void LLVMCodeGen::visit(common::Assosiate &Node) {
    Node.Child->accept(*this);

    IdVals[Node.Id] = CurVal;
}
