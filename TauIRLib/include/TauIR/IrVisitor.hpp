// ReSharper disable CppHiddenFunction
#pragma once

namespace tau::ir {

class Function;
enum class CompareCondition : u8;

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

#define SIMPLE_COMP_VISIT_DECL(TYPE, CONDITION) \
    void VisitComp##TYPE##CONDITION() noexcept {  \
        GetDerived().VisitComp##TYPE(CompareCondition::CONDITION);  \
    }

template<typename Derived>
class BaseIrVisitor
{
    DEFAULT_CONSTRUCT_PO(BaseIrVisitor);
    DEFAULT_DESTRUCT_VI(BaseIrVisitor);
    DEFAULT_CM_PO(BaseIrVisitor);
public:
    void Traverse(const u8* codePtr, const u8* endPtr) noexcept;
protected:
    Derived& GetDerived() noexcept { return *static_cast<Derived*>(this); }

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

    void VisitPushArg(const u16 argumentIndex) noexcept { }
    
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
        GetDerived().VisitConst(0xFFFFFFFF);
    }

    void VisitConst7F() noexcept
    {
        GetDerived().VisitConst(0x7FFFFFFF);
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

    void VisitCompI32(CompareCondition condition) noexcept { }

    SIMPLE_COMP_VISIT_DECL(I32, Above);
    SIMPLE_COMP_VISIT_DECL(I32, AboveOrEqual);
    SIMPLE_COMP_VISIT_DECL(I32, Below);
    SIMPLE_COMP_VISIT_DECL(I32, BelowOrEqual);
    SIMPLE_COMP_VISIT_DECL(I32, Equal);
    SIMPLE_COMP_VISIT_DECL(I32, Greater);
    SIMPLE_COMP_VISIT_DECL(I32, GreaterOrEqual);
    SIMPLE_COMP_VISIT_DECL(I32, Less);
    SIMPLE_COMP_VISIT_DECL(I32, LessOrEqual);
    SIMPLE_COMP_VISIT_DECL(I32, NotEqual);

    void VisitCompI64(CompareCondition condition) noexcept { }

    SIMPLE_COMP_VISIT_DECL(I64, Above);
    SIMPLE_COMP_VISIT_DECL(I64, AboveOrEqual);
    SIMPLE_COMP_VISIT_DECL(I64, Below);
    SIMPLE_COMP_VISIT_DECL(I64, BelowOrEqual);
    SIMPLE_COMP_VISIT_DECL(I64, Equal);
    SIMPLE_COMP_VISIT_DECL(I64, Greater);
    SIMPLE_COMP_VISIT_DECL(I64, GreaterOrEqual);
    SIMPLE_COMP_VISIT_DECL(I64, Less);
    SIMPLE_COMP_VISIT_DECL(I64, LessOrEqual);
    SIMPLE_COMP_VISIT_DECL(I64, NotEqual);

    void VisitCall(const u32 functionIndex) noexcept { }
    void VisitCallExt(const u32 functionIndex, const u16 moduleIndex) noexcept { }

    void VisitCallInd(const u16 localIndex) noexcept { };
    void VisitCallIndExt(const u16 localIndex) noexcept { };

    SIMPLE_VISIT_DECL(Ret);

    void VisitJumpPoint(const i32 offset) noexcept { }

    void VisitJump(const i32 offset) noexcept
    {
        GetDerived().VisitJumpPoint(offset);
    }

    void VisitJumpTrue(const i32 offset) noexcept
    { 
        GetDerived().VisitJumpPoint(offset);
    }

    void VisitJumpFalse(const i32 offset) noexcept
    {
        GetDerived().VisitJumpPoint(offset);
    }
protected:
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
