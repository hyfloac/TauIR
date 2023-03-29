#include "TauIR/IrWriter.hpp"
#include <TUMaths.hpp>
#include <new>
#include <cstring>

namespace tau::ir {

IrWriter::IrWriter(uSys initialBufferSize) noexcept
    : m_Buffer(new(::std::nothrow) u8[maxT(64, initialBufferSize)])
    , m_BufferSize(maxT(64, initialBufferSize))
    , m_WriteIndex(0)
{ }

IrWriter::~IrWriter() noexcept
{
    delete[] m_Buffer;
}

IrWriter::IrWriter(IrWriter&& move) noexcept
    : m_Buffer(move.m_Buffer)
    , m_BufferSize(move.m_BufferSize)
    , m_WriteIndex(move.m_WriteIndex)
{
    if(this != &move)
    {
        move.m_Buffer = nullptr;
    }
}

IrWriter& IrWriter::operator=(IrWriter&& move) noexcept
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

    move.m_Buffer = nullptr;

    return *this;
}

void IrWriter::EnsureSize(uSys additionalSize) noexcept
{
    if(m_WriteIndex + additionalSize <= m_BufferSize)
    {
        return;
    }

    const uSys newSize = maxT(m_BufferSize + (m_BufferSize >> 1), m_WriteIndex + additionalSize);
    u8* const newBuffer = new(::std::nothrow) u8[newSize];
    (void) ::std::memcpy(newBuffer, m_Buffer, sizeof(newBuffer[0]) * m_WriteIndex);
    m_BufferSize = newSize;
    delete[] m_Buffer;
    m_Buffer = newBuffer;
}

void IrWriter::WriteNop() noexcept
{
    WriteOpcode(Opcode::Nop);
}

void IrWriter::WritePush(const u16 localIndex) noexcept
{
    switch(localIndex)
    {
        case 0: WriteOpcode(Opcode::Push0); break;
        case 1: WriteOpcode(Opcode::Push1); break;
        case 2: WriteOpcode(Opcode::Push2); break;
        case 3: WriteOpcode(Opcode::Push3); break;
        default:
            WriteOpcode(Opcode::PushN);
            WriteT(localIndex);
            break;
    }
}

void IrWriter::WritePushArg(const u16 argumentIndex) noexcept
{
    switch(argumentIndex)
    {
        case 0: WriteOpcode(Opcode::PushArg0); break;
        case 1: WriteOpcode(Opcode::PushArg1); break;
        case 2: WriteOpcode(Opcode::PushArg2); break;
        case 3: WriteOpcode(Opcode::PushArg3); break;
        default:
            WriteOpcode(Opcode::PushArgN);
            WriteT(argumentIndex);
            break;
    }
}

void IrWriter::WritePushPtr(const u16 localIndex) noexcept
{
    WriteOpcode(Opcode::PushPtr);
    WriteT(localIndex);
}

void IrWriter::WritePushGlobal(const u32 globalIndex) noexcept
{
    WriteOpcode(Opcode::PushGlobal);
    WriteT(globalIndex);
}

void IrWriter::WritePushGlobalExt(const u32 globalIndex, const u16 moduleIndex) noexcept
{
    WriteOpcode(Opcode::PushGlobalExt);
    WriteT(globalIndex);
    WriteT(moduleIndex);
}

void IrWriter::WritePushGlobalPtr(const u32 globalIndex) noexcept
{
    WriteOpcode(Opcode::PushGlobalPtr);
    WriteT(globalIndex);
}

void IrWriter::WritePushGlobalExtPtr(const u32 globalIndex, const u16 moduleIndex) noexcept
{
    WriteOpcode(Opcode::PushGlobalExtPtr);
    WriteT(globalIndex);
    WriteT(moduleIndex);
}

void IrWriter::WritePop(const u16 localIndex) noexcept
{
    switch(localIndex)
    {
        case 0: WriteOpcode(Opcode::Pop0); break;
        case 1: WriteOpcode(Opcode::Pop1); break;
        case 2: WriteOpcode(Opcode::Pop2); break;
        case 3: WriteOpcode(Opcode::Pop3); break;
        default:
            WriteOpcode(Opcode::PopN);
            WriteT(localIndex);
            break;
    }
}

