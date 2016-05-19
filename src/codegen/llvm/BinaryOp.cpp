#include "LLVMCodeGen.h"

using namespace llvm;
using namespace codegen;
using namespace common;

void LLVMCodeGen::visit(Add &Node) {
    assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
           (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Left->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFAdd(Left, Right, "addtmp");
    } else {
        CurVal = Builder.CreateAdd(Left, Right, "addtmp");
    }
}

void LLVMCodeGen::visit(Sub &Node) {
    assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
           (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Right->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFSub(Left, Right, "subtmp");
    } else {
        CurVal = Builder.CreateSub(Left, Right, "subtmp");
    }
}

void LLVMCodeGen::visit(Mul &Node) {
    assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
           (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Right->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFMul(Left, Right, "multmp");
    } else {
        CurVal = Builder.CreateMul(Left, Right, "multmp");
    }
}

void LLVMCodeGen::visit(Div &Node) {
    assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
           (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Right->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFDiv(Left, Right, "divtmp");
    } else {
        CurVal = Builder.CreateSDiv(Left, Right, "divtmp");
    }
}

void LLVMCodeGen::visit(Mod &Node) {
    assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
           (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Right->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFRem(Left, Right, "modtmp");
    } else {
        CurVal = Builder.CreateSRem(Left, Right, "modtmp");
    }
}

void LLVMCodeGen::visit(And &Node) {
    assert(Node.Left->RetTy.Id == TypeId::BOOL &&
           Node.Right->RetTy.Id == TypeId::BOOL);

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    CurVal = Builder.CreateAnd(Left, Right, "andtmp");

    // Convert to int
    CurVal = Builder.CreateZExt(CurVal, Int);
}

void LLVMCodeGen::visit(Or &Node) {
    assert(Node.Left->RetTy.Id == TypeId::BOOL &&
           Node.Right->RetTy.Id == TypeId::BOOL);

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    CurVal = Builder.CreateOr(Left, Right, "ortmp");

    // Convert to int
    CurVal = Builder.CreateZExt(CurVal, Int);
}

void LLVMCodeGen::visit(Equal &Node) {
    // TODO
    //assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
    //       (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Right->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFCmpOEQ(Left, Right, "eqtmp");
    } else {
        CurVal = Builder.CreateICmpEQ(Left, Right, "eqtmp");
    }

    // Convert to int
    CurVal = Builder.CreateZExt(CurVal, Int);
}

void LLVMCodeGen::visit(NotEqual &Node) {
    // TODO
    //assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
    //       (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    if (Node.Left->Const && Node.Right->Const) {

        /*
        Node.Left->accept(*this);
        auto Left = static_cast<Constant *>(CurVal);
        Node.Right->accept(*this);
        auto Right = static_cast<Constant *>(CurVal);
         */
    }

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;


    if (Node.Right->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFCmpONE(Left, Right, "neqtmp");
    } else {
        CurVal = Builder.CreateICmpNE(Left, Right, "neqtmp");
    }

    // Convert to int
    CurVal = Builder.CreateZExt(CurVal, Int);
}

void LLVMCodeGen::visit(Lesser &Node) {
    assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
           (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Right->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFCmpOLT(Left, Right, "lttmp");
    } else {
        CurVal = Builder.CreateICmpSLT(Left, Right, "lttmp");
    }

    // Convert to int
    CurVal = Builder.CreateZExt(CurVal, Int);
}

void LLVMCodeGen::visit(Greater &Node) {
    assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
           (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Right->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFCmpOGT(Left, Right, "lttmp");
    } else {
        CurVal = Builder.CreateICmpSGT(Left, Right, "lttmp");
    }

    // Convert to int
    CurVal = Builder.CreateZExt(CurVal, Int);
}

void LLVMCodeGen::visit(LesserEq &Node) {
    assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
           (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Right->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFCmpOLE(Left, Right, "lttmp");
    } else {
        CurVal = Builder.CreateICmpSLE(Left, Right, "lttmp");
    }

    // Convert to int
    CurVal = Builder.CreateZExt(CurVal, Int);
}

void LLVMCodeGen::visit(GreaterEq &Node) {
    assert((Node.Left->RetTy.Id == TypeId::FLOAT && Node.Right->RetTy.Id == TypeId::FLOAT) ||
           (Node.Left->RetTy.Id == TypeId::INT && Node.Right->RetTy.Id == TypeId::INT));

    Node.Left->accept(*this);
    auto Left = CurVal;
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Right->RetTy.Id == TypeId::FLOAT) {
        CurVal = Builder.CreateFCmpOGE(Left, Right, "lttmp");
    } else {
        CurVal = Builder.CreateICmpSGE(Left, Right, "lttmp");
    }

    // Convert to int
    CurVal = Builder.CreateZExt(CurVal, Int);
}

void LLVMCodeGen::visit(To &Node) {
    Node.Child->accept(*this);

    switch (Node.TypeCast.Id) {
        case TypeId::FLOAT:
            if (Node.Child->RetTy == TypeId::BOOL)
                CurVal = Builder.CreateUIToFP(CurVal, Float, "casttmp");
            else if (Node.Child->RetTy == TypeId::INT ||
                     Node.Child->RetTy == TypeId::CHAR)
                CurVal = Builder.CreateSIToFP(CurVal, Float, "casttmp");
            break;
        case TypeId::INT:
            if (Node.Child->RetTy == TypeId::FLOAT)
                CurVal = Builder.CreateFPToSI(CurVal, Int, "casttmp");
            break;
        case TypeId::CHAR:
            if (Node.Child->RetTy == TypeId::FLOAT)
                CurVal = Builder.CreateFPToSI(CurVal, Int, "casttmp");
            break;
        case TypeId::BOOL:
            if (Node.Child->RetTy == TypeId::FLOAT)
                CurVal = Builder.CreateFPToSI(CurVal, Int, "casttmp");
            break;
        default:
            addError(Error::NotImplemented(Node.Loc));
    }
}
/*
void LLVMCodeGen::visit(common::Concat &Node) {
    Node.Right->accept(*this);
    auto Right = CurVal;

    if (Node.Left->Const && Node.Right->Const) {
        Node.Left->accept(*this);
        auto Left = CurVal;

    }
}
 */

