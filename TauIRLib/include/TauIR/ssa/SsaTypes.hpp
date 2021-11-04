#pragma once

#include <NumTypes.hpp>
#include <Objects.hpp>

namespace tau::ir::ssa {

enum class SsaType : u8
{
    Void   = 0x00,
    Bool   = 0x01,
    I8     = 0x02,
    I16    = 0x03,
    I32    = 0x04,
    I64    = 0x05,
    U8     = 0x06,
    U16    = 0x07,
    U32    = 0x08,
    U64    = 0x09,
    F16    = 0x0A,
    F32    = 0x0B,
    F64    = 0x0C,
    Char   = 0x0D,
    Bytes  = 0x7E,
    Custom = 0x7F
};

constexpr static inline SsaType StripPointer(const SsaType type) noexcept
{
    return static_cast<SsaType>(static_cast<u8>(type) & 0x7F);
}

constexpr static inline SsaType AddPointer(const SsaType type) noexcept
{
    return static_cast<SsaType>(static_cast<u8>(type) | 0x80);
}

constexpr static inline SsaType SetPointer(const SsaType type, const bool isPointer) noexcept
{
    return isPointer ? AddPointer(type) : StripPointer(type);
}

struct SsaCustomType final
{
    DEFAULT_CONSTRUCT_PU(SsaCustomType);
    DEFAULT_DESTRUCT(SsaCustomType);
    DEFAULT_CM_PU(SsaCustomType);

    SsaCustomType(SsaType type, u32 customType = -1) noexcept
        : Type(type)
        , CustomType(customType)
    { }

    SsaType Type;
    u32 CustomType;

    [[nodiscard]] uSys Size() const noexcept
    {
        return Type == SsaType::Custom ? sizeof(Type) + sizeof(CustomType) : sizeof(Type);
    }

    static inline constexpr uSys MinSize = sizeof(Type);
    static inline constexpr uSys MaxSize = sizeof(Type) + sizeof(CustomType);

    static_assert(sizeof(Type) == 1, "SsaType is not a single byte.");
    static_assert(sizeof(CustomType) == 4, "Custom Type is not a 4 byte u32.");
};

}
