#include "TauIR/IrToSsa.hpp"
#include "TauIR/Function.hpp"
#include "TauIR/Opcodes.hpp"
#include "TauIR/TypeInfo.hpp"

namespace tau::ir {

static ssa::SsaType GetSsaType(const TypeInfo& type) noexcept;

static ssa::SsaType GetSsaType(const TypeInfo* type) noexcept
{
    return GetSsaType(*TypeInfo::StripPointer(type));
}

ssa::SsaWriter IrToSsa::TransformFunction(const Function* const function) noexcept
{
    const u8* codePtr = reinterpret_cast<const u8*>(function->Address());

    SsaWriter writer;
    SsaFrameTracker frameTracker(function->LocalTypes().count());

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
            {
                const VarId localVar = frameTracker.GetLocal(0);
                const VarId newVar = writer.WriteAssignVariable(GetSsaType(function->LocalTypes()[0]), localVar);
                frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[0])->Size());
                break;
            }
            case Opcode::Push1:
            {
                const VarId localVar = frameTracker.GetLocal(1);
                const VarId newVar = writer.WriteAssignVariable(GetSsaType(function->LocalTypes()[1]), localVar);
                frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[1])->Size());
                break;
            }
            case Opcode::Push2:
            {
                const VarId localVar = frameTracker.GetLocal(2);
                const VarId newVar = writer.WriteAssignVariable(GetSsaType(function->LocalTypes()[2]), localVar);
                frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[2])->Size());
                break;
            }
            case Opcode::Push3:
            {
                const VarId localVar = frameTracker.GetLocal(3);
                const VarId newVar = writer.WriteAssignVariable(GetSsaType(function->LocalTypes()[3]), localVar);
                frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[3])->Size());
                break;
            }
            case Opcode::PushN:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                const VarId localVar = frameTracker.GetLocal(localIndex);
                const VarId newVar = writer.WriteAssignVariable(GetSsaType(function->LocalTypes()[localIndex]), localVar);
                frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[localIndex])->Size());
                break;
            }
            case Opcode::PushArg0:
            {
                const VarId newVar = writer.WriteAssignVariable(ssa::SsaType::U64, 0x80000000);
                frameTracker.PushFrame(newVar, 8);
                break;
            }
            case Opcode::PushArg1:
            {
                const VarId newVar = writer.WriteAssignVariable(ssa::SsaType::U64, 0x80000001);
                frameTracker.PushFrame(newVar, 8);
                break;
            }
            case Opcode::PushArg2:
            {
                const VarId newVar = writer.WriteAssignVariable(ssa::SsaType::U64, 0x80000002);
                frameTracker.PushFrame(newVar, 8);
                break;
            }
            case Opcode::PushArg3:
            {
                const VarId newVar = writer.WriteAssignVariable(ssa::SsaType::U64, 0x80000003);
                frameTracker.PushFrame(newVar, 8);
                break;
            }
            case Opcode::PushArgN:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                const VarId newVar = writer.WriteAssignVariable(ssa::SsaType::U64, localIndex | 0x80000000);
                frameTracker.PushFrame(newVar, 8);
                break;
            }
            case Opcode::PushPtr:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                const VarId localVar = frameTracker.GetLocal(localIndex);
                const VarId newVar = writer.WriteLoad(GetSsaType(function->LocalTypes()[localIndex]), localVar);
                frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[0])->Size());
                break;
            }
            case Opcode::Pop0:
            {
                HandlePop(function, writer, frameTracker, 0);
                break;
            }
            case Opcode::Pop1:
            {
                HandlePop(function, writer, frameTracker, 1);
                break;
            }
            case Opcode::Pop2:
            {
                HandlePop(function, writer, frameTracker, 2);
                break;
            }
            case Opcode::Pop3:
            {
                HandlePop(function, writer, frameTracker, 3);
                break;
            }
            case Opcode::PopN:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                HandlePop(function, writer, frameTracker, localIndex);
                break;
            }
            case Opcode::PopArg0:
            {
                HandlePop(function, writer, frameTracker, 0x80000000);
                break;
            }
            case Opcode::PopArg1:
            {
                HandlePop(function, writer, frameTracker, 0x80000001);
                break;
            }
            case Opcode::PopArg2:
            {
                HandlePop(function, writer, frameTracker, 0x80000002);
                break;
            }
            case Opcode::PopArg3:
            {
                HandlePop(function, writer, frameTracker, 0x80000003);
                break;
            }
            case Opcode::PopArgN:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                HandlePop(function, writer, frameTracker, localIndex | 0x80000000);
                break;
            }
            case Opcode::PopPtr:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                // const VarId localVar = frameTracker.GetLocal(localIndex);
                // const VarId frameVar = frameTracker.PopFrame(0);
                // writer.WriteStore(GetSsaType(function->LocalTypes()[localIndex]), localVar, frameVar);
                break;
            }
            case Opcode::PopCount:
            {
                break;
            }
            case Opcode::Dup1:
            {
                break;
            }
            case Opcode::Dup2:
            {
                break;
            }
            case Opcode::Dup4:
            {
                break;
            }
            case Opcode::Dup8:
            {
                break;
            }
            case Opcode::ExpandSX12:
            {
                break;
            }
            case Opcode::ExpandSX14:
            {
                break;
            }
            case Opcode::ExpandSX18:
            {
                break;
            }
            case Opcode::ExpandSX24:
            {
                break;
            }
            case Opcode::ExpandSX28:
            {
                break;
            }
            case Opcode::ExpandSX48:
            {
                break;
            }
            case Opcode::ExpandZX12:
            {
                break;
            }
            case Opcode::ExpandZX14:
            {
                break;
            }
            case Opcode::ExpandZX18:
            {
                break;
            }
            case Opcode::ExpandZX24:
            {
                break;
            }
            case Opcode::ExpandZX28:
            {
                break;
            }
            case Opcode::ExpandZX48:
            {
                break;
            }
            case Opcode::Trunc84:
            {
                break;
            }
            case Opcode::Trunc82:
            {
                break;
            }
            case Opcode::Trunc81:
            {
                break;
            }
            case Opcode::Trunc42:
            {
                break;
            }
            case Opcode::Trunc41:
            {
                break;
            }
            case Opcode::Trunc21:
            {
                break;
            }
            case Opcode::Const0:
            {
                break;
            }
            case Opcode::Const1:
            {
                break;
            }
            case Opcode::Const2:
            {
                break;
            }
            case Opcode::Const3:
            {
                break;
            }
            case Opcode::Const4:
            {
                break;
            }
            case Opcode::ConstFF:
            {
                break;
            }
            case Opcode::Const7F:
            {
                break;
            }
            case Opcode::ConstN:
            {
                break;
            }
            case Opcode::AddI32:
            {
                break;
            }
            case Opcode::AddI64:
            {
                break;
            }
            case Opcode::SubI32:
            {
                break;
            }
            case Opcode::SubI64:
            {
                break;
            }
            case Opcode::MulI32:
            {
                break;
            }
            case Opcode::MulI64:
            {
                break;
            }
            case Opcode::DivI32:
            {
                break;
            }
            case Opcode::DivI64:
            {
                break;
            }
            case Opcode::Call:
            {
                break;
            }
            case Opcode::CallExt:
            {
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

    return writer;
}

