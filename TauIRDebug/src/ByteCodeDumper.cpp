#include "TauIR/ByteCodeDumper.hpp"
#include "TauIR/Function.hpp"
#include "TauIR/Opcodes.hpp"
#include "TauIR/ssa/SsaOpcodes.hpp"
#include "TauIR/ssa/SsaTypes.hpp"
#include <ConPrinter.hpp>

#include "TauIR/Common.hpp"
#include "TauIR/Module.hpp"


namespace tau::ir {

static DynArray<const u8*> PreProcessFunction(const Function* function) noexcept;

template<typename T>
static T ReadCodeValue(const u8*& codePtr)
{
    const T ret = *reinterpret_cast<const T*>(codePtr);
    codePtr += sizeof(T);
    return ret;
}

static iSys ShouldPlaceLabel(const DynArray<const u8*>& labels, const u8* const codePtr) noexcept
{
    for(uSys i = 0; i < labels.Count(); ++i)
    {
        if(labels[i] == codePtr)
        {
            return static_cast<iSys>(i);
        }
    }

    return -1;
}

void DumpFunction(const tau::ir::Function* function, const uSys functionIndex, const ::std::vector<Ref<Module>>& modules) noexcept
{
    const u8* codePtr = function->Address();

    ConPrinter::Print("Func{}:\n", functionIndex);

    const DynArray<const u8*> labels = PreProcessFunction(function);

    while(true)
    {
        {
            iSys labelIndex = ShouldPlaceLabel(labels, codePtr);
            if(labelIndex != -1)
            {
                ConPrinter::PrintLn("  .{}:", labelIndex);
            }
        }
        
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
                ConPrinter::PrintLn("    Nop");
                break;
            case Opcode::Push0:
                ConPrinter::PrintLn("    Push.0");
                break;
            case Opcode::Push1:
                ConPrinter::PrintLn("    Push.1");
                break;
            case Opcode::Push2:
                ConPrinter::PrintLn("    Push.2");
                break;
            case Opcode::Push3:
                ConPrinter::PrintLn("    Push.3");
                break;
            case Opcode::PushN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Push.N {}", localIndex);
                break;
            }
            case Opcode::PushArg0:
                ConPrinter::PrintLn("    Push.Arg.0");
                break;
            case Opcode::PushArg1:
                ConPrinter::PrintLn("    Push.Arg.1");
                break;
            case Opcode::PushArg2:
                ConPrinter::PrintLn("    Push.Arg.2");
                break;
            case Opcode::PushArg3:
                ConPrinter::PrintLn("    Push.Arg.3");
                break;
            case Opcode::PushArgN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Push.Arg.N {}", localIndex);
                break;
            }
            case Opcode::PushPtr:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Push.Ptr {}", localIndex);
                break;
            }
            case Opcode::PushGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                
                ConPrinter::PrintLn("    Push.Global {}", globalIndex);
                break;
            }
            case Opcode::PushGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Push.Global.Ext {}, {}", globalIndex, moduleIndex);
                break;
            }
            case Opcode::PushGlobalPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                
                ConPrinter::PrintLn("    Push.Global.Ptr {}", globalIndex);
                break;
            }
            case Opcode::PushGlobalExtPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Push.Global.Ext.Ptr {}, {}", globalIndex, moduleIndex);
                break;
            }
            case Opcode::Pop0:
                ConPrinter::PrintLn("    Pop.0");
                break;
            case Opcode::Pop1:
                ConPrinter::PrintLn("    Pop.1");
                break;
            case Opcode::Pop2:
                ConPrinter::PrintLn("    Pop.2");
                break;
            case Opcode::Pop3:
                ConPrinter::PrintLn("    Pop.3");
                break;
            case Opcode::PopN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Pop.N {}", localIndex);
                break;
            }
            case Opcode::PopArg0:
                ConPrinter::PrintLn("    Pop.Arg.0");
                break;
            case Opcode::PopArg1:
                ConPrinter::PrintLn("    Pop.Arg.1");
                break;
            case Opcode::PopArg2:
                ConPrinter::PrintLn("    Pop.Arg.2");
                break;
            case Opcode::PopArg3:
                ConPrinter::PrintLn("    Pop.Arg.3");
                break;
            case Opcode::PopArgN:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Pop.Arg.N {}", localIndex);
                break;
            }
            case Opcode::PopPtr:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Pop.Ptr {}", localIndex);
                break;
            }
            case Opcode::PopGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                
                ConPrinter::PrintLn("    Pop.Global {}", globalIndex);
                break;
            }
            case Opcode::PopGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Pop.Global.Ext {}, {}", globalIndex, moduleIndex);
                break;
            }
            case Opcode::PopGlobalPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                
                ConPrinter::PrintLn("    Pop.Global.Ptr {}", globalIndex);
                break;
            }
            case Opcode::PopGlobalExtPtr:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Pop.Global.Ext.Ptr {}, {}", globalIndex, moduleIndex);
                break;
            }
            case Opcode::PopCount:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Pop.Count {}", localIndex);
                break;
            }
            case Opcode::Dup1:
                ConPrinter::PrintLn("    Dup.1");
                break;
            case Opcode::Dup2:
                ConPrinter::PrintLn("    Dup.2");
                break;
            case Opcode::Dup4:
                ConPrinter::PrintLn("    Dup.4");
                break;
            case Opcode::Dup8:
                ConPrinter::PrintLn("    Dup.8");
                break;
            case Opcode::ExpandSX12:
                ConPrinter::PrintLn("    Expand.SX.1.2");
                break;
            case Opcode::ExpandSX14:
                ConPrinter::PrintLn("    Expand.SX.1.4");
                break;
            case Opcode::ExpandSX18:
                ConPrinter::PrintLn("    Expand.SX.1.8");
                break;
            case Opcode::ExpandSX24:
                ConPrinter::PrintLn("    Expand.SX.2.4");
                break;
            case Opcode::ExpandSX28:
                ConPrinter::PrintLn("    Expand.SX.2.8");
                break;
            case Opcode::ExpandSX48:
                ConPrinter::PrintLn("    Expand.SX.4.8");
                break;
            case Opcode::ExpandZX12:
                ConPrinter::PrintLn("    Expand.ZX.1.2");
                break;
            case Opcode::ExpandZX14:
                ConPrinter::PrintLn("    Expand.ZX.1.4");
                break;
            case Opcode::ExpandZX18:
                ConPrinter::PrintLn("    Expand.ZX.1.8");
                break;
            case Opcode::ExpandZX24:
                ConPrinter::PrintLn("    Expand.ZX.2.4");
                break;
            case Opcode::ExpandZX28:
                ConPrinter::PrintLn("    Expand.ZX.2.8");
                break;
            case Opcode::ExpandZX48:
                ConPrinter::PrintLn("    Expand.ZX.4.8");
                break;
            case Opcode::Trunc84:
                ConPrinter::PrintLn("    Trunc.8.4");
                break;
            case Opcode::Trunc82:
                ConPrinter::PrintLn("    Trunc.8.2");
                break;
            case Opcode::Trunc81:
                ConPrinter::PrintLn("    Trunc.8.1");
                break;
            case Opcode::Trunc42:
                ConPrinter::PrintLn("    Trunc.4.2");
                break;
            case Opcode::Trunc41:
                ConPrinter::PrintLn("    Trunc.4.1");
                break;
            case Opcode::Trunc21:
                ConPrinter::PrintLn("    Trunc.2.1");
                break;
            case Opcode::Load:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Load {}, {}", localIndex, addressIndex);
                break;
            }
            case Opcode::LoadGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u16>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Load.Global {}, {}", globalIndex, addressIndex);
                break;
            }
            case Opcode::LoadGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Load.Global.Ext {}, {}, {}", globalIndex, addressIndex, moduleIndex);
                break;
            }
            case Opcode::Store:
            {
                const u16 localIndex = ReadCodeValue<u16>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Store {}, {}", localIndex, addressIndex);
                break;
            }
            case Opcode::StoreGlobal:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::PrintLn("    Store.Global {}, {}", globalIndex, addressIndex);
                break;
            }
            case Opcode::StoreGlobalExt:
            {
                const u32 globalIndex = ReadCodeValue<u32>(codePtr);
                const u16 addressIndex = ReadCodeValue<u16>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);
                
                ConPrinter::Print("    Store.Global.Ext {}, {}, {}", globalIndex, addressIndex, moduleIndex);
                break;
            }
            case Opcode::Const0:
                ConPrinter::PrintLn("    Const.0");
                break;
            case Opcode::Const1:
                ConPrinter::PrintLn("    Const.1");
                break;
            case Opcode::Const2:
                ConPrinter::PrintLn("    Const.2");
                break;
            case Opcode::Const3:
                ConPrinter::PrintLn("    Const.3");
                break;
            case Opcode::Const4:
                ConPrinter::PrintLn("    Const.4");
                break;
            case Opcode::ConstFF:
                ConPrinter::PrintLn("    Const.FF");
                break;
            case Opcode::Const7F:
                ConPrinter::PrintLn("    Const.7F");
                break;
            case Opcode::ConstN:
            {
                const u32 constant = ReadCodeValue<u32>(codePtr);
                
                ConPrinter::PrintLn("    Const.N {}", constant);
                break;
            }
            case Opcode::AddI32:
                ConPrinter::PrintLn("    Add.i32");
                break;
            case Opcode::AddI64:
                ConPrinter::PrintLn("    Add.i64");
                break;
            case Opcode::SubI32:
                ConPrinter::PrintLn("    Sub.i32");
                break;
            case Opcode::SubI64:
                ConPrinter::PrintLn("    Sub.i64");
                break;
            case Opcode::MulI32:
                ConPrinter::PrintLn("    Mul.i32");
                break;
            case Opcode::MulI64:
                ConPrinter::PrintLn("    Mul.i64");
                break;
            case Opcode::DivI32:
                ConPrinter::PrintLn("    Div.i32");
                break;
            case Opcode::DivI64:
                ConPrinter::PrintLn("    Div.i64");
                break;
            case Opcode::CompI32Above:
                ConPrinter::PrintLn("    Comp.i32.Above");
                break;
            case Opcode::CompI32AboveOrEqual:
                ConPrinter::PrintLn("    Comp.i32.AboveOrEqual");
                break;
            case Opcode::CompI32Below:
                ConPrinter::PrintLn("    Comp.i32.Below");
                break;
            case Opcode::CompI32BelowOrEqual:
                ConPrinter::PrintLn("    Comp.i32.BelowOrEqual");
                break;
            case Opcode::CompI32Equal:
                ConPrinter::PrintLn("    Comp.i32.Equal");
                break;
            case Opcode::CompI32Greater:
                ConPrinter::PrintLn("    Comp.i32.Greater");
                break;
            case Opcode::CompI32GreaterOrEqual:
                ConPrinter::PrintLn("    Comp.i32.GreaterOrEqual");
                break;
            case Opcode::CompI32Less:
                ConPrinter::PrintLn("    Comp.i32.Less");
                break;
            case Opcode::CompI32LessOrEqual:
                ConPrinter::PrintLn("    Comp.i32.LessOrEqual");
                break;
            case Opcode::CompI32NotEqual:
                ConPrinter::PrintLn("    Comp.i32.NotEqual");
                break;
            case Opcode::CompI64Above:
                ConPrinter::PrintLn("    Comp.i64.Above");
                break;
            case Opcode::CompI64AboveOrEqual:
                ConPrinter::PrintLn("    Comp.i64.AboveOrEqual");
                break;
            case Opcode::CompI64Below:
                ConPrinter::PrintLn("    Comp.i64.Below");
                break;
            case Opcode::CompI64BelowOrEqual:
                ConPrinter::PrintLn("    Comp.i64.BelowOrEqual");
                break;
            case Opcode::CompI64Equal:
                ConPrinter::PrintLn("    Comp.i64.Equal");
                break;
            case Opcode::CompI64Greater:
                ConPrinter::PrintLn("    Comp.i64.Greater");
                break;
            case Opcode::CompI64GreaterOrEqual:
                ConPrinter::PrintLn("    Comp.i64.GreaterOrEqual");
                break;
            case Opcode::CompI64Less:
                ConPrinter::PrintLn("    Comp.i64.Less");
                break;
            case Opcode::CompI64LessOrEqual:
                ConPrinter::PrintLn("    Comp.i64.LessOrEqual");
                break;
            case Opcode::CompI64NotEqual:
                ConPrinter::PrintLn("    Comp.i64.NotEqual");
                break;
            case Opcode::Call:
            {
                const u32 targetFunctionIndex = ReadCodeValue<u32>(codePtr);
                
                ConPrinter::PrintLn("    Call <Func{}>", targetFunctionIndex);
                break;
            }
            case Opcode::CallExt:
            {
                const u32 targetFunctionIndex = ReadCodeValue<u32>(codePtr);
                const u16 moduleIndex = ReadCodeValue<u16>(codePtr);

                ConPrinter::Print("    Call.Ext ");
                
                if(moduleIndex >= modules.size() || modules[moduleIndex]->Name().Length() == 0)
                {
                    ConPrinter::Print(moduleIndex);
                }
                else
                {
                    ConPrinter::Print(modules[moduleIndex]->Name());
                }

                ConPrinter::PrintLn(":<Func{}>", targetFunctionIndex);
                break;
            }
            case Opcode::CallInd:
                ConPrinter::PrintLn("    Call.Ind");
                break;
            case Opcode::Jump:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);

                const iSys labelIndex = ShouldPlaceLabel(labels, codePtr + offset);

                ConPrinter::PrintLn("    Jump .{}", labelIndex);
                break;
            }
            case Opcode::JumpTrue:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);

                const iSys labelIndex = ShouldPlaceLabel(labels, codePtr + offset);

                ConPrinter::PrintLn("    Jump.True .{}", labelIndex);
                break;
            }
            case Opcode::JumpFalse:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);

                const iSys labelIndex = ShouldPlaceLabel(labels, codePtr + offset);

                ConPrinter::PrintLn("    Jump.False .{}", labelIndex);
                break;
            }
            default: break;
        }
    }
}