void IrWriter::WritePopArg(const u16 argumentIndex) noexcept
{
    switch(argumentIndex)
    {
        case 0: WriteOpcode(Opcode::PopArg0); break;
        case 1: WriteOpcode(Opcode::PopArg1); break;
        case 2: WriteOpcode(Opcode::PopArg2); break;
        case 3: WriteOpcode(Opcode::PopArg3); break;
        default:
            WriteOpcode(Opcode::PopArgN);
            WriteT(argumentIndex);
            break;
    }
}

void IrWriter::WritePopPtr(const u16 localIndex) noexcept
{
    WriteOpcode(Opcode::PopPtr);
    WriteT(localIndex);
}

void IrWriter::WritePopGlobal(const u32 globalIndex) noexcept
{
    WriteOpcode(Opcode::PopGlobal);
    WriteT(globalIndex);
}

void IrWriter::WritePopGlobalExt(const u32 globalIndex, const u16 moduleIndex) noexcept
{
    WriteOpcode(Opcode::PopGlobalExt);
    WriteT(globalIndex);
    WriteT(moduleIndex);
}

void IrWriter::WritePopGlobalPtr(const u32 globalIndex) noexcept
{
    WriteOpcode(Opcode::PopGlobalPtr);
    WriteT(globalIndex);
}

void IrWriter::WritePopGlobalExtPtr(const u32 globalIndex, const u16 moduleIndex) noexcept
{
    WriteOpcode(Opcode::PopGlobalExtPtr);
    WriteT(globalIndex);
    WriteT(moduleIndex);
}

void IrWriter::WriteDup(const uSys byteCount) noexcept
{
    switch(byteCount)
    {
        case 1: WriteOpcode(Opcode::Dup1); break;
        case 2: WriteOpcode(Opcode::Dup2); break;
        case 4: WriteOpcode(Opcode::Dup4); break;
        case 8: WriteOpcode(Opcode::Dup8); break;
        default: break;
    }
}

void IrWriter::WriteExpandSX(const uSys fromSize, const uSys toSize) noexcept
{
    switch(fromSize)
    {
        case 1:
            switch(toSize)
            {
                case 2: WriteOpcode(Opcode::ExpandSX12); break;
                case 4: WriteOpcode(Opcode::ExpandSX14); break;
                case 8: WriteOpcode(Opcode::ExpandSX18); break;
                default: break;
            }
            break;
        case 2:
            switch(toSize)
            {
                case 4: WriteOpcode(Opcode::ExpandSX24); break;
                case 8: WriteOpcode(Opcode::ExpandSX28); break;
                default: break;
            }
            break;
        case 4:
            switch(toSize)
            {
                case 8: WriteOpcode(Opcode::ExpandSX48); break;
                default: break;
            }
            break;
        default: break;
    }
}

void IrWriter::WriteExpandZX(const uSys fromSize, const uSys toSize) noexcept
{
    switch(fromSize)
    {
        case 1:
            switch(toSize)
            {
                case 2: WriteOpcode(Opcode::ExpandZX12); break;
                case 4: WriteOpcode(Opcode::ExpandZX14); break;
                case 8: WriteOpcode(Opcode::ExpandZX18); break;
                default: break;
            }
            break;
        case 2:
            switch(toSize)
            {
                case 4: WriteOpcode(Opcode::ExpandZX24); break;
                case 8: WriteOpcode(Opcode::ExpandZX28); break;
                default: break;
            }
            break;
        case 4:
            switch(toSize)
            {
                case 8: WriteOpcode(Opcode::ExpandZX48); break;
                default: break;
            }
            break;
        default: break;
    }
}

