#pragma once

#include <NumTypes.hpp>
#include <Objects.hpp>
#include <vector>
#include <String.hpp>

#ifndef TAUIR_DEBUG_TYPES
  #ifdef _DEBUG
    #define TAUIR_DEBUG_TYPES 1
  #else
    #define TAUIR_DEBUG_TYPES 0
  #endif
#endif

namespace tau::ir::ssa {

typedef u32 VarId;

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

constexpr static inline bool IsPointer(const SsaType type) noexcept
{
    return (static_cast<u8>(type) & 0x80) == 0x80;
}

constexpr static inline uSys TypeValueSize(const SsaType type)
{
    if(IsPointer(type))
    {
        return 8;
    }

    switch(type)
    {
        case SsaType::Void: return 0;
        case SsaType::Bool: return 1;
        case SsaType::I8:   return 1;
        case SsaType::I16:  return 2;
        case SsaType::I32:  return 4;
        case SsaType::I64:  return 8;
        case SsaType::U8:   return 1;
        case SsaType::U16:  return 2;
        case SsaType::U32:  return 4;
        case SsaType::U64:  return 8;
        case SsaType::F16:  return 2;
        case SsaType::F32:  return 4;
        case SsaType::F64:  return 8;
        case SsaType::Char: return 1;
        default: return IntMaxMin<uSys>::Max;
    }
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

    [[nodiscard]] uSys Size() const noexcept
    {
        return Type == SsaType::Custom || Type == SsaType::Bytes ? sizeof(Type) + sizeof(CustomType) : sizeof(Type);
    }

    SsaType Type;
    u32 CustomType;

    static inline constexpr uSys MinSize = sizeof(Type);
    static inline constexpr uSys MaxSize = sizeof(Type) + sizeof(CustomType);

    static_assert(sizeof(Type) == 1, "SsaType is not a single byte.");
    static_assert(sizeof(CustomType) == 4, "Custom Type is not a 4 byte u32.");
};

struct SsaCustomTypeDescriptor final
{
    DEFAULT_CONSTRUCT_PU(SsaCustomTypeDescriptor);
    DEFAULT_DESTRUCT(SsaCustomTypeDescriptor);
    DEFAULT_CM_PU(SsaCustomTypeDescriptor);

    SsaCustomTypeDescriptor(const u32 typeId, const u32 size) noexcept
        : TypeId(typeId)
        , Size(size)
    { }

    u32 TypeId;
    u32 Size;
};

struct SsaCustomTypeDebugNode final
{
    DEFAULT_CONSTRUCT_PU(SsaCustomTypeDebugNode);
    DEFAULT_CM_PU(SsaCustomTypeDebugNode);

    SsaCustomTypeDebugNode(const C8DynString& name, const u32 offset, const u32 size) noexcept
        : Name(name)
        , Offset(offset)
        , Size(size)
        , Next(nullptr)
    { }

    SsaCustomTypeDebugNode(C8DynString&& name, const u32 offset, const u32 size) noexcept
        : Name(::std::move(name))
        , Offset(offset)
        , Size(size)
        , Next(nullptr)
    { }

    ~SsaCustomTypeDebugNode() noexcept
    {
	    delete Next;
    }

#if TAUIR_DEBUG_TYPES
    void* operator new(::std::size_t sz) noexcept;
    void operator delete(void* ptr) noexcept;
#endif

    C8DynString Name;
    u32 Offset;
    u32 Size;
    SsaCustomTypeDebugNode* Next;
};

struct SsaCustomTypeDebugDescriptor final
{
    DEFAULT_CONSTRUCT_PU(SsaCustomTypeDebugDescriptor);
    DEFAULT_CM_PU(SsaCustomTypeDebugDescriptor);

    SsaCustomTypeDebugDescriptor(const u32 typeId, const u32 size) noexcept
        : Descriptor(typeId, size)
		, CurrentOffset(0)
        , Head(nullptr)
        , Tail(nullptr)
    { }

    ~SsaCustomTypeDebugDescriptor() noexcept
    {
        delete Head; // This will delete the entire chain.
    }

    [[nodiscard]] operator SsaCustomTypeDescriptor() const noexcept { return Descriptor; }

    SsaCustomTypeDescriptor Descriptor;
    C8DynString Name;
    u32 CurrentOffset;
    SsaCustomTypeDebugNode* Head;
    SsaCustomTypeDebugNode* Tail;
};

class SsaCustomTypeRegistry final
{
    DEFAULT_CONSTRUCT_PU(SsaCustomTypeRegistry);
    DEFAULT_DESTRUCT(SsaCustomTypeRegistry);
    DEFAULT_CM_PU(SsaCustomTypeRegistry);
public:
#if TAUIR_DEBUG_TYPES
    using TypeEntry = SsaCustomTypeDebugDescriptor;
#else
    using TypeEntry = SsaCustomTypeDescriptor;
#endif
public:
    u32 RegisterType(const u32 size) noexcept
    {
        const u32 ret = static_cast<u32>(m_TypeMap.size());
        m_TypeMap.emplace_back(ret, size);
        return ret;
    }

    [[nodiscard]] SsaCustomTypeDescriptor operator[](const u32 typeId) const noexcept
    {
        return m_TypeMap[typeId];
    }

#if TAUIR_DEBUG_TYPES
    void AttachDebugName(const u32 typeId, const C8DynString& name) noexcept
    {
        m_TypeMap[typeId].Name = name;
    }

    void AttachDebugName(const u32 typeId, C8DynString&& name) noexcept
    {
        m_TypeMap[typeId].Name = ::std::move(name);
    }

    void AttachDebugElement(const u32 typeId, const C8DynString& name, const u32 size) noexcept
    {
        TypeEntry& entry = m_TypeMap[typeId];

        SsaCustomTypeDebugNode* node = new SsaCustomTypeDebugNode(name, entry.CurrentOffset, size);
        entry.CurrentOffset += size;
        if(entry.Tail == nullptr)
        {
            entry.Head = node;
            entry.Tail = node;
        }
        else
        {
            entry.Tail->Next = node;
            entry.Tail = node;
        }
    }
#endif
private:
    ::std::vector<TypeEntry> m_TypeMap;
};

}
