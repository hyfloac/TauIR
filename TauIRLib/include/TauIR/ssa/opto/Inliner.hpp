#pragma once
#include "TauIR/Module.hpp"
#include "TauIR/ssa/SsaVisitor.hpp"
#include "ReWriteVisitor.hpp"

namespace tau::ir::ssa::opto {


// Copy all the instructions, inline any acceptable functions, and shift the variable ID's appropriately after each inline.
// ReSharper disable CppHidingFunction
class InlinerVisitor final : public SsaVisitor<InlinerVisitor>
{
	DEFAULT_DESTRUCT(InlinerVisitor);
	DELETE_CM(InlinerVisitor);
public:
	InlinerVisitor(const SsaCustomTypeRegistry& registry, const ModuleRef& module) noexcept
		: SsaVisitor(registry)
	    , m_Module(module)
	{ }

	[[nodiscard]] const SsaWriter& Writer() const noexcept { return m_Writer; }

	void UpdateAttachment(Function* const function) noexcept
	{
	    {
	        SsaWriterFunctionAttachment* const ssaWriterAttachment = function->FindAttachment<SsaWriterFunctionAttachment>();

	        if(ssaWriterAttachment)
	        {
	            ssaWriterAttachment->Writer() = ::std::move(m_Writer);
	        }
	    }

	    {
            const SsaFunctionAttachment* const ssaAttachment = function->FindAttachment<SsaFunctionAttachment>();

            if(ssaAttachment)
            {
                function->RemoveAttachment<SsaFunctionAttachment>();
                function->Attach<SsaFunctionAttachment>(m_Writer.Buffer(), m_Writer.Size(), m_Writer.IdIndex(), m_Writer.VarTypeMap());
            }
	    }
	}
public:
    bool PreTraversal(const u8* const codePtr, const uSys size, const VarId maxId) noexcept
    {
        m_NewVarMap.resize(maxId + 1);
        return true;
    }

    bool VisitNop() noexcept
    {
        m_Writer.WriteNop();
        return true;
    }

    bool VisitLabel(const VarId label) noexcept
    {
        m_NewVarMap[label] = m_Writer.WriteLabel();
        return true;
    }

    bool VisitAssignImmediate(const VarId newVar, const SsaCustomType type, const void* const value, const uSys size) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteAssignImmediate(type, value, size);
        return true;
    }

    bool VisitAssignVar(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteAssignVariable(type, TransformVar(var));
        return true;
    }

