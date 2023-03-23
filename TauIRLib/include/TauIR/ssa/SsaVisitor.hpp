#pragma once

#include <Objects.hpp>
#include "SsaTypes.hpp"
#include "SsaOpcodes.hpp"
#include "TauIR/Opcodes.hpp"
#include "TauIR/Function.hpp"
#include "TauIR/ssa/SsaFunctionAttachment.hpp"
#include <cstring>
#include <DynArray.hpp>

namespace tau::ir::ssa {

namespace internal {

template<typename T>
static inline T ReadType(const u8* const codePtr, uSys& i) noexcept
{
    T ret{};
    (void) ::std::memcpy(::std::addressof(ret), codePtr + i, sizeof(T));
    i += sizeof(T);
    return ret;
}

template<>
inline SsaCustomType ReadType<SsaCustomType>(const u8* const codePtr, uSys& i) noexcept
{
    const SsaType typeId = static_cast<SsaType>(codePtr[i++]);

    u32 customType = -1;

    if(StripPointer(typeId) == SsaType::Bytes || StripPointer(typeId) == SsaType::Custom)
    {
        customType = ReadType<u32>(codePtr, i);
    }

    return SsaCustomType(typeId, customType);
}

}

template<typename Derived>
class SsaVisitor
{
    DEFAULT_DESTRUCT_VI(SsaVisitor);
    DEFAULT_CM_PO(SsaVisitor);
protected:
    SsaVisitor(const SsaCustomTypeRegistry& registry) noexcept
        : m_Registry(&registry)
    { }
public:
    bool Traverse(const u8* codePtr, uSys size, VarId maxId) noexcept;

    bool Traverse(const Function* const function) noexcept
    {
        const SsaFunctionAttachment* ssaAttachment = function->FindAttachment<SsaFunctionAttachment>();

        if(!ssaAttachment)
        {
            return false;
        }

        return Traverse(ssaAttachment->Writer().Buffer(), ssaAttachment->Writer().Size(), ssaAttachment->Writer().IdIndex());
    }
protected:
    // ReSharper disable once CppHiddenFunction
    bool PreTraversal(const u8* const codePtr, const uSys size, const VarId maxId) noexcept { return true; }

    // ReSharper disable once CppHiddenFunction
    bool PostTraversal() noexcept { return true; }

