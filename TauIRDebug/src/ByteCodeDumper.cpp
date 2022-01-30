#include "TauIR/ByteCodeDumper.hpp"
#include "TauIR/Function.hpp"
#include "TauIR/Opcodes.hpp"
#include "TauIR/ssa/SsaOpcodes.hpp"
#include "TauIR/ssa/SsaTypes.hpp"
#include <ConPrinter.hpp>



namespace tau::ir {
 
void DumpFunction(const tau::ir::Function* function, const uSys functionIndex) noexcept
{
    const u8* codePtr = reinterpret_cast<const u8*>(function->Address());

    ConPrinter::Print("Func{}:\n", functionIndex);

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
            ConPrinter::Print("    Ret\n");
            break;
        }

        switch(opcode)
        {
            case Opcode::Nop:
                ConPrinter::Print("    Nop\n");
                break;
            case Opcode::Push0:
                ConPrinter::Print("    Push.0\n");
                break;
            case Opcode::Push1:
                ConPrinter::Print("    Push.1\n");
                break;
            case Opcode::Push2:
                ConPrinter::Print("    Push.2\n");
                break;
            case Opcode::Push3:
                ConPrinter::Print("    Push.3\n");
                break;
            case Opcode::PushN:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Push.N {}\n", localIndex);
                break;
            }
            case Opcode::PushArg0:
                ConPrinter::Print("    Push.Arg.0\n");
                break;
            case Opcode::PushArg1:
                ConPrinter::Print("    Push.Arg.1\n");
                break;
            case Opcode::PushArg2:
                ConPrinter::Print("    Push.Arg.2\n");
                break;
            case Opcode::PushArg3:
                ConPrinter::Print("    Push.Arg.3\n");
                break;
            case Opcode::PushArgN:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Push.Arg.N {}\n", localIndex);
                break;
            }
            case Opcode::PushPtr:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Push.Ptr {}\n", localIndex);
                break;
            }
            case Opcode::PushGlobal:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                
                ConPrinter::Print("    Push.Global {}\n", globalIndex);
                break;
            }
            case Opcode::PushGlobalExt:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                const u16 moduleIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Push.Global.Ext {}, {}\n", globalIndex, moduleIndex);
                break;
            }
            case Opcode::PushGlobalPtr:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                
                ConPrinter::Print("    Push.Global.Ptr {}\n", globalIndex);
                break;
            }
            case Opcode::PushGlobalExtPtr:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                const u16 moduleIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Push.Global.Ext.Ptr {}, {}\n", globalIndex, moduleIndex);
                break;
            }
            case Opcode::Pop0:
                ConPrinter::Print("    Pop.0\n");
                break;
            case Opcode::Pop1:
                ConPrinter::Print("    Pop.1\n");
                break;
            case Opcode::Pop2:
                ConPrinter::Print("    Pop.2\n");
                break;
            case Opcode::Pop3:
                ConPrinter::Print("    Pop.3\n");
                break;
            case Opcode::PopN:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Pop.N {}\n", localIndex);
                break;
            }
            case Opcode::PopArg0:
                ConPrinter::Print("    Pop.Arg.0\n");
                break;
            case Opcode::PopArg1:
                ConPrinter::Print("    Pop.Arg.1\n");
                break;
            case Opcode::PopArg2:
                ConPrinter::Print("    Pop.Arg.2\n");
                break;
            case Opcode::PopArg3:
                ConPrinter::Print("    Pop.Arg.3\n");
                break;
            case Opcode::PopArgN:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Pop.Arg.N {}\n", localIndex);
                break;
            }
            case Opcode::PopPtr:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Pop.Ptr {}\n", localIndex);
                break;
            }
            case Opcode::PopGlobal:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                
                ConPrinter::Print("    Pop.Global {}\n", globalIndex);
                break;
            }
            case Opcode::PopGlobalExt:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                const u16 moduleIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Pop.Global.Ext {}, {}\n", globalIndex, moduleIndex);
                break;
            }
            case Opcode::PopGlobalPtr:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                
                ConPrinter::Print("    Pop.Global.Ptr {}\n", globalIndex);
                break;
            }
            case Opcode::PopGlobalExtPtr:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                const u16 moduleIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Pop.Global.Ext.Ptr {}, {}\n", globalIndex, moduleIndex);
                break;
            }
            case Opcode::PopCount:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Pop.Count {}\n", localIndex);
                break;
            }
            case Opcode::Dup1:
                ConPrinter::Print("    Dup.1\n");
                break;
            case Opcode::Dup2:
                ConPrinter::Print("    Dup.2\n");
                break;
            case Opcode::Dup4:
                ConPrinter::Print("    Dup.4\n");
                break;
            case Opcode::Dup8:
                ConPrinter::Print("    Dup.8\n");
                break;
            case Opcode::ExpandSX12:
                ConPrinter::Print("    Expand.SX.1.2\n");
                break;
            case Opcode::ExpandSX14:
                ConPrinter::Print("    Expand.SX.1.4\n");
                break;
            case Opcode::ExpandSX18:
                ConPrinter::Print("    Expand.SX.1.8\n");
                break;
            case Opcode::ExpandSX24:
                ConPrinter::Print("    Expand.SX.2.4\n");
                break;
            case Opcode::ExpandSX28:
                ConPrinter::Print("    Expand.SX.2.8\n");
                break;
            case Opcode::ExpandSX48:
                ConPrinter::Print("    Expand.SX.4.8\n");
                break;
            case Opcode::ExpandZX12:
                ConPrinter::Print("    Expand.ZX.1.2\n");
                break;
            case Opcode::ExpandZX14:
                ConPrinter::Print("    Expand.ZX.1.4\n");
                break;
            case Opcode::ExpandZX18:
                ConPrinter::Print("    Expand.ZX.1.8\n");
                break;
            case Opcode::ExpandZX24:
                ConPrinter::Print("    Expand.ZX.2.4\n");
                break;
            case Opcode::ExpandZX28:
                ConPrinter::Print("    Expand.ZX.2.8\n");
                break;
            case Opcode::ExpandZX48:
                ConPrinter::Print("    Expand.ZX.4.8\n");
                break;
            case Opcode::Trunc84:
                ConPrinter::Print("    Trunc.8.4\n");
                break;
            case Opcode::Trunc82:
                ConPrinter::Print("    Trunc.8.2\n");
                break;
            case Opcode::Trunc81:
                ConPrinter::Print("    Trunc.8.1\n");
                break;
            case Opcode::Trunc42:
                ConPrinter::Print("    Trunc.4.2\n");
                break;
            case Opcode::Trunc41:
                ConPrinter::Print("    Trunc.4.1\n");
                break;
            case Opcode::Trunc21:
                ConPrinter::Print("    Trunc.2.1\n");
                break;
            case Opcode::Load:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                const u16 addressIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Load {}, {}\n", localIndex, addressIndex);
                break;
            }
            case Opcode::LoadGlobal:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                const u16 addressIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Load.Global {}, {}\n", globalIndex, addressIndex);
                break;
            }
            case Opcode::LoadGlobalExt:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                const u16 addressIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                const u16 moduleIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Load.Global.Ext {}, {}, {}\n", globalIndex, addressIndex, moduleIndex);
                break;
            }
            case Opcode::Store:
            {
                const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                const u16 addressIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Store {}, {}\n", localIndex, addressIndex);
                break;
            }
            case Opcode::StoreGlobal:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                const u16 addressIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Store.Global {}, {}\n", globalIndex, addressIndex);
                break;
            }
            case Opcode::StoreGlobalExt:
            {
                const u32 globalIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                const u16 addressIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                const u16 moduleIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Store.Global.Ext {}, {}, {}\n", globalIndex, addressIndex, moduleIndex);
                break;
            }
            case Opcode::Const0:
                ConPrinter::Print("    Const.0\n");
                break;
            case Opcode::Const1:
                ConPrinter::Print("    Const.1\n");
                break;
            case Opcode::Const2:
                ConPrinter::Print("    Const.2\n");
                break;
            case Opcode::Const3:
                ConPrinter::Print("    Const.3\n");
                break;
            case Opcode::Const4:
                ConPrinter::Print("    Const.4\n");
                break;
            case Opcode::ConstFF:
                ConPrinter::Print("    Const.FF\n");
                break;
            case Opcode::Const7F:
                ConPrinter::Print("    Const.7F\n");
                break;
            case Opcode::ConstN:
            {
                const u32 constant = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                
                ConPrinter::Print("    Const.N {}\n", constant);
                break;
            }
            case Opcode::AddI32:
                ConPrinter::Print("    Add.i32\n");
                break;
            case Opcode::AddI64:
                ConPrinter::Print("    Add.i64\n");
                break;
            case Opcode::SubI32:
                ConPrinter::Print("    Sub.i32\n");
                break;
            case Opcode::SubI64:
                ConPrinter::Print("    Sub.i64\n");
                break;
            case Opcode::MulI32:
                ConPrinter::Print("    Mul.i32\n");
                break;
            case Opcode::MulI64:
                ConPrinter::Print("    Mul.i64\n");
                break;
            case Opcode::DivI32:
                ConPrinter::Print("    Div.i32\n");
                break;
            case Opcode::DivI64:
                ConPrinter::Print("    Div.i64\n");
                break;
            case Opcode::Call:
            {
                const u32 targetFunctionIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                
                ConPrinter::Print("    Call <Func{}>\n", targetFunctionIndex);
                break;
            }
            case Opcode::CallExt:
            {
                const u32 targetFunctionIndex = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                const u16 moduleIndex = *reinterpret_cast<const u16*>(codePtr);
                codePtr += 2;
                
                ConPrinter::Print("    Call.Ext <Func{}>, {}\n", targetFunctionIndex, moduleIndex);
                break;
            }
            case Opcode::CallInd:
                ConPrinter::Print("    Call.Ind\n");
                break;
            case Opcode::Jump:
            {
                const u32 offset = *reinterpret_cast<const u32*>(codePtr);
                codePtr += 4;
                
                ConPrinter::Print("    Jump {}\n", offset);
                break;
            }
            case Opcode::Ret:
                ConPrinter::print("    Ret");
                return;
        }
    }
}

