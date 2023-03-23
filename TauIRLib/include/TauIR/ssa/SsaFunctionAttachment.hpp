#pragma once

#include "TauIR/Function.hpp"
#include "TauIR/ssa/SsaWriter.hpp"

namespace tau::ir::ssa {

class SsaFunctionAttachment final : public FunctionAttachment
{
    DEFAULT_CONSTRUCT_PU(SsaFunctionAttachment);
    DELETE_CM(SsaFunctionAttachment);
    DEFAULT_DESTRUCT(SsaFunctionAttachment);
    RTT_IMPL(SsaFunctionAttachment, FunctionAttachment);
public:
    SsaFunctionAttachment(SsaWriter&& writer) noexcept
        : m_Writer(::std::move(writer))
    { }

    [[nodiscard]] const SsaWriter& Writer() const noexcept { return m_Writer; }
    [[nodiscard]]       SsaWriter& Writer()       noexcept { return m_Writer; }
private:
    SsaWriter m_Writer;
};

}
