// ReSharper disable CppHiddenFunction
#pragma once

#include "TauIR/Module.hpp"
#include "Common.hpp"
#include <vector>


namespace tau::ir {

class Function;

#define SIMPLE_TRAVERSE_DECL(OPCODE)   \
    void Traverse##OPCODE() noexcept { \
        GetDerived().Visit##OPCODE();  \
    }

#define SIMPLE_VISIT_DECL(OPCODE) \
    void Visit##OPCODE() noexcept { }

#define SIMPLE_1_VALUE_VISIT_DECL(OPERATION, VAL) \
    void Visit##OPERATION##VAL() noexcept {  \
        GetDerived().Visit##OPERATION(VAL);  \
    }

#define SIMPLE_2_VALUE_VISIT_DECL(OPERATION, VAL0, VAL1) \
    void Visit##OPERATION##VAL0##VAL1() noexcept {  \
        GetDerived().Visit##OPERATION(VAL0, VAL1);  \
    }

template<typename Derived>
class BaseIrVisitor
{
    DEFAULT_CONSTRUCT_PU(BaseIrVisitor);
    DEFAULT_DESTRUCT_VI(BaseIrVisitor);
    DEFAULT_CM_PU(BaseIrVisitor);
public:
    Derived& GetDerived() noexcept { return *static_cast<Derived*>(this); }

    void Traverse(const tau::ir::Function* function) noexcept;
public:
    void PreTraverse(const u8* const codePtr) noexcept
    {
        GetDerived().PreVisit(codePtr);
    }

    SIMPLE_TRAVERSE_DECL(Nop);
    SIMPLE_TRAVERSE_DECL(Push0);
    SIMPLE_TRAVERSE_DECL(Push1);
    SIMPLE_TRAVERSE_DECL(Push2);
    SIMPLE_TRAVERSE_DECL(Push3);

    void TraversePushN(const u16 localIndex) noexcept
    {
        GetDerived().VisitPushN(localIndex);
    }
    
    SIMPLE_TRAVERSE_DECL(PushArg0);
    SIMPLE_TRAVERSE_DECL(PushArg1);
    SIMPLE_TRAVERSE_DECL(PushArg2);
    SIMPLE_TRAVERSE_DECL(PushArg3);

    void TraversePushArgN(const u16 localIndex) noexcept
    {
        GetDerived().VisitPushArgN(localIndex);
    }

    void TraversePushPtr(const u16 localIndex) noexcept
    {
        GetDerived().VisitPushPtr(localIndex);
    }

    void TraversePushGlobal(const u32 globalIndex) noexcept
    {
        GetDerived().VisitPushGlobal(globalIndex);
    }

    void TraversePushGlobalExt(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        GetDerived().VisitPushGlobalExt(globalIndex, moduleIndex);
    }

    void TraversePushGlobalPtr(const u32 globalIndex) noexcept
    {
        GetDerived().VisitPushGlobalPtr(globalIndex);
    }

    void TraversePushGlobalExtPtr(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        GetDerived().VisitPushGlobalExtPtr(globalIndex, moduleIndex);
    }
    
    SIMPLE_TRAVERSE_DECL(Pop0);
    SIMPLE_TRAVERSE_DECL(Pop1);
    SIMPLE_TRAVERSE_DECL(Pop2);
    SIMPLE_TRAVERSE_DECL(Pop3);

    void TraversePopN(const u16 localIndex) noexcept
    {
        GetDerived().VisitPopN(localIndex);
    }
    
    SIMPLE_TRAVERSE_DECL(PopArg0);
    SIMPLE_TRAVERSE_DECL(PopArg1);
    SIMPLE_TRAVERSE_DECL(PopArg2);
    SIMPLE_TRAVERSE_DECL(PopArg3);

    void TraversePopArgN(const u16 localIndex) noexcept
    {
        GetDerived().VisitPopArgN(localIndex);
    }

    void TraversePopPtr(const u16 localIndex) noexcept
    {
        GetDerived().VisitPopPtr(localIndex);
    }

    void TraversePopGlobal(const u32 globalIndex) noexcept
    {
        GetDerived().VisitPopGlobal(globalIndex);
    }