    bool VisitExpandSX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteExpandSX(newType, oldType, TransformVar(var));
        return true;
    }

    bool VisitExpandZX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteExpandZX(newType, oldType, TransformVar(var));
        return true;
    }

    bool VisitTrunc(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteTrunc(newType, oldType, TransformVar(var));
        return true;
    }

    bool VisitRCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteRCast(newType, oldType, TransformVar(var));
        return true;
    }

    bool VisitBCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteBCast(newType, oldType, TransformVar(var));
        return true;
    }
                                         
    bool VisitLoad(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteLoad(type, TransformVar(var));
        return true;
    }
    
    bool VisitStoreV(const SsaCustomType type, const VarId destination, const VarId source) noexcept
    {
        m_Writer.WriteStoreV(type, TransformVar(destination), TransformVar(source));
        return true;
    }
    
    bool VisitStoreI(const SsaCustomType type, const VarId destination, const void* const value, const uSys size) noexcept
    {
        m_Writer.WriteStoreI(type, TransformVar(destination), value, size);
        return true;
    }
    
    bool VisitComputePtr(const VarId newVar, const VarId base, const VarId index, const i8 multiplier, const i16 offset) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteComputePtr(TransformVar(base), TransformVar(index), multiplier, offset);
        return true;
    }
    
    bool VisitBinOpVToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const VarId b) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteBinOpVtoV(operation, type, TransformVar(a), TransformVar(b));
        return true;
    }
    
    bool VisitBinOpVToI(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteBinOpVtoI(operation, type, a, aSize, TransformVar(b));
        return true;
    }
    
    bool VisitBinOpIToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteBinOpItoV(operation, type, TransformVar(a), b, bSize);
        return true;
    }
    
    bool VisitSplit(const VarId baseIndex, const SsaCustomType aType, const VarId a, const uSys splitCount, const SsaCustomType* const splitTypes) noexcept
    {
        const VarId computedBaseIndex = m_Writer.WriteSplit(aType, TransformVar(a), static_cast<u32>(splitCount), splitTypes);
        for(uSys i = 0; i < splitCount; ++i)
        {
            m_NewVarMap[baseIndex + i] = static_cast<VarId>(computedBaseIndex + i);
        }
        return true;
    }
    
    bool VisitJoin(const VarId newVar, const SsaCustomType newType, const uSys joinCount, const SsaCustomType* const joinTypes, const VarId* const joinVars) noexcept
    {
        DynArray<VarId> newJoinVars(joinCount);
        for(uSys i = 0; i < joinCount; ++i)
        {
            newJoinVars[i] = m_NewVarMap[joinVars[i]];
        }
        m_NewVarMap[newVar] = m_Writer.WriteJoin(newType, static_cast<u32>(joinCount), joinTypes, newJoinVars);
        return true;
    }
    
    bool VisitCompVToV(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const VarId a, const VarId b) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteCompVtoV(operation, type, TransformVar(a), TransformVar(b));
        return true;
    }
    
    bool VisitCompVToI(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteCompVtoI(operation, type, a, aSize, TransformVar(b));
        return true;
    }
    
    bool VisitCompIToV(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
    {
        m_NewVarMap[newVar] = m_Writer.WriteCompItoV(operation, type, TransformVar(a), b, bSize);
        return true;
    }
    
    bool VisitBranch(const VarId label) noexcept
    {
        m_Writer.WriteBranch(TransformVar(label));
        return true;
    }
    
    bool VisitBranchCond(const VarId labelTrue, const VarId labelFalse, const VarId conditionVar) noexcept
    {
        m_Writer.WriteBranchCond(TransformVar(labelTrue), TransformVar(labelFalse), TransformVar(conditionVar));
        return true;
    }
     
    bool VisitCall(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount) noexcept
    {
        const Function* function = m_Module->Functions()[functionIndex];
        if(!ShouldInlineFunction(function))
        {
            m_NewVarMap[newVar] = m_Writer.WriteCall(functionIndex, baseIndex, parameterCount);
        }
        else
        {
            InlineFunction(function, baseIndex, parameterCount, newVar);
        }
        
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
    
    bool VisitRet(const SsaCustomType returnType, const VarId var) noexcept
    {
        m_Writer.WriteRet(returnType, TransformVar(var));
        return true;
    }
private:
    [[nodiscard]] VarId TransformVar(const VarId var)
    {
        if((var & 0x80000000) == 0x80000000)
        {
            return var;
        }

        return m_NewVarMap[var];
    }

    [[nodiscard]] bool ShouldInlineFunction(const Function* const function) const noexcept
    {
        if(function->Flags().InlineControl == InlineControl::NoInline)
        {
            return false;
        }

        if(function->Flags().OptimizationControl == OptimizationControl::NoOptimize)
        {
            return false;
        }

        if(function->Module()->IsNative())
        {
            return false;
        }

        if(function->Flags().InlineControl == InlineControl::ForceInline)
        {
            return true;
        }

        if(function->CodeSize() <= 64)
        {
            return true;
        }

        if(function->Flags().InlineControl == InlineControl::InlineHint && function->CodeSize() <= 256)
        {
            return true;
        }

        return false;
    }

    void InlineFunction(const Function* const function, const VarId baseIndex, const u32 parameterCount, const VarId newVar) noexcept
    {
        ReWriteVisitor rewriter(Registry(), m_Writer, m_NewVarMap, baseIndex, parameterCount, newVar);
        rewriter.Traverse(function);
    }
private:
	SsaWriter m_Writer;
    ::std::vector<VarId> m_NewVarMap;
	ModuleRef m_Module;
};

}