static DynArray<const u8*> PreProcessFunction(const Function* function) noexcept
{
    const u8* codePtr = function->Address();

    uSys jumpCount = 0;

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
            case Opcode::PushN:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PushArgN:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PushPtr:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PushGlobal:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::PushGlobalExt:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PushGlobalPtr:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::PushGlobalExtPtr:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopN:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopArgN:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopPtr:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopGlobal:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::PopGlobalExt:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopGlobalPtr:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::PopGlobalExtPtr:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopCount:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::Load:
            {
                ReadCodeValue<u16>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::LoadGlobal:
            {
                ReadCodeValue<u16>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::LoadGlobalExt:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::Store:
            {
                ReadCodeValue<u16>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::StoreGlobal:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::StoreGlobalExt:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::ConstN:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::Call:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::CallExt:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::Jump:
            {
                ReadCodeValue<u32>(codePtr);
                ++jumpCount;
                break;
            }
            case Opcode::JumpTrue:
            {
                ReadCodeValue<u32>(codePtr);
                ++jumpCount;
                break;
            }
            case Opcode::JumpFalse:
            {
                ReadCodeValue<u32>(codePtr);
                ++jumpCount;
                break;
            }
            default: break;
        }
    }

    codePtr = function->Address();

    DynArray<const u8*> labels(jumpCount);

    uSys writeIndex = 0;

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
            case Opcode::PushN:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PushArgN:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PushPtr:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PushGlobal:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::PushGlobalExt:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PushGlobalPtr:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::PushGlobalExtPtr:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopN:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopArgN:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopPtr:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopGlobal:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::PopGlobalExt:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopGlobalPtr:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::PopGlobalExtPtr:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::PopCount:
            {
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::Load:
            {
                ReadCodeValue<u16>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::LoadGlobal:
            {
                ReadCodeValue<u16>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::LoadGlobalExt:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::Store:
            {
                ReadCodeValue<u16>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::StoreGlobal:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::StoreGlobalExt:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::ConstN:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::Call:
            {
                ReadCodeValue<u32>(codePtr);
                break;
            }
            case Opcode::CallExt:
            {
                ReadCodeValue<u32>(codePtr);
                ReadCodeValue<u16>(codePtr);
                break;
            }
            case Opcode::Jump:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);
                labels[writeIndex++] = codePtr + offset;
                break;
            }
            case Opcode::JumpTrue:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);
                labels[writeIndex++] = codePtr + offset;
                break;
            }
            case Opcode::JumpFalse:
            {
                const i32 offset = ReadCodeValue<i32>(codePtr);
                labels[writeIndex++] = codePtr + offset;
                break;
            }
            default: break;
        }
    }

    return labels;
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
            ConPrinter::Print("<<<");
            break;
        case SsaBinaryOperation::BarrelShiftRight:
            ConPrinter::Print(">>>");
            break;
        case SsaBinaryOperation::Comp:
            ConPrinter::Print("==");
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
                ConPrinter::PrintLn("    Nop");
                break;
            case SsaOpcode::Label:
                ConPrinter::PrintLn("  .{}", idIndex++);
                break;
            case SsaOpcode::AssignImmediate:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = ", idIndex++);

                PrintValue(type, codePtr, i, registry);
                ConPrinter::PrintLn();

                break;
            }
            case SsaOpcode::AssignVariable:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = ", idIndex++);
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::PrintLn();

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
                ConPrinter::PrintLn(" %{}", ReadType<VarId>(codePtr, i));

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
                ConPrinter::PrintLn(" %{}", ReadType<VarId>(codePtr, i));

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
                ConPrinter::PrintLn(" %{}", ReadType<VarId>(codePtr, i));

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
                ConPrinter::PrintLn(" %{}", ReadType<VarId>(codePtr, i));

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
                ConPrinter::PrintLn(" %{}", ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::Load:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::PrintLn(" %{} = Load %{}", idIndex++, ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::StoreV:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  Store ");
                PrintType(type);
                ConPrinter::PrintLn(" %{}, %{}", ReadType<VarId>(codePtr, i), ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::StoreI:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  Store ");
                PrintType(type);
                ConPrinter::Print(" %{}, ", ReadType<VarId>(codePtr, i));
                PrintValue(type, codePtr, i, registry);
                ConPrinter::PrintLn();

                break;
            }
            case SsaOpcode::ComputePtr:
            {
                ConPrinter::Print("  void* %{} = ComputePtr ", idIndex++);

                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::Print(" + ");
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::PrintLn(" * {} + {}", ReadType<i8>(codePtr, i), ReadType<i16>(codePtr, i));

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
                ConPrinter::PrintLn();

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
                ConPrinter::PrintLn();

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
                ConPrinter::PrintLn();

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

                ConPrinter::PrintLn(" ]");

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
                
                ConPrinter::PrintLn(" ]");

                break;
            }
            case SsaOpcode::Branch:
            {
                ConPrinter::PrintLn("  Branch .{}", ReadType<VarId>(codePtr, i));

                break;
            }
            case SsaOpcode::BranchCond:
            {
                ConPrinter::Print("  Branch .{}, .{}, ", ReadType<VarId>(codePtr, i), ReadType<VarId>(codePtr, i));
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::PrintLn();

                break;
            }
            case SsaOpcode::Call:
            {
                ConPrinter::PrintLn("  Call {}(), {}-{}", ReadType<u32>(codePtr, i), ReadType<u32>(codePtr, i), ReadType<u32>(codePtr, i));

                break;
            }
            case SsaOpcode::CallExt:
            {
                const u32 function = ReadType<u32>(codePtr, i);
                const u32 baseIndex = ReadType<u32>(codePtr, i);
                const u32 parameterCount = ReadType<u32>(codePtr, i);
                const u16 moduleId = ReadType<u16>(codePtr, i);

                ConPrinter::PrintLn("  Call.Ext {}:{}(), {}-{}", moduleId, function, baseIndex, parameterCount);

                break;
            }
            case SsaOpcode::CallInd:
            {
                ConPrinter::PrintLn("  Call.Ind {}(), {}-{}", ReadType<u32>(codePtr, i), ReadType<u32>(codePtr, i), ReadType<u32>(codePtr, i));

                ConPrinter::Print("  Call.Ind ");
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::PrintLn(", {}-{}", ReadType<u32>(codePtr, i), ReadType<u32>(codePtr, i));

                break;
            }
            case SsaOpcode::Ret:
            {
                ConPrinter::PrintLn("  Ret");

                break;
            }
        }
    }
}

} }