    void TraversePopGlobalExt(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        GetDerived().VisitPopGlobalExt(globalIndex, moduleIndex);
    }

    void TraversePopGlobalPtr(const u32 globalIndex) noexcept
    {
        GetDerived().VisitPopGlobalPtr(globalIndex);
    }

    void TraversePopGlobalExtPtr(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        GetDerived().VisitPopGlobalExtPtr(globalIndex, moduleIndex);
    }

    void TraversePopCount(const u16 byteCount) noexcept
    {
        GetDerived().VisitPopCount(byteCount);
    }
    
    SIMPLE_TRAVERSE_DECL(Dup1);
    SIMPLE_TRAVERSE_DECL(Dup2);
    SIMPLE_TRAVERSE_DECL(Dup4);
    SIMPLE_TRAVERSE_DECL(Dup8);

    SIMPLE_TRAVERSE_DECL(ExpandSX12);
    SIMPLE_TRAVERSE_DECL(ExpandSX14);
    SIMPLE_TRAVERSE_DECL(ExpandSX18);
    SIMPLE_TRAVERSE_DECL(ExpandSX24);
    SIMPLE_TRAVERSE_DECL(ExpandSX28);
    SIMPLE_TRAVERSE_DECL(ExpandSX48);
    SIMPLE_TRAVERSE_DECL(ExpandZX12);
    SIMPLE_TRAVERSE_DECL(ExpandZX14);
    SIMPLE_TRAVERSE_DECL(ExpandZX18);
    SIMPLE_TRAVERSE_DECL(ExpandZX24);
    SIMPLE_TRAVERSE_DECL(ExpandZX28);
    SIMPLE_TRAVERSE_DECL(ExpandZX48);
    SIMPLE_TRAVERSE_DECL(Trunc84);
    SIMPLE_TRAVERSE_DECL(Trunc82);
    SIMPLE_TRAVERSE_DECL(Trunc81);
    SIMPLE_TRAVERSE_DECL(Trunc42);
    SIMPLE_TRAVERSE_DECL(Trunc41);
    SIMPLE_TRAVERSE_DECL(Trunc21);

    void TraverseLoad(const u16 localIndex, const u16 addressIndex) noexcept
    {
        GetDerived().VisitLoad(localIndex, addressIndex);
    }

    void TraverseLoadGlobal(const u32 globalIndex, const u16 addressIndex) noexcept
    {
        GetDerived().VisitLoadGlobal(globalIndex, addressIndex);
    }

    void TraverseLoadGlobalExt(const u32 globalIndex, const u16 addressIndex, const u16 moduleIndex) noexcept
    {
        GetDerived().VisitLoadGlobalExt(globalIndex, addressIndex, moduleIndex);
    }

    void TraverseStore(const u16 localIndex, const u16 addressIndex) noexcept
    {
        GetDerived().VisitStore(localIndex, addressIndex);
    }

    void TraverseStoreGlobal(const u32 globalIndex, const u16 addressIndex) noexcept
    {
        GetDerived().VisitStoreGlobal(globalIndex, addressIndex);
    }

    void TraverseStoreGlobalExt(const u32 globalIndex, const u16 addressIndex, const u16 moduleIndex) noexcept
    {
        GetDerived().VisitStoreGlobalExt(globalIndex, addressIndex, moduleIndex);
    }

    SIMPLE_TRAVERSE_DECL(Const0);
    SIMPLE_TRAVERSE_DECL(Const1);
    SIMPLE_TRAVERSE_DECL(Const2);
    SIMPLE_TRAVERSE_DECL(Const3);
    SIMPLE_TRAVERSE_DECL(Const4);
    SIMPLE_TRAVERSE_DECL(ConstFF);
    SIMPLE_TRAVERSE_DECL(Const7F);

    void TraverseConstN(const u32 constant) noexcept
    {
        GetDerived().VisitConstN(constant);
    }

