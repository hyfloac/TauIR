#pragma once

#include <NumTypes.hpp>
#include <String.hpp>

namespace tau::ir {

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
    TypeInfo(const uSys size, const C8DynString& name) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Name(name)
    { }
    
    TypeInfo(const uSys size, C8DynString&& name) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Name(::std::move(name))
    { }

    TypeInfo(const uSys size, const c8* name) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Name(name)
    { }

    [[nodiscard]] uSys Size() const noexcept { return m_Size; }
    [[nodiscard]] uSys   Id() const noexcept { return m_Id;   }

    [[nodiscard]] const C8DynString& Name()     const noexcept { return m_Name; }
    [[nodiscard]] operator const C8DynString&() const noexcept { return m_Name; }
public:
    void* operator new(::std::size_t sz) noexcept;
    void operator delete(void* ptr) noexcept;

    [[nodiscard]] static bool IsPointer(const TypeInfo* typeInfo) noexcept;
    
    [[nodiscard]] static       TypeInfo* StripPointer(      TypeInfo* typeInfo) noexcept;
    [[nodiscard]] static const TypeInfo* StripPointer(const TypeInfo* typeInfo) noexcept;
    
    [[nodiscard]] static       TypeInfo* AddPointer(      TypeInfo* typeInfo) noexcept;
    [[nodiscard]] static const TypeInfo* AddPointer(const TypeInfo* typeInfo) noexcept;
    
    [[nodiscard]] static       TypeInfo* SetPointer(      TypeInfo* typeInfo, bool isPointer) noexcept;
    [[nodiscard]] static const TypeInfo* SetPointer(const TypeInfo* typeInfo, bool isPointer) noexcept;
private:
    static uSys GenerateId() noexcept;
private:
    uSys m_Size;
    uSys m_Id;
    C8DynString m_Name;
};

}

static inline bool operator ==(const tau::ir::TypeInfo& left, const tau::ir::TypeInfo& right) noexcept
{
    return left.Id() == right.Id();
}

static inline bool operator !=(const tau::ir::TypeInfo& left, const tau::ir::TypeInfo& right) noexcept
{
    return !(left == right);
}
