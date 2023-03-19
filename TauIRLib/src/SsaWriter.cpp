#include "TauIR/ssa/SsaWriter.hpp"
#include <TUMaths.hpp>
#include <new>
#include <cstring>
#include <stdexcept>

#define MAX_ARGUMENT_COUNT (256)

namespace tau::ir::ssa {

constexpr static inline uSys GetOpCodeSize(SsaOpcode opcode) noexcept
{
    return static_cast<u16>(opcode) & 0x8000 ? 2 : 1;
}

SsaWriter::SsaWriter(const uSys initialBufferSize) noexcept
    : m_Buffer(new(::std::nothrow) u8[maxT(64, initialBufferSize)])
    , m_BufferSize(maxT(64, initialBufferSize))
    , m_WriteIndex(0)
    , m_IdIndex(0)
{
    m_VarTypeMap.emplace_back();
}

SsaWriter::~SsaWriter() noexcept
{
    delete[] m_Buffer;
}

SsaWriter::SsaWriter(SsaWriter&& move) noexcept
    : m_Buffer(move.m_Buffer)
    , m_BufferSize(move.m_BufferSize)
    , m_WriteIndex(move.m_WriteIndex)
    , m_IdIndex(move.m_IdIndex)
    , m_VarTypeMap(::std::move(move.m_VarTypeMap))
{
    if(this != &move)
    {
        move.m_Buffer = nullptr;
    }
}

SsaWriter& SsaWriter::operator=(SsaWriter&& move) noexcept
{
    if(this == &move)
    {
        return *this;
    }

    if(!move.m_Buffer)
    {
        return *this;
    }

    m_Buffer = move.m_Buffer;
    m_BufferSize = move.m_BufferSize;
    m_WriteIndex = move.m_WriteIndex;
    m_IdIndex = move.m_IdIndex;
    m_VarTypeMap = ::std::move(move.m_VarTypeMap);

    move.m_Buffer = nullptr;

    return *this;
}

void SsaWriter::EnsureSize(const uSys additionalSize) noexcept
{
    if(m_WriteIndex + additionalSize <= m_BufferSize)
    {
        return;
    }

    const uSys newSize = maxT(m_BufferSize + (m_BufferSize >> 1), m_WriteIndex + additionalSize);
    u8* const newBuffer = new(::std::nothrow) u8[newSize];
    (void) ::std::memcpy(newBuffer, m_Buffer, m_BufferSize);
    m_BufferSize = newSize;
    delete[] m_Buffer;
    m_Buffer = newBuffer;
}

void SsaWriter::WriteNop() noexcept
{
    WriteOpcode(SsaOpcode::Nop);
}

VarId SsaWriter::WriteLabel() noexcept
{
    WriteOpcode(SsaOpcode::Label);
    m_VarTypeMap.emplace_back(SsaType::Void);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteAssignImmediate(const SsaCustomType varType, const void* const value, const uSys valueSize) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::AssignImmediate) + varType.Size() + valueSize);
    WriteOpcode(SsaOpcode::AssignImmediate);
    WriteType(varType);
    WriteRaw(value, valueSize);
    m_VarTypeMap.push_back(varType);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteAssignVariable(const SsaCustomType varType, const VarId var) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::AssignVariable) + varType.Size() + sizeof(var));
    WriteOpcode(SsaOpcode::AssignVariable);
    WriteType(varType);
    WriteT(var);
    m_VarTypeMap.push_back(varType);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteExpandSX(const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::ExpandSX) + newType.Size() + oldType.Size() + sizeof(var));
    WriteOpcode(SsaOpcode::ExpandSX);
    WriteType(newType);
    WriteType(oldType);
    WriteT(var);
    m_VarTypeMap.push_back(newType);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteExpandZX(const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::ExpandZX) + newType.Size() + oldType.Size() + sizeof(var));
    WriteOpcode(SsaOpcode::ExpandZX);
    WriteType(newType);
    WriteType(oldType);
    WriteT(var);
    m_VarTypeMap.push_back(newType);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteTrunc(const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::Trunc) + newType.Size() + oldType.Size() + sizeof(var));
    WriteOpcode(SsaOpcode::Trunc);
    WriteType(newType);
    WriteType(oldType);
    WriteT(var);
    m_VarTypeMap.push_back(newType);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteRCast(const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::RCast) + newType.Size() + oldType.Size() + sizeof(var));
    WriteOpcode(SsaOpcode::RCast);
    WriteType(newType);
    WriteType(oldType);
    WriteT(var);
    m_VarTypeMap.push_back(newType);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteBCast(const SsaCustomType newType, const SsaCustomType oldType, const VarId var) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::BCast) + newType.Size() + oldType.Size() + sizeof(var));
    WriteOpcode(SsaOpcode::BCast);
    WriteType(newType);
    WriteType(oldType);
    WriteT(var);
    m_VarTypeMap.push_back(newType);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteLoad(const SsaCustomType type, const VarId var) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::Load) + type.Size() + sizeof(var));
    WriteOpcode(SsaOpcode::Load);
    WriteType(type);
    WriteT(var);
    m_VarTypeMap.push_back(type);
    return ++m_IdIndex;
}

