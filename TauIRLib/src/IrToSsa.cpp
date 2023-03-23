#include "TauIR/IrToSsa.hpp"
#include "TauIR/Function.hpp"
#include "TauIR/Opcodes.hpp"
#include "TauIR/TypeInfo.hpp"
#include "TauIR/IrVisitor.hpp"
#include "TauIR/Module.hpp"
#include "TauIR/FunctionNameMangler.hpp"
#include "TauIR/ssa/SsaFunctionAttachment.hpp"

namespace tau::ir {

static ssa::SsaType GetSsaType(const TypeInfo& type) noexcept;

static ssa::SsaType GetSsaType(const TypeInfo* type) noexcept
{
    return GetSsaType(*TypeInfo::StripPointer(type));
}

static ssa::SsaType GetSignedSizeType(const uSys size) noexcept
{
    switch(size)
    {
        case 1: return ssa::SsaType::I8;
        case 2: return ssa::SsaType::I16;
        case 4: return ssa::SsaType::I32;
        case 8: return ssa::SsaType::I64;
        default: return ssa::SsaType::Void;
    }
}

static ssa::SsaType GetUnsignedSizeType(const uSys size) noexcept
{
    switch(size)
    {
        case 1: return ssa::SsaType::U8;
        case 2: return ssa::SsaType::U16;
        case 4: return ssa::SsaType::U32;
        case 8: return ssa::SsaType::U64;
        default: return ssa::SsaType::Void;
    }
}

class IrToSsaVisitor final : public BaseIrVisitor<IrToSsaVisitor>
{
    DEFAULT_DESTRUCT(IrToSsaVisitor);
    DELETE_CM(IrToSsaVisitor);
public:
    using SsaWriter = ssa::SsaWriter;
    using SsaFrameTracker = ssa::SsaFrameTracker;
    using VarId = ssa::VarId;
public:
    IrToSsaVisitor(const Function* const function, const ModuleList& modules, const u16 currentModule) noexcept
        : m_Function(function)
        , m_Writer{}
        , m_FrameTracker(function->LocalTypes().count())
        , m_Modules(modules)
        , m_CurrentModule(currentModule)
    { }

    [[nodiscard]] const ssa::SsaWriter& Writer() const noexcept { return m_Writer; }
    [[nodiscard]]       ssa::SsaWriter& Writer()       noexcept { return m_Writer; }
public:
    void VisitPush(const u16 localIndex) noexcept
    {
        const VarId localVar = m_FrameTracker.GetLocal(localIndex);
        const VarId newVar = m_Writer.WriteAssignVariable(GetSsaType(m_Function->LocalTypes()[localIndex]), localVar);
        m_FrameTracker.PushFrame(newVar, TypeInfo::StripPointer(m_Function->LocalTypes()[localIndex])->Size());
    }

    void VisitPushArg(const u16 argumentIndex) noexcept
    {
        const VarId argVar = m_FrameTracker.GetArgument(argumentIndex);
        const VarId newVar = m_Writer.WriteAssignVariable(ssa::SsaType::U64, argVar);
        m_FrameTracker.PushFrame(newVar, 8);
    }

    void VisitPushPtr(const u16 localIndex) noexcept
    {
        const VarId localVar = m_FrameTracker.GetLocal(localIndex);
        const VarId newVar = m_Writer.WriteLoad(GetSsaType(m_Function->LocalTypes()[localIndex]), localVar);
        m_FrameTracker.PushFrame(newVar, TypeInfo::StripPointer(m_Function->LocalTypes()[localIndex])->Size());
    }

    void VisitPop(const u16 localIndex) noexcept
    {
        IrToSsa::PopLocal(m_Function, m_Writer, m_FrameTracker, localIndex);
    }

    void VisitPopArg(const u16 argumentIndex) noexcept
    {
        IrToSsa::PopArgument(m_Function, m_Writer, m_FrameTracker, argumentIndex);
    }

    void VisitPopPtr(const u16 localIndex) noexcept
    {
        const TypeInfo& typeInfo = *TypeInfo::StripPointer(m_Function->LocalTypes()[localIndex]);

        const VarId dataDest = m_FrameTracker.GetLocal(localIndex);
        const VarId newVar = IrToSsa::PopRaw(m_Writer, m_FrameTracker, typeInfo.Size(), GetSsaType(typeInfo));

        m_Writer.WriteStoreV(GetSsaType(typeInfo), dataDest, newVar);
    }