    SIMPLE_TRAVERSE_DECL(AddI32);
    SIMPLE_TRAVERSE_DECL(AddI64);
    SIMPLE_TRAVERSE_DECL(SubI32);
    SIMPLE_TRAVERSE_DECL(SubI64);
    SIMPLE_TRAVERSE_DECL(MulI32);
    SIMPLE_TRAVERSE_DECL(MulI64);
    SIMPLE_TRAVERSE_DECL(DivI32);
    SIMPLE_TRAVERSE_DECL(DivI64);
    SIMPLE_TRAVERSE_DECL(CompI32Above);
    SIMPLE_TRAVERSE_DECL(CompI32AboveOrEqual);
    SIMPLE_TRAVERSE_DECL(CompI32Below);
    SIMPLE_TRAVERSE_DECL(CompI32BelowOrEqual);
    SIMPLE_TRAVERSE_DECL(CompI32Equal);
    SIMPLE_TRAVERSE_DECL(CompI32Greater);
    SIMPLE_TRAVERSE_DECL(CompI32GreaterOrEqual);
    SIMPLE_TRAVERSE_DECL(CompI32Less);
    SIMPLE_TRAVERSE_DECL(CompI32LessOrEqual);
    SIMPLE_TRAVERSE_DECL(CompI32NotEqual);
    SIMPLE_TRAVERSE_DECL(CompI64Above);
    SIMPLE_TRAVERSE_DECL(CompI64AboveOrEqual);
    SIMPLE_TRAVERSE_DECL(CompI64Below);
    SIMPLE_TRAVERSE_DECL(CompI64BelowOrEqual);
    SIMPLE_TRAVERSE_DECL(CompI64Equal);
    SIMPLE_TRAVERSE_DECL(CompI64Greater);
    SIMPLE_TRAVERSE_DECL(CompI64GreaterOrEqual);
    SIMPLE_TRAVERSE_DECL(CompI64Less);
    SIMPLE_TRAVERSE_DECL(CompI64LessOrEqual);
    SIMPLE_TRAVERSE_DECL(CompI64NotEqual);

    void TraverseCall(const u32 functionIndex) noexcept
    {
        GetDerived().VisitCall(functionIndex);
    }

    void TraverseCallExt(const u32 functionIndex, const u16 moduleIndex) noexcept
    {
        GetDerived().VisitCallExt(functionIndex, moduleIndex);
    }

    SIMPLE_TRAVERSE_DECL(CallInd);
    SIMPLE_TRAVERSE_DECL(Ret);

    void TraverseJump(const i32 offset) noexcept
    {
        GetDerived().VisitJump(offset);
    }

    void TraverseJumpTrue(const i32 offset) noexcept
    {
        GetDerived().VisitJumpTrue(offset);
    }

    void TraverseJumpFalse(const i32 offset) noexcept
    {
        GetDerived().VisitJumpFalse(offset);
    }
public:
    void PreVisit(const u8* const codePtr) noexcept { }

    SIMPLE_VISIT_DECL(Nop)

    void VisitPush(const u16 localIndex) noexcept { }
    
    SIMPLE_1_VALUE_VISIT_DECL(Push, 0);
    SIMPLE_1_VALUE_VISIT_DECL(Push, 1);
    SIMPLE_1_VALUE_VISIT_DECL(Push, 2);
    SIMPLE_1_VALUE_VISIT_DECL(Push, 3);
        
    void VisitPushN(const u16 localIndex) noexcept
    {
        GetDerived().VisitPush(localIndex);
    }

    void VisitPushArg(const u16 localIndex) noexcept { }
    
    SIMPLE_1_VALUE_VISIT_DECL(PushArg, 0);
    SIMPLE_1_VALUE_VISIT_DECL(PushArg, 1);
    SIMPLE_1_VALUE_VISIT_DECL(PushArg, 2);
    SIMPLE_1_VALUE_VISIT_DECL(PushArg, 3);

    void VisitPushArgN(const u16 localIndex) noexcept
    {
        GetDerived().VisitPushArg(localIndex);
    }

    void VisitPushPtr(const u16 localIndex) noexcept { }
    void VisitPushGlobal(const u32 globalIndex) noexcept { }
    void VisitPushGlobalExt(const u32 globalIndex, const u16 moduleIndex) noexcept { }
    void VisitPushGlobalPtr(const u32 globalIndex) noexcept { }
    void VisitPushGlobalExtPtr(const u32 globalIndex, const u16 moduleIndex) noexcept { }

