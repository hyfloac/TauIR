#pragma once

#include <Objects.hpp>
#include <NumTypes.hpp>
#include <deque>
#include <vector>

#include "SsaTypes.hpp"
#include "SsaOpcodes.hpp"

namespace tau::ir::ssa {

class SsaWriter
{
    DELETE_COPY(SsaWriter);
public:
    using VarId = u32;
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
    void WriteStore(SsaCustomType type, VarId destPtr, VarId sourceVar) noexcept;
    VarId WriteComputePtr(VarId base, VarId index, i8 multiplier, i16 offset) noexcept;
    VarId WriteBinOpVtoV(SsaBinaryOperation operation, SsaCustomType type, VarId a, VarId b);
    VarId WriteBinOpVtoI(SsaBinaryOperation operation, SsaCustomType type, const void* aValue, uSys aSize, VarId b);
    VarId WriteBinOpItoV(SsaBinaryOperation operation, SsaCustomType type, VarId a, const void* bValue, uSys bSize);
    VarId WriteSplit(SsaCustomType aType, VarId a, u32 n, const SsaCustomType* t) noexcept;
    VarId WriteJoin(SsaCustomType outType, u32 n, const SsaCustomType* t, const VarId* v) noexcept;

    [[nodiscard]] SsaCustomType GetVarType(const VarId var) const noexcept { return m_VarTypeMap[var]; }
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
    ::std::vector<SsaCustomType> m_VarTypeMap;
};

class SsaFrameTracker final
{
    DELETE_COPY(SsaFrameTracker);
public:
    using VarId = SsaWriter::VarId;
    struct FrameInfo final
    {
        DEFAULT_DESTRUCT(FrameInfo);
        DEFAULT_CM_PU(FrameInfo);

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
    FrameInfo CheckFrame() const;

    void SetLocal(VarId var, uSys local);
    VarId GetLocal(uSys local) const;
private:
    ::std::deque<FrameInfo> m_Frame;
    VarId* m_Locals;
#ifdef _DEBUG
    uSys m_LocalCount;
#endif
};

}