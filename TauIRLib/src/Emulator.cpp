#include "TauIR/Emulator.hpp"

#include "TauIR/Function.hpp"
#include "TauIR/Module.hpp"
#include "TauIR/Opcodes.hpp"
#include "TauIR/TypeInfo.hpp"

namespace tau::ir {

void Emulator::Execute() noexcept
{
    // There needs to be at least one module to run.
    // There should probably be more for native libraries.
    if(m_Modules.empty())
    {
        return;
    }

    // Get a reference to the first module.
    // The entry-point is the first function in the first module.
    const ModuleRef& mainModule = m_Modules[0];

    // If the first module is null we have some problems.
    if(!mainModule)
    {
        return;
    }

    // If there are 0 functions in the module we don't have to run.
    if(mainModule->Functions().count() == 0)
    {
        return;
    }

    const Function* const entryPoint = mainModule->Functions()[0];

    Executor(entryPoint, mainModule.Get());
}

template<typename T>
static T ReadCodeValue(const u8*& codePtr)
{
    const T ret = *reinterpret_cast<const T*>(codePtr);
    codePtr += sizeof(T);
    return ret;
}

void Emulator::Executor(const Function* function, const Module* module) noexcept
{
#define CALL_PUSH() \
    PushValueLocal(m_LocalsStackPointer); \
    PushValueLocal(localsHead);           \
    PushValueLocal(module);               \
    PushValueLocal(function);             \
    PushValueLocal(codePtr)
#define RET_POP() \
    codePtr = PopValueLocal<const u8*>();          \
    function = PopValueLocal<const Function*>();  \
    module = PopValueLocal<const Module*>();      \
    localsHead = PopValueLocal<uSys>();           \
    m_LocalsStackPointer = PopValueLocal<uSys>()

    const u8* codePtr = function->Address();

    uSys callDepth = 0;
    uSys localsHead = m_LocalsStackPointer;
    m_LocalsStackPointer += function->LocalSize();
    
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
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
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
                const u8 argumentIndex = ReadCodeValue<u8>(codePtr);
                PushArgument(argumentIndex);
                break;
            }
            case Opcode::PushPtr:
            {
                // Read the index of the pointer to pop into.
                const u16 localIndex = ReadCodeValue<u16>(codePtr);

                // Get the size and offset of the local.
                const auto [localSize, localOffset] = GetLocalUnderlyingSizeAndOffset(function, localIndex);

                // Load the pointer from the locals.
                const void* localPointer = GetLocal<void*>(function, localsHead, localIndex);

                // Copy the value.
                (void) ::std::memcpy(m_ExecutionStack.arr() + m_ExecutionStackPointer, localPointer, localSize);

                // Offset the stack.
                m_ExecutionStackPointer += localSize;
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
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
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
                const u8 argumentIndex = ReadCodeValue<u8>(codePtr);
                PopArgument(argumentIndex);
                break;
            }
            case Opcode::PopPtr:
            {
                // Read the index of the pointer to pop into.
                const u16 localIndex = ReadCodeValue<u16>(codePtr);

                // Get the size and offset of the local.
                const auto [localSize, _] = GetLocalUnderlyingSizeAndOffset(function, localIndex);

                // Load the pointer from the locals.
                void* localPointer = GetLocal<void*>(function, localsHead, localIndex);

                // Offset the stack.
                m_ExecutionStackPointer -= localSize;

                // Copy the value.
                (void) ::std::memcpy(localPointer, m_ExecutionStack.arr() + m_ExecutionStackPointer, localSize);

                break;
            }
            case Opcode::PopCount:
            {
                const u16 count = ReadCodeValue<u16>(codePtr);

                m_ExecutionStackPointer -= count;
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
            case Opcode::Load:
            {
                const u16 loadVar = ReadCodeValue<u16>(codePtr);
                const u16 addressVar = ReadCodeValue<u16>(codePtr);

                // Get the pointer to load from.
                const void* addressPtr = GetLocal<void*>(function, localsHead, addressVar);
                // Copy from that pointer into the local.
                SetLocal(function, localsHead, loadVar, addressPtr);

                break;
            }
            case Opcode::Store:
            {
                const u16 loadVar = ReadCodeValue<u16>(codePtr);
                const u16 addressVar = ReadCodeValue<u16>(codePtr);

                // Get the size and offset of the local.
                const auto [storageSize, localOffset] = GetLocalSizeAndOffset(function, loadVar);

                // Get the pointer to store into.
                void* addressPtr = GetLocal<void*>(function, localsHead, addressVar);

                // Copy from the local into the address.
                (void) ::std::memcpy(addressPtr, m_LocalsStack.arr() + localsHead + localOffset, storageSize);

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
                const u32 constant = ReadCodeValue<u32>(codePtr);
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
            case Opcode::CompI32Above:
            case Opcode::CompI32AboveOrEqual:
            case Opcode::CompI32Below:
            case Opcode::CompI32BelowOrEqual:
            case Opcode::CompI32Equal:
            case Opcode::CompI32Greater:
            case Opcode::CompI32GreaterOrEqual:
            case Opcode::CompI32Less:
            case Opcode::CompI32LessOrEqual:
            case Opcode::CompI32NotEqual:
            {
                const i32 a = PopValue<i32>();
                const i32 b = PopValue<i32>();

                u8 result;

                switch(opcode)
                {
                    case Opcode::CompI32Above:
                        result = static_cast<u32>(a) > static_cast<u32>(b) ? 1 : 0;
                        break;
                    case Opcode::CompI32AboveOrEqual:
                        result = static_cast<u32>(a) >= static_cast<u32>(b) ? 1 : 0;
                        break;
                    case Opcode::CompI32Below:
                        result = static_cast<u32>(a) < static_cast<u32>(b) ? 1 : 0;
                        break;
                    case Opcode::CompI32BelowOrEqual:
                        result = static_cast<u32>(a) <= static_cast<u32>(b) ? 1 : 0;
                        break;
                    case Opcode::CompI32Equal:
                        result = a == b ? 1 : 0;
                        break;
                    case Opcode::CompI32Greater:
                        result = a > b ? 1 : 0;
                        break;
                    case Opcode::CompI32GreaterOrEqual:
                        result = a >= b ? 1 : 0;
                        break;
                    case Opcode::CompI32Less:
                        result = a < b ? 1 : 0;
                        break;
                    case Opcode::CompI32LessOrEqual:
                        result = a <= b ? 1 : 0;
                        break;
                    case Opcode::CompI32NotEqual:
                        result = a != b ? 1 : 0;
                        break;
                    default:
                        result = false;
                        break;
                }
                
                PushValue<u8>(result);
                break;
            }
            case Opcode::CompI64Above:
            case Opcode::CompI64AboveOrEqual:
            case Opcode::CompI64Below:
            case Opcode::CompI64BelowOrEqual:
            case Opcode::CompI64Equal:
            case Opcode::CompI64Greater:
            case Opcode::CompI64GreaterOrEqual:
            case Opcode::CompI64Less:
            case Opcode::CompI64LessOrEqual:
            case Opcode::CompI64NotEqual:
            {
                const i64 a = PopValue<i64>();
                const i64 b = PopValue<i64>();

                u8 result;

                switch(opcode)
                {
                    case Opcode::CompI64Above:
                        result = static_cast<u64>(a) > static_cast<u64>(b) ? 1 : 0;
                        break;
                    case Opcode::CompI64AboveOrEqual:
                        result = static_cast<u64>(a) >= static_cast<u64>(b) ? 1 : 0;
                        break;
                    case Opcode::CompI64Below:
                        result = static_cast<u64>(a) < static_cast<u64>(b) ? 1 : 0;
                        break;
                    case Opcode::CompI64BelowOrEqual:
                        result = static_cast<u64>(a) <= static_cast<u64>(b) ? 1 : 0;
                        break;
                    case Opcode::CompI64Equal:
                        result = a == b ? 1 : 0;
                        break;
                    case Opcode::CompI64Greater:
                        result = a > b ? 1 : 0;
                        break;
                    case Opcode::CompI64GreaterOrEqual:
                        result = a >= b ? 1 : 0;
                        break;
                    case Opcode::CompI64Less:
                        result = a < b ? 1 : 0;
                        break;
                    case Opcode::CompI64LessOrEqual:
                        result = a <= b ? 1 : 0;
                        break;
                    case Opcode::CompI64NotEqual:
                        result = a != b ? 1 : 0;
                        break;
                    default:
                        result = false;
                        break;
                }

                PushValue<u8>(result);
                break;
            }
            case Opcode::Call:
            {
                const u32 functionIndex = ReadCodeValue<u32>(codePtr);
                
                CALL_PUSH();

                const Function* nextFunction = module->Functions()[functionIndex];

                codePtr = nextFunction->Address();
                localsHead = m_LocalsStackPointer;
                m_LocalsStackPointer += nextFunction->LocalSize();
                ++callDepth;
                break;
            }
            case Opcode::CallExt:
            {
                const u32 functionIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                
                const Module* const targetModule = m_Modules[moduleIndex].Get();

                if(targetModule->IsNative())
                {
                    ::tau::ir::CallNativeFunctionPointer(targetModule->Functions()[functionIndex], m_Arguments, m_ExecutionStack, m_ExecutionStackPointer);
                }
                else
                {
                    CALL_PUSH();

                    const Function* nextFunction = targetModule->Functions()[functionIndex];

                    codePtr = nextFunction->Address();
                    localsHead = m_LocalsStackPointer;
                    m_LocalsStackPointer += nextFunction->LocalSize();
                    ++callDepth;
                }
                break;
            }
            case Opcode::CallInd:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                const u32 functionIndex = GetLocal<u32>(function, localsHead, localIndex);

                CALL_PUSH();

                const Function* nextFunction = module->Functions()[functionIndex];

                codePtr = nextFunction->Address();
                localsHead = m_LocalsStackPointer;
                m_LocalsStackPointer += nextFunction->LocalSize();
                ++callDepth;
                break;
            }
            case Opcode::CallIndExt:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                const u16 moduleIndex = PopValue<u16>();

                const u32 functionIndex = GetLocal<u32>(function, localsHead, localIndex);

                const Module* const targetModule = m_Modules[moduleIndex].Get();

                if(targetModule->IsNative())
                {
                    ::tau::ir::CallNativeFunctionPointer(targetModule->Functions()[functionIndex], m_Arguments, m_ExecutionStack, m_ExecutionStackPointer);
                }
                else
                {
                    CALL_PUSH();

                    const Function* nextFunction = targetModule->Functions()[functionIndex];

                    codePtr = nextFunction->Address();
                    localsHead = m_LocalsStackPointer;
                    m_LocalsStackPointer += nextFunction->LocalSize();
                    ++callDepth;
                }
                break;
            }
            case Opcode::Ret:
            {
                if(callDepth == 0)
                {
                    return;
                }
                --callDepth;
                RET_POP();

                break;
            }
            case Opcode::Jump:
            {
                const i32 jumpOffset = ReadCodeValue<i32>(codePtr);
                codePtr += jumpOffset;

                break;
            }
            case Opcode::JumpTrue:
            {
                const i32 jumpOffset = ReadCodeValue<i32>(codePtr);
                const u8 condition = PopValue<u8>();

                if(condition != 0)
                {
                    codePtr += jumpOffset;
                }

                break;
            }
            case Opcode::JumpFalse:
            {
                const i32 jumpOffset = ReadCodeValue<i32>(codePtr);
                const u8 condition = PopValue<u8>();

                if(condition == 0)
                {
                    codePtr += jumpOffset;
                }

                break;
            }
            default:
                break;
        }
    }
}

