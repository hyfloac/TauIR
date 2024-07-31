#pragma once

#include "TauIR/Function.hpp"
#include "SsaTypes.hpp"
#include "SsaVisitor.hpp"
#include "SsaReturnTypeAnalysis.hpp"
#include <DynArray.hpp>

namespace tau::ir::ssa {

struct SsaVariableTypeAndOffset final
{
    DEFAULT_CONSTRUCT_PU(SsaVariableTypeAndOffset);
    DEFAULT_DESTRUCT(SsaVariableTypeAndOffset);
    DEFAULT_CM_PU(SsaVariableTypeAndOffset);
public:
    SsaCustomType Type;
    uSys Offset;

    SsaVariableTypeAndOffset(const SsaCustomType& type, const uSys offset) noexcept
        : Type(type)
        , Offset(offset)
    { }
};

class SsaVariableAnalysisAttachment final : public FunctionAttachment
{
    DEFAULT_CONSTRUCT_PU(SsaVariableAnalysisAttachment);
    DEFAULT_DESTRUCT(SsaVariableAnalysisAttachment);
    DELETE_CM(SsaVariableAnalysisAttachment);
    RTT_IMPL(SsaVariableAnalysisAttachment, FunctionAttachment);
public:
    SsaVariableAnalysisAttachment(const DynArray<SsaVariableTypeAndOffset>& variables) noexcept
        : m_Variables(variables)
    { }

    SsaVariableAnalysisAttachment(DynArray<SsaVariableTypeAndOffset>&& variables) noexcept
        : m_Variables(::std::move(variables))
    { }

    [[nodiscard]] const DynArray<SsaVariableTypeAndOffset>& Variables() const noexcept { return m_Variables; }
private:
    DynArray<SsaVariableTypeAndOffset> m_Variables;
};

// ReSharper disable CppHidingFunction
class SsaVariableAnalysisVisitor final : public SsaVisitor<SsaVariableAnalysisVisitor>
{
    DEFAULT_DESTRUCT(SsaVariableAnalysisVisitor);
    DELETE_CM(SsaVariableAnalysisVisitor);
public:
    SsaVariableAnalysisVisitor(const SsaCustomTypeRegistry& registry, const ModuleRef& module) noexcept
        : SsaVisitor(registry)
        , m_Module(module)
        , m_Variables()
        , m_CurrentOffset(0)
    { }

    [[nodiscard]] const DynArray<SsaVariableTypeAndOffset>& Variables() const noexcept { return m_Variables; }

    void UpdateAttachment(Function* const function) noexcept
    {
        if(function->FindAttachment<SsaVariableAnalysisAttachment>())
        {
            function->RemoveAttachment<SsaVariableAnalysisAttachment>();
        }

        function->Attach<SsaVariableAnalysisAttachment>(::std::move(m_Variables));
    }
public:
    bool PreTraversal(const u8* const codePtr, const uSys size, const VarId maxId) noexcept
    {
        m_Variables = DynArray<SsaVariableTypeAndOffset>(maxId);
        m_CurrentOffset = 0;
        return true;
    }

    bool VisitLabel(const VarId label) noexcept
    {
        m_Variables[label - 1] = SsaVariableTypeAndOffset(SsaCustomType(SsaType::Label), m_CurrentOffset);
        m_CurrentOffset += TypeValueSize(SsaType::Label);
        return true;
    }

    bool VisitAssignImmediate(const VarId newVar, const SsaCustomType type, const void* const value, const uSys size) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(type, m_CurrentOffset);
        m_CurrentOffset += size;
        return true;
    }

    bool VisitAssignVar(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(type, m_CurrentOffset);
        m_CurrentOffset += type.Size();
        return true;
    }

