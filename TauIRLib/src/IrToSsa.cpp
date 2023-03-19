#include "TauIR/IrToSsa.hpp"
#include "TauIR/Function.hpp"
#include "TauIR/Opcodes.hpp"
#include "TauIR/TypeInfo.hpp"
#include "TauIR/IrVisitor.hpp"
#include "TauIR/Module.hpp"

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
    IrToSsaVisitor(const Function* const function, const ::std::vector<Ref<Module>>& modules, const u16 currentModule) noexcept
        : m_Function(function)
        , m_Writer{}
        , m_FrameTracker(function->LocalTypes().count())
        , m_Modules(&modules)
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

    u32 HandleCallSite(const u32 functionIndex, const u32 moduleIndex) noexcept
    {
        const Function* targetFunction = Modules()[moduleIndex]->Functions()[functionIndex];
        const DynArray<FunctionArgument>& args = targetFunction->Arguments();

        const VarId baseIndex = m_Writer.IdIndex() + 1;

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

    void VisitRet() noexcept
    {
        const VarId argVar = m_FrameTracker.GetArgument(0);
        m_Writer.WriteRet(ssa::SsaType::U64, argVar);
    }
private:
    [[nodiscard]] const ::std::vector<Ref<Module>>& Modules() noexcept { return *m_Modules; }
private:
    const Function* m_Function;
    SsaWriter m_Writer;
    SsaFrameTracker m_FrameTracker;
    const ::std::vector<Ref<Module>>* m_Modules;
    u16 m_CurrentModule;
};