void IrToSsa::HandlePop(const Function* function, SsaWriter& writer, SsaFrameTracker& frameTracker, uSys localIndex)
{
    // Get the local type that we're going to pop into.
    const TypeInfo& typeInfo = *TypeInfo::StripPointer(function->LocalTypes()[0]);
    // Pop the first frame.
    auto frame = frameTracker.PopFrame(typeInfo.Size());
    // Track the total popped frame size for later.
    uSys frameSize = frame.Size;
    // If the total popped frame size matches the local type size, assign new var, otherwise join multiple vars.
    if(frameSize == typeInfo.Size())
    {
        // Assign new var from the popped frame.
        const VarId newVar = writer.WriteAssignVariable(GetSsaType(typeInfo), frame.Var);
        // Track the local.
        frameTracker.SetLocal(newVar, 0);
    }
    else
    {
        // The max number of elements we can reasonably expect to show up in a single merge-pop. This will be the source of many buffer overflow exploits.
        constexpr uSys packSize = 16;

        // Setup a buffer of types for the join.
        ssa::SsaCustomType types[packSize];
        // Setup a buffer of vars for the join.
        SsaWriter::VarId vars[packSize];

        // The current buffer write index. We will write in reverse order to prevent having to do an array reverse later.
        iSys index = packSize - 1;

        // Write the first popped frame type to the last element.
        types[index] = writer.GetVarType(frame.Var);
        // Write the first popped frame var to the last element.
        vars[index] = frame.Var;

        // Loop until we've popped enough frame to meet the size of the local type.
        while(frameSize < typeInfo.Size())
        {
            // Pop the frame.
            frame = frameTracker.PopFrame(typeInfo.Size());

            // Decrement the index before writing, we'll use this index later, so it's better to decrement before rather than after.
            --index;
            // Write the most recently popped frame type.
            types[index] = writer.GetVarType(frame.Var);
            // Write the most recent popped frame var.
            vars[index] = frame.Var;
            // Add the frame size on to the total popped frame size.
            frameSize += frame.Size;
        }

        // If frame size is larger than the local type size we'll need to perform a split.
        if(frameSize > typeInfo.Size())
        {
            // Find how many bytes spill over the highest byte of local type.
            const u32 sizeSpill = static_cast<u32>(frameSize - typeInfo.Size());
            // Setup an array to split the most recently popped frame into two vars.
            ssa::SsaCustomType splitTypes[2];
            // Set the first split type to be raw bytes with the spill size.
            splitTypes[0] = ssa::SsaCustomType(ssa::SsaType::Bytes, sizeSpill);
            // Set the second split type to be the raw bytes with remainder size. This is the one that will be stored into the local type.
            splitTypes[1] = ssa::SsaCustomType(ssa::SsaType::Bytes, static_cast<u32>(frame.Size - sizeSpill));

            // Write the split, splitBase is the index of the first var, splitBase + 1 is the index of the var that will be stored into the local type.
            const VarId splitBase = writer.WriteSplit(writer.GetVarType(frame.Var), frame.Var, 2, splitTypes);
            // Replace the most recent type with the second split type.
            types[index] = splitTypes[1];
            // Replace the most recent var with the second split var.
            vars[index] = splitBase + 1;
            // Push the first split var onto the frame tracker stack.
            frameTracker.PushFrame(splitBase, sizeSpill);
        }

        // Write the join of all the vars.
        const VarId newVar = writer.WriteJoin(GetSsaType(typeInfo), 0, types + index, vars + index);
        // Track the local.
        frameTracker.SetLocal(newVar, 0);
    }
}

    
static ssa::SsaType GetSsaType(const TypeInfo& type) noexcept
{
    if(type == TypeInfo::Void)
    {
        return ssa::SsaType::Void;
    }
    else if(type == TypeInfo::Bool)
    {
        return ssa::SsaType::Bool;
    }
    else if(type == TypeInfo::I8)
    {
        return ssa::SsaType::I8;
    }
    else if(type == TypeInfo::I16)
    {
        return ssa::SsaType::I16;
    }
    else if(type == TypeInfo::I32)
    {
        return ssa::SsaType::I32;
    }
    else if(type == TypeInfo::I64)
    {
        return ssa::SsaType::I64;
    }
    else if(type == TypeInfo::U8)
    {
        return ssa::SsaType::U8;
    }
    else if(type == TypeInfo::U16)
    {
        return ssa::SsaType::U16;
    }
    else if(type == TypeInfo::U32)
    {
        return ssa::SsaType::U32;
    }
    else if(type == TypeInfo::U64)
    {
        return ssa::SsaType::U64;
    }
    else if(type == TypeInfo::F32)
    {
        return ssa::SsaType::F32;
    }
    else if(type == TypeInfo::F64)
    {
        return ssa::SsaType::F64;
    }
    else if(type == TypeInfo::Char)
    {
        return ssa::SsaType::Char;
    }
    else
    {
        return ssa::SsaType::Custom;
    }
}

}
