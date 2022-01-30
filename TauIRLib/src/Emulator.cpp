#include "TauIR/Emulator.hpp"

#include "TauIR/Function.hpp"
#include "TauIR/Module.hpp"
#include "TauIR/Opcodes.hpp"
#include "TauIR/TypeInfo.hpp"

namespace tau::ir {

void Emulator::Execute() noexcept
{
    if(m_Modules.empty())
    {
        return;
    }
    
    const Ref<Module>& mainModule = m_Modules[0];

    if(!mainModule)
    {
        return;
    }

    if(mainModule->Functions().count() == 0)
    {
        return;
    }

    const Function* const entryPoint = mainModule->Functions()[0];

    ExecuteFunction(entryPoint, mainModule.Get());
}

void Emulator::ExecuteFunction(const Function* const function, const Module* const module) noexcept
{
    const u8* codePtr = function->Address();

    // Unused while emulating.
    // const uPtr prevFunctionPtr = *(reinterpret_cast<const uPtr*>(m_Stack.arr() + m_StackPointer) - 1);

    const uSys localsHead = m_StackPointer;
    m_StackPointer += function->LocalSize();

#if 0
    for(uSys i = 0; i <  function->LocalSize(); ++i)
    {
        m_Stack[localsHead + i] = static_cast<u8>(i);
    }
#endif
    
    while(true)
    {
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
            (void) PopValue<u32>();
            break;
        }

        switch(opcode)
        {
            case Opcode::Nop:
                break;
            case Opcode::Push0:
                PushLocal(function, localsHead, 0);
                break;
            case Opcode::Push1:
                PushLocal(function, localsHead, 1);
                break;
            case Opcode::Push2:
                PushLocal(function, localsHead, 2);
                break;
            case Opcode::Push3:
                PushLocal(function, localsHead, 3);
                break;
            case Opcode::PushN:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;

                PushLocal(function, localsHead, localIndex);
                break;
            }
            case Opcode::PushArg0:
                PushArgument(0);
                break;
            case Opcode::PushArg1:
                PushArgument(1);
                break;
            case Opcode::PushArg2:
                PushArgument(2);
                break;
            case Opcode::PushArg3:
                PushArgument(3);
                break;
            case Opcode::PushArgN:
            {
                const u8 argumentIndex = *codePtr;
                ++codePtr;

                PushArgument(argumentIndex);
                break;
            }
            case Opcode::PushPtr:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;

                
                uSys offset = 0;

                if(localIndex > 0)
                {
                    offset = function->LocalOffsets()[localIndex - 1];
                }

                const uSys size = TypeInfo::StripPointer(function->LocalTypes()[localIndex])->Size();

                void* localPointer;

                (void) ::std::memcpy(&localPointer, m_Stack.arr() + localsHead + offset, sizeof(void*));

                (void) ::std::memcpy(m_Stack.arr() + m_StackPointer, localPointer, size);

                m_StackPointer += size;
                break;
            }
            case Opcode::Pop0:
                PopLocal(function, localsHead, 0);
                break;
            case Opcode::Pop1:
                PopLocal(function, localsHead, 1);
                break;
            case Opcode::Pop2:
                PopLocal(function, localsHead, 2);
                break;
            case Opcode::Pop3:
                PopLocal(function, localsHead, 3);
                break;
            case Opcode::PopN:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;

