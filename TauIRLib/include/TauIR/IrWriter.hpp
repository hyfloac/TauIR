#pragma once

#include <Objects.hpp>
#include <NumTypes.hpp>

#include "TauIR/Opcodes.hpp"

namespace tau::ir {


class IrWriter
{
    DELETE_COPY(IrWriter);
public:
    IrWriter(uSys initialBufferSize = 64) noexcept;
    ~IrWriter() noexcept;

    IrWriter(IrWriter&& move) noexcept;
    IrWriter& operator=(IrWriter&& move) noexcept;

    void WriteNop() noexcept;
    void WritePush(u16 localIndex) noexcept;
    void WritePushArg(u16 argumentIndex) noexcept;
    void WritePushPtr(u16 localIndex) noexcept;
    void WritePushGlobal(u32 globalIndex) noexcept;
    void WritePushGlobalExt(u32 globalIndex, u16 moduleIndex) noexcept;
    void WritePushGlobalPtr(u32 globalIndex) noexcept;
    void WritePushGlobalExtPtr(u32 globalIndex, u16 moduleIndex) noexcept;
    void WritePop(u16 localIndex) noexcept;
    void WritePopArg(u16 argumentIndex) noexcept;
    void WritePopPtr(u16 localIndex) noexcept;
    void WritePopGlobal(u32 globalIndex) noexcept;
    void WritePopGlobalExt(u32 globalIndex, u16 moduleIndex) noexcept;
    void WritePopGlobalPtr(u32 globalIndex) noexcept;
    void WritePopGlobalExtPtr(u32 globalIndex, u16 moduleIndex) noexcept;
    void WriteDup(uSys byteCount) noexcept;
    void WriteExpandSX(uSys fromSize, uSys toSize) noexcept;
    void WriteExpandZX(uSys fromSize, uSys toSize) noexcept;
    void WriteTrunc(uSys fromSize, uSys toSize) noexcept;
    void WriteLoad(u16 valueLocalIndex, u16 pointerLocalIndex) noexcept;
    void WriteLoadGlobal(u32 valueGlobalIndex, u16 pointerLocalIndex) noexcept;
    void WriteLoadGlobalExt(u32 valueGlobalIndex, u16 pointerLocalIndex, u16 moduleIndex) noexcept;
    void WriteStore(u16 pointerLocalIndex, u16 valueLocalIndex) noexcept;
    void WriteStoreGlobal(u16 pointerLocalIndex, u32 valueGlobalIndex) noexcept;
    void WriteStoreGlobalExt(u16 pointerLocalIndex, u32 valueGlobalIndex, u16 moduleIndex) noexcept;
    void WriteConstant(u32 constant) noexcept;
    void WriteAddI32() noexcept;
    void WriteAddI64() noexcept;
    void WriteSubI32() noexcept;
    void WriteSubI64() noexcept;
    void WriteMulI32() noexcept;
    void WriteMulI64() noexcept;
    void WriteDivI32() noexcept;
    void WriteDivI64() noexcept;
    void WriteCompI32(CompareCondition cond) noexcept;
    void WriteCompI64(CompareCondition cond) noexcept;
    void WriteCall(u32 functionIndex) noexcept;
    void WriteCallExt(u32 functionIndex, u16 moduleIndex) noexcept;
    void WriteCallInd(u32 functionPointerIndex) noexcept;
    void WriteCallIndExt(u32 functionPointerIndex, u16 moduleIndex) noexcept;
    void WriteRet() noexcept;
    void WriteJump(i32 offset) noexcept;
    void WriteJumpTrue(i32 offset) noexcept;
    void WriteJumpFalse(i32 offset) noexcept;

    [[nodiscard]] const u8* Buffer() const noexcept { return m_Buffer; }
    [[nodiscard]] uSys Size() const noexcept { return m_WriteIndex; }
private:
    void EnsureSize(uSys additionalSize) noexcept;

    void WriteRaw(const void* value, uSys size) noexcept;

    template<typename T>
    void WriteT(const T value) noexcept
    {
        WriteRaw(&value, sizeof(T));
    }

    void WriteOpcode(Opcode opcode) noexcept;
private:
    u8* m_Buffer;
    uSys m_BufferSize;
    uSys m_WriteIndex;
};

}
