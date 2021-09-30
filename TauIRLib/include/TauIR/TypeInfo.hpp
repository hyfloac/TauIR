#pragma once

#include <NumTypes.hpp>
#include <String.hpp>

namespace tau::ir {

class TypeInfo final
{
    DEFAULT_DESTRUCT(TypeInfo);
    DEFAULT_CM_PU(TypeInfo);
public:
    TypeInfo(const uSys size, const DynString& name) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Name(name)
    { }
    
    TypeInfo(const uSys size, DynString&& name) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Name(::std::move(name))
    { }

    TypeInfo(const uSys size, const char* name) noexcept
        : m_Size(size)
        , m_Id(GenerateId())
        , m_Name(name)
    { }

    [[nodiscard]] uSys Size() const noexcept { return m_Size; }
    [[nodiscard]] uSys   Id() const noexcept { return m_Id;   }

    [[nodiscard]] const DynString& Name()     const noexcept { return m_Name; }
    [[nodiscard]] operator const DynString&() const noexcept { return m_Name; }
public:
    void* operator new(::std::size_t sz) noexcept;
    void operator delete(void* ptr) noexcept;
private:
    static uSys GenerateId() noexcept;
private:
    uSys m_Size;
    uSys m_Id;
    DynString m_Name;
};

}


static bool operator ==(const tau::ir::TypeInfo& left, const tau::ir::TypeInfo& right) noexcept
{
    return left.Id() == right.Id();
}

static bool operator !=(const tau::ir::TypeInfo& left, const tau::ir::TypeInfo& right) noexcept
{
    return !(left == right);
}
