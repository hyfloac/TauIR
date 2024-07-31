#pragma once

#include "TauIR/Function.hpp"
#include "SsaTypes.hpp"
#include "SsaVisitor.hpp"

namespace tau::ir::ssa {

class SsaReturnTypeAnalysisAttachment final : public FunctionAttachment
{
    DEFAULT_CONSTRUCT_PU(SsaReturnTypeAnalysisAttachment);
    DEFAULT_DESTRUCT(SsaReturnTypeAnalysisAttachment);
    DELETE_CM(SsaReturnTypeAnalysisAttachment);
    RTT_IMPL(SsaReturnTypeAnalysisAttachment, FunctionAttachment);
public:
    SsaReturnTypeAnalysisAttachment(const SsaCustomType& returnType) noexcept
        : m_ReturnType(returnType)
    { }

    [[nodiscard]] const SsaCustomType& ReturnType() const noexcept { return m_ReturnType; }
private:
    SsaCustomType m_ReturnType;
};

// ReSharper disable CppHidingFunction
class SsaReturnTypeAnalysisVisitor final : public SsaVisitor<SsaReturnTypeAnalysisVisitor>
{
    DEFAULT_DESTRUCT(SsaReturnTypeAnalysisVisitor);
    DELETE_CM(SsaReturnTypeAnalysisVisitor);
public:
    SsaReturnTypeAnalysisVisitor(const SsaCustomTypeRegistry& registry) noexcept
        : SsaVisitor(registry)
        , m_ReturnType()
    { }

    [[nodiscard]] const SsaCustomType& ReturnType() const noexcept { return m_ReturnType; }

    void UpdateAttachment(Function* const function) noexcept
    {
        if(function->FindAttachment<SsaReturnTypeAnalysisAttachment>())
        {
            function->RemoveAttachment<SsaReturnTypeAnalysisAttachment>();
        }

        function->Attach<SsaReturnTypeAnalysisAttachment>(m_ReturnType);
    }
public:
    bool VisitRet(const SsaCustomType returnType, const VarId var) noexcept
    {
        m_ReturnType = returnType;
        return true;
    }
private:
    SsaCustomType m_ReturnType;
};

}
