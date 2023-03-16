#pragma once

#include "TauIR/ssa/SsaVisitor.hpp"
#include "TauIR/ssa/SsaWriter.hpp"

namespace tau::ir::ssa::opto {

namespace internal {

struct ConstantPropLinkage final
{
	DEFAULT_CONSTRUCT_PU(ConstantPropLinkage);
	DEFAULT_DESTRUCT(ConstantPropLinkage);
	DEFAULT_CM_PU(ConstantPropLinkage);

	enum EType
	{
		// Just a variable, will be kept in output
		Variable,
		// A reference to a variable that is not a literal, will not be kept in output
		VariableReference,
		// A literal value, will be kept in output
		ValueLiteral,
		// A reference to a literal value, will not be kept in output
		ValueReference
	};

	// The type that this linkage is
	EType Type;
	// The var this linkage points to, if this is a ValueLiteral it will be -1
	VarId Var;
	// A pointer to the raw value
	const void* Value;
	// The raw literal size
	uSys Size;

	ConstantPropLinkage(const VarId var) noexcept
		: Type(Variable)
		, Var(var)
		, Value(nullptr)
		, Size(0)
	{ }

	ConstantPropLinkage(const void* const value, const uSys size) noexcept
		: Type(ValueLiteral)
		, Var(-1)
		, Value(value)
		, Size(size)
	{ }

	ConstantPropLinkage(const VarId var, const void* const value, const uSys size) noexcept
		: Type(ValueReference)
		, Var(var)
		, Value(value)
		, Size(size)
	{ }

	ConstantPropLinkage(const VarId var, const ConstantPropLinkage& copy) noexcept
		: Type(copy.IsVar() ? VariableReference : ValueReference)
		, Var(var)
		, Value(copy.Value)
		, Size(copy.Size)
	{ }

	[[nodiscard]] bool IsVar() const noexcept { return Type == Variable || Type == VariableReference; }
};

template<typename T>
static inline T RotateLeft(const T n, T c)
{
	const T mask = (CHAR_BIT * sizeof(n) - 1);  // assumes width is a power of 2.
	
	c &= mask;
	return static_cast<T>((n << c) | (n >> ((-c) & mask)));
}

template<typename T>
static inline T RotateRight(const T n, T c)
{
	const T mask = (CHAR_BIT * sizeof(n) - 1);
	
	c &= mask;
	return static_cast<T>((n >> c) | (n << ((-c) & mask)));
}

}

// ReSharper disable CppHidingFunction
class ConstantPropVisitor final : public SsaVisitor<ConstantPropVisitor>
{
	DEFAULT_DESTRUCT(ConstantPropVisitor);
	DELETE_CM(ConstantPropVisitor);
public:
	ConstantPropVisitor(const SsaCustomTypeRegistry& registry) noexcept
		: SsaVisitor(registry)
	{ }

	[[nodiscard]] const SsaWriter& Writer() const noexcept { return m_Writer; }
public:
	bool PreTraversal(const u8* const codePtr, const uSys size, const VarId maxId) noexcept
	{
		if(m_Linkages.Count() != maxId)
		{
			m_Linkages = DynArray<internal::ConstantPropLinkage>(maxId + 1);
			m_NewVarMap = DynArray<VarId>(maxId + 1);
		}

		(void) ::std::memset(m_NewVarMap.Array(), 0xFF, m_NewVarMap.Size() * sizeof(VarId));

		for(VarId i = 0; i < maxId + 1; ++i)
		{
			m_Linkages[i] = internal::ConstantPropLinkage(i);
		}

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
		m_Linkages[newVar] = internal::ConstantPropLinkage(value, size);
		m_NewVarMap[newVar] = m_Writer.WriteAssignImmediate(type, value, size);

		return true;
	}

