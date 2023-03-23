// ReSharper disable CppHidingFunction
#include "TauIR/ByteCodeDumper.hpp"
#include "TauIR/Function.hpp"
#include "TauIR/Opcodes.hpp"
#include "TauIR/ssa/SsaOpcodes.hpp"
#include "TauIR/ssa/SsaTypes.hpp"
#include "TauIR/ssa/SsaFunctionAttachment.hpp"
#include <ConPrinter.hpp>

#include "TauIR/Common.hpp"
#include "TauIR/Module.hpp"
#include "TauIR/IrVisitor.hpp"

namespace tau::ir {

#define VISIT_PRINT_0(OPCODE)                \
    void Visit##OPCODE() noexcept {          \
        ConPrinter::PrintLn("    " #OPCODE); \
    }

#define VISIT_PRINT_1(OPCODE, OP0)                    \
    void Visit##OPCODE##OP0() noexcept {              \
        ConPrinter::PrintLn("    " #OPCODE "." #OP0); \
    }

#define VISIT_PRINT_2(OPCODE, OP0, OP1)                        \
    void Visit##OPCODE##OP0##OP1() noexcept {                  \
        ConPrinter::PrintLn("    " #OPCODE "." #OP0 "." #OP1); \
    }

#define VISIT_PRINT_0_I32(OPCODE)                   \
    void Visit##OPCODE##I32() noexcept {            \
        ConPrinter::PrintLn("    " #OPCODE ".i32"); \
    }

#define VISIT_PRINT_0_I64(OPCODE)                   \
    void Visit##OPCODE##I64() noexcept {            \
        ConPrinter::PrintLn("    " #OPCODE ".i64"); \
    }

#define VISIT_PRINT_1_I32(OPCODE, OP0)                       \
    void Visit##OPCODE##I32##OP0() noexcept {                \
        ConPrinter::PrintLn("    " #OPCODE ".i32" "." #OP0); \
    }

#define VISIT_PRINT_1_I64(OPCODE, OP0)                       \
    void Visit##OPCODE##I64##OP0() noexcept {                \
        ConPrinter::PrintLn("    " #OPCODE ".i64" "." #OP0); \
    }

class DumpVisitor final : public BaseIrVisitor<DumpVisitor>
{
    DEFAULT_DESTRUCT(DumpVisitor);
    DEFAULT_CM_PU(DumpVisitor);
public:
    DumpVisitor(const DynArray<const u8*>& labels, const ::std::vector<StrongRef<Module>>* modules, const u16 currentModule) noexcept
        : m_Labels(labels)
        , m_Modules(modules)
        , m_CurrCodePtr(nullptr)
        , m_CurrentModule(currentModule)
    { }

    // DumpVisitor(const DynArray<const u8*>& labels, ::std::vector<Ref<Module>>&& modules, const u16 currentModule) noexcept
    //     : m_Labels(labels)
    //     , m_Modules(::std::move(modules))
    //     , m_CurrCodePtr(nullptr)
    //     , m_CurrentModule(currentModule)
    // { }
    //
    // DumpVisitor(DynArray<const u8*>&& labels, const ::std::vector<Ref<Module>>& modules, const u16 currentModule) noexcept
    //     : m_Labels(::std::move(labels))
    //     , m_Modules(modules)
    //     , m_CurrCodePtr(nullptr)
    //     , m_CurrentModule(currentModule)
    // { }
    
    DumpVisitor(DynArray<const u8*>&& labels, const ::std::vector<StrongRef<Module>>* modules, const u16 currentModule) noexcept
        : m_Labels(::std::move(labels))
        , m_Modules(::std::move(modules))
        , m_CurrCodePtr(nullptr)
        , m_CurrentModule(currentModule)
    { }

    void Reset(const DynArray<const u8*>& labels, const ::std::vector<StrongRef<Module>>* modules, const u16 currentModule) noexcept
    {
        m_Labels = labels;
        m_Modules = modules;
        m_CurrCodePtr = nullptr;
        m_CurrentModule = currentModule;
    }
    
