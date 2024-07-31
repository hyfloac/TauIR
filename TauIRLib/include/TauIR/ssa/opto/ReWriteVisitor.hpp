#pragma once
#include "TauIR/ssa/SsaVisitor.hpp"

namespace tau::ir::ssa::opto {

// ReSharper disable CppHidingFunction
class ReWriteVisitor final : public SsaVisitor<ReWriteVisitor>
{
    DEFAULT_DESTRUCT(ReWriteVisitor);
    DELETE_CM(ReWriteVisitor);
public:
    ReWriteVisitor(const SsaCustomTypeRegistry& registry, SsaWriter& writer, ::std::vector<VarId>& newVarMap, const VarId baseArg, const u32 argCount, const VarId retVar) noexcept
        : SsaVisitor(registry)
        , m_Writer(&writer)
        , m_NewVarMap(&newVarMap)
        , m_BaseArg(baseArg)
        , m_ArgCount(argCount)
        , m_RetVar(retVar)
        , m_OldVarMapSize(0)
    { }

    [[nodiscard]] SsaWriter& Writer() noexcept { return *m_Writer; }
    [[nodiscard]] const SsaWriter& Writer() const noexcept { return *m_Writer; }

    void UpdateAttachment(Function* const function) noexcept
    {
        {
            SsaWriterFunctionAttachment* const ssaWriterAttachment = function->FindAttachment<SsaWriterFunctionAttachment>();

            if(ssaWriterAttachment)
            {
                ssaWriterAttachment->Writer() = ::std::move(Writer());
            }
        }

        {
            const SsaFunctionAttachment* const ssaAttachment = function->FindAttachment<SsaFunctionAttachment>();

            if(ssaAttachment)
            {
                function->RemoveAttachment<SsaFunctionAttachment>();
                function->Attach<SsaFunctionAttachment>(Writer().Buffer(), Writer().Size(), Writer().IdIndex(), Writer().VarTypeMap());
            }
        }
    }

private:
    [[nodiscard]] auto& NewVarMap() noexcept { return *m_NewVarMap; }
public:
    bool PreTraversal(const u8* const codePtr, const uSys size, const VarId maxId) noexcept
    {
        m_OldVarMapSize = NewVarMap().size() - 1;
        for(uSys i = 0; i < maxId; ++i)
        {
            NewVarMap().push_back(0);
        }
        return true;
    }

    bool VisitNop() noexcept
    {
        Writer().WriteNop();
        return true;
    }

    bool VisitLabel(const VarId label) noexcept
    {
        NewVarMap()[label + m_OldVarMapSize] = Writer().WriteLabel();
        return true;
    }

    bool VisitAssignImmediate(const VarId newVar, const SsaCustomType type, const void* const value, const uSys size) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteAssignImmediate(type, value, size);
        return true;
    }

    bool VisitAssignVar(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteAssignVariable(type, TransformVar(var));
        return true;
    }