	bool VisitAssignVar(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
	{
		if((var & 0x80000000) != 0) // var points to an argument
		{
			m_Linkages[newVar] = internal::ConstantPropLinkage(var);
			m_NewVarMap[newVar] = var;
		}
		else
		{
			const VarId source = FindSourceVar(var);

			m_Linkages[newVar] = internal::ConstantPropLinkage(source, m_Linkages[var]);
			m_NewVarMap[newVar] = source;
		}

		return true;
	}

	bool VisitExpandSX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		if((var & 0x80000000) != 0 || m_Linkages[var].IsVar())
		{
			m_NewVarMap[newVar] = m_Writer.WriteExpandSX(newType, oldType, FindSourceVar(var));
		}
		else
		{
			if(IsPointer(newType.Type) || IsPointer(oldType.Type))
			{
				return false;
			}

			if(newType.CustomType != static_cast<u32>(-1) || oldType.CustomType != static_cast<u32>(-1))
			{
				return false;
			}

			switch(newType.Type)
			{
				case SsaType::Void: // Can't expand to a void
				case SsaType::Bool: // Can't expand to a bool
				case SsaType::F16:  // Can't expand FP
				case SsaType::F32:  // Can't expand FP
				case SsaType::F64:  // Can't expand FP
				case SsaType::I8:   // Can't expand to minimum type size
				case SsaType::U8:   // Can't expand to minimum type size
					return false;
				default:
					break;
			}

			switch(oldType.Type)
			{
				case SsaType::Void: // Can't expand from a void
				case SsaType::Bool: // Can't expand from a bool
				case SsaType::F16:  // Can't expand FP
				case SsaType::F32:  // Can't expand FP
				case SsaType::F64:  // Can't expand FP
				case SsaType::I64:  // Can't expand from maximum type size
				case SsaType::U64:  // Can't expand from maximum type size
					return false;
				default:
					break;
			}

			ExpandTypeSX(m_Linkages[var].Value, newVar, newType, oldType);
		}

		return true;
	}

	bool VisitExpandZX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		if((var & 0x80000000) != 0 || m_Linkages[var].IsVar())
		{
			m_NewVarMap[newVar] = m_Writer.WriteExpandSX(newType, oldType, FindSourceVar(var));
		}
		else
		{
			if(IsPointer(newType.Type) || IsPointer(oldType.Type))
			{
				return false;
			}

			if(newType.CustomType != static_cast<u32>(-1) || oldType.CustomType != static_cast<u32>(-1))
			{
				return false;
			}

			switch(newType.Type)
			{
				case SsaType::Void: // Can't expand to a void
				case SsaType::Bool: // Can't expand to a bool
				case SsaType::F16:  // Can't expand FP
				case SsaType::F32:  // Can't expand FP
				case SsaType::F64:  // Can't expand FP
				case SsaType::I8:   // Can't expand to minimum type size
				case SsaType::U8:   // Can't expand to minimum type size
					return false;
				default:
					break;
			}

			switch(oldType.Type)
			{
				case SsaType::Void: // Can't expand from a void
				case SsaType::Bool: // Can't expand from a bool
				case SsaType::F16:  // Can't expand FP
				case SsaType::F32:  // Can't expand FP
				case SsaType::F64:  // Can't expand FP
				case SsaType::I64:  // Can't expand from maximum type size
				case SsaType::U64:  // Can't expand from maximum type size
					return false;
				default:
					break;
			}

			ExpandTypeZX(m_Linkages[var].Value, newVar, newType, oldType);
		}

		return true;
	}

	bool VisitTrunc(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		if((var & 0x80000000) != 0 || m_Linkages[var].IsVar())
		{
			m_NewVarMap[newVar] = m_Writer.WriteExpandSX(newType, oldType, FindSourceVar(var));
		}
		else
		{
			if(IsPointer(newType.Type) || IsPointer(oldType.Type))
			{
				return false;
			}

			if(newType.CustomType != static_cast<u32>(-1) || oldType.CustomType != static_cast<u32>(-1))
			{
				return false;
			}

			switch(newType.Type)
			{
				case SsaType::Void: // Can't truncate to a void
				case SsaType::Bool: // Can't truncate to a bool
				case SsaType::F16:  // Can't truncate FP
				case SsaType::F32:  // Can't truncate FP
				case SsaType::F64:  // Can't truncate FP
				case SsaType::I64:  // Can't truncate to maximum type size
				case SsaType::U64:  // Can't truncate to maximum type size
					return false;
				default:
					break;
			}

			switch(oldType.Type)
			{
				case SsaType::Void: // Can't truncate from a void
				case SsaType::Bool: // Can't truncate from a bool
				case SsaType::F16:  // Can't truncate FP
				case SsaType::F32:  // Can't truncate FP
				case SsaType::F64:  // Can't truncate FP
				case SsaType::I8:   // Can't truncate from minimum type size
				case SsaType::U8:   // Can't truncate from minimum type size
					return false;
				default:
					break;
			}

			TruncType(m_Linkages[var].Value, newVar, newType, oldType);
		}

		return true;
	}