                PopLocal(function, localsHead, localIndex);
                break;
            }
            case Opcode::PopArg0:
                PopArgument(0);
                break;
            case Opcode::PopArg1:
                PopArgument(1);
                break;
            case Opcode::PopArg2:
                PopArgument(2);
                break;
            case Opcode::PopArg3:
                PopArgument(3);
                break;
            case Opcode::PopArgN:
            {
                const u8 argumentIndex = *codePtr;
                ++codePtr;

                PopArgument(argumentIndex);
                break;
            }
            case Opcode::PopPtr:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;

                uSys offset = 0;

                if(localIndex > 0)
                {
                    offset = function->LocalOffsets()[localIndex - 1];
                }

                const uSys size = TypeInfo::StripPointer(function->LocalTypes()[localIndex])->Size();

                void* localPointer;

                (void) ::std::memcpy(&localPointer, m_Stack.arr() + localsHead + offset, sizeof(void*));
                
                m_StackPointer -= size;

                (void) ::std::memcpy(localPointer, m_Stack.arr() + m_StackPointer, size);

                break;
            }
            case Opcode::PopCount:
            {
                const u16 count = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;

                m_StackPointer -= count;
                break;
            }
            case Opcode::Dup1:
            {
                DuplicateVal(1);
                break;
            }
            case Opcode::Dup2:
            {
                DuplicateVal(2);
                break;
            }
            case Opcode::Dup4:
            {
                DuplicateVal(4);
                break;
            }
            case Opcode::Dup8:
            {
                DuplicateVal(8);
                break;
            }
            case Opcode::ExpandSX12:
            {
                ResizeVal<i8, i16>();
                break;
            }
            case Opcode::ExpandSX14:
            {
                ResizeVal<i8, i32>();
                break;
            }
            case Opcode::ExpandSX18:
            {
                ResizeVal<i8, i64>();
                break;
            }
            case Opcode::ExpandSX24:
            {
                ResizeVal<i16, i32>();
                break;
            }
            case Opcode::ExpandSX28:
            {
                ResizeVal<i16, i64>();
                break;
            }
            case Opcode::ExpandSX48:
            {
                ResizeVal<i32, i64>();
                break;
            }
            case Opcode::ExpandZX12:
            {
                ResizeVal<u8, u16>();
                break;
            }
            case Opcode::ExpandZX14:
            {
                ResizeVal<u8, u32>();
                break;
            }
            case Opcode::ExpandZX18:
            {
                ResizeVal<u8, u64>();
                break;
            }
            case Opcode::ExpandZX24:
            {
                ResizeVal<u16, u32>();
                break;
            }
            case Opcode::ExpandZX28:
            {
                ResizeVal<u16, u64>();
                break;
            }
            case Opcode::ExpandZX48:
            {
                ResizeVal<u32, u64>();
                break;
            }
            case Opcode::Trunc84:
            {
                ResizeVal<u64, u32>();
                break;
            }
            case Opcode::Trunc82:
            {
                ResizeVal<u64, u16>();
                break;
            }
            case Opcode::Trunc81:
            {
                ResizeVal<u64, u8>();
                break;
            }
            case Opcode::Trunc42:
            {
                ResizeVal<u32, u16>();
                break;
            }
            case Opcode::Trunc41:
            {
                ResizeVal<u32, u8>();
                break;
            }
            case Opcode::Trunc21:
            {
                ResizeVal<u16, u8>();
                break;
            }
            case Opcode::Const0:
            {
                PushValue<u32>(0);
                break;
            }
            case Opcode::Const1:
            {
                PushValue<u32>(1);
                break;
            }
            case Opcode::Const2:
            {
                PushValue<u32>(2);
                break;
            }
            case Opcode::Const3:
            {
                PushValue<u32>(3);
                break;
            }
            case Opcode::Const4:
            {
                PushValue<u32>(4);
                break;
            }
            case Opcode::ConstFF:
            {
                PushValue<u32>(0xFFFFFFFF);
                break;
            }
            case Opcode::Const7F:
            {
                PushValue<u32>(0x7FFFFFFF);
                break;
            }
            case Opcode::ConstN:
            {
                const u32 constant = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;

                PushValue<u32>(constant);
                break;
            }
            case Opcode::AddI32:
            {
                const i32 a = PopValue<i32>();
                const i32 b = PopValue<i32>();

                const i32 result = a + b;
                PushValue<i32>(result);
                break;
            }
            case Opcode::AddI64:
            {
                const i64 a = PopValue<i64>();
                const i64 b = PopValue<i64>();

                const i64 result = a + b;
                PushValue<i64>(result);
                break;
            }
            case Opcode::SubI32:
            {
                const i32 a = PopValue<i32>();
                const i32 b = PopValue<i32>();

                const i32 result = a - b;
                PushValue<i32>(result);
                break;
            }
            case Opcode::SubI64:
            {
                const i64 a = PopValue<i64>();
                const i64 b = PopValue<i64>();

                const i64 result = a - b;
                PushValue<i64>(result);
                break;
            }
            case Opcode::MulI32:
            {
                const i32 a = PopValue<i32>();
                const i32 b = PopValue<i32>();

                const i32 result = a * b;
                PushValue<i32>(result);
                break;
            }
            case Opcode::MulI64:
            {
                const i64 a = PopValue<i64>();
                const i64 b = PopValue<i64>();

                const i64 result = a * b;
                PushValue<i64>(result);
                break;
            }
            case Opcode::DivI32:
            {
                const i32 a = PopValue<i32>();
                const i32 b = PopValue<i32>();

                const i32 quotient = a / b;
                const i32 remainder = a % b;
                PushValue<i32>(quotient);
                PushValue<i32>(remainder);
                break;
            }
            case Opcode::DivI64:
            {
                const i64 a = PopValue<i64>();
                const i64 b = PopValue<i64>();
                
                const i64 quotient = a / b;
                const i64 remainder = a % b;
                PushValue<i64>(quotient);
                PushValue<i64>(remainder);
                break;
            }
            case Opcode::Call:
            {
                const u32 functionIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;

                PushValue(reinterpret_cast<uSys>(codePtr));
                ExecuteFunction(module->Functions()[functionIndex], module);
                break;
            }
            case Opcode::CallExt:
            {
                const u32 functionIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                const u16 moduleIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;

                PushValue(reinterpret_cast<uSys>(codePtr));
                const Module* const targetModule = m_Modules[moduleIndex].Get();

                ExecuteFunction(targetModule->Functions()[functionIndex], targetModule);
                break;
            }
            case Opcode::CallInd:
            {
                break;
            }
            default:
                break;
        }
    }

    m_StackPointer = localsHead - sizeof(uPtr);
}

