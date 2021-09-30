#pragma once

#include <NumTypes.hpp>
#include <DynArray.hpp>
#include <TUMaths.hpp>

namespace tau::ir {

class TypeInfo;

class Function final
{
    DEFAULT_DESTRUCT(Function);
    DEFAULT_CM_PU(Function);
public:
    Function(const uPtr address, const DynArray<const TypeInfo*>& localTypes) noexcept
        : m_Address(address)
        , m_LocalSize(0)
        , m_LocalTypes(localTypes)
        , m_LocalOffsets(maxT(m_LocalTypes.count() - 1, 0))
    { LoadLocalOffsets(); }
    
    Function(const uPtr address, DynArray<const TypeInfo*>&& localTypes) noexcept
        : m_Address(address)
        , m_LocalSize(0)
        , m_LocalTypes(::std::move(localTypes))
        , m_LocalOffsets(maxT(m_LocalTypes.count() - 1, 0))
    { LoadLocalOffsets(); }

    [[nodiscard]] uSys   Address() const noexcept { return m_Address;   }
    [[nodiscard]] uSys LocalSize() const noexcept { return m_LocalSize; }
    [[nodiscard]] const DynArray<const TypeInfo*>& LocalTypes() const noexcept { return m_LocalTypes;   }
    [[nodiscard]] const DynArray<uSys>&          LocalOffsets() const noexcept { return m_LocalOffsets; }
public: 
    [[nodiscard]] void* operator new(::std::size_t sz) noexcept;
    void operator delete(void* ptr) noexcept;
private:
    void LoadLocalOffsets() noexcept;
private:
    uPtr m_Address;
    uSys m_LocalSize;
    DynArray<const TypeInfo*> m_LocalTypes;
    DynArray<uSys> m_LocalOffsets;
};

}