	bool VisitRCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		if((var & 0x80000000) != 0 || m_Linkages[var].IsVar())
		{
			m_NewVarMap[newVar] = m_Writer.WriteRCast(newType, oldType, FindSourceVar(var));
		}
		else
		{
			const internal::ConstantPropLinkage& linkage = m_Linkages[var];

			uSys newTypeSize = TypeValueSize(newType.Type);
			uSys oldTypeSize = TypeValueSize(oldType.Type);

			if(newType.Type == SsaType::Bytes)
			{
				newTypeSize = newType.CustomType;
			}
			else if(newType.Type == SsaType::Custom)
			{
				newTypeSize = Registry()[newType.CustomType].Size;
			}

			if(oldType.Type == SsaType::Bytes)
			{
				oldTypeSize = oldType.CustomType;
			}
			else if(oldType.Type == SsaType::Custom)
			{
				oldTypeSize = Registry()[oldType.CustomType].Size;
			}

			if(newTypeSize != oldTypeSize)
			{
				return false;
			}

			m_NewVarMap[newVar] = m_Writer.WriteAssignImmediate(newType, linkage.Value, linkage.Size);
			m_Linkages[newVar] = internal::ConstantPropLinkage(linkage.Value, linkage.Size);
		}

		return true;
	}

	bool VisitBCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
	{
		if((var & 0x80000000) != 0 || m_Linkages[var].IsVar())
		{
			m_NewVarMap[newVar] = m_Writer.WriteBCast(newType, oldType, FindSourceVar(var));
			return true;
		}

		return VisitRCast(newVar, newType, oldType, var);
	}

	bool VisitLoad(const VarId newVar, const SsaCustomType type, const VarId var) noexcept
	{
		m_NewVarMap[newVar] = m_Writer.WriteLoad(type, FindSourceVar(var));
		return true;
	}

	bool VisitStoreV(const SsaCustomType type, const VarId destination, const VarId source) noexcept
	{
		if((source & 0x80000000) != 0)
		{
			m_Writer.WriteStoreV(type, FindSourceVar(destination), source);
		}

		if(m_Linkages[source].IsVar())
		{
			m_Writer.WriteStoreV(type, FindSourceVar(destination), FindSourceVar(source));
		}
		else
		{
			m_Writer.WriteStoreI(type, FindSourceVar(destination), m_Linkages[source].Value, m_Linkages[source].Size);
		}
		return true;
	}

	bool VisitStoreI(const SsaCustomType type, const VarId destination, const void* const value, const uSys size) noexcept
	{
		m_Writer.WriteStoreI(type, FindSourceVar(destination), value, size);
		return true;
	}

	bool VisitComputePtr(const VarId newVar, const VarId base, const VarId index, const i8 multiplier, const i16 offset) noexcept
	{
		constexpr u64 unassigned_value = -1;

		u64 basePtr = unassigned_value;
		u64 scaledIndex = unassigned_value;
		
		if((base & 0x80000000) == 0 && !m_Linkages[base].IsVar())
		{
			const internal::ConstantPropLinkage& baseLinkage = m_Linkages[base];

			if(baseLinkage.Size != 8)
			{
				return false;
			}

			basePtr = *reinterpret_cast<const u64*>(baseLinkage.Value);
		}

		if((index & 0x80000000) == 0 && !m_Linkages[index].IsVar())
		{
			const internal::ConstantPropLinkage& indexLinkage = m_Linkages[index];

			if(indexLinkage.Size != 8)
			{
				return false;
			}

			scaledIndex = *reinterpret_cast<const u64*>(indexLinkage.Value) * multiplier;
		}

		if(basePtr != unassigned_value && scaledIndex != unassigned_value)
		{
			const u64 computedPtr = basePtr + scaledIndex + offset;
			m_NewVarMap[newVar] = m_Writer.WriteAssignImmediate(SsaCustomType(AddPointer(SsaType::Void)), &computedPtr, sizeof(computedPtr));
			m_Linkages[newVar] = internal::ConstantPropLinkage(m_Writer.Buffer() + m_Writer.Size() - sizeof(computedPtr), sizeof(computedPtr));
		}
		else if(basePtr == unassigned_value && scaledIndex != unassigned_value)
		{
			const i64 scaledIndexOffset = static_cast<i64>(scaledIndex) + offset;
			if(scaledIndexOffset <= IntMaxMin<i16>::Max && scaledIndexOffset >= IntMaxMin<i16>::Min)
			{
				m_NewVarMap[newVar] = m_Writer.WriteComputePtr(base, base, 0, static_cast<i16>(scaledIndexOffset));
			}
			else
			{
				m_NewVarMap[newVar] = m_Writer.WriteComputePtr(base, index, multiplier, offset);
			}
		}
		else
		{
			m_NewVarMap[newVar] = m_Writer.WriteComputePtr(base, index, multiplier, offset);
		}

		return true;
	}

	bool VisitBinOpVToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const VarId b) noexcept
	{
		if(type.CustomType != static_cast<u32>(-1))
		{
			return false;
		}

		switch(type.Type)
		{
			case SsaType::Void:
			case SsaType::Bool:
				return false;
			default:
				break;
		}

		const bool aIsArg = (a & 0x80000000) != 0;
		const bool bIsArg = (b & 0x80000000) != 0;

		if(aIsArg && bIsArg)
		{
			m_NewVarMap[newVar] = m_Writer.WriteBinOpVtoV(operation, type, a, b);
			return true;
		}

		const bool aIsValue = !aIsArg && !m_Linkages[a].IsVar();
		const bool bIsValue = !bIsArg && !m_Linkages[b].IsVar();

		if(!aIsValue && !bIsValue)
		{
			m_NewVarMap[newVar] = m_Writer.WriteBinOpVtoV(operation, type, FindSourceVar(a), FindSourceVar(b));
		}
		else if(aIsValue && bIsValue)
		{
			if(m_Linkages[a].Size != m_Linkages[b].Size || m_Linkages[a].Size != TypeValueSize(type.Type))
			{
				return false;
			}

			EvalIToI(newVar, operation, type, TypeValueSize(type.Type), m_Linkages[a].Value, m_Linkages[b].Value);
		}
		else if(!aIsValue && bIsValue)
		{
			if(m_Linkages[b].Size != TypeValueSize(type.Type))
			{
				return false;
			}

			m_NewVarMap[newVar] = m_Writer.WriteBinOpItoV(operation, type, FindSourceVar(a), m_Linkages[b].Value, m_Linkages[b].Size);
		}
		else if(aIsValue && !bIsValue)
		{
			if(m_Linkages[a].Size != TypeValueSize(type.Type))
			{
				return false;
			}

			m_NewVarMap[newVar] = m_Writer.WriteBinOpVtoI(operation, type, m_Linkages[a].Value, m_Linkages[a].Size, FindSourceVar(b));
		}

		return true;
	}

	bool VisitBinOpVToI(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept
	{
		if(type.CustomType != static_cast<u32>(-1) || aSize != TypeValueSize(type.Type))
		{
			return false;
		}

		switch(type.Type)
		{
			case SsaType::Void:
			case SsaType::Bool:
				return false;
			default:
				break;
		}
		
		const bool bIsArg = (b & 0x80000000) != 0;

		if(bIsArg)
		{
			m_NewVarMap[newVar] = m_Writer.WriteBinOpVtoI(operation, type, a, aSize, b);
			return true;
		}
		
		const bool bIsValue = !bIsArg && !m_Linkages[b].IsVar();

		if(!bIsValue)
		{
			m_NewVarMap[newVar] = m_Writer.WriteBinOpVtoI(operation, type, a, aSize, FindSourceVar(b));
		}
		else
		{
			EvalIToI(newVar, operation, type, TypeValueSize(type.Type), a, m_Linkages[b].Value);
		}

		return true;
	}

	bool VisitBinOpIToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept
	{
		if(type.CustomType != static_cast<u32>(-1) || bSize != TypeValueSize(type.Type))
		{
			return false;
		}

		switch(type.Type)
		{
			case SsaType::Void:
			case SsaType::Bool:
				return false;
			default:
				break;
		}

		const bool aIsArg = (a & 0x80000000) != 0;

		if(aIsArg)
		{
			m_NewVarMap[newVar] = m_Writer.WriteBinOpItoV(operation, type, a, b, bSize);
			return true;
		}

		const bool aIsValue = !aIsArg && !m_Linkages[a].IsVar();

		if(!aIsValue)
		{
			m_NewVarMap[newVar] = m_Writer.WriteBinOpItoV(operation, type, FindSourceVar(a), b, bSize);
		}
		else
		{
			EvalIToI(newVar, operation, type, TypeValueSize(type.Type), m_Linkages[a].Value, b);
		}

		return true;
	}
private:
	template<typename TOut, typename TIn>
	void TransformType(const void* const buffer, const VarId newVar, const SsaCustomType newType) noexcept
	{
		const TIn rawValue = *reinterpret_cast<const TIn*>(buffer);
		const TOut transformedValue = static_cast<TOut>(rawValue);
		m_NewVarMap[newVar] = m_Writer.WriteAssignImmediate(newType, &transformedValue, sizeof(transformedValue));
		m_Linkages[newVar] = internal::ConstantPropLinkage(m_Writer.Buffer() + m_Writer.Size() - sizeof(transformedValue), sizeof(transformedValue));
	}

	template<typename TOut>
	void ExpandTypeSX0(const void* const buffer, const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType) noexcept
	{
		switch(oldType.Type)
		{
			case SsaType::I8:
			case SsaType::U8:
				TransformType<TOut, i8>(buffer, newVar, newType);
				break;
			case SsaType::I16:
			case SsaType::U16:
				TransformType<TOut, i16>(buffer, newVar, newType);
				break;
			case SsaType::I32:
			case SsaType::U32:
				TransformType<TOut, i32>(buffer, newVar, newType);
				break;
			case SsaType::I64:
			case SsaType::U64:
				TransformType<TOut, i64>(buffer, newVar, newType);
				break;
			default:
				break;
		}
	}

	template<typename TOut>
	void ExpandTypeZX0(const void* const buffer, const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType) noexcept
	{
		switch(oldType.Type)
		{
			case SsaType::I8:
			case SsaType::U8:
				TransformType<TOut, u8>(buffer, newVar, newType);
				break;
			case SsaType::I16:
			case SsaType::U16:
				TransformType<TOut, u16>(buffer, newVar, newType);
				break;
			case SsaType::I32:
			case SsaType::U32:
				TransformType<TOut, u32>(buffer, newVar, newType);
				break;
			case SsaType::I64:
			case SsaType::U64:
				TransformType<TOut, u64>(buffer, newVar, newType);
				break;
			default:
				break;
		}
	}

	template<typename TOut>
	void TruncType0(const void* const buffer, const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType) noexcept
	{
		switch(oldType.Type)
		{
			case SsaType::I8:
				TransformType<TOut, i8>(buffer, newVar, newType);
				break;
			case SsaType::U8:
				TransformType<TOut, u8>(buffer, newVar, newType);
				break;
			case SsaType::I16:
				TransformType<TOut, i16>(buffer, newVar, newType);
				break;
			case SsaType::U16:
				TransformType<TOut, u16>(buffer, newVar, newType);
				break;
			case SsaType::I32:
				TransformType<TOut, i32>(buffer, newVar, newType);
				break;
			case SsaType::U32:
				TransformType<TOut, u32>(buffer, newVar, newType);
				break;
			case SsaType::I64:
				TransformType<TOut, i64>(buffer, newVar, newType);
				break;
			case SsaType::U64:
				TransformType<TOut, u64>(buffer, newVar, newType);
				break;
			default:
				break;
		}
	}

	void ExpandTypeSX(const void* const buffer, const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType) noexcept
	{
		switch(newType.Type)
		{
			case SsaType::I8:
			case SsaType::U8:
				ExpandTypeSX0<i8>(buffer, newVar, newType, oldType);
				break;
			case SsaType::I16:
			case SsaType::U16:
				ExpandTypeSX0<i16>(buffer, newVar, newType, oldType);
				break;
			case SsaType::I32:
			case SsaType::U32:
				ExpandTypeSX0<i32>(buffer, newVar, newType, oldType);
				break;
			case SsaType::I64:
			case SsaType::U64:
				ExpandTypeSX0<i64>(buffer, newVar, newType, oldType);
				break;
			default:
				break;
		}
	}

	void ExpandTypeZX(const void* const buffer, const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType) noexcept
	{
		switch(newType.Type)
		{
			case SsaType::I8:
			case SsaType::U8:
				ExpandTypeZX0<u8>(buffer, newVar, newType, oldType);
				break;
			case SsaType::I16:
			case SsaType::U16:
				ExpandTypeZX0<u16>(buffer, newVar, newType, oldType);
				break;
			case SsaType::I32:
			case SsaType::U32:
				ExpandTypeZX0<u32>(buffer, newVar, newType, oldType);
				break;
			case SsaType::I64:
			case SsaType::U64:
				ExpandTypeZX0<u64>(buffer, newVar, newType, oldType);
				break;
			default:
				break;
		}
	}
	
	void TruncType(const void* const buffer, const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType) noexcept
	{
		switch(newType.Type)
		{
			case SsaType::I8:
				TruncType0<i8>(buffer, newVar, newType, oldType);
				break;
			case SsaType::U8:
				TruncType0<u8>(buffer, newVar, newType, oldType);
				break;
			case SsaType::I16:
				TruncType0<i16>(buffer, newVar, newType, oldType);
				break;
			case SsaType::U16:
				TruncType0<u16>(buffer, newVar, newType, oldType);
				break;
			case SsaType::I32:
				TruncType0<i32>(buffer, newVar, newType, oldType);
				break;
			case SsaType::U32:
				TruncType0<u32>(buffer, newVar, newType, oldType);
				break;
			case SsaType::I64:
				TruncType0<i64>(buffer, newVar, newType, oldType);
				break;
			case SsaType::U64:
				TruncType0<u64>(buffer, newVar, newType, oldType);
				break;
			default:
				break;
		}
	}

	template<typename T>
	void EvalIToI0(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const T a, const T b)
	{
		T result{};

		switch(operation)
		{
			case SsaBinaryOperation::Add:
				result = static_cast<T>(a + b);
				break;
			case SsaBinaryOperation::Sub:
				result = static_cast<T>(a - b);
				break;
			case SsaBinaryOperation::Mul:
				result = static_cast<T>(a * b);
				break;
			case SsaBinaryOperation::Div:
				result = static_cast<T>(a / b);
				break;
			case SsaBinaryOperation::Rem:
				result = static_cast<T>(a % b);
				break;
			case SsaBinaryOperation::BitShiftLeft:
				result = static_cast<T>(a << b);
				break;
			case SsaBinaryOperation::BitShiftRight:
				result = static_cast<T>(a >> b);
				break;
			case SsaBinaryOperation::BarrelShiftLeft:
				result = internal::RotateLeft(a, b);
				break;
			case SsaBinaryOperation::BarrelShiftRight:
				result = internal::RotateRight(a, b);
				break;
			case SsaBinaryOperation::Comp:
				throw 0;
				break;
		}

		m_NewVarMap[newVar] = m_Writer.WriteAssignImmediate(type, &result, sizeof(result));
		m_Linkages[newVar] = internal::ConstantPropLinkage(m_Writer.Buffer() + m_Writer.Size() - sizeof(result), sizeof(result));
	}

	void EvalIToI(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const uSys bufferSize, const void* const aBuffer, const void* const bBuffer)
	{
		if(IsPointer(type.Type))
		{
			EvalIToI0(newVar, operation, type, *reinterpret_cast<const u64*>(aBuffer), *reinterpret_cast<const u64*>(bBuffer));
		}
		else
		{
			switch(type.Type)
			{
				case SsaType::I8:
					EvalIToI0(newVar, operation, type, *reinterpret_cast<const i8*>(aBuffer), *reinterpret_cast<const i8*>(bBuffer));
					break;
				case SsaType::U8:
					EvalIToI0(newVar, operation, type, *reinterpret_cast<const u8*>(aBuffer), *reinterpret_cast<const u8*>(bBuffer));
					break;
				case SsaType::I16:
					EvalIToI0(newVar, operation, type, *reinterpret_cast<const i16*>(aBuffer), *reinterpret_cast<const i16*>(bBuffer));
					break;
				case SsaType::U16:
					EvalIToI0(newVar, operation, type, *reinterpret_cast<const u16*>(aBuffer), *reinterpret_cast<const u16*>(bBuffer));
					break;
				case SsaType::I32:
					EvalIToI0(newVar, operation, type, *reinterpret_cast<const i32*>(aBuffer), *reinterpret_cast<const i32*>(bBuffer));
					break;
				case SsaType::U32:
					EvalIToI0(newVar, operation, type, *reinterpret_cast<const u32*>(aBuffer), *reinterpret_cast<const u32*>(bBuffer));
					break;
				case SsaType::I64:
					EvalIToI0(newVar, operation, type, *reinterpret_cast<const i64*>(aBuffer), *reinterpret_cast<const i64*>(bBuffer));
					break;
				case SsaType::U64:
					EvalIToI0(newVar, operation, type, *reinterpret_cast<const u64*>(aBuffer), *reinterpret_cast<const u64*>(bBuffer));
					break;
				default:
					break;
			}
		}
	}

	VarId FindSourceVar(const VarId var)
	{
		if((var & 0x80000000) != 0)
		{
			return var;
		}

		return m_NewVarMap[var];
	}
private:
	DynArray<internal::ConstantPropLinkage> m_Linkages;
	DynArray<VarId> m_NewVarMap;
	SsaWriter m_Writer;
};

}
