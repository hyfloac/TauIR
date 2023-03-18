// ReSharper disable CppHidingFunction
#include "TauIR/ByteCodeDumper.hpp"
#include "TauIR/Function.hpp"
#include "TauIR/ssa/SsaOpcodes.hpp"
#include "TauIR/ssa/SsaTypes.hpp"
#include <ConPrinter.hpp>

#include "TauIR/Common.hpp"
#include "TauIR/Module.hpp"
#include "TauIR/IrVisitor.hpp"

namespace tau::ir {

static DynArray<const u8*> PreProcessFunction(const Function* function) noexcept;

class DumpVisitor : public BaseIrVisitor<DumpVisitor>
{
    DEFAULT_DESTRUCT(DumpVisitor);
    DEFAULT_CM_PU(DumpVisitor);
public:
    DumpVisitor(const DynArray<const u8*>& labels, const ::std::vector<Ref<Module>>* modules) noexcept
        : m_Labels(labels)
        , m_Modules(modules)
        , m_CurrCodePtr(nullptr)
    { }

    // DumpVisitor(const DynArray<const u8*>& labels, ::std::vector<Ref<Module>>&& modules) noexcept
    //     : m_Labels(labels)
    //     , m_Modules(::std::move(modules))
    //     , m_CurrCodePtr(nullptr)
    // { }
    //
    // DumpVisitor(DynArray<const u8*>&& labels, const ::std::vector<Ref<Module>>& modules) noexcept
    //     : m_Labels(::std::move(labels))
    //     , m_Modules(modules)
    //     , m_CurrCodePtr(nullptr)
    // { }
    
    DumpVisitor(DynArray<const u8*>&& labels, const ::std::vector<Ref<Module>>* modules) noexcept
        : m_Labels(::std::move(labels))
        , m_Modules(::std::move(modules))
        , m_CurrCodePtr(nullptr)
    { }

    void Reset(const DynArray<const u8*>& labels, const ::std::vector<Ref<Module>>* modules) noexcept
    {
        m_Labels = labels;
        m_Modules = modules;
        m_CurrCodePtr = nullptr;
    }
    
    // void Reset(const DynArray<const u8*>& labels, ::std::vector<Ref<Module>>&& modules) noexcept
    // {
    //     m_Labels = labels;
    //     m_Modules = ::std::move(modules);
    //     m_CurrCodePtr = nullptr;
    // }
    //
    // void Reset(DynArray<const u8*>&& labels, const ::std::vector<Ref<Module>>& modules) noexcept
    // {
    //     m_Labels = ::std::move(labels);
    //     // m_Modules = modules;
    //     m_CurrCodePtr = nullptr;
    // }
    
    void Reset(DynArray<const u8*>&& labels, ::std::vector<Ref<Module>>* modules) noexcept
    {
        m_Labels = ::std::move(labels);
        m_Modules = ::std::move(modules);
        m_CurrCodePtr = nullptr;
    }

    void PreVisit(const u8* const codePtr) noexcept
    {
        const iSys labelIndex = ShouldPlaceLabel(codePtr);
        if(labelIndex != -1)
        {
            ConPrinter::PrintLn("  .L{}:", labelIndex);
        }

        m_CurrCodePtr = codePtr;
    }

    void VisitNop() noexcept
    { ConPrinter::PrintLn("    Nop"); }

    void VisitPush0() noexcept
    { ConPrinter::PrintLn("    Push.0"); }

    void VisitPush1() noexcept
    { ConPrinter::PrintLn("    Push.1"); }

    void VisitPush2() noexcept
    { ConPrinter::PrintLn("    Push.2"); }

    void VisitPush3() noexcept
    { ConPrinter::PrintLn("    Push.3"); }

