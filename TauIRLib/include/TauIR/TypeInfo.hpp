#pragma once

#include <NumTypes.hpp>
#include <String.hpp>
#include "Common.hpp"

namespace tau::ir {

union TypeInfoFlags final
{
    DEFAULT_DESTRUCT(TypeInfoFlags);
    DEFAULT_CM_PU(TypeInfoFlags);

    u32 Packed;
    struct
    {
        u32 IsValueType : 1;
        u32 IsObject : 1;
        u32 IsFunction : 1;
        u32 IsIntegral : 1;
        u32 IsBinaryFloatingPoint : 1;
        u32 IsDecimalFloatingPoint : 1;
        u32 IsCharacter : 1;
        u32 IsSigned : 1;
        u32 Unused : 24;
    };

    constexpr TypeInfoFlags() noexcept
        : Packed(0)
    { }

    constexpr TypeInfoFlags(
        const bool isValueType,
        const bool isObject,
        const bool isFunction,
        const bool isIntegral,
        const bool isBinaryFloatingPoint,
        const bool isDecimalFloatingPoint,
        const bool isCharacter,
        const bool isSigned
    ) noexcept
        : IsValueType(isValueType)
        , IsObject(isObject)
        , IsFunction(isFunction)
        , IsIntegral(isIntegral)
        , IsBinaryFloatingPoint(isBinaryFloatingPoint)
        , IsDecimalFloatingPoint(isDecimalFloatingPoint)
        , IsCharacter(isCharacter)
        , IsSigned(isSigned)
        , Unused(0)
    { }

    [[nodiscard]] static constexpr TypeInfoFlags            Void() noexcept { return TypeInfoFlags(false, false, false, false, false, false, false, false); }
    [[nodiscard]] static constexpr TypeInfoFlags          Object() noexcept { return TypeInfoFlags(false,  true, false, false, false, false, false, false); }
    [[nodiscard]] static constexpr TypeInfoFlags        Function() noexcept { return TypeInfoFlags(false, false,  true, false, false, false, false, false); }
    [[nodiscard]] static constexpr TypeInfoFlags   SignedInteger() noexcept { return TypeInfoFlags( true, false, false,  true, false, false, false,  true); }
    [[nodiscard]] static constexpr TypeInfoFlags UnsignedInteger() noexcept { return TypeInfoFlags( true, false, false,  true, false, false, false, false); }
    [[nodiscard]] static constexpr TypeInfoFlags           Float() noexcept { return TypeInfoFlags( true, false, false, false,  true, false, false,  true); }
    [[nodiscard]] static constexpr TypeInfoFlags         Decimal() noexcept { return TypeInfoFlags( true, false, false, false, false,  true, false,  true); }
    [[nodiscard]] static constexpr TypeInfoFlags            Char() noexcept { return TypeInfoFlags( true, false, false,  true, false, false,  true, false); }
};

class TypeInfo final
{
    DEFAULT_DESTRUCT(TypeInfo);
    DEFAULT_CM_PU(TypeInfo);
public:
    static const TypeInfo Void;
    static const TypeInfo Bool;
    static const TypeInfo I8;
    static const TypeInfo I16;
    static const TypeInfo I32;
    static const TypeInfo I64;
    static const TypeInfo U8;
    static const TypeInfo U16;
    static const TypeInfo U32;
    static const TypeInfo U64;
    static const TypeInfo F32;
    static const TypeInfo F64;
    static const TypeInfo Char;
public:
    TypeInfo(const uSys size, const TypeInfoFlags flags, const C8DynString& name) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Flags(flags)
        , m_Name(name)
    { }
    
    TypeInfo(const uSys size, const TypeInfoFlags flags, C8DynString&& name) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Flags(flags)
        , m_Name(::std::move(name))
    { }

    TypeInfo(const uSys size, const TypeInfoFlags flags, const c8* const name) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Flags(flags)
        , m_Name(name)
    { }

    TypeInfo(const uSys size, const TypeInfoFlags flags, const char* const name) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Flags(flags)
        , m_Name(reinterpret_cast<const char8_t*>(name))
    { }

    TypeInfo(const uSys size, const TypeInfoFlags flags) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Flags(flags)
    { }

    [[nodiscard]] uSys Size() const noexcept { return m_Size; }
    [[nodiscard]] uSys   Id() const noexcept { return m_Id;   }

    [[nodiscard]] TypeInfoFlags Flags() const noexcept { return m_Flags; }

    [[nodiscard]] const C8DynString& Name()     const noexcept { return m_Name; }
    [[nodiscard]] operator const C8DynString&() const noexcept { return m_Name; }
public:
    void* operator new(::std::size_t sz) noexcept;
    void operator delete(void* ptr) noexcept;

    [[nodiscard]] static bool IsPointer(const TypeInfo* typeInfo) noexcept { return CheckPointerTag<1>(typeInfo); }
    
    [[nodiscard]] static       TypeInfo* StripPointer(      TypeInfo* typeInfo) noexcept { return UnTagPointer<7>(typeInfo); }
    [[nodiscard]] static const TypeInfo* StripPointer(const TypeInfo* typeInfo) noexcept { return StripPointer(const_cast<TypeInfo*>(typeInfo)); }
    
    [[nodiscard]] static       TypeInfo* AddPointer(      TypeInfo* typeInfo) noexcept { return TagPointer<1>(typeInfo); }
    [[nodiscard]] static const TypeInfo* AddPointer(const TypeInfo* typeInfo) noexcept { return AddPointer(const_cast<TypeInfo*>(typeInfo)); }
    
    [[nodiscard]] static       TypeInfo* SetPointer(      TypeInfo* typeInfo, bool isPointer) noexcept;
    [[nodiscard]] static const TypeInfo* SetPointer(const TypeInfo* typeInfo, bool isPointer) noexcept;

    template<bool IsPointer>
    [[nodiscard]] static       TypeInfo* SetPointer(      TypeInfo* typeInfo) noexcept;

    template<bool IsPointer>
    [[nodiscard]] static const TypeInfo* SetPointer(const TypeInfo* typeInfo) noexcept;
private:
    static uSys GenerateId() noexcept;
private:
    uSys m_Size;
    uSys m_Id;
    TypeInfoFlags m_Flags;
    C8DynString m_Name;
};

inline TypeInfo* TypeInfo::SetPointer(TypeInfo* typeInfo, const bool isPointer) noexcept
{
    return isPointer ? AddPointer(typeInfo) : StripPointer(typeInfo);
}

inline const TypeInfo* TypeInfo::SetPointer(const TypeInfo* typeInfo, const bool isPointer) noexcept
{
    return SetPointer(const_cast<TypeInfo*>(typeInfo), isPointer);
}

template<bool IsPointer>
inline TypeInfo* TypeInfo::SetPointer(TypeInfo* typeInfo) noexcept
{
    if constexpr(IsPointer)
    {
        return AddPointer(typeInfo);
    }
    else
    {
        return StripPointer(typeInfo);
    }
}

template<bool IsPointer>
inline const TypeInfo* TypeInfo::SetPointer(const TypeInfo* typeInfo) noexcept
{
    return SetPointer<IsPointer>(const_cast<TypeInfo*>(typeInfo));
}

}

inline bool operator ==(const tau::ir::TypeInfo& left, const tau::ir::TypeInfo& right) noexcept
{
    return left.Id() == right.Id();
}

inline bool operator !=(const tau::ir::TypeInfo& left, const tau::ir::TypeInfo& right) noexcept
{
    return !(left == right);
}
