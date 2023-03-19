#pragma once

#if 1
#include "IrVisitor.hpp"
#endif

#include "TauIR/Opcodes.hpp"

namespace tau::ir {

#define SIMPLE_TRAVERSE(OPCODE) case Opcode::OPCODE: GetDerived().Visit##OPCODE(); break

template<typename Derived>
void BaseIrVisitor<Derived>::Traverse(const u8* codePtr, const u8* const endPtr) noexcept
{
    while(true)
    {
        if(codePtr >= endPtr)
        {
            return;
        }

        GetDerived().PreVisit(codePtr);

        u16 opcodeRaw = *codePtr;
        ++codePtr;

        // Read Second Byte
        if(opcodeRaw & 0x80)
        {
            opcodeRaw <<= 8;
            opcodeRaw |= *codePtr;
            ++codePtr;
        }

        const Opcode opcode = static_cast<Opcode>(opcodeRaw);

        switch(opcode)
        {
            SIMPLE_TRAVERSE(Nop);
            SIMPLE_TRAVERSE(Push0);
            SIMPLE_TRAVERSE(Push1);
            SIMPLE_TRAVERSE(Push2);
            SIMPLE_TRAVERSE(Push3);
            case Opcode::PushN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPushN(localIndex);
                break;
            }
            SIMPLE_TRAVERSE(PushArg0);
            SIMPLE_TRAVERSE(PushArg1);
            SIMPLE_TRAVERSE(PushArg2);
            SIMPLE_TRAVERSE(PushArg3);
            case Opcode::PushArgN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPushArgN(localIndex);
                break;
            }
            case Opcode::PushPtr:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPushPtr(localIndex);
                break;
            }
            case Opcode::PushGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                GetDerived().VisitPushGlobal(globalIndex);
                break;
            }
            case Opcode::PushGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPushGlobalExt(globalIndex, moduleIndex);
                break;
            }
            case Opcode::PushGlobalPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                GetDerived().VisitPushGlobalPtr(globalIndex);
                break;
            }
            case Opcode::PushGlobalExtPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPushGlobalExtPtr(globalIndex, moduleIndex);
                break;
            }
            SIMPLE_TRAVERSE(Pop0);
            SIMPLE_TRAVERSE(Pop1);
            SIMPLE_TRAVERSE(Pop2);
            SIMPLE_TRAVERSE(Pop3);
            case Opcode::PopN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPopN(localIndex);
                break;
            }
            SIMPLE_TRAVERSE(PopArg0);
            SIMPLE_TRAVERSE(PopArg1);
            SIMPLE_TRAVERSE(PopArg2);
            SIMPLE_TRAVERSE(PopArg3);
            case Opcode::PopArgN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPopArgN(localIndex);
                break;
            }
            case Opcode::PopPtr:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPopPtr(localIndex);
                break;
            }
            case Opcode::PopGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                GetDerived().VisitPopGlobal(globalIndex);
                break;
            }
            case Opcode::PopGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPopGlobalExt(globalIndex, moduleIndex);
                break;
            }
            case Opcode::PopGlobalPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                GetDerived().VisitPopGlobalPtr(globalIndex);
                break;
            }
            case Opcode::PopGlobalExtPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPopGlobalExtPtr(globalIndex, moduleIndex);
                break;
            }
            case Opcode::PopCount:
            {
                const u16 byteCount = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitPopCount(byteCount);
                break;
            }
            SIMPLE_TRAVERSE(Dup1);
            SIMPLE_TRAVERSE(Dup2);
            SIMPLE_TRAVERSE(Dup4);
            SIMPLE_TRAVERSE(Dup8);
            SIMPLE_TRAVERSE(ExpandSX12);
            SIMPLE_TRAVERSE(ExpandSX14);
            SIMPLE_TRAVERSE(ExpandSX18);
            SIMPLE_TRAVERSE(ExpandSX24);
            SIMPLE_TRAVERSE(ExpandSX28);
            SIMPLE_TRAVERSE(ExpandSX48);
            SIMPLE_TRAVERSE(ExpandZX12);
            SIMPLE_TRAVERSE(ExpandZX14);
            SIMPLE_TRAVERSE(ExpandZX18);
            SIMPLE_TRAVERSE(ExpandZX24);
            SIMPLE_TRAVERSE(ExpandZX28);
            SIMPLE_TRAVERSE(ExpandZX48);
            SIMPLE_TRAVERSE(Trunc84);
            SIMPLE_TRAVERSE(Trunc82);
            SIMPLE_TRAVERSE(Trunc81);
            SIMPLE_TRAVERSE(Trunc42);
            SIMPLE_TRAVERSE(Trunc41);
            SIMPLE_TRAVERSE(Trunc21);
            case Opcode::Load:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitLoad(localIndex, addressIndex);
                break;
            }
            case Opcode::LoadGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u16>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitLoadGlobal(globalIndex, addressIndex);
                break;
            }
            case Opcode::LoadGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitLoadGlobalExt(globalIndex, addressIndex, moduleIndex);
                break;
            }
            case Opcode::Store:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitStore(localIndex, addressIndex);
                break;
            }
            case Opcode::StoreGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitStoreGlobal(globalIndex, addressIndex);
                break;
            }
            case Opcode::StoreGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitStoreGlobalExt(globalIndex, addressIndex, moduleIndex);
                break;
            }
            SIMPLE_TRAVERSE(Const0);
            SIMPLE_TRAVERSE(Const1);
            SIMPLE_TRAVERSE(Const2);
            SIMPLE_TRAVERSE(Const3);
            SIMPLE_TRAVERSE(Const4);
            SIMPLE_TRAVERSE(ConstFF);
            SIMPLE_TRAVERSE(Const7F);
            case Opcode::ConstN:
            {
                const u32 constant = ReadCodeValue<u32>(codePtr);
                GetDerived().VisitConstN(constant);
                break;
            }
            SIMPLE_TRAVERSE(AddI32);
            SIMPLE_TRAVERSE(AddI64);
            SIMPLE_TRAVERSE(SubI32);
            SIMPLE_TRAVERSE(SubI64);
            SIMPLE_TRAVERSE(MulI32);
            SIMPLE_TRAVERSE(MulI64);
            SIMPLE_TRAVERSE(DivI32);
            SIMPLE_TRAVERSE(DivI64);
            SIMPLE_TRAVERSE(CompI32Above);
            SIMPLE_TRAVERSE(CompI32AboveOrEqual);
            SIMPLE_TRAVERSE(CompI32Below);
            SIMPLE_TRAVERSE(CompI32BelowOrEqual);
            SIMPLE_TRAVERSE(CompI32Equal);
            SIMPLE_TRAVERSE(CompI32Greater);
            SIMPLE_TRAVERSE(CompI32GreaterOrEqual);
            SIMPLE_TRAVERSE(CompI32Less);
            SIMPLE_TRAVERSE(CompI32LessOrEqual);
            SIMPLE_TRAVERSE(CompI32NotEqual);
            SIMPLE_TRAVERSE(CompI64Above);
            SIMPLE_TRAVERSE(CompI64AboveOrEqual);
            SIMPLE_TRAVERSE(CompI64Below);
            SIMPLE_TRAVERSE(CompI64BelowOrEqual);
            SIMPLE_TRAVERSE(CompI64Equal);
            SIMPLE_TRAVERSE(CompI64Greater);
            SIMPLE_TRAVERSE(CompI64GreaterOrEqual);
            SIMPLE_TRAVERSE(CompI64Less);
            SIMPLE_TRAVERSE(CompI64LessOrEqual);
            SIMPLE_TRAVERSE(CompI64NotEqual);
            case Opcode::Call:
            {
                const u32 targetFunctionIndex = ReadCodeValue<u32>(codePtr);
                GetDerived().VisitCall(targetFunctionIndex);
                break;
            }
            case Opcode::CallExt:
            {
                const u32 targetFunctionIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                GetDerived().VisitCallExt(targetFunctionIndex, moduleIndex);
                break;
            }
            SIMPLE_TRAVERSE(CallInd);
            SIMPLE_TRAVERSE(CallIndExt);
            SIMPLE_TRAVERSE(Ret);
            case Opcode::Jump:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);
                GetDerived().VisitJump(offset);
                break;
            }
            case Opcode::JumpTrue:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);
                GetDerived().VisitJumpTrue(offset);
                break;
            }
            case Opcode::JumpFalse:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);
                GetDerived().VisitJumpFalse(offset);
                break;
            }
            default: break;
        }
    }
}

#undef SIMPLE_TRAVERSE

}