    void VisitPushN(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Push.N {}", localIndex); }

    void VisitPushArg0() noexcept
    { ConPrinter::PrintLn("    Push.Arg.0"); }

    void VisitPushArg1() noexcept
    { ConPrinter::PrintLn("    Push.Arg.1"); }

    void VisitPushArg2() noexcept
    { ConPrinter::PrintLn("    Push.Arg.2"); }

    void VisitPushArg3() noexcept
    { ConPrinter::PrintLn("    Push.Arg.3"); }

    void VisitPushArgN(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Push.Arg.N {}", localIndex); }

    void VisitPushPtr(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Push.Ptr {}", localIndex); }

    void VisitPushGlobal(const u32 globalIndex) noexcept
    { ConPrinter::PrintLn("    Push.Global {}", globalIndex); }

    void VisitPushGlobalExt(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Push.Global.Ext ");

        if(moduleIndex >= Modules().size() || Modules()[moduleIndex]->Name().Length() == 0)
        {
            ConPrinter::Print(moduleIndex);
        }
        else
        {
            ConPrinter::Print(Modules()[moduleIndex]->Name());
        }

        ConPrinter::PrintLn(":{}", globalIndex);
    }

    void VisitPushGlobalPtr(const u32 globalIndex) noexcept
    { ConPrinter::PrintLn("    Push.Global.Ptr {}", globalIndex); }

    void VisitPushGlobalExtPtr(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Push.Global.Ext.Ptr ");

        if(moduleIndex >= Modules().size() || Modules()[moduleIndex]->Name().Length() == 0)
        {
            ConPrinter::Print(moduleIndex);
        }
        else
        {
            ConPrinter::Print(Modules()[moduleIndex]->Name());
        }

        ConPrinter::PrintLn(":{}", globalIndex);
    }

    void VisitPop0() noexcept
    { ConPrinter::PrintLn("    Pop.0"); }

    void VisitPop1() noexcept
    { ConPrinter::PrintLn("    Pop.1"); }

    void VisitPop2() noexcept
    { ConPrinter::PrintLn("    Pop.2"); }

    void VisitPop3() noexcept
    { ConPrinter::PrintLn("    Pop.3"); }

    void VisitPopN(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Pop.N {}", localIndex); }

    void VisitPopArg0() noexcept
    { ConPrinter::PrintLn("    Pop.Arg.0"); }

    void VisitPopArg1() noexcept
    { ConPrinter::PrintLn("    Pop.Arg.1"); }

    void VisitPopArg2() noexcept
    { ConPrinter::PrintLn("    Pop.Arg.2"); }

    void VisitPopArg3() noexcept
    { ConPrinter::PrintLn("    Pop.Arg.3"); }

    void VisitPopArgN(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Pop.Arg.N {}", localIndex); }

    void VisitPopPtr(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Pop.Ptr {}", localIndex); }

    void VisitPopGlobal(const u32 globalIndex) noexcept
    { ConPrinter::PrintLn("    Pop.Global {}", globalIndex); }

    void VisitPopGlobalExt(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Pop.Global.Ext ");

        if(moduleIndex >= Modules().size() || Modules()[moduleIndex]->Name().Length() == 0)
        {
            ConPrinter::Print(moduleIndex);
        }
        else
        {
            ConPrinter::Print(Modules()[moduleIndex]->Name());
        }

        ConPrinter::PrintLn(":{}", globalIndex);
    }

    void VisitPopGlobalPtr(const u32 globalIndex) noexcept
    { ConPrinter::PrintLn("    Pop.Global.Ptr {}", globalIndex); }

    void VisitPopGlobalExtPtr(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Pop.Global.Ext.Ptr ");

        if(moduleIndex >= Modules().size() || Modules()[moduleIndex]->Name().Length() == 0)
        {
            ConPrinter::Print(moduleIndex);
        }
        else
        {
            ConPrinter::Print(Modules()[moduleIndex]->Name());
        }

        ConPrinter::PrintLn(":{}", globalIndex);
    }

    void VisitPopCount(const u16 byteCount) noexcept
    { ConPrinter::PrintLn("    Pop.Count {}", byteCount); }

    void VisitDup(const uSys byteCount) noexcept
    { ConPrinter::PrintLn("    Dup.{}", byteCount); }

    void VisitExpandSX(const uSys fromSize, const uSys toSize) noexcept
    { ConPrinter::PrintLn("    Expand.SX.{}.{}", fromSize, toSize); }

    void VisitExpandZX(const uSys fromSize, const uSys toSize) noexcept
    { ConPrinter::PrintLn("    Expand.ZX.{}.{}", fromSize, toSize); }

    void VisitTrunc(const uSys fromSize, const uSys toSize) noexcept
    { ConPrinter::PrintLn("    Trunc.{}.{}", fromSize, toSize); }

    void VisitLoad(const u16 localIndex, const u16 addressIndex) noexcept
    { ConPrinter::PrintLn("    Load {}, {}", localIndex, addressIndex); }

    void VisitLoadGlobal(const u32 globalIndex, const u16 addressIndex) noexcept
    { ConPrinter::PrintLn("    Load.Global {}, {}", globalIndex, addressIndex); }

    void VisitLoadGlobalExt(const u32 globalIndex, const u16 addressIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Load.Global.Ext ");

        if(moduleIndex >= Modules().size() || Modules()[moduleIndex]->Name().Length() == 0)
        {
            ConPrinter::Print(moduleIndex);
        }
        else
        {
            ConPrinter::Print(Modules()[moduleIndex]->Name());
        }

        ConPrinter::PrintLn(":{}, {}", globalIndex, addressIndex);
    }

    void VisitStore(const u16 localIndex, const u16 addressIndex) noexcept
    { ConPrinter::PrintLn("    Store {}, {}", localIndex, addressIndex); }

    void VisitStoreGlobal(const u32 globalIndex, const u16 addressIndex) noexcept
    { ConPrinter::PrintLn("    Store.Global {}, {}", globalIndex, addressIndex); }

    void VisitStoreGlobalExt(const u32 globalIndex, const u16 addressIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Store.Global.Ext ");

        if(moduleIndex >= Modules().size() || Modules()[moduleIndex]->Name().Length() == 0)
        {
            ConPrinter::Print(moduleIndex);
        }
        else
        {
            ConPrinter::Print(Modules()[moduleIndex]->Name());
        }

        ConPrinter::PrintLn(":{}, {}", globalIndex, addressIndex);
    }

    void VisitConst0() noexcept
    { ConPrinter::PrintLn("    Const.0"); }

    void VisitConst1() noexcept
    { ConPrinter::PrintLn("    Const.1"); }

    void VisitConst2() noexcept
    { ConPrinter::PrintLn("    Const.2"); }

    void VisitConst3() noexcept
    { ConPrinter::PrintLn("    Const.3"); }

    void VisitConst4() noexcept
    { ConPrinter::PrintLn("    Const.4"); }

    void VisitConstFF() noexcept
    { ConPrinter::PrintLn("    Const.FF"); }

    void VisitConst7F() noexcept
    { ConPrinter::PrintLn("    Const.7F"); }

    void VisitConstN(const u32 constant) noexcept
    { ConPrinter::PrintLn("    Const.N {}", constant); }

    void VisitAddI32() noexcept
    { ConPrinter::PrintLn("    Add.i32"); }

    void VisitAddI64() noexcept
    { ConPrinter::PrintLn("    Add.i64"); }

    void VisitSubI32() noexcept
    { ConPrinter::PrintLn("    Sub.i32"); }

    void VisitSubI64() noexcept
    { ConPrinter::PrintLn("    Sub.i64"); }

    void VisitMulI32() noexcept
    { ConPrinter::PrintLn("    Mul.i32"); }

    void VisitMulI64() noexcept
    { ConPrinter::PrintLn("    Mul.i64"); }

    void VisitDivI32() noexcept
    { ConPrinter::PrintLn("    Div.i32"); }

    void VisitDivI64() noexcept
    { ConPrinter::PrintLn("    Div.i64"); }

    void VisitCompI32Above() noexcept
    { ConPrinter::PrintLn("    Comp.i32.Above"); }

    void VisitCompI32AboveOrEqual() noexcept
    { ConPrinter::PrintLn("    Comp.i32.AboveOrEqual"); }

    void VisitCompI32Below() noexcept
    { ConPrinter::PrintLn("    Comp.i32.Below"); }

    void VisitCompI32BelowOrEqual() noexcept
    { ConPrinter::PrintLn("    Comp.i32.BelowOrEqual"); }

    void VisitCompI32Greater() noexcept
    { ConPrinter::PrintLn("    Comp.i32.Greater"); }

    void VisitCompI32GreaterOrEqual() noexcept
    { ConPrinter::PrintLn("    Comp.i32.GreaterOrEqual"); }

    void VisitCompI32Less() noexcept
    { ConPrinter::PrintLn("    Comp.i32.Less"); }

    void VisitCompI32LessOrEqual() noexcept
    { ConPrinter::PrintLn("    Comp.i32.LessOrEqual"); }

    void VisitCompI32NotEqual() noexcept
    { ConPrinter::PrintLn("    Comp.i32.NotEqual"); }

    void VisitCompI64Above() noexcept
    { ConPrinter::PrintLn("    Comp.i64.Above"); }

    void VisitCompI64AboveOrEqual() noexcept
    { ConPrinter::PrintLn("    Comp.i64.AboveOrEqual"); }

    void VisitCompI64Below() noexcept
    { ConPrinter::PrintLn("    Comp.i64.Below"); }

    void VisitCompI64BelowOrEqual() noexcept
    { ConPrinter::PrintLn("    Comp.i64.BelowOrEqual"); }

    void VisitCompI64Greater() noexcept
    { ConPrinter::PrintLn("    Comp.i64.Greater"); }

    void VisitCompI64GreaterOrEqual() noexcept
    { ConPrinter::PrintLn("    Comp.i64.GreaterOrEqual"); }

    void VisitCompI64Less() noexcept
    { ConPrinter::PrintLn("    Comp.i64.Less"); }

    void VisitCompI64LessOrEqual() noexcept
    { ConPrinter::PrintLn("    Comp.i64.LessOrEqual"); }

    void VisitCompI64NotEqual() noexcept
    { ConPrinter::PrintLn("    Comp.i64.NotEqual"); }

    void VisitCall(const u32 functionIndex) noexcept
    { ConPrinter::PrintLn("    Call <Func{}>", functionIndex); }

    void VisitCallExt(const u32 functionIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Call.Ext ");

        if(moduleIndex >= Modules().size() || Modules()[moduleIndex]->Name().Length() == 0)
        {
            ConPrinter::Print(moduleIndex);
        }
        else
        {
            ConPrinter::Print(Modules()[moduleIndex]->Name());
        }

        ConPrinter::PrintLn(":<Func{}>", functionIndex);
    }

    void VisitCallInd() noexcept
    { ConPrinter::PrintLn("    Call.Ind"); }

    void VisitRet() noexcept
    { ConPrinter::PrintLn("    Ret"); }

    void VisitJump(const i32 offset) noexcept
    {
        const iSys labelIndex = ShouldPlaceLabel(m_CurrCodePtr + 5 + offset);
        ConPrinter::PrintLn("    Jump .L{}", labelIndex);
    }

    void VisitJumpTrue(const i32 offset) noexcept
    {
        const iSys labelIndex = ShouldPlaceLabel(m_CurrCodePtr + 5 + offset);
        ConPrinter::PrintLn("    Jump.True .L{}", labelIndex);
    }

    void VisitJumpFalse(const i32 offset) noexcept
    {
        const iSys labelIndex = ShouldPlaceLabel(m_CurrCodePtr + 5 + offset);
        ConPrinter::PrintLn("    Jump.False .L{}", labelIndex);
    }
private:
    [[nodiscard]] iSys ShouldPlaceLabel(const u8* const codePtr) noexcept
    {
        for(uSys i = 0; i < m_Labels.Count(); ++i)
        {
            if(m_Labels[i] == codePtr)
            {
                return static_cast<iSys>(i);
            }
        }

        return -1;
    }

    [[nodiscard]] const ::std::vector<Ref<Module>>& Modules() const noexcept { return *m_Modules; }
private:
    DynArray<const u8*> m_Labels;
    const ::std::vector<Ref<Module>>* m_Modules;
    const u8* m_CurrCodePtr;
};

void DumpFunction(const tau::ir::Function* function, const uSys functionIndex, const ::std::vector<Ref<Module>>& modules) noexcept
{
    ConPrinter::Print("Func{}:\n", functionIndex);

    const DynArray<const u8*> labels = PreProcessFunction(function);

    DumpVisitor dumpVisitor(labels, &modules);
    dumpVisitor.Traverse(function);
}

class VisitorJumpCount : public BaseIrVisitor<VisitorJumpCount>
{
    DEFAULT_DESTRUCT(VisitorJumpCount);
    DEFAULT_CM_PU(VisitorJumpCount);
public:
    VisitorJumpCount() noexcept
        : m_JumpCount(0)
    { }

    [[nodiscard]] uSys JumpCount() const noexcept { return m_JumpCount; }

    void Reset() noexcept
    {
        m_JumpCount = 0;
    }

    void VisitJump(const i32 offset) noexcept
    {
        ++m_JumpCount;
    }

    void VisitJumpTrue(const i32 offset) noexcept
    {
        ++m_JumpCount;
    }

    void VisitJumpFalse(const i32 offset) noexcept
    {
        ++m_JumpCount;
    }
private:
    uSys m_JumpCount;
};

class VisitorLabeler : public BaseIrVisitor<VisitorLabeler>
{
    DEFAULT_DESTRUCT(VisitorLabeler);
    DEFAULT_CM_PU(VisitorLabeler);
public:
    VisitorLabeler(const uSys jumpCount) noexcept
        : m_Labels(jumpCount)
        , m_CurrCodePtr(nullptr)
        , m_WriteIndex(0)
    { }

    [[nodiscard]] const DynArray<const u8*>& Labels() const noexcept { return m_Labels; }

    void Reset(const uSys jumpCount) noexcept
    {
        m_Labels = DynArray<const u8*>(jumpCount);
        m_CurrCodePtr = nullptr;
        m_WriteIndex = 0;
    }

    void PreVisit(const u8* const codePtr) noexcept
    {
        m_CurrCodePtr = codePtr;
    }

    void VisitJump(const i32 offset) noexcept
    {
        m_Labels[m_WriteIndex++] = m_CurrCodePtr + 5 + offset;
    }

    void VisitJumpTrue(const i32 offset) noexcept
    {
        m_Labels[m_WriteIndex++] = m_CurrCodePtr + 5 + offset;
    }

    void VisitJumpFalse(const i32 offset) noexcept
    {
        m_Labels[m_WriteIndex++] = m_CurrCodePtr + 5 + offset;
    }
private:
    DynArray<const u8*> m_Labels;
    const u8* m_CurrCodePtr;
    uSys m_WriteIndex;
};

static DynArray<const u8*> PreProcessFunction(const Function* function) noexcept
{
    VisitorJumpCount jumpCountVisitor;
    jumpCountVisitor.Traverse(function);
    
    VisitorLabeler labelerVisitor(jumpCountVisitor.JumpCount());
    labelerVisitor.Traverse(function);

    return labelerVisitor.Labels();
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