void SsaWriter::WriteStoreV(const SsaCustomType type, const VarId destPtr, const VarId sourceVar) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::StoreV) + type.Size() + sizeof(destPtr) + sizeof(sourceVar));
    WriteOpcode(SsaOpcode::StoreV);
    WriteType(type);
    WriteT(destPtr);
    WriteT(sourceVar);
}

void SsaWriter::WriteStoreI(const SsaCustomType type, const VarId destPtr, const void* const value, const uSys size) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::StoreI) + type.Size() + sizeof(destPtr) + size);
    WriteOpcode(SsaOpcode::StoreI);
    WriteType(type);
    WriteT(destPtr);
    WriteRaw(value, size);
}

VarId SsaWriter::WriteComputePtr(const VarId base, const VarId index, const i8 multiplier, const i16 offset) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::ComputePtr) + sizeof(base) + sizeof(index) + sizeof(multiplier) + sizeof(offset));
    WriteOpcode(SsaOpcode::ComputePtr);
    WriteT(base);
    WriteT(index);
    WriteT(multiplier);
    WriteT(offset);
    m_VarTypeMap.emplace_back(AddPointer(SsaType::Void));
    return ++m_IdIndex;
}

VarId SsaWriter::WriteBinOpVtoV(const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const VarId b)
{
    EnsureSize(GetOpCodeSize(SsaOpcode::BinOpVtoV) + sizeof(operation) + type.Size() + sizeof(a) + sizeof(b));
    WriteOpcode(SsaOpcode::BinOpVtoV);
    WriteT(operation);
    WriteType(type);
    WriteT(a);
    WriteT(b);
    m_VarTypeMap.push_back(type);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteBinOpVtoI(const SsaBinaryOperation operation, const SsaCustomType type, const void* const aValue, const uSys aSize, const VarId b)
{
    EnsureSize(GetOpCodeSize(SsaOpcode::BinOpVtoI) + sizeof(operation) + type.Size() + aSize + sizeof(b));
    WriteOpcode(SsaOpcode::BinOpVtoI);
    WriteT(operation);
    WriteType(type);
    WriteRaw(aValue, aSize);
    WriteT(b);
    m_VarTypeMap.push_back(type);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteBinOpItoV(const SsaBinaryOperation operation, const SsaCustomType type, const VarId a, const void* const bValue, const uSys bSize)
{
    EnsureSize(GetOpCodeSize(SsaOpcode::BinOpItoV) + sizeof(operation) + type.Size() + sizeof(a) + bSize);
    WriteOpcode(SsaOpcode::BinOpItoV);
    WriteT(operation);
    WriteType(type);
    WriteT(a);
    WriteRaw(bValue, bSize);
    m_VarTypeMap.push_back(type);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteCall(const u32 function, const u32 baseIndex, const u32 parameterCount)
{
    EnsureSize(GetOpCodeSize(SsaOpcode::Call) + 3 * sizeof(u32));
    WriteOpcode(SsaOpcode::Call);
    WriteT(function);
    WriteT(baseIndex);
    WriteT(parameterCount);
    m_VarTypeMap.push_back(SsaType::U64);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteCallExt(const u32 function, const u32 baseIndex, const u32 parameterCount, const u16 moduleIndex)
{
    EnsureSize(GetOpCodeSize(SsaOpcode::CallExt) + 3 * sizeof(u32) + sizeof(moduleIndex));
    WriteOpcode(SsaOpcode::CallExt);
    WriteT(function);
    WriteT(baseIndex);
    WriteT(parameterCount);
    WriteT(moduleIndex);
    m_VarTypeMap.push_back(SsaType::U64);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteCallInd(const u32 functionPointer, const u32 baseIndex, const u32 parameterCount)
{
    EnsureSize(GetOpCodeSize(SsaOpcode::CallInd) + 3 * sizeof(u32));
    WriteOpcode(SsaOpcode::CallInd);
    WriteT(functionPointer);
    WriteT(baseIndex);
    WriteT(parameterCount);
    m_VarTypeMap.push_back(SsaType::U64);
    return ++m_IdIndex;
}

VarId SsaWriter::WriteCallIndExt(const u32 functionPointer, const u32 baseIndex, const u32 parameterCount, const u32 modulePointer)
{
    EnsureSize(GetOpCodeSize(SsaOpcode::CallIndExt) + 4 * sizeof(u32));
    WriteOpcode(SsaOpcode::CallIndExt);
    WriteT(functionPointer);
    WriteT(baseIndex);
    WriteT(parameterCount);
    WriteT(modulePointer);
    m_VarTypeMap.push_back(SsaType::U64);
    return ++m_IdIndex;
}

void SsaWriter::WriteRet(const SsaCustomType returnType, const VarId var) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::Ret) + returnType.Size() + sizeof(var));
    WriteOpcode(SsaOpcode::Ret);
    WriteType(returnType);
    WriteT(var);
}

VarId SsaWriter::WriteSplit(const SsaCustomType aType, const VarId a, const u32 n, const SsaCustomType* const t) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::Split) + sizeof(a) + sizeof(n) + n * SsaCustomType::MaxSize);
    WriteOpcode(SsaOpcode::Split);
    WriteType(aType);
    WriteT(a);
    WriteT(n);
    for(uSys i = 0; i < n; ++i)
    {
        WriteType(t[i]);
        m_VarTypeMap.push_back(t[i]);
    }

    const VarId ret = m_IdIndex;
    m_IdIndex += n;
    return ret + 1;
}