    void VisitPop(const u16 localIndex) noexcept { }
    
    SIMPLE_1_VALUE_VISIT_DECL(Pop, 0);
    SIMPLE_1_VALUE_VISIT_DECL(Pop, 1);
    SIMPLE_1_VALUE_VISIT_DECL(Pop, 2);
    SIMPLE_1_VALUE_VISIT_DECL(Pop, 3);
        
    void VisitPopN(const u16 localIndex) noexcept
    {
        GetDerived().VisitPop(localIndex);
    }

    void VisitPopArg(const u16 localIndex) noexcept { }
    
    SIMPLE_1_VALUE_VISIT_DECL(PopArg, 0);
    SIMPLE_1_VALUE_VISIT_DECL(PopArg, 1);
    SIMPLE_1_VALUE_VISIT_DECL(PopArg, 2);
    SIMPLE_1_VALUE_VISIT_DECL(PopArg, 3);
        
    void VisitPopArgN(const u16 localIndex) noexcept
    {
        GetDerived().VisitPopArg(localIndex);
    }

    void VisitPopPtr(const u16 localIndex) noexcept { }
    void VisitPopGlobal(const u32 globalIndex) noexcept { }
    void VisitPopGlobalExt(const u32 globalIndex, const u16 moduleIndex) noexcept { }
    void VisitPopGlobalPtr(const u32 globalIndex) noexcept { }
    void VisitPopGlobalExtPtr(const u32 globalIndex, const u16 moduleIndex) noexcept { }
    void VisitPopCount(const u16 byteCount) noexcept { }

    void VisitDup(const uSys byteCount) noexcept { }

    SIMPLE_1_VALUE_VISIT_DECL(Dup, 1);
    SIMPLE_1_VALUE_VISIT_DECL(Dup, 2);
    SIMPLE_1_VALUE_VISIT_DECL(Dup, 4);
    SIMPLE_1_VALUE_VISIT_DECL(Dup, 8);
        
    void VisitExpandSX(const uSys fromSize, const uSys toSize) noexcept { }

    SIMPLE_2_VALUE_VISIT_DECL(ExpandSX, 1, 2);
    SIMPLE_2_VALUE_VISIT_DECL(ExpandSX, 1, 4);
    SIMPLE_2_VALUE_VISIT_DECL(ExpandSX, 1, 8);
    SIMPLE_2_VALUE_VISIT_DECL(ExpandSX, 2, 4);
    SIMPLE_2_VALUE_VISIT_DECL(ExpandSX, 2, 8);
    SIMPLE_2_VALUE_VISIT_DECL(ExpandSX, 4, 8);
        
    void VisitExpandZX(const uSys fromSize, const uSys toSize) noexcept { }
    
    SIMPLE_2_VALUE_VISIT_DECL(ExpandZX, 1, 2);
    SIMPLE_2_VALUE_VISIT_DECL(ExpandZX, 1, 4);
    SIMPLE_2_VALUE_VISIT_DECL(ExpandZX, 1, 8);
    SIMPLE_2_VALUE_VISIT_DECL(ExpandZX, 2, 4);
    SIMPLE_2_VALUE_VISIT_DECL(ExpandZX, 2, 8);
    SIMPLE_2_VALUE_VISIT_DECL(ExpandZX, 4, 8);

    void VisitTrunc(const uSys fromSize, const uSys toSize) noexcept { }
    
    SIMPLE_2_VALUE_VISIT_DECL(Trunc, 8, 4);
    SIMPLE_2_VALUE_VISIT_DECL(Trunc, 8, 2);
    SIMPLE_2_VALUE_VISIT_DECL(Trunc, 8, 1);
    SIMPLE_2_VALUE_VISIT_DECL(Trunc, 4, 2);
    SIMPLE_2_VALUE_VISIT_DECL(Trunc, 4, 1);
    SIMPLE_2_VALUE_VISIT_DECL(Trunc, 2, 1);