void Emulator::PushLocal(const Function* const function, const uSys localsHead, const u16 local) noexcept
{
    // Get the size and offset of the local.
    const auto [size, offset] = GetLocalSizeAndOffset(function, local);

    // Copy the value.
    (void) ::std::memcpy(m_ExecutionStack.arr() + m_ExecutionStackPointer, m_LocalsStack.arr() + localsHead + offset, size);

    // Adjust the stack pointer.
    m_ExecutionStackPointer += size;
}

void Emulator::PopLocal(const Function* const function, const uSys localsHead, const u16 local) noexcept
{
    // Get the size and offset of the local.
    const auto [size, offset] = GetLocalSizeAndOffset(function, local);

    // Adjust the stack pointer.
    m_ExecutionStackPointer -= size;

    // Copy the value.
    (void) ::std::memcpy(m_LocalsStack.arr() + localsHead + offset, m_ExecutionStack.arr() + m_ExecutionStackPointer, size);
}

void Emulator::PushArgument(const uSys argument) noexcept
{
    // Check if the target argument is greater than the number of arguments we allow.
    if(argument >= MaxArgumentRegisters)
    {
        return;
    }

    // Copy the value.
    (void) ::std::memcpy(m_ExecutionStack.arr() + m_ExecutionStackPointer, m_Arguments.arr() + argument, sizeof(ArgumentRegisterType));

    // Adjust the stack pointer.
    m_ExecutionStackPointer += sizeof(ArgumentRegisterType);
}