    void VisitPopCount(const u16 byteCount) noexcept
    {
        // Pop popCount bytes from the stack and store it in a discard raw byte buffer.
        (void) IrToSsa::PopRaw(m_Writer, m_FrameTracker, byteCount, ssa::SsaType::Bytes);
    }

    void VisitDup(const uSys byteCount) noexcept
    {
        // Pop off 1 byte from the stack.
        const VarId dupeTarget = IrToSsa::PopRaw(m_Writer, m_FrameTracker, byteCount, ssa::SsaType::Bytes);
        // Push the popped data back onto the stack twice.
        m_FrameTracker.PushFrame(dupeTarget, byteCount);
        m_FrameTracker.PushFrame(dupeTarget, byteCount);
    }

    void VisitExpandSX(const uSys fromSize, const uSys toSize) noexcept
    {
        // Pop off `fromSize` bytes from the stack.
        const VarId expandTarget = IrToSsa::PopRaw(m_Writer, m_FrameTracker, fromSize, GetSignedSizeType(fromSize));
        // Sign Extend to `toSize` bytes.
        const VarId expanded = m_Writer.WriteExpandSX(GetSignedSizeType(toSize), GetSignedSizeType(fromSize), expandTarget);
        // Push onto stack.
        m_FrameTracker.PushFrame(expanded, toSize);
    }

    void VisitExpandZX(const uSys fromSize, const uSys toSize) noexcept
    {
        // Pop off `fromSize` bytes from the stack.
        const VarId expandTarget = IrToSsa::PopRaw(m_Writer, m_FrameTracker, fromSize, GetUnsignedSizeType(fromSize));
        // Zero Extend to `toSize` bytes.
        const VarId expanded = m_Writer.WriteExpandZX(GetUnsignedSizeType(toSize), GetUnsignedSizeType(fromSize), expandTarget);
        // Push onto stack.
        m_FrameTracker.PushFrame(expanded, toSize);
    }

    void VisitTrunc(const uSys fromSize, const uSys toSize) noexcept
    {
        // Pop off `fromSize` bytes from the stack.
        const VarId truncTarget = IrToSsa::PopRaw(m_Writer, m_FrameTracker, fromSize, GetUnsignedSizeType(fromSize));
        // Truncate to `toSize` bytes.
        const VarId expanded = m_Writer.WriteTrunc(GetUnsignedSizeType(toSize), GetUnsignedSizeType(fromSize), truncTarget);
        // Push onto stack.
        m_FrameTracker.PushFrame(expanded, toSize);
    }

    void VisitConst(const u32 constant) noexcept
    {
        // Assign the variable.
        const VarId constantVar = m_Writer.WriteAssignImmediate(ssa::SsaType::U32, &constant, sizeof(constant));
        // Push onto the stack.
        m_FrameTracker.PushFrame(constantVar, 4);
    }

