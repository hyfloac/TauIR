#pragma once

#include "TauIR/ssa/SsaVisitor.hpp"
#include "TauIR/ssa/SsaWriter.hpp"

namespace tau::ir::ssa::opto {

class UsageAnalysisFunctionAttachment : public FunctionAttachment
{
    DEFAULT_CONSTRUCT_PU(UsageAnalysisFunctionAttachment);
    DEFAULT_DESTRUCT(UsageAnalysisFunctionAttachment);
    DELETE_CM(UsageAnalysisFunctionAttachment);
    RTT_IMPL(UsageAnalysisFunctionAttachment, FunctionAttachment);
public:
    using TUsageMap = ::std::unordered_multimap<VarId, VarId>;
public:
    UsageAnalysisFunctionAttachment(const TUsageMap& usageMap) noexcept
        : m_UsageMap(usageMap)
    { }

    [[nodiscard]] const TUsageMap& UsageMap() const noexcept { return m_UsageMap; }
    [[nodiscard]]       TUsageMap& UsageMap()       noexcept { return m_UsageMap; }
private:
    TUsageMap m_UsageMap;
};

class UsageAnalyzerVisitor final : public SsaVisitor<UsageAnalyzerVisitor>
{
    DEFAULT_DESTRUCT(UsageAnalyzerVisitor);
    DELETE_CM(UsageAnalyzerVisitor);
public:
    using TUsageMap = UsageAnalysisFunctionAttachment::TUsageMap;
public:
	UsageAnalyzerVisitor(const SsaCustomTypeRegistry& registry) noexcept
		: SsaVisitor(registry)
	    , m_UsageMap()
	{ }

    [[nodiscard]] const TUsageMap& UsageMap() const noexcept { return m_UsageMap; }

    void UpdateAttachment(Function* const function) noexcept
    {
        function->Attach<UsageAnalysisFunctionAttachment>(m_UsageMap);
    }
public:
    bool PreTraversal(const u8* const codePtr, const uSys size, const VarId maxId) noexcept
    {
        m_UsageMap = TUsageMap();

        return true;
    }

    bool HandleUsage(const VarId newVar, const VarId var) noexcept
    {
        if((var & 0x80000000) == 0) // var does not point to an argument
        {
            m_UsageMap.insert({ var, newVar });
        }

        return true;
    }

    bool VisitAssignVar(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
    {
        return HandleUsage(newVar, var);
    }

    bool VisitExpandSX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var)
    {
        return HandleUsage(newVar, var);
    }

    bool VisitExpandZX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var)
    {
        return HandleUsage(newVar, var);
    }

    bool VisitTrunc(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var)
    {
        return HandleUsage(newVar, var);
    }