    // void Reset(const DynArray<const u8*>& labels, ::std::vector<Ref<Module>>&& modules, const u16 currentModule) noexcept
    // {
    //     m_Labels = labels;
    //     m_Modules = ::std::move(modules);
    //     m_CurrCodePtr = nullptr;
    //     m_CurrentModule = currentModule;
    // }
    //
    // void Reset(DynArray<const u8*>&& labels, const ::std::vector<Ref<Module>>& modules, const u16 currentModule) noexcept
    // {
    //     m_Labels = ::std::move(labels);
    //     // m_Modules = modules;
    //     m_CurrCodePtr = nullptr;
    //     m_CurrentModule = currentModule;
    // }
    
    void Reset(DynArray<const u8*>&& labels, ::std::vector<StrongRef<Module>>* modules, const u16 currentModule) noexcept
    {
        m_Labels = ::std::move(labels);
        m_Modules = ::std::move(modules);
        m_CurrCodePtr = nullptr;
        m_CurrentModule = currentModule;
    }
public:
    void PreVisit(const u8* const codePtr) noexcept
    {
        const iSys labelIndex = ShouldPlaceLabel(codePtr);
        if(labelIndex != -1)
        {
            ConPrinter::PrintLn("  .L{}:", labelIndex);
        }

        m_CurrCodePtr = codePtr;
    }

    VISIT_PRINT_0(Nop);
    VISIT_PRINT_1(Push, 0);
    VISIT_PRINT_1(Push, 1);
    VISIT_PRINT_1(Push, 2);
    VISIT_PRINT_1(Push, 3);