ssa::SsaWriter IrToSsa::TransformFunction(const Function* const function, const ::std::vector<Ref<Module>>& modules, const u16 currentModule) noexcept
{
    const u8* codePtr = function->Address();

    IrToSsaVisitor visitor(function, modules, currentModule);
    visitor.Traverse(function->Address(), function->Address() + function->CodeSize());

    // SsaWriter writer;
    // SsaFrameTracker frameTracker(function->LocalTypes().count());
    //
    // while(false)
    // {
    //     u16 opcodeRaw = *codePtr;
    //     ++codePtr;
    //
    //     // Read Second Byte
    //     if(opcodeRaw & 0x80)
    //     {
    //         opcodeRaw <<= 8;
    //         opcodeRaw |= *codePtr;
    //         ++codePtr;
    //     }
    //
    //     const Opcode opcode = static_cast<Opcode>(opcodeRaw);
    //     
    //     if(opcode == Opcode::Ret)
    //     {
    //         break;
    //     }
    //
    //     switch(opcode)
    //     {
    //         case Opcode::Nop:
    //             break;
    //         case Opcode::Push0:
    //         {
    //             const VarId localVar = frameTracker.GetLocal(0);
    //             const VarId newVar = writer.WriteAssignVariable(GetSsaType(function->LocalTypes()[0]), localVar);
    //             frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[0])->Size());
    //             break;
    //         }
    //         case Opcode::Push1:
    //         {
    //             const VarId localVar = frameTracker.GetLocal(1);
    //             const VarId newVar = writer.WriteAssignVariable(GetSsaType(function->LocalTypes()[1]), localVar);
    //             frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[1])->Size());
    //             break;
    //         }
    //         case Opcode::Push2:
    //         {
    //             const VarId localVar = frameTracker.GetLocal(2);
    //             const VarId newVar = writer.WriteAssignVariable(GetSsaType(function->LocalTypes()[2]), localVar);
    //             frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[2])->Size());
    //             break;
    //         }
    //         case Opcode::Push3:
    //         {
    //             const VarId localVar = frameTracker.GetLocal(3);
    //             const VarId newVar = writer.WriteAssignVariable(GetSsaType(function->LocalTypes()[3]), localVar);
    //             frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[3])->Size());
    //             break;
    //         }
    //         case Opcode::PushN:
    //         {
    //             const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
    //             codePtr += 2;
    //             
    //             const VarId localVar = frameTracker.GetLocal(localIndex);
    //             const VarId newVar = writer.WriteAssignVariable(GetSsaType(function->LocalTypes()[localIndex]), localVar);
    //             frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[localIndex])->Size());
    //             break;
    //         }
    //         case Opcode::PushArg0:
    //         {
    //             const VarId newVar = writer.WriteAssignVariable(ssa::SsaType::U64, 0x80000000);
    //             frameTracker.PushFrame(newVar, 8);
    //             break;
    //         }
    //         case Opcode::PushArg1:
    //         {
    //             const VarId newVar = writer.WriteAssignVariable(ssa::SsaType::U64, 0x80000001);
    //             frameTracker.PushFrame(newVar, 8);
    //             break;
    //         }
    //         case Opcode::PushArg2:
    //         {
    //             const VarId newVar = writer.WriteAssignVariable(ssa::SsaType::U64, 0x80000002);
    //             frameTracker.PushFrame(newVar, 8);
    //             break;
    //         }
    //         case Opcode::PushArg3:
    //         {
    //             const VarId newVar = writer.WriteAssignVariable(ssa::SsaType::U64, 0x80000003);
    //             frameTracker.PushFrame(newVar, 8);
    //             break;
    //         }
    //         case Opcode::PushArgN:
    //         {
    //             const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
    //             codePtr += 2;
    //             
    //             const VarId newVar = writer.WriteAssignVariable(ssa::SsaType::U64, localIndex | 0x80000000);
    //             frameTracker.PushFrame(newVar, 8);
    //             break;
    //         }
    //         case Opcode::PushPtr:
    //         {
    //             const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
    //             codePtr += 2;
    //             
    //             const VarId localVar = frameTracker.GetLocal(localIndex);
    //             const VarId newVar = writer.WriteLoad(GetSsaType(function->LocalTypes()[localIndex]), localVar);
    //             frameTracker.PushFrame(newVar, TypeInfo::StripPointer(function->LocalTypes()[0])->Size());
    //             break;
    //         }
    //         case Opcode::Pop0:
    //         {
    //             PopLocal(function, writer, frameTracker, 0);
    //             break;
    //         }
    //         case Opcode::Pop1:
    //         {
    //             PopLocal(function, writer, frameTracker, 1);
    //             break;
    //         }
    //         case Opcode::Pop2:
    //         {
    //             PopLocal(function, writer, frameTracker, 2);
    //             break;
    //         }
    //         case Opcode::Pop3:
    //         {
    //             PopLocal(function, writer, frameTracker, 3);
    //             break;
    //         }
    //         case Opcode::PopN:
    //         {
    //             const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
    //             codePtr += 2;
    //             
    //             PopLocal(function, writer, frameTracker, localIndex);
    //             break;
    //         }
    //         case Opcode::PopArg0:
    //         {
    //             PopArgument(function, writer, frameTracker, 0);
    //             break;
    //         }
    //         case Opcode::PopArg1:
    //         {
    //             PopArgument(function, writer, frameTracker, 1);
    //             break;
    //         }
    //         case Opcode::PopArg2:
    //         {
    //             PopArgument(function, writer, frameTracker, 2);
    //             break;
    //         }
    //         case Opcode::PopArg3:
    //         {
    //             PopArgument(function, writer, frameTracker, 3);
    //             break;
    //         }
    //         case Opcode::PopArgN:
    //         {
    //             const u8 argIndex = *codePtr;
    //             ++codePtr;
    //             
    //             PopArgument(function, writer, frameTracker, argIndex);
    //             break;
    //         }
    //         case Opcode::PopPtr:
    //         {
    //             const u16 localIndex = *reinterpret_cast<const u16*>(codePtr);
    //             codePtr += 2;
    //             
    //             const TypeInfo& typeInfo = *TypeInfo::StripPointer(function->LocalTypes()[localIndex]);
    //             
    //             const VarId dataDest = frameTracker.GetLocal(localIndex);
    //             const VarId newVar = PopRaw(writer, frameTracker, typeInfo.Size(), GetSsaType(typeInfo));
    //
    //             writer.WriteStoreV(GetSsaType(typeInfo), dataDest, newVar);
    //             break;
    //         }
    //         case Opcode::PopCount:
    //         {
    //             const u16 popCount = *reinterpret_cast<const u16*>(codePtr);
    //             codePtr += 2;
    //
    //             // Pop popCount bytes from the stack and store it in a discard raw byte buffer.
    //             (void) PopRaw(writer, frameTracker, popCount, ssa::SsaType::Bytes);
    //             break;
    //         }
    //         case Opcode::Dup1:
    //         {
    //             // Pop off 1 byte from the stack.
    //             const VarId dupeTarget = PopRaw(writer, frameTracker, 1, ssa::SsaType::Bytes);
    //             // Push the popped data back onto the stack twice.
    //             frameTracker.PushFrame(dupeTarget, 1);
    //             frameTracker.PushFrame(dupeTarget, 1);
    //             break;
    //         }
    //         case Opcode::Dup2:
    //         {
    //             // Pop off 2 bytes from the stack.
    //             const VarId dupeTarget = PopRaw(writer, frameTracker, 2, ssa::SsaType::Bytes);
    //             // Push the popped data back onto the stack twice.
    //             frameTracker.PushFrame(dupeTarget, 2);
    //             frameTracker.PushFrame(dupeTarget, 2);
    //             break;
    //         }
    //         case Opcode::Dup4:
    //         {
    //             // Pop off 4 bytes from the stack.
    //             const VarId dupeTarget = PopRaw(writer, frameTracker, 4, ssa::SsaType::Bytes);
    //             // Push the popped data back onto the stack twice.
    //             frameTracker.PushFrame(dupeTarget, 4);
    //             frameTracker.PushFrame(dupeTarget, 4);
    //             break;
    //         }
    //         case Opcode::Dup8:
    //         {
    //             // Pop off 8 bytes from the stack.
    //             const VarId dupeTarget = PopRaw(writer, frameTracker, 8, ssa::SsaType::Bytes);
    //             // Push the popped data back onto the stack twice.
    //             frameTracker.PushFrame(dupeTarget, 8);
    //             frameTracker.PushFrame(dupeTarget, 8);
    //             break;
    //         }
    //         case Opcode::ExpandSX12:
    //         {
    //             // Pop off 1 byte from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 1, ssa::SsaType::Bytes);
    //             // Sign Extend to 2 bytes.
    //             const VarId expanded = writer.WriteExpandSX(ssa::SsaType::I16, ssa::SsaType::I8, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 2);
    //             break;
    //         }
    //         case Opcode::ExpandSX14:
    //         {
    //             // Pop off 1 byte from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 1, ssa::SsaType::Bytes);
    //             // Sign Extend to 4 bytes.
    //             const VarId expanded = writer.WriteExpandSX(ssa::SsaType::I32, ssa::SsaType::I8, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 4);
    //             break;
    //         }
    //         case Opcode::ExpandSX18:
    //         {
    //             // Pop off 1 byte from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 1, ssa::SsaType::Bytes);
    //             // Sign Extend to 8 bytes.
    //             const VarId expanded = writer.WriteExpandSX(ssa::SsaType::I64, ssa::SsaType::I8, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 8);
    //             break;
    //         }
    //         case Opcode::ExpandSX24:
    //         {
    //             // Pop off 2 bytes from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 2, ssa::SsaType::Bytes);
    //             // Sign Extend to 4 bytes.
    //             const VarId expanded = writer.WriteExpandSX(ssa::SsaType::I32, ssa::SsaType::I16, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 4);
    //             break;
    //         }
    //         case Opcode::ExpandSX28:
    //         {
    //             // Pop off 2 bytes from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 2, ssa::SsaType::Bytes);
    //             // Sign Extend to 8 bytes.
    //             const VarId expanded = writer.WriteExpandSX(ssa::SsaType::I64, ssa::SsaType::I16, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 8);
    //             break;
    //         }
    //         case Opcode::ExpandSX48:
    //         {
    //             // Pop off 4 bytes from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 4, ssa::SsaType::Bytes);
    //             // Sign Extend to 8 bytes.
    //             const VarId expanded = writer.WriteExpandSX(ssa::SsaType::I64, ssa::SsaType::I32, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 8);
    //             break;
    //         }
    //         case Opcode::ExpandZX12:
    //         {
    //             // Pop off 1 byte from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 1, ssa::SsaType::Bytes);
    //             // Zero Extend to 2 bytes.
    //             const VarId expanded = writer.WriteExpandZX(ssa::SsaType::U16, ssa::SsaType::U8, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 2);
    //             break;
    //         }
    //         case Opcode::ExpandZX14:
    //         {
    //             // Pop off 1 byte from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 1, ssa::SsaType::Bytes);
    //             // Zero Extend to 4 bytes.
    //             const VarId expanded = writer.WriteExpandZX(ssa::SsaType::U32, ssa::SsaType::U8, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 4);
    //             break;
    //         }
    //         case Opcode::ExpandZX18:
    //         {
    //             // Pop off 1 byte from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 1, ssa::SsaType::Bytes);
    //             // Zero Extend to 8 bytes.
    //             const VarId expanded = writer.WriteExpandZX(ssa::SsaType::U64, ssa::SsaType::U8, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 8);
    //             break;
    //         }
    //         case Opcode::ExpandZX24:
    //         {
    //             // Pop off 2 bytes from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 2, ssa::SsaType::Bytes);
    //             // Zero Extend to 4 bytes.
    //             const VarId expanded = writer.WriteExpandZX(ssa::SsaType::U32, ssa::SsaType::U16, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 4);
    //             break;
    //         }
    //         case Opcode::ExpandZX28:
    //         {
    //             // Pop off 2 bytes from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 2, ssa::SsaType::Bytes);
    //             // Zero Extend to 8 bytes.
    //             const VarId expanded = writer.WriteExpandZX(ssa::SsaType::U64, ssa::SsaType::U16, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 8);
    //             break;
    //         }
    //         case Opcode::ExpandZX48:
    //         {
    //             // Pop off 4 bytes from the stack.
    //             const VarId expandTarget = PopRaw(writer, frameTracker, 4, ssa::SsaType::Bytes);
    //             // Zero Extend to 8 bytes.
    //             const VarId expanded = writer.WriteExpandZX(ssa::SsaType::U64, ssa::SsaType::U16, expandTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 8);
    //             break;
    //         }
    //         case Opcode::Trunc84:
    //         {
    //             // Pop off 8 bytes from the stack.
    //             const VarId truncTarget = PopRaw(writer, frameTracker, 8, ssa::SsaType::Bytes);
    //             // Truncate to 4 bytes.
    //             const VarId expanded = writer.WriteTrunc(ssa::SsaType::U32, ssa::SsaType::U64, truncTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 4);
    //             break;
    //         }
    //         case Opcode::Trunc82:
    //         {
    //             // Pop off 8 bytes from the stack.
    //             const VarId truncTarget = PopRaw(writer, frameTracker, 8, ssa::SsaType::Bytes);
    //             // Truncate to 2 bytes.
    //             const VarId expanded = writer.WriteTrunc(ssa::SsaType::U16, ssa::SsaType::U64, truncTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 2);
    //             break;
    //         }
    //         case Opcode::Trunc81:
    //         {
    //             // Pop off 8 bytes from the stack.
    //             const VarId truncTarget = PopRaw(writer, frameTracker, 8, ssa::SsaType::Bytes);
    //             // Truncate to 1 byte.
    //             const VarId expanded = writer.WriteTrunc(ssa::SsaType::U8, ssa::SsaType::U64, truncTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 1);
    //             break;
    //         }
    //         case Opcode::Trunc42:
    //         {
    //             // Pop off 4 bytes from the stack.
    //             const VarId truncTarget = PopRaw(writer, frameTracker, 4, ssa::SsaType::Bytes);
    //             // Truncate to 2 bytes.
    //             const VarId expanded = writer.WriteTrunc(ssa::SsaType::U16, ssa::SsaType::U32, truncTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 2);
    //             break;
    //         }
    //         case Opcode::Trunc41:
    //         {
    //             // Pop off 4 bytes from the stack.
    //             const VarId truncTarget = PopRaw(writer, frameTracker, 4, ssa::SsaType::Bytes);
    //             // Truncate to 1 byte.
    //             const VarId expanded = writer.WriteTrunc(ssa::SsaType::U8, ssa::SsaType::U32, truncTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 1);
    //             break;
    //         }
    //         case Opcode::Trunc21:
    //         {
    //             // Pop off 2 bytes from the stack.
    //             const VarId truncTarget = PopRaw(writer, frameTracker, 2, ssa::SsaType::Bytes);
    //             // Truncate to 1 byte.
    //             const VarId expanded = writer.WriteTrunc(ssa::SsaType::U8, ssa::SsaType::U16, truncTarget);
    //             // Push onto stack.
    //             frameTracker.PushFrame(expanded, 1);
    //             break;
    //         }
    //         case Opcode::Const0:
    //         {
    //             // The constant to push onto the stack.
    //             constexpr u32 value = 0;
    //             // Assign the variable.
    //             const VarId constant = writer.WriteAssignImmediate(ssa::SsaType::U32, &value, sizeof(value));
    //             // Push onto the stack.
    //             frameTracker.PushFrame(constant, 4);
    //             break;
    //         }
    //         case Opcode::Const1:
    //         {
    //             // The constant to push onto the stack.
    //             constexpr u32 value = 1;
    //             // Assign the variable.
    //             const VarId constant = writer.WriteAssignImmediate(ssa::SsaType::U32, &value, sizeof(value));
    //             // Push onto the stack.
    //             frameTracker.PushFrame(constant, 4);
    //             break;
    //         }
    //         case Opcode::Const2:
    //         {
    //             // The constant to push onto the stack.
    //             constexpr u32 value = 2;
    //             // Assign the variable.
    //             const VarId constant = writer.WriteAssignImmediate(ssa::SsaType::U32, &value, sizeof(value));
    //             // Push onto the stack.
    //             frameTracker.PushFrame(constant, 4);
    //             break;
    //         }
    //         case Opcode::Const3:
    //         {
    //             // The constant to push onto the stack.
    //             constexpr u32 value = 3;
    //             // Assign the variable.
    //             const VarId constant = writer.WriteAssignImmediate(ssa::SsaType::U32, &value, sizeof(value));
    //             // Push onto the stack.
    //             frameTracker.PushFrame(constant, 4);
    //             break;
    //         }
    //         case Opcode::Const4:
    //         {
    //             // The constant to push onto the stack.
    //             constexpr u32 value = 4;
    //             // Assign the variable.
    //             const VarId constant = writer.WriteAssignImmediate(ssa::SsaType::U32, &value, sizeof(value));
    //             // Push onto the stack.
    //             frameTracker.PushFrame(constant, 4);
    //             break;
    //         }
    //         case Opcode::ConstFF:
    //         {
    //             // The constant to push onto the stack.
    //             constexpr u32 value = 0xFFFFFFFF;
    //             // Assign the variable.
    //             const VarId constant = writer.WriteAssignImmediate(ssa::SsaType::U32, &value, sizeof(value));
    //             // Push onto the stack.
    //             frameTracker.PushFrame(constant, 4);
    //             break;
    //         }
    //         case Opcode::Const7F:
    //         {
    //             // The constant to push onto the stack.
    //             constexpr u32 value = 0x7FFFFFFF;
    //             // Assign the variable.
    //             const VarId constant = writer.WriteAssignImmediate(ssa::SsaType::U32, &value, sizeof(value));
    //             // Push onto the stack.
    //             frameTracker.PushFrame(constant, 4);
    //             break;
    //         }
    //         case Opcode::ConstN:
    //         {
    //             // The constant to push onto the stack.
    //             const u32 value = *reinterpret_cast<const u32*>(codePtr);
    //             codePtr += 4;
    //
    //             // Assign the variable.
    //             const VarId constant = writer.WriteAssignImmediate(ssa::SsaType::U32, &value, sizeof(value));
    //             // Push onto the stack.
    //             frameTracker.PushFrame(constant, 4);
    //             break;
    //         }
    //         case Opcode::AddI32:
    //         {
    //             // Pop 4 bytes from the stack into register B.
    //             const VarId regB = PopRaw(writer, frameTracker, 4, ssa::SsaType::U32);
    //             // Pop 4 bytes from the stack into register A.
    //             const VarId regA = PopRaw(writer, frameTracker, 4, ssa::SsaType::U32);
    //             // Add B to A.
    //             const VarId res = writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Add, ssa::SsaType::U32, regA, regB);
    //             // Push result onto the stack.
    //             frameTracker.PushFrame(res, 4);
    //             break;
    //         }
    //         case Opcode::AddI64:
    //         {
    //             // Pop 8 bytes from the stack into register B.
    //             const VarId regB = PopRaw(writer, frameTracker, 8, ssa::SsaType::U64);
    //             // Pop 8 bytes from the stack into register A.
    //             const VarId regA = PopRaw(writer, frameTracker, 8, ssa::SsaType::U64);
    //             // Add B to A.
    //             const VarId res = writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Add, ssa::SsaType::U64, regA, regB);
    //             // Push result onto the stack.
    //             frameTracker.PushFrame(res, 8);
    //             break;
    //         }
    //         case Opcode::SubI32:
    //         {
    //             // Pop 4 bytes from the stack into register B.
    //             const VarId regB = PopRaw(writer, frameTracker, 4, ssa::SsaType::U32);
    //             // Pop 4 bytes from the stack into register A.
    //             const VarId regA = PopRaw(writer, frameTracker, 4, ssa::SsaType::U32);
    //             // Subtract B from A.
    //             const VarId res = writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Sub, ssa::SsaType::U32, regA, regB);
    //             // Push result onto the stack.
    //             frameTracker.PushFrame(res, 4);
    //             break;
    //         }
    //         case Opcode::SubI64:
    //         {
    //             // Pop 8 bytes from the stack into register B.
    //             const VarId regB = PopRaw(writer, frameTracker, 8, ssa::SsaType::U64);
    //             // Pop 8 bytes from the stack into register A.
    //             const VarId regA = PopRaw(writer, frameTracker, 8, ssa::SsaType::U64);
    //             // Subtract B from A.
    //             const VarId res = writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Sub, ssa::SsaType::U64, regA, regB);
    //             // Push result onto the stack.
    //             frameTracker.PushFrame(res, 8);
    //             break;
    //         }
    //         case Opcode::MulI32:
    //         {
    //             // Pop 4 bytes from the stack into register B.
    //             const VarId regB = PopRaw(writer, frameTracker, 4, ssa::SsaType::U32);
    //             // Pop 4 bytes from the stack into register A.
    //             const VarId regA = PopRaw(writer, frameTracker, 4, ssa::SsaType::U32);
    //             // Multiply A by B.
    //             const VarId res = writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Mul, ssa::SsaType::U32, regA, regB);
    //             // Push result onto the stack.
    //             frameTracker.PushFrame(res, 4);
    //             break;
    //         }
    //         case Opcode::MulI64:
    //         {
    //             // Pop 8 bytes from the stack into register B.
    //             const VarId regB = PopRaw(writer, frameTracker, 8, ssa::SsaType::U64);
    //             // Pop 8 bytes from the stack into register A.
    //             const VarId regA = PopRaw(writer, frameTracker, 8, ssa::SsaType::U64);
    //             // Multiply A by B.
    //             const VarId res = writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Mul, ssa::SsaType::U64, regA, regB);
    //             // Push result onto the stack.
    //             frameTracker.PushFrame(res, 8);
    //             break;
    //         }
    //         case Opcode::DivI32:
    //         {
    //             // Pop 4 bytes from the stack into register B.
    //             const VarId regB = PopRaw(writer, frameTracker, 4, ssa::SsaType::U32);
    //             // Pop 4 bytes from the stack into register A.
    //             const VarId regA = PopRaw(writer, frameTracker, 4, ssa::SsaType::U32);
    //             // Divide A by B.
    //             const VarId quotient = writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Div, ssa::SsaType::U32, regA, regB);
    //             // Modulo A by B.
    //             const VarId remainder = writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Rem, ssa::SsaType::U32, regA, regB);
    //             // Push the quotient onto the stack.
    //             frameTracker.PushFrame(quotient, 4);
    //             // Push the remainder onto the stack.
    //             frameTracker.PushFrame(remainder, 4);
    //             break;
    //         }
    //         case Opcode::DivI64:
    //         {
    //             // Pop 8 bytes from the stack into register B.
    //             const VarId regB = PopRaw(writer, frameTracker, 8, ssa::SsaType::U64);
    //             // Pop 8 bytes from the stack into register A.
    //             const VarId regA = PopRaw(writer, frameTracker, 8, ssa::SsaType::U64);
    //             // Divide A by B.
    //             const VarId quotient = writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Div, ssa::SsaType::U64, regA, regB);
    //             // Modulo A by B.
    //             const VarId remainder = writer.WriteBinOpVtoV(ssa::SsaBinaryOperation::Rem, ssa::SsaType::U64, regA, regB);
    //             // Push the quotient onto the stack.
    //             frameTracker.PushFrame(quotient, 8);
    //             // Push the remainder onto the stack.
    //             frameTracker.PushFrame(remainder, 8);
    //             break;
    //         }
    //         case Opcode::Call:
    //         {
    //             break;
    //         }
    //         case Opcode::CallExt:
    //         {
    //             break;
    //         }
    //         case Opcode::CallInd:
    //         {
    //             break;
    //         }
    //         default:
    //             break;
    //     }
    // }

    return ::std::move(visitor.Writer());
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
        const VarId newVar = writer.WriteAssignVariable(ssaType, frame.Var);
        return newVar;
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