    bool VisitExpandSX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(newType, m_CurrentOffset);
        m_CurrentOffset += newType.Size();
        return true;
    }

    bool VisitExpandZX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(newType, m_CurrentOffset);
        m_CurrentOffset += newType.Size();
        return true;
    }

    bool VisitTrunc(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(newType, m_CurrentOffset);
        m_CurrentOffset += newType.Size();
        return true;
    }

    bool VisitRCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(newType, m_CurrentOffset);
        m_CurrentOffset += newType.Size();
        return true;
    }

    bool VisitBCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(newType, m_CurrentOffset);
        m_CurrentOffset += newType.Size();
        return true;
    }

    bool VisitLoad(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(type, m_CurrentOffset);
        m_CurrentOffset += type.Size();
        return true;
    }

    bool VisitComputePtr(const VarId newVar, const VarId base, const VarId index, const i8 multiplier, const i16 offset) noexcept
    {
        const SsaCustomType type(SetPointer(SsaType::Void, true));
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(type, m_CurrentOffset);
        m_CurrentOffset += type.Size();
        return true;
    }

    bool VisitBinOpVToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const VarId b) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(type, m_CurrentOffset);
        m_CurrentOffset += type.Size();
        return true;
    }

    bool VisitBinOpVToI(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(type, m_CurrentOffset);
        m_CurrentOffset += type.Size();
        return true;
    }

    bool VisitBinOpIToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(type, m_CurrentOffset);
        m_CurrentOffset += type.Size();
        return true;
    }

    bool VisitSplit(const VarId baseIndex, const SsaCustomType aType, const VarId a, const uSys splitCount, const SsaCustomType* const splitTypes) noexcept
    {
        for(uSys i = 0; i < splitCount; ++i)
        {
            m_Variables[baseIndex + i - 1] = SsaVariableTypeAndOffset(splitTypes[i], m_CurrentOffset);
            m_CurrentOffset += splitTypes[i].Size();
        }
        return true;
    }

    bool VisitJoin(const VarId newVar, const SsaCustomType newType, const uSys joinCount, const SsaCustomType* const joinTypes, const VarId* const joinVars) noexcept
    {
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(newType, m_CurrentOffset);
        m_CurrentOffset += newType.Size();
        return true;
    }

    bool VisitCompVToV(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const VarId a, const VarId b) noexcept
    {
        const SsaCustomType newType(SsaType::Bool);
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(newType, m_CurrentOffset);
        m_CurrentOffset += newType.Size();
        return true;
    }

    bool VisitCompVToI(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
    {
        const SsaCustomType newType(SsaType::Bool);
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(newType, m_CurrentOffset);
        m_CurrentOffset += newType.Size();
        return true;
    }

    bool VisitCompIToV(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
    {
        const SsaCustomType newType(SsaType::Bool);
        m_Variables[newVar - 1] = SsaVariableTypeAndOffset(newType, m_CurrentOffset);
        m_CurrentOffset += newType.Size();
        return true;
    }

    bool VisitCall(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount) noexcept
    {

        const Function* function = m_Module->Functions()[functionIndex];
        function->FindAttachment<Ssa>()

        return true;
    }

    bool VisitCallExt(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount, const u16 moduleIndex) noexcept
    {
        const Function* function = m_Module->Imports()[moduleIndex].Functions()[functionIndex];
        if(!ShouldInlineFunction(function))
        {
            m_NewVarMap[newVar] = m_Writer.WriteCallExt(functionIndex, baseIndex, parameterCount, moduleIndex);
        }
        else
        {
            InlineFunction(function, baseIndex, parameterCount, newVar);
        }

        return true;
    }

    bool VisitCallInd(const VarId newVar, const VarId functionPointer, const VarId baseIndex, const u32 parameterCount) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteCallInd(TransformVar(functionPointer), TransformVar(baseIndex), parameterCount);
        return true;
    }

    bool VisitCallIndExt(const VarId newVar, const VarId functionPointer, const VarId baseIndex, const u32 parameterCount, const VarId modulePointer) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteCallIndExt(TransformVar(functionPointer), TransformVar(baseIndex), parameterCount, TransformVar(modulePointer));
        return true;
    }
private:
    ModuleRef m_Module;
    DynArray<SsaVariableTypeAndOffset> m_Variables;
    uSys m_CurrentOffset;
};

}
