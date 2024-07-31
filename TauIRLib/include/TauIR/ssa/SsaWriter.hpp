#pragma once

#include <Objects.hpp>
#include <NumTypes.hpp>
#include <deque>
#include <vector>

#include "SsaTypes.hpp"
#include "SsaOpcodes.hpp"
#include "TauIR/Opcodes.hpp"

namespace tau::ir::ssa {

class SsaWriter
{
    DELETE_COPY(SsaWriter);
public:
    using VarTypeMapType = ::std::vector<SsaCustomType>;
public:
    SsaWriter(uSys initialBufferSize = 64) noexcept;
    ~SsaWriter() noexcept;

    SsaWriter(SsaWriter&& move) noexcept;
    SsaWriter& operator=(SsaWriter&& move) noexcept;

    void WriteNop() noexcept;
    VarId WriteLabel() noexcept;
    VarId WriteAssignImmediate(SsaCustomType varType, const void* value, uSys valueSize) noexcept;
    VarId WriteAssignVariable(SsaCustomType varType, VarId var) noexcept;
    VarId WriteExpandSX(SsaCustomType newType, SsaCustomType oldType, VarId var) noexcept;
    VarId WriteExpandZX(SsaCustomType newType, SsaCustomType oldType, VarId var) noexcept;
    VarId WriteTrunc(SsaCustomType newType, SsaCustomType oldType, VarId var) noexcept;
    VarId WriteRCast(SsaCustomType newType, SsaCustomType oldType, VarId var) noexcept;
    VarId WriteBCast(SsaCustomType newType, SsaCustomType oldType, VarId var) noexcept;
    VarId WriteLoad(SsaCustomType type, VarId var) noexcept;
    void WriteStoreV(SsaCustomType type, VarId destPtr, VarId sourceVar) noexcept;
    void WriteStoreI(SsaCustomType type, VarId destPtr, const void* value, uSys size) noexcept;
    VarId WriteComputePtr(VarId base, VarId index, i8 multiplier, i16 offset) noexcept;
    VarId WriteBinOpVtoV(SsaBinaryOperation operation, SsaCustomType type, VarId a, VarId b) noexcept;
    VarId WriteBinOpVtoI(SsaBinaryOperation operation, SsaCustomType type, const void* aValue, uSys aSize, VarId b) noexcept;
    VarId WriteBinOpItoV(SsaBinaryOperation operation, SsaCustomType type, VarId a, const void* bValue, uSys bSize) noexcept;
    VarId WriteSplit(SsaCustomType aType, VarId a, u32 n, const SsaCustomType* t) noexcept;
    VarId WriteJoin(SsaCustomType outType, u32 n, const SsaCustomType* t, const VarId* v) noexcept;
    VarId WriteCompVtoV(CompareCondition condition, SsaCustomType type, VarId a, VarId b) noexcept;
    VarId WriteCompVtoI(CompareCondition condition, SsaCustomType type, const void* aValue, uSys aSize, VarId b) noexcept;
    VarId WriteCompItoV(CompareCondition condition, SsaCustomType type, VarId a, const void* bValue, uSys bSize) noexcept;
    void WriteBranch(VarId label) noexcept;
    void WriteBranchCond(VarId labelTrue, VarId labelFalse, VarId conditionVar) noexcept;
    VarId WriteCall(u32 function, u32 baseIndex, u32 parameterCount) noexcept;
    VarId WriteCallExt(u32 function, u32 baseIndex, u32 parameterCount, u16 module) noexcept;
    VarId WriteCallInd(VarId functionPointer, VarId baseIndex, u32 parameterCount) noexcept;
    VarId WriteCallIndExt(VarId functionPointer, VarId baseIndex, u32 parameterCount, VarId modulePointer) noexcept;
    void WriteRet(SsaCustomType returnType, VarId var) noexcept;

    [[nodiscard]] SsaCustomType GetVarType(const VarId var) const noexcept { return m_VarTypeMap[var]; }

    [[nodiscard]] const u8* Buffer() const noexcept { return m_Buffer; }
    [[nodiscard]] uSys Size() const noexcept { return m_WriteIndex; }
    [[nodiscard]] VarId IdIndex() const noexcept { return m_IdIndex; }
    [[nodiscard]] const VarTypeMapType& VarTypeMap() const noexcept { return m_VarTypeMap; }
private:
    void EnsureSize(uSys additionalSize) noexcept;
    
    void WriteRaw(const void* value, uSys size) noexcept;

    template<typename T>
    void WriteT(const T value) noexcept
    {
        WriteRaw(&value, sizeof(T));
    }

    void WriteOpcode(SsaOpcode opcode) noexcept;
    void WriteType(SsaCustomType type) noexcept;
private:
    u8* m_Buffer;
    uSys m_BufferSize;
    uSys m_WriteIndex;
    VarId m_IdIndex;
    VarTypeMapType m_VarTypeMap;
};

/**
 * \brief Tracks variables as they're pushed and popped from the stack.
 */
class SsaFrameTracker final
{
    DELETE_COPY(SsaFrameTracker);
public:
    struct FrameInfo final
    {
        DEFAULT_CONSTRUCT_PU(FrameInfo);
        DEFAULT_DESTRUCT(FrameInfo);
        DEFAULT_CM_PU(FrameInfo);
    public:
        VarId Var;
        uSys Size;

        FrameInfo(const VarId var, const uSys size) noexcept
            : Var(var)
            , Size(size)
        { }
    };
public:
    SsaFrameTracker(uSys localCount) noexcept;
    ~SsaFrameTracker() noexcept;

    SsaFrameTracker(SsaFrameTracker&& move) noexcept;
    SsaFrameTracker& operator=(SsaFrameTracker&& move) noexcept;

    void PushFrame(VarId var, uSys size);
    FrameInfo PopFrame(uSys size);
    [[nodiscard]] FrameInfo CheckFrame() const;

    void SetLocal(VarId var, uSys local);
    [[nodiscard]] VarId GetLocal(uSys local) const;

    void SetArgument(VarId var, uSys arg);
    [[nodiscard]] VarId GetArgument(uSys arg) const;
private:
    ::std::deque<FrameInfo> m_Frame;
    VarId* m_Locals;
    VarId* m_Arguments;
#ifdef _DEBUG
    uSys m_LocalCount;
#endif
};

}