    void VisitBinOp(const uSys size, const ssa::SsaBinaryOperation operation, const ssa::SsaType type) noexcept
    {
        // Pop `size` bytes from the stack into register B.
        const VarId regB = IrToSsa::PopRaw(m_Writer, m_FrameTracker, size, type);
        // Pop `size` bytes from the stack into register A.
        const VarId regA = IrToSsa::PopRaw(m_Writer, m_FrameTracker, size, type);
        // Operate B to A.
        const VarId res = m_Writer.WriteBinOpVtoV(operation, type, regA, regB);
        // Push result onto the stack.
        m_FrameTracker.PushFrame(res, size);
    }

#define VISIT_BASIC_BIN_OP_I32(OPERATION) \
    void Visit##OPERATION##I32() noexcept { \
        VisitBinOp(4, ssa::SsaBinaryOperation::OPERATION, ssa::SsaType::U32); \
    }

#define VISIT_BASIC_BIN_OP_I64(OPERATION) \
    void Visit##OPERATION##I64() noexcept { \
        VisitBinOp(8, ssa::SsaBinaryOperation::OPERATION, ssa::SsaType::U64); \
    }

    VISIT_BASIC_BIN_OP_I32(Add);
    VISIT_BASIC_BIN_OP_I64(Add);
    VISIT_BASIC_BIN_OP_I32(Sub);
    VISIT_BASIC_BIN_OP_I64(Sub);
    VISIT_BASIC_BIN_OP_I32(Mul);
    VISIT_BASIC_BIN_OP_I64(Mul);

    void VisitDivI32() noexcept
    {
        // Pop 4 bytes from the stack into register B.
        const VarId regB = IrToSsa::PopRaw(m_Writer, m_FrameTracker, 4, ssa::SsaType::U32);
        // Pop 4 bytes from the stack into register A.
        const VarId regA = IrToSsa::PopRaw(m_Writer, m_FrameTracker, 4, ssa::SsaType::U32);
        // Divide A by B.
        const VarId quotient = m_Writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Div, ssa::SsaType::U32, regA, regB);
        // Modulo A by B.
        const VarId remainder = m_Writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Rem, ssa::SsaType::U32, regA, regB);
        // Push the quotient onto the stack.
        m_FrameTracker.PushFrame(quotient, 4);
        // Push the remainder onto the stack.
        m_FrameTracker.PushFrame(remainder, 4);
    }

    void VisitDivI64() noexcept
    {
        // Pop 8 bytes from the stack into register B.
        const VarId regB = IrToSsa::PopRaw(m_Writer, m_FrameTracker, 8, ssa::SsaType::U64);
        // Pop 8 bytes from the stack into register A.
        const VarId regA = IrToSsa::PopRaw(m_Writer, m_FrameTracker, 8, ssa::SsaType::U64);
        // Divide A by B.
        const VarId quotient = m_Writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Div, ssa::SsaType::U64, regA, regB);
        // Modulo A by B.
        const VarId remainder = m_Writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Rem, ssa::SsaType::U64, regA, regB);
        // Push the quotient onto the stack.
        m_FrameTracker.PushFrame(quotient, 8);
        // Push the remainder onto the stack.
        m_FrameTracker.PushFrame(remainder, 8);
    }

    void VisitComp(const uSys size, const CompareCondition condition, const ssa::SsaType type) noexcept
    {
        // Pop `size` bytes from the stack into register B.
        const VarId regB = IrToSsa::PopRaw(m_Writer, m_FrameTracker, size, type);
        // Pop `size` bytes from the stack into register A.
        const VarId regA = IrToSsa::PopRaw(m_Writer, m_FrameTracker, size, type);
        // Operate B to A.
        const VarId res = m_Writer.WriteCompVtoV(condition, type, regA, regB);
        // Push result onto the stack.
        m_FrameTracker.PushFrame(res, size);
    }

    void VisitCompI32(const CompareCondition condition) noexcept
    {
        VisitComp(4, condition, ssa::SsaType::U32);
    }

    void VisitCompI64(const CompareCondition condition) noexcept
    {
        VisitComp(8, condition, ssa::SsaType::U64);
    }

    u32 HandleFunctionArgs(const DynArray<FunctionArgument>& args) noexcept
    {
        for(uSys i = 0; i < args.Length(); ++i)
        {
            if(args[i].IsRegister)
            {
                const VarId argVar = m_FrameTracker.GetArgument(args[i].RegisterOrStackOffset);
                (void) m_Writer.WriteAssignVariable(ssa::SsaType::U64, argVar);
            }
            else
            {
                const VarId argVar = IrToSsa::PopRaw(m_Writer, m_FrameTracker, 8, ssa::SsaType::U64);
                (void) m_Writer.WriteAssignVariable(ssa::SsaType::U64, argVar);
            }
        }

        return static_cast<u32>(args.Length());
    }

    u32 HandleCallSite(const u32 functionIndex, const u32 moduleIndex) noexcept
    {
        const Function* targetFunction = m_Modules[moduleIndex]->Functions()[functionIndex];
        return HandleFunctionArgs(targetFunction->Arguments());
    }

    void VisitCall(const u32 functionIndex) noexcept
    {
        const VarId baseIndex = m_Writer.IdIndex() + 1;
        const u32 argCount = HandleCallSite(functionIndex, m_CurrentModule);
        const VarId retId = m_Writer.WriteCall(functionIndex, baseIndex, argCount);
        m_FrameTracker.SetArgument(retId, 0);
    }

    void VisitCallExt(const u32 functionIndex, const u16 moduleIndex) noexcept
    {
        const VarId baseIndex = m_Writer.IdIndex() + 1;
        const u32 argCount = HandleCallSite(functionIndex, moduleIndex);
        const VarId retId = m_Writer.WriteCallExt(functionIndex, baseIndex, argCount, moduleIndex);
        m_FrameTracker.SetArgument(retId, 0);
    }

    u32 HandleIndirectCallSite(const u16 localIndex) noexcept
    {
        const TypeInfo* const functionType = m_Function->LocalTypes()[localIndex];

        if(functionType->Size() != 4)
        {
            return 0;
        }
        
        return HandleFunctionArgs(DeMangleFunctionName(functionType->Name()));
    }

    void VisitCallInd(const u16 localIndex) noexcept
    {
        const VarId baseIndex = m_Writer.IdIndex() + 1;
        const u32 argCount = HandleIndirectCallSite(localIndex);
        const VarId retId = m_Writer.WriteCallInd(m_FrameTracker.GetLocal(localIndex), baseIndex, argCount);
        m_FrameTracker.SetArgument(retId, 0);
    }

    void VisitCallIndExt(const u16 localIndex) noexcept
    {
        const VarId baseIndex = m_Writer.IdIndex() + 1;
        const u32 argCount = HandleIndirectCallSite(localIndex);
        const VarId moduleIndex = IrToSsa::PopRaw(m_Writer, m_FrameTracker, 2, ssa::SsaType::U16);
        const VarId retId = m_Writer.WriteCallIndExt(m_FrameTracker.GetLocal(localIndex), baseIndex, argCount, moduleIndex);
        m_FrameTracker.SetArgument(retId, 0);
    }

    void VisitRet() noexcept
    {
        const VarId argVar = m_FrameTracker.GetArgument(0);
        m_Writer.WriteRet(ssa::SsaType::U64, argVar);
    }