namespace ssa {

template<typename T>
static inline T ReadType(const u8* const codePtr, uSys& i) noexcept
{
    T ret {};
    (void) ::std::memcpy(::std::addressof(ret), codePtr + i, sizeof(T));
    i += sizeof(T);
    return ret;
}

template<>
inline SsaCustomType ReadType<SsaCustomType>(const u8* const codePtr, uSys& i) noexcept
{
    const SsaType typeId = static_cast<SsaType>(codePtr[i++]);

    u32 customType = -1;

    if(StripPointer(typeId) == SsaType::Bytes || StripPointer(typeId) == SsaType::Custom)
    {
        customType = ReadType<u32>(codePtr, i);
    }

    return SsaCustomType(typeId, customType);
}

void PrintType(const SsaCustomType type) noexcept
{
    if(StripPointer(type.Type) == SsaType::Custom)
    {
        ConPrinter::Print("<{}>", type.CustomType);
    }
    else if(StripPointer(type.Type) == SsaType::Bytes)
    {
        ConPrinter::Print("bytes<{}>", type.CustomType);
    }
    else
    {
        switch(StripPointer(type.Type))
        {
        	case SsaType::Void:
                ConPrinter::Print("void");
                break;
            case SsaType::Bool:
                ConPrinter::Print("bool");
                break;
            case SsaType::I8:
                ConPrinter::Print("i8");
                break;
            case SsaType::I16:
                ConPrinter::Print("i16");
                break;
            case SsaType::I32:
                ConPrinter::Print("i32");
                break;
            case SsaType::I64:
                ConPrinter::Print("i64");
                break;
            case SsaType::U8:
                ConPrinter::Print("u8");
                break;
            case SsaType::U16:
                ConPrinter::Print("u16");
                break;
            case SsaType::U32:
                ConPrinter::Print("u32");
                break;
            case SsaType::U64:
                ConPrinter::Print("u64");
                break;
            case SsaType::F16:
                ConPrinter::Print("f16");
                break;
            case SsaType::F32:
                ConPrinter::Print("f32");
                break;
            case SsaType::F64:
                ConPrinter::Print("f64");
                break;
            case SsaType::Char:
                ConPrinter::Print("char");
                break;
            default:
                ConPrinter::Print("unknown");
                break;
        }
    }

    if(IsPointer(type.Type))
    {
        ConPrinter::Print('*');
    }
}

void PrintValue(const SsaCustomType type, const u8* const codePtr, uSys& i, const SsaCustomTypeRegistry& registry) noexcept
{
    if(IsPointer(type.Type))
    {
        ConPrinter::Print("0x{XP}", ReadType<u64>(codePtr, i));
        return;
    }

    switch(type.Type)
    {
        case SsaType::Bool:
            ConPrinter::Print(ReadType<bool>(codePtr, i) ? "true" : "false");
            break;
        case SsaType::I8:
            ConPrinter::Print(ReadType<i8>(codePtr, i));
            break;
        case SsaType::I16:
            ConPrinter::Print(ReadType<i16>(codePtr, i));
            break;
        case SsaType::I32:
            ConPrinter::Print(ReadType<i32>(codePtr, i));
            break;
        case SsaType::I64:
            ConPrinter::Print(ReadType<i64>(codePtr, i));
            break;
        case SsaType::U8:
            ConPrinter::Print(ReadType<u8>(codePtr, i));
            break;
        case SsaType::U16:
            ConPrinter::Print(ReadType<u16>(codePtr, i));
            break;
        case SsaType::U32:
            ConPrinter::Print(ReadType<u32>(codePtr, i));
            break;
        case SsaType::U64:
            ConPrinter::Print(ReadType<u64>(codePtr, i));
            break;
        case SsaType::F16:
            ConPrinter::Print("{X}f", ReadType<u16>(codePtr, i));
            break;
        case SsaType::F32:
            ConPrinter::Print(ReadType<f32>(codePtr, i));
            break;
        case SsaType::F64:
            ConPrinter::Print(ReadType<f64>(codePtr, i));
            break;
        case SsaType::Char:
            ConPrinter::Print("''\\x{XP}''", ReadType<u8>(codePtr, i));
            break;
        case SsaType::Bytes:
            for(uSys j = 0; j < type.CustomType; ++j)
            {
                if(j != 0)
                {
                    ConPrinter::Print(' ');
                }
                ConPrinter::Print("{XP}", ReadType<u8>(codePtr, i));
            }
            break;
        case SsaType::Custom:
            ConPrinter::Print("custom");
            break;
        default:
            ConPrinter::Print("unknown");
            break;
    }
}

static void PrintBinaryOp(const SsaBinaryOperation op) noexcept
{
	switch(op)
	{
		case SsaBinaryOperation::Add:
            ConPrinter::Print('+');
            break;
		case SsaBinaryOperation::Sub:
            ConPrinter::Print('-');
            break;
		case SsaBinaryOperation::Mul:
            ConPrinter::Print('*');
            break;
		case SsaBinaryOperation::Div:
            ConPrinter::Print('/');
            break;
		case SsaBinaryOperation::Rem:
            ConPrinter::Print('%');
            break;
		case SsaBinaryOperation::BitShiftLeft:
            ConPrinter::Print("<<");
            break;
		case SsaBinaryOperation::BitShiftRight:
            ConPrinter::Print(">>");
            break;
        case SsaBinaryOperation::BarrelShiftLeft:
            ConPrinter::Print("<<");
            break;
        case SsaBinaryOperation::BarrelShiftRight:
            ConPrinter::Print(">>>");
            break;
	}
}

void PrintVar(const VarId var)
{
	if((var & 0x80000000) != 0)
	{
        ConPrinter::Print("%a{}", var & 0x7FFFFFFF);
	}
    else
    {
        ConPrinter::Print("%{}", var);
    }
}

void DumpSsa(const u8* codePtr, const uSys length, const uSys functionIndex, const SsaCustomTypeRegistry& registry) noexcept
{
    ConPrinter::Print("Func{}:\n", functionIndex);

    uSys idIndex = 1;

    for(uSys i = 0; i < length;)
    {
        u16 opcodeRaw = codePtr[i++];

        // Read Second Byte
        if(opcodeRaw & 0x80)
        {
            opcodeRaw <<= 8;
            opcodeRaw |= codePtr[i++];
        }

        const SsaOpcode opcode = static_cast<SsaOpcode>(opcodeRaw);

        switch(opcode)
        {
            case SsaOpcode::Nop:
                ConPrinter::Print("    Nop\n");
                break;
            case SsaOpcode::Label:
                ConPrinter::Print("  .{}\n", idIndex++);
                break;
            case SsaOpcode::AssignImmediate:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = ", idIndex++);

                PrintValue(type, codePtr, i, registry);
                ConPrinter::Print('\n');

                break;
            }
            case SsaOpcode::AssignVariable:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = ", idIndex++);
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::Print('\n');

                break;
            }
            case SsaOpcode::ExpandSX:
            {
                const SsaCustomType newType = ReadType<SsaCustomType>(codePtr, i);
                const SsaCustomType oldType = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(newType);
                ConPrinter::Print(" %{} = Expand.SX ", idIndex++);
                PrintType(oldType);
                ConPrinter::Print(" %{}\n", ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::ExpandZX:
            {
                const SsaCustomType newType = ReadType<SsaCustomType>(codePtr, i);
                const SsaCustomType oldType = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(newType);
                ConPrinter::Print(" %{} = Expand.ZX ", idIndex++);
                PrintType(oldType);
                ConPrinter::Print(" %{}\n", ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::Trunc:
            {
                const SsaCustomType newType = ReadType<SsaCustomType>(codePtr, i);
                const SsaCustomType oldType = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(newType);
                ConPrinter::Print(" %{} = Trunc ", idIndex++);
                PrintType(oldType);
                ConPrinter::Print(" %{}\n", ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::RCast:
            {
                const SsaCustomType newType = ReadType<SsaCustomType>(codePtr, i);
                const SsaCustomType oldType = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(newType);
                ConPrinter::Print(" %{} = RCast ", idIndex++);
                PrintType(oldType);
                ConPrinter::Print(" %{}\n", ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::BCast:
            {
                const SsaCustomType newType = ReadType<SsaCustomType>(codePtr, i);
                const SsaCustomType oldType = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(newType);
                ConPrinter::Print(" %{} = BCast ", idIndex++);
                PrintType(oldType);
                ConPrinter::Print(" %{}\n", ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::Load:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = Load %{}\n", idIndex++, ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::StoreV:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  Store ");
                PrintType(type);
                ConPrinter::Print(" %{}, %{}\n", ReadType<VarId>(codePtr, i), ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::StoreI:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  Store ");
                PrintType(type);
                ConPrinter::Print(" %{}, ", ReadType<VarId>(codePtr, i));
                PrintValue(type, codePtr, i, registry);
                ConPrinter::Print('\n');

                break;
            }
            case SsaOpcode::ComputePtr:
            {
                ConPrinter::Print("  void* %{} = ComputePtr ", idIndex++);

                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::Print(" + ");
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::Print(" * {} + {}\n", ReadType<i8>(codePtr, i), ReadType<i16>(codePtr, i));

                break;
            }
            case SsaOpcode::BinOpVtoV:
            {
                const SsaBinaryOperation op = ReadType<SsaBinaryOperation>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = ", idIndex++);
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::Print(' ');
                PrintBinaryOp(op);
                ConPrinter::Print(' ');
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::Print('\n');

                break;
            }
            case SsaOpcode::BinOpVtoI:
            {
                const SsaBinaryOperation op = ReadType<SsaBinaryOperation>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = ", idIndex++);
                PrintValue(type, codePtr, i, registry);
                ConPrinter::Print(' ');
                PrintBinaryOp(op);
                ConPrinter::Print(' ');
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::Print('\n');

                break;
            }
            case SsaOpcode::BinOpItoV:
            {
                const SsaBinaryOperation op = ReadType<SsaBinaryOperation>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = ", idIndex++);
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::Print(' ');
                PrintBinaryOp(op);
                ConPrinter::Print(' ');
                PrintValue(type, codePtr, i, registry);
                ConPrinter::Print('\n');

                break;
            }
            case SsaOpcode::Split:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  Split ");
                PrintType(type);
                ConPrinter::Print(" %{}, [ ", ReadType<VarId>(codePtr, i));
                const u32 n = ReadType<u32>(codePtr, i);

                for(u32 j = 0; j < n; ++j)
                {
                    if(j != 0)
                    {
                        ConPrinter::Print(", ");
                    }
                    PrintType(ReadType<SsaCustomType>(codePtr, i));
                }

                ConPrinter::Print(" ]\n");

                break;
            }
            case SsaOpcode::Join:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = Join [ ", idIndex++);
                const u32 n = ReadType<u32>(codePtr, i);

                for(u32 j = 0; j < n; ++j)
                {
                    if(j != 0)
                    {
                        ConPrinter::Print(", ");
                    }
                    const SsaCustomType typeN = ReadType<SsaCustomType>(codePtr, i);
                    PrintType(typeN);
                    ConPrinter::Print(' ');
                    PrintValue(typeN, codePtr, i, registry);
                }
                
                ConPrinter::Print(" ]\n");

                break;
            }
        }
    }
}

} }
