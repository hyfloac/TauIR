#pragma once

#if 1
#include "IrVisitor.hpp"
#endif

#include "TauIR/Function.hpp"
#include "TauIR/Opcodes.hpp"

namespace tau::ir {

#define SIMPLE_TRAVERSE(OPCODE) case Opcode::OPCODE: Traverse##OPCODE(); break

template<typename Derived>
void BaseIrVisitor<Derived>::Traverse(const tau::ir::Function* function) noexcept
{
    const u8* codePtr = function->Address();

    while(true)
    {
        PreTraverse(codePtr);

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

        if(opcode == Opcode::Ret)
        {
            TraverseRet();
            break;
        }

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
                TraversePushN(localIndex);
                break;
            }
            SIMPLE_TRAVERSE(PushArg0);
            SIMPLE_TRAVERSE(PushArg1);
            SIMPLE_TRAVERSE(PushArg2);
            SIMPLE_TRAVERSE(PushArg3);
            case Opcode::PushArgN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                TraversePushArgN(localIndex);
                break;
            }
            case Opcode::PushPtr:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                TraversePushPtr(localIndex);
                break;
            }
            case Opcode::PushGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                TraversePushGlobal(globalIndex);
                break;
            }
            case Opcode::PushGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                TraversePushGlobalExt(globalIndex, moduleIndex);
                break;
            }
            case Opcode::PushGlobalPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                TraversePushGlobalPtr(globalIndex);
                break;
            }
            case Opcode::PushGlobalExtPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                TraversePushGlobalExtPtr(globalIndex, moduleIndex);
                break;
            }
            SIMPLE_TRAVERSE(Pop0);
            SIMPLE_TRAVERSE(Pop1);
            SIMPLE_TRAVERSE(Pop2);
            SIMPLE_TRAVERSE(Pop3);
            case Opcode::PopN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                TraversePopN(localIndex);
                break;
            }
            SIMPLE_TRAVERSE(PopArg0);
            SIMPLE_TRAVERSE(PopArg1);
            SIMPLE_TRAVERSE(PopArg2);
            SIMPLE_TRAVERSE(PopArg3);
            case Opcode::PopArgN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                TraversePopArgN(localIndex);
                break;
            }
            case Opcode::PopPtr:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                TraversePopPtr(localIndex);
                break;
            }
            case Opcode::PopGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                TraversePopGlobal(globalIndex);
                break;
            }
            case Opcode::PopGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                TraversePopGlobalExt(globalIndex, moduleIndex);
                break;
            }
            case Opcode::PopGlobalPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                TraversePopGlobalPtr(globalIndex);
                break;
            }
            case Opcode::PopGlobalExtPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                TraversePopGlobalExtPtr(globalIndex, moduleIndex);
                break;
            }
            case Opcode::PopCount:
            {
                const u16 byteCount = ReadCodeValue<u16>(codePtr);
                TraversePopCount(byteCount);
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
                TraverseLoad(localIndex, addressIndex);
                break;
            }
            case Opcode::LoadGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u16>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                TraverseLoadGlobal(globalIndex, addressIndex);
                break;
            }
            case Opcode::LoadGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                TraverseLoadGlobalExt(globalIndex, addressIndex, moduleIndex);
                break;
            }
            case Opcode::Store:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                TraverseStore(localIndex, addressIndex);
                break;
            }
            case Opcode::StoreGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                TraverseStoreGlobal(globalIndex, addressIndex);
                break;
            }
            case Opcode::StoreGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                TraverseStoreGlobalExt(globalIndex, addressIndex, moduleIndex);
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
                TraverseConstN(constant);
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
                TraverseCall(targetFunctionIndex);
                break;
            }
            case Opcode::CallExt:
            {
                const u32 targetFunctionIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                TraverseCallExt(targetFunctionIndex, moduleIndex);
                break;
            }
            SIMPLE_TRAVERSE(CallInd);
            case Opcode::Jump:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);
                TraverseJump(offset);
                break;
            }
            case Opcode::JumpTrue:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);
                TraverseJumpTrue(offset);
                break;
            }
            case Opcode::JumpFalse:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);
                TraverseJumpFalse(offset);
                break;
            }
            default: break;
        }
    }
}

#undef SIMPLE_TRAVERSE

}