    bool VisitExpandSX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteExpandSX(newType, oldType, TransformVar(var));
        return true;
    }

    bool VisitExpandZX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteExpandZX(newType, oldType, TransformVar(var));
        return true;
    }

    bool VisitTrunc(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteTrunc(newType, oldType, TransformVar(var));
        return true;
    }

    bool VisitRCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteRCast(newType, oldType, TransformVar(var));
        return true;
    }

    bool VisitBCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteBCast(newType, oldType, TransformVar(var));
        return true;
    }

    bool VisitLoad(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteLoad(type, TransformVar(var));
        return true;
    }

    bool VisitStoreV(const SsaCustomType type, const VarId destination, const VarId source) noexcept
    {
        Writer().WriteStoreV(type, TransformVar(destination), TransformVar(source));
        return true;
    }

    bool VisitStoreI(const SsaCustomType type, const VarId destination, const void* const value, const uSys size) noexcept
    {
        Writer().WriteStoreI(type, TransformVar(destination), value, size);
        return true;
    }

    bool VisitComputePtr(const VarId newVar, const VarId base, const VarId index, const i8 multiplier, const i16 offset) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteComputePtr(TransformVar(base), TransformVar(index), multiplier, offset);
        return true;
    }

    bool VisitBinOpVToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const VarId b) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteBinOpVtoV(operation, type, TransformVar(a), TransformVar(b));
        return true;
    }

    bool VisitBinOpVToI(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteBinOpVtoI(operation, type, a, aSize, TransformVar(b));
        return true;
    }

    bool VisitBinOpIToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteBinOpItoV(operation, type, TransformVar(a), b, bSize);
        return true;
    }

    bool VisitSplit(const VarId baseIndex, const SsaCustomType aType, const VarId a, const uSys splitCount, const SsaCustomType* const splitTypes) noexcept
    {
        const VarId computedBaseIndex = Writer().WriteSplit(aType, TransformVar(a), static_cast<u32>(splitCount), splitTypes);
        for(uSys i = 0; i < splitCount; ++i)
        {
            NewVarMap()[baseIndex + i] = static_cast<VarId>(computedBaseIndex + i);
        }
        return true;
    }

    bool VisitJoin(const VarId newVar, const SsaCustomType newType, const uSys joinCount, const SsaCustomType* const joinTypes, const VarId* const joinVars) noexcept
    {
        DynArray<VarId> newJoinVars(joinCount);
        for(uSys i = 0; i < joinCount; ++i)
        {
            newJoinVars[i] = TransformVar(joinVars[i]);
        }
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteJoin(newType, static_cast<u32>(joinCount), joinTypes, newJoinVars);
        return true;
    }

    bool VisitCompVToV(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const VarId a, const VarId b) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteCompVtoV(operation, type, TransformVar(a), TransformVar(b));
        return true;
    }

    bool VisitCompVToI(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteCompVtoI(operation, type, a, aSize, TransformVar(b));
        return true;
    }

    bool VisitCompIToV(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteCompItoV(operation, type, TransformVar(a), b, bSize);
        return true;
    }

    bool VisitBranch(const VarId label) noexcept
    {
        Writer().WriteBranch(TransformVar(label));
        return true;
    }

    bool VisitBranchCond(const VarId labelTrue, const VarId labelFalse, const VarId conditionVar) noexcept
    {
        Writer().WriteBranchCond(TransformVar(labelTrue), TransformVar(labelFalse), TransformVar(conditionVar));
        return true;
    }

    bool VisitCall(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteCall(functionIndex, baseIndex, parameterCount);
        return true;
    }

    bool VisitCallExt(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount, const u16 moduleIndex) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteCallExt(functionIndex, baseIndex, parameterCount, moduleIndex);
        return true;
    }

    bool VisitCallInd(const VarId newVar, const VarId functionPointer, const VarId baseIndex, const u32 parameterCount) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteCallInd(TransformVar(functionPointer), TransformVar(baseIndex), parameterCount);
        return true;
    }

    bool VisitCallIndExt(const VarId newVar, const VarId functionPointer, const VarId baseIndex, const u32 parameterCount, const VarId modulePointer) noexcept
    {
        NewVarMap()[newVar + m_OldVarMapSize] = Writer().WriteCallIndExt(TransformVar(functionPointer), TransformVar(baseIndex), parameterCount, TransformVar(modulePointer));
        return true;
    }

    bool VisitRet(const SsaCustomType returnType, const VarId var) noexcept
    {
        // Writer().WriteRet(returnType, TransformVar(var));
        NewVarMap()[m_RetVar] = TransformVar(var);
        return true;
    }
private:
    [[nodiscard]] VarId TransformVar(const VarId var)
    {
        if((var & 0x80000000) == 0x80000000)
        {
            return NewVarMap()[m_BaseArg + (var & 0x7FFFFFFF)];
        }

        return NewVarMap()[var + m_OldVarMapSize];
    }
private:
    SsaWriter* m_Writer;
    ::std::vector<VarId>* m_NewVarMap;
    VarId m_BaseArg;
    [[maybe_unused]] u32 m_ArgCount;
    VarId m_RetVar;
    uSys m_OldVarMapSize;
};

}