void IrWriter::WriteTrunc(const uSys fromSize, const uSys toSize) noexcept
{
    switch(fromSize)
    {
        case 2:
            switch(toSize)
            {
                case 1: WriteOpcode(Opcode::Trunc21); break;
                default: break;
            }
            break;
        case 4:
            switch(toSize)
            {
                case 1: WriteOpcode(Opcode::Trunc41); break;
                case 2: WriteOpcode(Opcode::Trunc42); break;
                default: break;
            }
            break;
        case 8:
            switch(toSize)
            {
                case 1: WriteOpcode(Opcode::Trunc81); break;
                case 2: WriteOpcode(Opcode::Trunc82); break;
                case 4: WriteOpcode(Opcode::Trunc84); break;
                default: break;
            }
            break;
        default: break;
    }
}

void IrWriter::WriteLoad(const u16 valueLocalIndex, const u16 pointerLocalIndex) noexcept
{
    WriteOpcode(Opcode::Load);
    WriteT(valueLocalIndex);
    WriteT(pointerLocalIndex);
}

void IrWriter::WriteLoadGlobal(const u32 valueGlobalIndex, const u16 pointerLocalIndex) noexcept
{
    WriteOpcode(Opcode::LoadGlobal);
    WriteT(valueGlobalIndex);
    WriteT(pointerLocalIndex);
}

void IrWriter::WriteLoadGlobalExt(const u32 valueGlobalIndex, const u16 pointerLocalIndex, const u16 moduleIndex) noexcept
{
    WriteOpcode(Opcode::LoadGlobal);
    WriteT(valueGlobalIndex);
    WriteT(pointerLocalIndex);
    WriteT(moduleIndex);
}

void IrWriter::WriteStore(const u16 pointerLocalIndex, const u16 valueLocalIndex) noexcept
{
    WriteOpcode(Opcode::StoreGlobalExt);
    WriteT(pointerLocalIndex);
    WriteT(valueLocalIndex);
}

void IrWriter::WriteStoreGlobal(const u16 pointerLocalIndex, const u32 valueGlobalIndex) noexcept
{
    WriteOpcode(Opcode::StoreGlobal);
    WriteT(pointerLocalIndex);
    WriteT(valueGlobalIndex);
}

void IrWriter::WriteStoreGlobalExt(const u16 pointerLocalIndex, const u32 valueGlobalIndex, const u16 moduleIndex) noexcept
{
    WriteOpcode(Opcode::StoreGlobalExt);
    WriteT(pointerLocalIndex);
    WriteT(valueGlobalIndex);
    WriteT(moduleIndex);
}

void IrWriter::WriteConstant(const u32 constant) noexcept
{
    switch(constant)
    {
        case 0: WriteOpcode(Opcode::Const0); break;
        case 1: WriteOpcode(Opcode::Const1); break;
        case 2: WriteOpcode(Opcode::Const2); break;
        case 3: WriteOpcode(Opcode::Const3); break;
        case 4: WriteOpcode(Opcode::Const4); break;
        case 0x7FFFFFFF: WriteOpcode(Opcode::Const7F); break;
        case 0xFFFFFFFF: WriteOpcode(Opcode::ConstFF); break;
        default:
            WriteOpcode(Opcode::ConstN);
            WriteT(constant);
    }
}

void IrWriter::WriteAddI32() noexcept
{
    WriteOpcode(Opcode::AddI32);
}

void IrWriter::WriteAddI64() noexcept
{
    WriteOpcode(Opcode::AddI64);
}

void IrWriter::WriteSubI32() noexcept
{
    WriteOpcode(Opcode::SubI32);
}

void IrWriter::WriteSubI64() noexcept
{
    WriteOpcode(Opcode::SubI64);
}

void IrWriter::WriteMulI32() noexcept
{
    WriteOpcode(Opcode::MulI32);
}

void IrWriter::WriteMulI64() noexcept
{
    WriteOpcode(Opcode::MulI64);
}

void IrWriter::WriteDivI32() noexcept
{
    WriteOpcode(Opcode::DivI32);
}

void IrWriter::WriteDivI64() noexcept
{
    WriteOpcode(Opcode::DivI64);
}