	bool VisitRCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		return HandleUsage(newVar, var);
	}

	bool VisitBCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		return HandleUsage(newVar, var);
	}

	bool VisitLoad(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
	{
		return HandleUsage(newVar, var);
	}

	bool VisitStoreV(const SsaCustomType type, const VarId destination, const VarId source) noexcept
	{
		(void) HandleUsage(destination, destination);
		return HandleUsage(source, source);
	}

	bool VisitStoreI(const SsaCustomType type, const VarId destination, const void* const value, const uSys size) noexcept
	{
		return HandleUsage(destination, destination);
	}

	bool VisitComputePtr(const VarId newVar, const VarId base, const VarId index, const i8 multiplier, const i16 offset) noexcept
	{
		(void) HandleUsage(newVar, base);
		return HandleUsage(newVar, index);
	}

	bool VisitBinOpVToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const VarId b) noexcept
	{
		(void) HandleUsage(newVar, a);
		return HandleUsage(newVar, b);
	}

	bool VisitBinOpVToI(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
	{
		return HandleUsage(newVar, b);
	}

	bool VisitBinOpIToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
	{
		return HandleUsage(newVar, a);
	}

	bool VisitCompVToV(const VarId newVar, const CompareCondition condition, const SsaCustomType type, const VarId a, const VarId b) noexcept
	{
		(void) HandleUsage(newVar, a);
		return HandleUsage(newVar, b);
	}

	bool VisitCompVToI(const VarId newVar, const CompareCondition condition, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
	{
		return HandleUsage(newVar, b);
	}

	bool VisitCompIToV(const VarId newVar, const CompareCondition condition, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
	{
		return HandleUsage(newVar, a);
	}

	bool VisitCall(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount) noexcept
	{
		for(u32 i = 0; i < parameterCount; ++i)
		{
			(void) HandleUsage(newVar, baseIndex + i);
		}

		return HandleUsage(newVar, newVar);
	}

	bool VisitCallExt(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount, const u16 moduleIndex) noexcept
	{
		for(u32 i = 0; i < parameterCount; ++i)
		{
			(void) HandleUsage(newVar, baseIndex + i);
		}

		return HandleUsage(newVar, newVar);
	}

	bool VisitCallInd(const VarId newVar, const VarId functionPointer, const VarId baseIndex, const u32 parameterCount) noexcept
	{
		(void) HandleUsage(newVar, functionPointer);

		for(u32 i = 0; i < parameterCount; ++i)
		{
			(void) HandleUsage(newVar, baseIndex + i);
		}

		return HandleUsage(newVar, newVar);
	}

	bool VisitCallIndExt(const VarId newVar, const VarId functionPointer, const VarId baseIndex, const u32 parameterCount, const VarId modulePointer) noexcept
	{
		(void) HandleUsage(newVar, functionPointer);
		(void) HandleUsage(newVar, modulePointer);

		for(u32 i = 0; i < parameterCount; ++i)
		{
			(void) HandleUsage(newVar, baseIndex + i);
		}

		return HandleUsage(newVar, newVar);
	}

	bool VisitRet(const SsaCustomType returnType, const VarId var) noexcept
	{
		return HandleUsage(var, var);
	}
private:
    TUsageMap m_UsageMap;
};

class DeadCodeEliminationVisitor final : public SsaVisitor<DeadCodeEliminationVisitor>
{
	DEFAULT_DESTRUCT(DeadCodeEliminationVisitor);
	DELETE_CM(DeadCodeEliminationVisitor);
public:
	using TUsageMap = UsageAnalysisFunctionAttachment::TUsageMap;
public:
	DeadCodeEliminationVisitor(const SsaCustomTypeRegistry& registry, const Function* const function) noexcept
		: SsaVisitor(registry)
        , m_Writer()
	    , m_UsageMap(nullptr)
	    , m_NewVarMap()
	{
        const UsageAnalysisFunctionAttachment* analysis = function->FindAttachment<UsageAnalysisFunctionAttachment>();
		if(analysis)
		{
			m_UsageMap = &analysis->UsageMap();
		}
	}

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

		function->RemoveAttachment<UsageAnalysisFunctionAttachment>();
	}
public:
	bool PreTraversal(const u8* const codePtr, const uSys size, const VarId maxId) noexcept
	{
		m_NewVarMap = DynArray<VarId>(maxId + 1);

		(void) ::std::memset(m_NewVarMap.Array(), 0xFF, m_NewVarMap.Size() * sizeof(VarId));
		
		m_Writer = SsaWriter(size * 3);

		return true;
	}

	bool VisitLabel(const VarId label) noexcept
	{
		m_NewVarMap[label] = m_Writer.WriteLabel();

		return true;
	}

	bool VisitAssignImmediate(const VarId newVar, const SsaCustomType type, const void* const value, const uSys size) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteAssignImmediate(type, value, size);

		return true;
	}

	bool VisitAssignVar(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteAssignVariable(type, FindSourceVar(var));

		return true;
	}

	bool VisitExpandSX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteExpandSX(newType, oldType, FindSourceVar(var));

		return true;
	}

	bool VisitExpandZX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteExpandZX(newType, oldType, FindSourceVar(var));

		return true;
	}

	bool VisitTrunc(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteTrunc(newType, oldType, FindSourceVar(var));

		return true;
	}

	bool VisitRCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteRCast(newType, oldType, FindSourceVar(var));

		return true;
	}

	bool VisitBCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteBCast(newType, oldType, FindSourceVar(var));

		return true;
	}

	bool VisitLoad(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteLoad(type, FindSourceVar(var));

		return true;
	}

	bool VisitStoreV(const SsaCustomType type, const VarId destination, const VarId source) noexcept
	{
		m_Writer.WriteStoreV(type, FindSourceVar(destination), FindSourceVar(source));
		return true;
	}

	bool VisitStoreI(const SsaCustomType type, const VarId destination, const void* const value, const uSys size) noexcept
	{
		m_Writer.WriteStoreI(type, FindSourceVar(destination), value, size);
		return true;
	}

	bool VisitComputePtr(const VarId newVar, const VarId base, const VarId index, const i8 multiplier, const i16 offset) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteComputePtr(FindSourceVar(base), FindSourceVar(index), multiplier, offset);

		return true;
	}

	bool VisitBinOpVToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const VarId b) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteBinOpVtoV(operation, type, FindSourceVar(a), FindSourceVar(b));

		return true;
	}

	bool VisitBinOpVToI(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteBinOpVtoI(operation, type, a, aSize, FindSourceVar(b));

		return true;
	}

	bool VisitBinOpIToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteBinOpItoV(operation, type, FindSourceVar(a), b, bSize);

		return true;
	}

	bool VisitCompVToV(const VarId newVar, const CompareCondition condition, const SsaCustomType type, const VarId a, const VarId b) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteCompVtoV(condition, type, FindSourceVar(a), FindSourceVar(b));

		return true;
	}

	bool VisitCompVToI(const VarId newVar, const CompareCondition condition, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteCompVtoI(condition, type, a, aSize, b);

		return true;
	}

	bool VisitCompIToV(const VarId newVar, const CompareCondition condition, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
	{
		if(!ConfirmUsage(newVar))
		{
			return true;
		}

		m_NewVarMap[newVar] = m_Writer.WriteCompItoV(condition, type, FindSourceVar(a), b, bSize);

		return true;
	}

	bool VisitCall(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount) noexcept
	{
		m_NewVarMap[newVar] = m_Writer.WriteCall(functionIndex, FindSourceVar(baseIndex), parameterCount);

		return true;
	}

	bool VisitCallExt(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount, const u16 moduleIndex) noexcept
	{
		m_NewVarMap[newVar] = m_Writer.WriteCallExt(functionIndex, FindSourceVar(baseIndex), parameterCount, moduleIndex);

		return true;
	}

	bool VisitCallInd(const VarId newVar, const VarId functionPointer, const VarId baseIndex, const u32 parameterCount) noexcept
	{
		m_NewVarMap[newVar] = m_Writer.WriteCallInd(FindSourceVar(functionPointer), FindSourceVar(baseIndex), parameterCount);

		return true;
	}

	bool VisitCallIndExt(const VarId newVar, const VarId functionPointer, const VarId baseIndex, const u32 parameterCount, const VarId modulePointer) noexcept
	{
		m_NewVarMap[newVar] = m_Writer.WriteCallIndExt(FindSourceVar(functionPointer), FindSourceVar(baseIndex), parameterCount, FindSourceVar(modulePointer));

		return true;
	}

	bool VisitRet(const SsaCustomType returnType, const VarId var) noexcept
	{
		m_Writer.WriteRet(returnType, FindSourceVar(var));
		return true;
	}
private:
	[[nodiscard]] const TUsageMap& UsageMap() const noexcept { return *m_UsageMap; }

	VarId FindSourceVar(const VarId var)
	{
		if((var & 0x80000000) != 0)
		{
			return var;
		}

		return m_NewVarMap[var];
	}

	bool ConfirmUsage(const VarId var) noexcept
	{
		auto iter = UsageMap().find(var);

		while(iter != UsageMap().end())
		{
			if((*iter).second == var)
			{
				return true;
			}

			if(ConfirmUsage((*iter).second))
			{
				return true;
			}

			++iter;
		}

		return false;
	}
private:
	SsaWriter m_Writer;
	const TUsageMap* m_UsageMap;
	DynArray<VarId> m_NewVarMap;
};

}