private:
    const Function* m_Function;
    SsaWriter m_Writer;
    SsaFrameTracker m_FrameTracker;
    ModuleList m_Modules;
    u16 m_CurrentModule;
};

void IrToSsa::TransformFunction(Function* const function, const ModuleList& modules, const u16 currentModule) noexcept
{
    IrToSsaVisitor visitor(function, modules, currentModule);
    visitor.Traverse(function->Address(), function->Address() + function->CodeSize());

    function->Attach<ssa::SsaFunctionAttachment>(::std::move(visitor.Writer()));
}
    
IrToSsa::VarId IrToSsa::PopRaw(SsaWriter& writer, SsaFrameTracker& frameTracker, const uSys size, const ssa::SsaType ssaType)
{
    // Pop the first frame.
    auto frame = frameTracker.PopFrame(size);
    // Track the total popped frame size for later.
    uSys frameSize = frame.Size;
    // If the total popped frame size matches the local type size, assign new var, otherwise join multiple vars.
    if(frameSize == size)
    {
        // Assign new var from the popped frame.
        // const VarId newVar = writer.WriteAssignVariable(ssaType, frame.Var);
        return frame.Var;
    }
    else
    {
        // The max number of elements we can reasonably expect to show up in a single merge-pop. This will be the source of many buffer overflow exploits.
        constexpr uSys packSize = 16;

        // Setup a buffer of types for the join.
        ssa::SsaCustomType types[packSize];
        // Setup a buffer of vars for the join.
        VarId vars[packSize];

        // The current buffer write index. We will write in reverse order to prevent having to do an array reverse later.
        iSys index = packSize - 1;

        // Write the first popped frame type to the last element.
        types[index] = writer.GetVarType(frame.Var);
        // Write the first popped frame var to the last element.
        vars[index] = frame.Var;

        // Loop until we've popped enough frame to meet the size of the local type.
        while(frameSize < size)
        {
            // Pop the frame.
            frame = frameTracker.PopFrame(size);

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
        if(frameSize > size)
        {
            // Find how many bytes spill over the highest byte of local type.
            const u32 sizeSpill = static_cast<u32>(frameSize - size);
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
        const VarId newVar = writer.WriteJoin(ssaType, 0, types + index, vars + index);
        return newVar;
    }
}

IrToSsa::VarId IrToSsa::PopLocal(const Function* const function, SsaWriter& writer, SsaFrameTracker& frameTracker, const VarId localIndex)
{
    // Get the local type that we're going to pop into.
    const TypeInfo& typeInfo = *TypeInfo::StripPointer(function->LocalTypes()[localIndex]);
    const VarId newVar = PopRaw(writer, frameTracker, typeInfo.Size(), GetSsaType(typeInfo));
    frameTracker.SetLocal(newVar, localIndex);
    return newVar;
}
    
IrToSsa::VarId IrToSsa::PopArgument(const Function* const function, SsaWriter& writer, SsaFrameTracker& frameTracker, const VarId argIndex)
{
    const VarId newVar = PopRaw(writer, frameTracker, 8, ssa::SsaType::U64);
    frameTracker.SetArgument(newVar, argIndex);
    return newVar;
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