    void VisitPushN(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Push.N {}", localIndex); }

    VISIT_PRINT_2(Push, Arg, 0);
    VISIT_PRINT_2(Push, Arg, 1);
    VISIT_PRINT_2(Push, Arg, 2);
    VISIT_PRINT_2(Push, Arg, 3);
    
    void VisitPushArgN(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Push.Arg.N {}", localIndex); }

    void VisitPushPtr(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Push.Ptr {}", localIndex); }

    void VisitPushGlobal(const u32 globalIndex) noexcept
    { ConPrinter::PrintLn("    Push.Global {}", globalIndex); }

    void VisitPushGlobalExt(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Push.Global.Ext ");
        PrintModule(moduleIndex);
        ConPrinter::PrintLn(":{}", globalIndex);
    }

    void VisitPushGlobalPtr(const u32 globalIndex) noexcept
    { ConPrinter::PrintLn("    Push.Global.Ptr {}", globalIndex); }

    void VisitPushGlobalExtPtr(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Push.Global.Ext.Ptr ");
        PrintModule(moduleIndex);
        ConPrinter::PrintLn(":{}", globalIndex);
    }

    VISIT_PRINT_1(Pop, 0);
    VISIT_PRINT_1(Pop, 1);
    VISIT_PRINT_1(Pop, 2);
    VISIT_PRINT_1(Pop, 3);

    void VisitPopN(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Pop.N {}", localIndex); }

    VISIT_PRINT_2(Pop, Arg, 0);
    VISIT_PRINT_2(Pop, Arg, 1);
    VISIT_PRINT_2(Pop, Arg, 2);
    VISIT_PRINT_2(Pop, Arg, 3);

    void VisitPopArgN(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Pop.Arg.N {}", localIndex); }

    void VisitPopPtr(const u16 localIndex) noexcept
    { ConPrinter::PrintLn("    Pop.Ptr {}", localIndex); }

    void VisitPopGlobal(const u32 globalIndex) noexcept
    { ConPrinter::PrintLn("    Pop.Global {}", globalIndex); }

    void VisitPopGlobalExt(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Pop.Global.Ext ");
        PrintModule(moduleIndex);
        ConPrinter::PrintLn(":{}", globalIndex);
    }

    void VisitPopGlobalPtr(const u32 globalIndex) noexcept
    { ConPrinter::PrintLn("    Pop.Global.Ptr {}", globalIndex); }

    void VisitPopGlobalExtPtr(const u32 globalIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Pop.Global.Ext.Ptr ");
        PrintModule(moduleIndex);
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
        PrintModule(moduleIndex);
        ConPrinter::PrintLn(":{}, {}", globalIndex, addressIndex);
    }

    void VisitStore(const u16 localIndex, const u16 addressIndex) noexcept
    { ConPrinter::PrintLn("    Store {}, {}", localIndex, addressIndex); }

    void VisitStoreGlobal(const u32 globalIndex, const u16 addressIndex) noexcept
    { ConPrinter::PrintLn("    Store.Global {}, {}", globalIndex, addressIndex); }

    void VisitStoreGlobalExt(const u32 globalIndex, const u16 addressIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Store.Global.Ext ");
        PrintModule(moduleIndex);
        ConPrinter::PrintLn(":{}, {}", globalIndex, addressIndex);
    }

    VISIT_PRINT_1(Const, 0);
    VISIT_PRINT_1(Const, 1);
    VISIT_PRINT_1(Const, 2);
    VISIT_PRINT_1(Const, 3);
    VISIT_PRINT_1(Const, 4);
    VISIT_PRINT_1(Const, FF);
    VISIT_PRINT_1(Const, 7F);
    
    void VisitConstN(const u32 constant) noexcept
    { ConPrinter::PrintLn("    Const.N {}", constant); }

    VISIT_PRINT_0_I32(Add);
    VISIT_PRINT_0_I64(Add);
    VISIT_PRINT_0_I32(Sub);
    VISIT_PRINT_0_I64(Sub);
    VISIT_PRINT_0_I32(Mul);
    VISIT_PRINT_0_I64(Mul);
    VISIT_PRINT_0_I32(Div);
    VISIT_PRINT_0_I64(Div);

    VISIT_PRINT_1_I32(Comp, Above);
    VISIT_PRINT_1_I32(Comp, AboveOrEqual);
    VISIT_PRINT_1_I32(Comp, Below);
    VISIT_PRINT_1_I32(Comp, BelowOrEqual);
    VISIT_PRINT_1_I32(Comp, Greater);
    VISIT_PRINT_1_I32(Comp, GreaterOrEqual);
    VISIT_PRINT_1_I32(Comp, Less);
    VISIT_PRINT_1_I32(Comp, LessOrEqual);
    VISIT_PRINT_1_I32(Comp, NotEqual);

    VISIT_PRINT_1_I64(Comp, Above);
    VISIT_PRINT_1_I64(Comp, AboveOrEqual);
    VISIT_PRINT_1_I64(Comp, Below);
    VISIT_PRINT_1_I64(Comp, BelowOrEqual);
    VISIT_PRINT_1_I64(Comp, Greater);
    VISIT_PRINT_1_I64(Comp, GreaterOrEqual);
    VISIT_PRINT_1_I64(Comp, Less);
    VISIT_PRINT_1_I64(Comp, LessOrEqual);
    VISIT_PRINT_1_I64(Comp, NotEqual);
    
    void VisitCall(const u32 functionIndex) noexcept
    {
        ConPrinter::Print("    Call ");
        PrintFunction(functionIndex, m_CurrentModule);
        ConPrinter::PrintLn();
    }

    void VisitCallExt(const u32 functionIndex, const u16 moduleIndex) noexcept
    {
        ConPrinter::Print("    Call.Ext ");
        PrintModule(moduleIndex);
        ConPrinter::Print(':');
        PrintFunction(functionIndex, moduleIndex);
        ConPrinter::PrintLn();
    }

    void VisitCallInd(const u16 localIndex) noexcept
    {
        ConPrinter::PrintLn("    Call.Ind {}", localIndex);
    }

    void VisitCallIndExt(const u16 localIndex) noexcept
    {
        ConPrinter::PrintLn("    Call.Ind.Ext {}", localIndex);
    }

    VISIT_PRINT_0(Ret);

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

    void PrintModule(const u16 moduleIndex) noexcept
    {
        if(moduleIndex >= Modules().size() || Modules()[moduleIndex]->Name().Length() == 0)
        {
            ConPrinter::Print(moduleIndex);
        }
        else
        {
            ConPrinter::Print(Modules()[moduleIndex]->Name());
        }
    }

    void PrintFunction(const u32 functionIndex, const u16 moduleIndex) noexcept
    {
        if(moduleIndex < Modules().size())
        {
            const DynArray<Function*>& functions = Modules()[moduleIndex]->Functions();

            if(functionIndex < functions.Size())
            {
                if(functions[functionIndex]->Name().Length() != 0)
                {
                    ConPrinter::Print(functions[functionIndex]->Name());
                    return;
                }
            }
        }

        ConPrinter::Print("<Func{}>", functionIndex);
    }

    [[nodiscard]] const ::std::vector<StrongRef<Module>>& Modules() const noexcept { return *m_Modules; }
private:
    DynArray<const u8*> m_Labels;
    const ::std::vector<StrongRef<Module>>* m_Modules;
    const u8* m_CurrCodePtr;
    u16 m_CurrentModule;
};

#undef VISIT_PRINT_1_I64
#undef VISIT_PRINT_1_I32
#undef VISIT_PRINT_0_I64
#undef VISIT_PRINT_0_I32
#undef VISIT_PRINT_2
#undef VISIT_PRINT_1
#undef VISIT_PRINT_0

static DynArray<const u8*> PreProcessFunction(const Function* function) noexcept;

void DumpFunction(const tau::ir::Function* function, const uSys functionIndex, const ModuleList& modules, const u16 moduleIndex) noexcept
{
    if(function->Name().Length() != 0)
    {
        ConPrinter::PrintLn("{}:", function->Name());
    }
    else
    {
        ConPrinter::PrintLn("Func{}:", functionIndex);
    }

    const DynArray<const u8*> labels = PreProcessFunction(function);

    DumpVisitor dumpVisitor(labels, &modules, moduleIndex);
    dumpVisitor.Traverse(function->Address(), function->Address() + function->CodeSize());
}

class VisitorJumpCount final : public BaseIrVisitor<VisitorJumpCount>
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

    void VisitJumpPoint(const i32 offset) noexcept
    {
        ++m_JumpCount;
    }
private:
    uSys m_JumpCount;
};

class VisitorLabeler final : public BaseIrVisitor<VisitorLabeler>
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

    void VisitJumpPoint(const i32 offset) noexcept
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
    jumpCountVisitor.Traverse(function->Address(), function->Address() + function->CodeSize());
    
    VisitorLabeler labelerVisitor(jumpCountVisitor.JumpCount());
    labelerVisitor.Traverse(function->Address(), function->Address() + function->CodeSize());

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
	}
}

static void PrintCompareCondition(const CompareCondition condition) noexcept
{
    switch(condition)
    {
        case CompareCondition::Above:
            ConPrinter::Print("&>");
            break;
        case CompareCondition::AboveOrEqual:
            ConPrinter::Print("&>=");
            break;
        case CompareCondition::Below:
            ConPrinter::Print("&<");
            break;
        case CompareCondition::BelowOrEqual:
            ConPrinter::Print("&<=");
            break;
        case CompareCondition::Equal:
            ConPrinter::Print("==");
            break;
        case CompareCondition::Greater:
            ConPrinter::Print('>');
            break;
        case CompareCondition::GreaterOrEqual:
            ConPrinter::Print(">=");
            break;
        case CompareCondition::Less:
            ConPrinter::Print('<');
            break;
        case CompareCondition::LessOrEqual:
            ConPrinter::Print("<=");
            break;
        case CompareCondition::NotEqual:
            ConPrinter::Print("!=");
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
                ConPrinter::PrintLn("  .{}:", idIndex++);
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
            case SsaOpcode::CompVtoV:
            {
                const CompareCondition condition = ReadType<CompareCondition>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = ", idIndex++);
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::Print(' ');
                PrintCompareCondition(condition);
                ConPrinter::Print(' ');
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::PrintLn();

                break;
            }
            case SsaOpcode::CompVtoI:
            {
                const CompareCondition condition = ReadType<CompareCondition>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = ", idIndex++);
                PrintValue(type, codePtr, i, registry);
                ConPrinter::Print(' ');
                PrintCompareCondition(condition);
                ConPrinter::Print(' ');
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::PrintLn();

                break;
            }
            case SsaOpcode::CompItoV:
            {
                const CompareCondition condition = ReadType<CompareCondition>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);

                ConPrinter::Print("  ");
                PrintType(type);
                ConPrinter::Print(" %{} = ", idIndex++);
                PrintVar(ReadType<VarId>(codePtr, i));
                ConPrinter::Print(' ');
                PrintCompareCondition(condition);
                ConPrinter::Print(' ');
                PrintValue(type, codePtr, i, registry);
                ConPrinter::PrintLn();

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
                const u32 function = ReadType<u32>(codePtr, i);
                const u32 baseIndex = ReadType<u32>(codePtr, i);
                const u32 parameterCount = ReadType<u32>(codePtr, i);

                ConPrinter::PrintLn("  u64 %{} = Call <Func{}>(), %{}-{}", idIndex++, function, baseIndex, parameterCount);
                break;
            }
            case SsaOpcode::CallExt:
            {
                const u32 function = ReadType<u32>(codePtr, i);
                const u32 baseIndex = ReadType<u32>(codePtr, i);
                const u32 parameterCount = ReadType<u32>(codePtr, i);
                const u16 moduleId = ReadType<u16>(codePtr, i);

                ConPrinter::PrintLn("  u64 %{} = Call.Ext {}:<Func{}>(), %{}-{}", idIndex++, moduleId, function, baseIndex, parameterCount);

                break;
            }
            case SsaOpcode::CallInd:
            {
                const VarId functionPointer = ReadType<VarId>(codePtr, i);
                const u32 baseIndex = ReadType<u32>(codePtr, i);
                const u32 parameterCount = ReadType<u32>(codePtr, i);
                
                ConPrinter::PrintLn("  u64 %{} = Call.Ind %{}(), %{}-{}", idIndex++, functionPointer, baseIndex, parameterCount);
                
                break;
            }
            case SsaOpcode::CallIndExt:
            {
                const VarId functionPointer = ReadType<VarId>(codePtr, i);
                const u32 baseIndex = ReadType<u32>(codePtr, i);
                const u32 parameterCount = ReadType<u32>(codePtr, i);
                const VarId modulePointer = ReadType<VarId>(codePtr, i);
                
                ConPrinter::PrintLn("  u64 %{} = Call.Ind.Ext %{}:%{}(), %{}-{}", idIndex++, modulePointer, functionPointer, baseIndex, parameterCount);
                
                break;
            }
            case SsaOpcode::Ret:
            {
                const SsaCustomType returnType = ReadType<SsaCustomType>(codePtr, i);
                const u32 returnVar = ReadType<u32>(codePtr, i);

                ConPrinter::Print("  Ret ");
                PrintType(returnType);
                ConPrinter::PrintLn(" %{}", returnVar);

                break;
            }
        }
    }
}

void DumpSsa(const Function* const function, const uSys functionIndex, const SsaCustomTypeRegistry& registry) noexcept
{
    const SsaFunctionAttachment* const ssaAttachment = function->FindAttachment<SsaFunctionAttachment>();

    if(!ssaAttachment)
    {
        return;
    }

    DumpSsa(ssaAttachment->Writer().Buffer(), ssaAttachment->Writer().Size(), functionIndex, registry);
}
} }