void IrWriter::WriteCompI32(const CompareCondition cond) noexcept
{
    switch(cond)
    {
        case CompareCondition::Above:          WriteOpcode(Opcode::CompI32Above); break;
        case CompareCondition::AboveOrEqual:   WriteOpcode(Opcode::CompI32AboveOrEqual); break;
        case CompareCondition::Below:          WriteOpcode(Opcode::CompI32Below); break;
        case CompareCondition::BelowOrEqual:   WriteOpcode(Opcode::CompI32BelowOrEqual); break;
        case CompareCondition::Equal:          WriteOpcode(Opcode::CompI32Equal); break;
        case CompareCondition::Greater:        WriteOpcode(Opcode::CompI32Greater); break;
        case CompareCondition::GreaterOrEqual: WriteOpcode(Opcode::CompI32GreaterOrEqual); break;
        case CompareCondition::Less:           WriteOpcode(Opcode::CompI32Less); break;
        case CompareCondition::LessOrEqual:    WriteOpcode(Opcode::CompI32LessOrEqual); break;
        case CompareCondition::NotEqual:       WriteOpcode(Opcode::CompI32NotEqual); break;
        default: break;
    }
}

void IrWriter::WriteCompI64(const CompareCondition cond) noexcept
{
    switch(cond)
    {
        case CompareCondition::Above:          WriteOpcode(Opcode::CompI64Above); break;
        case CompareCondition::AboveOrEqual:   WriteOpcode(Opcode::CompI64AboveOrEqual); break;
        case CompareCondition::Below:          WriteOpcode(Opcode::CompI64Below); break;
        case CompareCondition::BelowOrEqual:   WriteOpcode(Opcode::CompI64BelowOrEqual); break;
        case CompareCondition::Equal:          WriteOpcode(Opcode::CompI64Equal); break;
        case CompareCondition::Greater:        WriteOpcode(Opcode::CompI64Greater); break;
        case CompareCondition::GreaterOrEqual: WriteOpcode(Opcode::CompI64GreaterOrEqual); break;
        case CompareCondition::Less:           WriteOpcode(Opcode::CompI64Less); break;
        case CompareCondition::LessOrEqual:    WriteOpcode(Opcode::CompI64LessOrEqual); break;
        case CompareCondition::NotEqual:       WriteOpcode(Opcode::CompI64NotEqual); break;
        default: break;
    }
}

void IrWriter::WriteCall(const u32 functionIndex) noexcept
{
    WriteOpcode(Opcode::Call);
    WriteT(functionIndex);
}

void IrWriter::WriteCallExt(const u32 functionIndex, const u16 moduleIndex) noexcept
{
    WriteOpcode(Opcode::CallExt);
    WriteT(functionIndex);
    WriteT(moduleIndex);
}

void IrWriter::WriteCallInd(const u32 functionPointerIndex) noexcept
{
    WriteOpcode(Opcode::CallInd);
    WriteT(functionPointerIndex);
}

void IrWriter::WriteCallIndExt(const u32 functionPointerIndex, const u16 moduleIndex) noexcept
{
    WriteOpcode(Opcode::CallIndExt);
    WriteT(functionPointerIndex);
    WriteT(moduleIndex);
}

void IrWriter::WriteRet() noexcept
{
    WriteOpcode(Opcode::Ret);
}

void IrWriter::WriteJump(const i32 offset) noexcept
{
    WriteOpcode(Opcode::Jump);
    WriteT(offset);
}

void IrWriter::WriteJumpTrue(const i32 offset) noexcept
{
    WriteOpcode(Opcode::JumpTrue);
    WriteT(offset);
}

void IrWriter::WriteJumpFalse(const i32 offset) noexcept
{
    WriteOpcode(Opcode::JumpFalse);
    WriteT(offset);
}

void IrWriter::WriteRaw(const void* const value, const uSys size) noexcept
{
    EnsureSize(size);
    (void) ::std::memcpy(m_Buffer + m_WriteIndex, value, size);
    m_WriteIndex += size;
}

void IrWriter::WriteOpcode(const Opcode opcode) noexcept
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

}