void Emulator::PushLocal(const Function* function, const uSys localsHead, const u16 local) noexcept
{
    uSys offset = 0;

    if(local > 0)
    {
        offset = function->LocalOffsets()[local - 1];
    }

    const TypeInfo* typeInfo = function->LocalTypes()[local];

    uSys size;

    if(TypeInfo::IsPointer(typeInfo))
    {
        size = sizeof(void*);
    }
    else
    {
        size = TypeInfo::StripPointer(typeInfo)->Size();
    }

    (void) ::std::memcpy(m_Stack.arr() + m_StackPointer, m_Stack.arr() + localsHead + offset, size);

    m_StackPointer += size;
}

void Emulator::PopLocal(const Function* function, const uSys localsHead, const u16 local) noexcept
{
    uSys offset = 0;

    if(local > 0)
    {
        offset = function->LocalOffsets()[local - 1];
    }
    
    const TypeInfo* typeInfo = function->LocalTypes()[local];

    uSys size;

    if(TypeInfo::IsPointer(typeInfo))
    {
        size = sizeof(void*);
    }
    else
    {
        size = TypeInfo::StripPointer(typeInfo)->Size();
    }
    m_StackPointer -= size;

    (void) ::std::memcpy(m_Stack.arr() + localsHead + offset, m_Stack.arr() + m_StackPointer, size);
}

void Emulator::PushArgument(const uSys argument) noexcept
{
    if(argument >= 64)
    {
        return;
    }

    (void) ::std::memcpy(m_Stack.arr() + m_StackPointer, m_Arguments.arr() + argument, sizeof(u64));

    m_StackPointer += sizeof(u64);
}

void Emulator::PopArgument(const uSys argument) noexcept
{
    if(argument >= 64)
    {
        return;
    }
    
    m_StackPointer -= sizeof(u64);

    (void) ::std::memcpy(m_Arguments.arr() + argument, m_Stack.arr() + m_StackPointer, sizeof(u64));
}

void Emulator::DuplicateVal(const uSys byteCount) noexcept
{
    (void) ::std::memcpy(m_Stack.arr() + m_StackPointer - byteCount, m_Stack.arr(), byteCount);
    m_StackPointer += byteCount;
}

}