void Emulator::PopArgument(const uSys argument) noexcept
{
    // Check if the target argument is greater than the number of arguments we allow.
    if(argument >= MaxArgumentRegisters)
    {
        return;
    }

    // Adjust the stack pointer.
    m_ExecutionStackPointer -= sizeof(ArgumentRegisterType);

    // Copy the value.
    (void) ::std::memcpy(m_Arguments.arr() + argument, m_ExecutionStack.arr() + m_ExecutionStackPointer, sizeof(ArgumentRegisterType));
}

void Emulator::DuplicateVal(const uSys byteCount) noexcept
{
    (void) ::std::memcpy(m_ExecutionStack.arr() + m_ExecutionStackPointer, m_ExecutionStack.arr() + m_ExecutionStackPointer - byteCount, byteCount);
    m_ExecutionStackPointer += byteCount;
}

void Emulator::SetLocal(const Function* const function, const uSys localsHead, const u16 local, const void* const data) noexcept
{
    // Get the size and offset of the local.
    const auto [size, offset] = GetLocalSizeAndOffset(function, local);

    // Copy the value.
    (void) ::std::memcpy(m_LocalsStack.arr() + localsHead + offset, data, size);
}

Emulator::SizeAndOffset Emulator::GetLocalSizeAndOffset(const Function* function, const u16 local) noexcept
{
    // The offset into our view of the locals stack.
    uSys offset = 0;

    // Local 0 is always at offset 0.
    if(local > 0)
    {
        // Get the offset from the function metadata.
        //   We decrement by one because we don't bother storing an offset of 0
        // for the first local.
        offset = function->LocalOffsets()[local - 1];
    }

    // Get the type info metadata for the value we're about to push.
    const TypeInfo* typeInfo = function->LocalTypes()[local];

    //   If the type is a pointer, we want the size of a pointer, and not
    // the underlying type. All pointers are the same size.
    if(TypeInfo::IsPointer(typeInfo))
    {
        return { PointerSize, offset };
    }
    else
    {
        //   Make sure the type info pointer doesn't have any tag metadata, and
        // then get its size.
        return { TypeInfo::StripPointer(typeInfo)->Size(), offset };
    }
}

Emulator::SizeAndOffset Emulator::GetLocalUnderlyingSizeAndOffset(const Function* function, const u16 local) noexcept
{
    // The offset into our view of the locals stack.
    uSys offset = 0;

    // Local 0 is always at offset 0.
    if(local > 0)
    {
        // Get the offset from the function metadata.
        //   We decrement by one because we don't bother storing an offset of 0
        // for the first local.
        offset = function->LocalOffsets()[local - 1];
    }

    // Get the type info metadata for the value we're about to push.
    const TypeInfo* typeInfo = function->LocalTypes()[local];
    
    //   Make sure the type info pointer doesn't have any tag metadata, and
    // then get its size.
    return { TypeInfo::StripPointer(typeInfo)->Size(), offset };
}

}