    void VisitLoad(const u16 localIndex, const u16 addressIndex) noexcept { }
    void VisitLoadGlobal(const u32 globalIndex, const u16 addressIndex) noexcept { }
    void VisitLoadGlobalExt(const u32 globalIndex, const u16 addressIndex, const u16 moduleIndex) noexcept { }
    void VisitStore(const u16 localIndex, const u16 addressIndex) noexcept { }
    void VisitStoreGlobal(const u32 globalIndex, const u16 addressIndex) noexcept { }
    void VisitStoreGlobalExt(const u32 globalIndex, const u16 addressIndex, const u16 moduleIndex) noexcept { }

    void VisitConst(const u32 constant) noexcept { }

    SIMPLE_1_VALUE_VISIT_DECL(Const, 0);
    SIMPLE_1_VALUE_VISIT_DECL(Const, 1);
    SIMPLE_1_VALUE_VISIT_DECL(Const, 2);
    SIMPLE_1_VALUE_VISIT_DECL(Const, 3);
    SIMPLE_1_VALUE_VISIT_DECL(Const, 4);

    void VisitConstFF() noexcept
    {
        GetDerived().VisitConst(0xFF);
    }

    void VisitConst7F() noexcept
    {
        GetDerived().VisitConst(0x7F);
    }

    void VisitConstN(const u32 constant) noexcept
    {
        GetDerived().VisitConst(constant);
    }

    SIMPLE_VISIT_DECL(AddI32);
    SIMPLE_VISIT_DECL(AddI64);
    SIMPLE_VISIT_DECL(SubI32);
    SIMPLE_VISIT_DECL(SubI64);
    SIMPLE_VISIT_DECL(MulI32);
    SIMPLE_VISIT_DECL(MulI64);
    SIMPLE_VISIT_DECL(DivI32);
    SIMPLE_VISIT_DECL(DivI64);
    SIMPLE_VISIT_DECL(CompI32Above);
    SIMPLE_VISIT_DECL(CompI32AboveOrEqual);
    SIMPLE_VISIT_DECL(CompI32Below);
    SIMPLE_VISIT_DECL(CompI32BelowOrEqual);
    SIMPLE_VISIT_DECL(CompI32Equal);
    SIMPLE_VISIT_DECL(CompI32Greater);
    SIMPLE_VISIT_DECL(CompI32GreaterOrEqual);
    SIMPLE_VISIT_DECL(CompI32Less);
    SIMPLE_VISIT_DECL(CompI32LessOrEqual);
    SIMPLE_VISIT_DECL(CompI32NotEqual);
    SIMPLE_VISIT_DECL(CompI64Above);
    SIMPLE_VISIT_DECL(CompI64AboveOrEqual);
    SIMPLE_VISIT_DECL(CompI64Below);
    SIMPLE_VISIT_DECL(CompI64BelowOrEqual);
    SIMPLE_VISIT_DECL(CompI64Equal);
    SIMPLE_VISIT_DECL(CompI64Greater);
    SIMPLE_VISIT_DECL(CompI64GreaterOrEqual);
    SIMPLE_VISIT_DECL(CompI64Less);
    SIMPLE_VISIT_DECL(CompI64LessOrEqual);
    SIMPLE_VISIT_DECL(CompI64NotEqual);

    void VisitCall(const u32 functionIndex) noexcept { }
    void VisitCallExt(const u32 functionIndex, const u16 moduleIndex) noexcept { }

    SIMPLE_VISIT_DECL(CallInd);
    SIMPLE_VISIT_DECL(Ret);

    void VisitJump(const i32 offset) noexcept { }
    void VisitJumpTrue(const i32 offset) noexcept { }
    void VisitJumpFalse(const i32 offset) noexcept { }
public:
    template<typename T>
    static T ReadCodeValue(const u8*& codePtr)
    {
        const T ret = *reinterpret_cast<const T*>(codePtr);
        codePtr += sizeof(T);
        return ret;
    }
};

#undef SIMPLE_2_VALUE_VISIT_DECL
#undef SIMPLE_1_VALUE_VISIT_DECL
#undef SIMPLE_VISIT_DECL
#undef SIMPLE_TRAVERSE_DECL

}

#include "IrVisitor.inl"
