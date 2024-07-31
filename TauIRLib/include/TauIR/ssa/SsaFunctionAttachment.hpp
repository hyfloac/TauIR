#pragma once

#include "TauIR/Function.hpp"
#include "TauIR/ssa/SsaWriter.hpp"

namespace tau::ir::ssa {

class SsaWriterFunctionAttachment final : public FunctionAttachment
{
    DEFAULT_CONSTRUCT_PU(SsaWriterFunctionAttachment);
    DEFAULT_DESTRUCT(SsaWriterFunctionAttachment);
    DELETE_CM(SsaWriterFunctionAttachment);
    RTT_IMPL(SsaWriterFunctionAttachment, FunctionAttachment);
public:
    SsaWriterFunctionAttachment(SsaWriter&& writer) noexcept
        : m_Writer(::std::move(writer))
    { }

    [[nodiscard]] const SsaWriter& Writer() const noexcept { return m_Writer; }
    [[nodiscard]]       SsaWriter& Writer()       noexcept { return m_Writer; }
private:
    SsaWriter m_Writer;
};

class SsaFunctionAttachment final : public FunctionAttachment
{
    DEFAULT_CONSTRUCT_PU(SsaFunctionAttachment);
    DEFAULT_DESTRUCT(SsaFunctionAttachment);
    DELETE_CM(SsaFunctionAttachment);
    RTT_IMPL(SsaFunctionAttachment, FunctionAttachment);
public:
    SsaFunctionAttachment(const u8* const buffer, const uSys bufferSize, const VarId maxVarId, const ::std::vector<SsaCustomType>& varTypeMap) noexcept
        : m_Buffer(bufferSize)
        , m_MaxVarId(maxVarId)
        , m_VarTypeMap(varTypeMap)
    {
        m_Buffer.MemCpyAll(buffer);
    }

    [[nodiscard]] const DynArray<u8>& Buffer() const noexcept { return m_Buffer; }
    [[nodiscard]] VarId MaxVarId() const noexcept { return m_MaxVarId; }
    [[nodiscard]] const ::std::vector<SsaCustomType>& VarTypeMap() const noexcept { return m_VarTypeMap; }

private:
    DynArray<u8> m_Buffer;
    VarId m_MaxVarId;
    ::std::vector<SsaCustomType> m_VarTypeMap;
};

}