VarId SsaWriter::WriteJoin(const SsaCustomType outType, const u32 n, const SsaCustomType* const t, const VarId* const v) noexcept
{
    EnsureSize(GetOpCodeSize(SsaOpcode::Join) + sizeof(n) + n * SsaCustomType::MaxSize + n * sizeof(v[0]));
    WriteOpcode(SsaOpcode::Join);
    WriteType(outType);
    WriteT(n);
    for(uSys i = 0; i < n; ++i)
    {
        WriteType(t[i]);
    }
    for(uSys i = 0; i < n; ++i)
    {
        WriteT(v[i]);
    }
    m_VarTypeMap.push_back(outType);
    return ++m_IdIndex;
}

void SsaWriter::WriteRaw(const void* const value, const uSys size) noexcept
{
    EnsureSize(size);
    (void) ::std::memcpy(m_Buffer + m_WriteIndex, value, size);
    m_WriteIndex += size;
}

void SsaWriter::WriteOpcode(const SsaOpcode opcode) noexcept
{
    if(static_cast<u16>(opcode) & 0x8000)
    {
        EnsureSize(2);
        WriteT<u8>(static_cast<u8>(static_cast<u16>(opcode) >> 8));
        WriteT<u8>(static_cast<u8>(static_cast<u16>(opcode) & 0xFF));
    }
    else
    {
        WriteT<u8>(static_cast<u8>(static_cast<u16>(opcode) & 0xFF));
    }
}

