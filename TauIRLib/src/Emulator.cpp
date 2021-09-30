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

    ExecuteFunction(entryPoint);
}

void Emulator::ExecuteFunction(const Function* function) noexcept
{
    const u8* codePtr = reinterpret_cast<const u8*>(reinterpret_cast<const void*>(function->Address()));

    // Unused while emulating.
    // const uPtr prevFunctionPtr = *(reinterpret_cast<const uPtr*>(m_Stack.arr() + m_StackPointer) - 1);

    const uSys localsHead = m_StackPointer;
    m_StackPointer += function->LocalSize();

#if 1
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

    const uSys size = function->LocalTypes()[local]->Size();

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

    const uSys size = function->LocalTypes()[local]->Size();

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

}