    // ReSharper disable once CppHiddenFunction
    bool VisitNop() noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitLabel(const VarId label) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitAssignImmediate(const VarId newVar, const SsaCustomType type, const void* const value, const uSys size) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitAssignVar(const VarId newVar, const SsaCustomType type, const VarId var) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitExpandSX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitExpandZX(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitTrunc(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept { return true; } 
    // ReSharper disable once CppHiddenFunction
    bool VisitRCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept { return true; } 
    // ReSharper disable once CppHiddenFunction
    bool VisitBCast(const VarId newVar, const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept { return true; } 
    // ReSharper disable once CppHiddenFunction                                     
    bool VisitLoad(const VarId newVar, const SsaCustomType type, const VarId var) noexcept { return true; }                                                                                     
    // ReSharper disable once CppHiddenFunction
    bool VisitStoreV(const SsaCustomType type, const VarId destination, const VarId source) noexcept { return true; }                                                                           
    // ReSharper disable once CppHiddenFunction
    bool VisitStoreI(const SsaCustomType type, const VarId destination, const void* const value, const uSys size) noexcept { return true; }                                                     
    // ReSharper disable once CppHiddenFunction
    bool VisitComputePtr(const VarId newVar, const VarId base, const VarId index, const i8 multiplier, const i16 offset) noexcept { return true; }                                              
    // ReSharper disable once CppHiddenFunction
    bool VisitBinOpVToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const VarId b) noexcept { return true; }                               
    // ReSharper disable once CppHiddenFunction
    bool VisitBinOpVToI(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept { return true; }       
    // ReSharper disable once CppHiddenFunction
    bool VisitBinOpIToV(const VarId newVar, const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept { return true; }       
    // ReSharper disable once CppHiddenFunction
    bool VisitSplit(const VarId baseIndex, const SsaCustomType aType, const VarId a, const uSys splitCount, const SsaCustomType* const splitTypes) noexcept { return true; }                                           
    // ReSharper disable once CppHiddenFunction
    bool VisitJoin(const VarId newVar, const SsaCustomType newType, const uSys joinCount, const SsaCustomType* const joinTypes, const VarId* const joinVars) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitCompVToV(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const VarId a, const VarId b) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitCompVToI(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const void* const a, const uSys aSize, const VarId b) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitCompIToV(const VarId newVar, const CompareCondition operation, const SsaCustomType type, const VarId a, const void* const b, const uSys bSize) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction 
    bool VisitCall(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitCallExt(const VarId newVar, const u32 functionIndex, const VarId baseIndex, const u32 parameterCount, const u16 moduleIndex) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitCallInd(const VarId newVar, const VarId functionPointer, const VarId baseIndex, const u32 parameterCount) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitCallIndExt(const VarId newVar, const VarId functionPointer, const VarId baseIndex, const u32 parameterCount, const VarId modulePointer) noexcept { return true; }
    // ReSharper disable once CppHiddenFunction
    bool VisitRet(const SsaCustomType returnType, const VarId var) noexcept { return true; }

    [[nodiscard]] const SsaCustomTypeRegistry& Registry() const noexcept { return *m_Registry; }
private:
    [[nodiscard]] Derived& GetDerived() noexcept { return *static_cast<Derived*>(this); }
private:
    const SsaCustomTypeRegistry* m_Registry;
};

template<typename Derived>
bool SsaVisitor<Derived>::Traverse(const u8* const codePtr, const uSys size, const VarId maxId) noexcept
{
    using namespace internal;

    if(!GetDerived().PreTraversal(codePtr, size, maxId))
    {
        return false;
    }

    VarId idIndex = 1;

    for(uSys i = 0; i < size;)
    {
        u16 opcodeRaw = codePtr[i++];

        // Read Second Byte
        if(opcodeRaw & 0x80)
        {
            opcodeRaw <<= 8;
            opcodeRaw |= codePtr[i++];
        }

        const SsaOpcode opcode = static_cast<SsaOpcode>(opcodeRaw);
        
        switch(opcode)
        {
            case SsaOpcode::Nop:
                if(!GetDerived().VisitNop())
                {
                    return false;
                }
                break;
            case SsaOpcode::Label:
                if(!GetDerived().VisitLabel(idIndex++))
                {
                    return false;
                }
                break;
            case SsaOpcode::AssignImmediate:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                u32 typeSize = static_cast<u32>(TypeValueSize(type.Type));

                if(StripPointer(type.Type) == SsaType::Bytes || StripPointer(type.Type) == SsaType::Custom)
                {
                    typeSize = Registry()[type.CustomType].Size;
                }

                if(!GetDerived().VisitAssignImmediate(idIndex++, type, codePtr + i, typeSize))
                {
                    return false;
                }

                i += typeSize;
                break;
            }
        	case SsaOpcode::AssignVariable:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const VarId var = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitAssignVar(idIndex++, type, var))
                {
	                return false;
                }
                break;
            }
        	case SsaOpcode::ExpandSX:
            {
                const SsaCustomType newType = ReadType<SsaCustomType>(codePtr, i);
                const SsaCustomType oldType = ReadType<SsaCustomType>(codePtr, i);
                const VarId var = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitExpandSX(idIndex++, newType, oldType, var))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::ExpandZX:
            {
                const SsaCustomType newType = ReadType<SsaCustomType>(codePtr, i);
                const SsaCustomType oldType = ReadType<SsaCustomType>(codePtr, i);
                const VarId var = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitExpandZX(idIndex++, newType, oldType, var))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::Trunc:
            {
                const SsaCustomType newType = ReadType<SsaCustomType>(codePtr, i);
                const SsaCustomType oldType = ReadType<SsaCustomType>(codePtr, i);
                const VarId var = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitTrunc(idIndex++, newType, oldType, var))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::RCast:
            {
                const SsaCustomType newType = ReadType<SsaCustomType>(codePtr, i);
                const SsaCustomType oldType = ReadType<SsaCustomType>(codePtr, i);
                const VarId var = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitRCast(idIndex++, newType, oldType, var))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::BCast:
            {
                const SsaCustomType newType = ReadType<SsaCustomType>(codePtr, i);
                const SsaCustomType oldType = ReadType<SsaCustomType>(codePtr, i);
                const VarId var = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitBCast(idIndex++, newType, oldType, var))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::Load:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const VarId var = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitLoad(idIndex++, type, var))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::StoreV:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const VarId dest = ReadType<VarId>(codePtr, i);
                const VarId src = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitStoreV(type, dest, src))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::StoreI:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const VarId dest = ReadType<VarId>(codePtr, i);
                u32 typeSize = static_cast<u32>(TypeValueSize(type.Type));

                if(StripPointer(type.Type) == SsaType::Bytes || StripPointer(type.Type) == SsaType::Custom)
                {
                    typeSize = Registry()[type.CustomType].Size;
                }

                if(!GetDerived().VisitStoreI(type, dest, codePtr + i, typeSize))
                {
                    return false;
                }

                i += typeSize;
                break;
            }
            case SsaOpcode::ComputePtr:
            {
                const VarId base = ReadType<VarId>(codePtr, i);
                const VarId index = ReadType<VarId>(codePtr, i);
                const i8 multiplier = ReadType<i8>(codePtr, i);
                const i16 offset = ReadType<i16>(codePtr, i);

                if(!GetDerived().VisitComputePtr(idIndex++, base, index, multiplier, offset))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::BinOpVtoV:
            {
                const SsaBinaryOperation op = ReadType<SsaBinaryOperation>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const VarId a = ReadType<VarId>(codePtr, i);
                const VarId b = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitBinOpVToV(idIndex++, op, type, a, b))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::BinOpVtoI:
            {
                const SsaBinaryOperation op = ReadType<SsaBinaryOperation>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const void* const aValue = codePtr + i;

                u32 typeSize = static_cast<u32>(TypeValueSize(type.Type));

                if(StripPointer(type.Type) == SsaType::Bytes || StripPointer(type.Type) == SsaType::Custom)
                {
                    typeSize = Registry()[type.CustomType].Size;
                }

                i += typeSize;

                const VarId b = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitBinOpVToI(idIndex++, op, type, aValue, typeSize, b))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::BinOpItoV:
            {
                const SsaBinaryOperation op = ReadType<SsaBinaryOperation>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const VarId a = ReadType<VarId>(codePtr, i);
                const void* const bValue = codePtr + i;

                u32 typeSize = static_cast<u32>(TypeValueSize(type.Type));

                if(StripPointer(type.Type) == SsaType::Bytes || StripPointer(type.Type) == SsaType::Custom)
                {
                    typeSize = Registry()[type.CustomType].Size;
                }

                i += typeSize;

                if(!GetDerived().VisitBinOpIToV(idIndex++, op, type, a, bValue, typeSize))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::Split:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const VarId a = ReadType<VarId>(codePtr, i);
                const u32 splitCount = ReadType<u32>(codePtr, i);

                if(splitCount <= 32)
                {
                    SsaCustomType types[32];

                    for(uSys j = 0; j < splitCount; ++j)
                    {
                        types[j] = ReadType<SsaCustomType>(codePtr, i);
                    }

                    if(!GetDerived().VisitSplit(idIndex, type, a, splitCount, types))
                    {
                        return false;
                    }
                }
                else
                {
                    DynArray<SsaCustomType> types(splitCount);

                    for(uSys j = 0; j < splitCount; ++j)
                    {
                        types[j] = ReadType<SsaCustomType>(codePtr, i);
                    }

                    if(!GetDerived().VisitSplit(idIndex, type, a, splitCount, types))
                    {
                        return false;
                    }
                }

                idIndex += splitCount;
                break;
            }
            case SsaOpcode::Join:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const u32 joinCount = ReadType<u32>(codePtr, i);

                if(joinCount <= 32)
                {
                    SsaCustomType types[32];
                    VarId vars[32];

                    for(uSys j = 0; j < joinCount; ++j)
                    {
                        types[j] = ReadType<SsaCustomType>(codePtr, i);
                    }

                    for(uSys j = 0; j < joinCount; ++j)
                    {
                        vars[j] = ReadType<VarId>(codePtr, i);
                    }

                    if(!GetDerived().VisitJoin(idIndex, type, joinCount, types, vars))
                    {
                        return false;
                    }
                }
                else
                {
                    void* raw = operator new(joinCount * (sizeof(SsaCustomType) + sizeof(VarId)));
                    SsaCustomType* const types = reinterpret_cast<SsaCustomType*>(raw);
                    VarId* const vars = reinterpret_cast<VarId*>(types + joinCount);

                    for(uSys j = 0; j < joinCount; ++j)
                    {
                        types[j] = ReadType<SsaCustomType>(codePtr, i);
                    }

                    for(uSys j = 0; j < joinCount; ++j)
                    {
                        vars[j] = ReadType<VarId>(codePtr, i);
                    }

                    if(!GetDerived().VisitJoin(idIndex, type, joinCount, types, vars))
                    {
                        operator delete(raw);
                        return false;
                    }
                    operator delete(raw);
                }

                idIndex += joinCount;
                break;
            }
            case SsaOpcode::CompVtoV:
            {
                const CompareCondition cond = ReadType<CompareCondition>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const VarId a = ReadType<VarId>(codePtr, i);
                const VarId b = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitCompVToV(idIndex++, cond, type, a, b))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::CompVtoI:
            {
                const CompareCondition cond = ReadType<CompareCondition>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const void* const aValue = codePtr + i;

                u32 typeSize = static_cast<u32>(TypeValueSize(type.Type));

                if(StripPointer(type.Type) == SsaType::Bytes || StripPointer(type.Type) == SsaType::Custom)
                {
                    typeSize = Registry()[type.CustomType].Size;
                }

                i += typeSize;

                const VarId b = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitCompVToI(idIndex++, cond, type, aValue, typeSize, b))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::CompItoV:
            {
                const CompareCondition cond = ReadType<CompareCondition>(codePtr, i);
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const VarId a = ReadType<VarId>(codePtr, i);
                const void* const bValue = codePtr + i;

                u32 typeSize = static_cast<u32>(TypeValueSize(type.Type));

                if(StripPointer(type.Type) == SsaType::Bytes || StripPointer(type.Type) == SsaType::Custom)
                {
                    typeSize = Registry()[type.CustomType].Size;
                }

                i += typeSize;

                if(!GetDerived().VisitCompIToV(idIndex++, cond, type, a, bValue, typeSize))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::Branch:
            case SsaOpcode::BranchCond:
            {
                throw 1;
                break;
            }
            case SsaOpcode::Call:
            {
                const u32 functionIndex = ReadType<u32>(codePtr, i);
                const VarId baseIndex = ReadType<VarId>(codePtr, i);
                const u32 parameterCount = ReadType<u32>(codePtr, i);

                if(!GetDerived().VisitCall(idIndex++, functionIndex, baseIndex, parameterCount))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::CallExt:
            {
                const u32 functionIndex = ReadType<u32>(codePtr, i);
                const VarId baseIndex = ReadType<VarId>(codePtr, i);
                const u32 parameterCount = ReadType<u32>(codePtr, i);
                const u16 moduleIndex = ReadType<u16>(codePtr, i);

                if(!GetDerived().VisitCallExt(idIndex++, functionIndex, baseIndex, parameterCount, moduleIndex))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::CallInd:
            {
                const VarId functionPointer = ReadType<VarId>(codePtr, i);
                const VarId baseIndex = ReadType<VarId>(codePtr, i);
                const u32 parameterCount = ReadType<u32>(codePtr, i);

                if(!GetDerived().VisitCallInd(idIndex++, functionPointer, baseIndex, parameterCount))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::CallIndExt:
            {
                const VarId functionPointer = ReadType<VarId>(codePtr, i);
                const VarId baseIndex = ReadType<VarId>(codePtr, i);
                const u32 parameterCount = ReadType<u32>(codePtr, i);
                const VarId modulePointer = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitCallIndExt(idIndex++, functionPointer, baseIndex, parameterCount, modulePointer))
                {
                    return false;
                }
                break;
            }
            case SsaOpcode::Ret:
            {
                const SsaCustomType type = ReadType<SsaCustomType>(codePtr, i);
                const VarId var = ReadType<VarId>(codePtr, i);

                if(!GetDerived().VisitRet(type, var))
                {
                    return false;
                }
                break;
            }
        }
    }

    return PostTraversal();
}

}