void SsaWriter::WriteType(SsaCustomType type) noexcept
{
    WriteT(type.Type);
    const SsaType rawType = StripPointer(type.Type);
    if(rawType == SsaType::Custom || rawType == SsaType::Bytes)
    {
        WriteT<u32>(type.CustomType);
    }
}


SsaFrameTracker::SsaFrameTracker(const uSys localCount) noexcept
    : m_Locals(new(::std::nothrow) VarId[localCount])
    , m_Arguments(new(::std::nothrow) VarId[MAX_ARGUMENT_COUNT])
#ifdef _DEBUG
    , m_LocalCount(localCount)
#endif
{
    for(u32 i = 0; i < MAX_ARGUMENT_COUNT; ++i)
    {
        m_Arguments[i] = i | 0x80000000;
    }

}

SsaFrameTracker::~SsaFrameTracker() noexcept
{
    delete[] m_Locals;
    delete[] m_Arguments;
}

SsaFrameTracker::SsaFrameTracker(SsaFrameTracker&& move) noexcept
    : m_Frame(::std::move(move.m_Frame))
    , m_Locals(move.m_Locals)
    , m_Arguments(move.m_Arguments)
#ifdef _DEBUG
    , m_LocalCount(move.m_LocalCount)
#endif
{
    if(this != &move)
    {
        move.m_Locals = nullptr;
        move.m_Arguments = nullptr;
    }
}

SsaFrameTracker& SsaFrameTracker::operator=(SsaFrameTracker&& move) noexcept
{
    if(this == &move)
    {
        return *this;
    }

    delete[] m_Locals;
    delete[] m_Arguments;

    m_Frame = ::std::move(move.m_Frame);
    m_Locals = move.m_Locals;
    m_Arguments = move.m_Arguments;
#ifdef _DEBUG
    m_LocalCount = move.m_LocalCount;
#endif

    move.m_Locals = nullptr;
    move.m_Arguments = nullptr;

    return *this;
}

void SsaFrameTracker::PushFrame(const VarId var, const uSys size)
{
    m_Frame.emplace_back(var, size);
}

SsaFrameTracker::FrameInfo SsaFrameTracker::PopFrame(const uSys size)
{
    const FrameInfo ret = m_Frame.back();
    m_Frame.pop_back();
    return ret;
}

SsaFrameTracker::FrameInfo SsaFrameTracker::CheckFrame() const
{
    return m_Frame.back();
}

void SsaFrameTracker::SetLocal(const VarId var, const uSys local)
{
#ifdef _DEBUG
    if(local > m_LocalCount)
    {
        throw ::std::invalid_argument("local");
    }
#endif

    m_Locals[local] = var;
}

VarId SsaFrameTracker::GetLocal(const uSys local) const
{
#ifdef _DEBUG
    if(local > m_LocalCount)
    {
        throw ::std::invalid_argument("local");
    }
#endif

    return m_Locals[local];
}

void SsaFrameTracker::SetArgument(const VarId var, const uSys arg)
{
#ifdef _DEBUG
    if(arg > MAX_ARGUMENT_COUNT)
    {
        throw ::std::invalid_argument("local");
    }
#endif

    m_Arguments[arg] = var;
}

VarId SsaFrameTracker::GetArgument(const uSys arg) const
{
#ifdef _DEBUG
    if(arg > MAX_ARGUMENT_COUNT)
    {
        throw ::std::invalid_argument("local");
    }
#endif

    return m_Arguments[arg];
}

}
