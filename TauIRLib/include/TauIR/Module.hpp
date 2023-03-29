#pragma once

#include <NumTypes.hpp>
#include <DynArray.hpp>
#include <String.hpp>

#include "Common.hpp"

namespace tau::ir {

class Function;

using FunctionList = DynArray<Function*>;

class Module final
{
    DELETE_COPY(Module);
    DEFAULT_MOVE_PU(Module);
public:
    Module(FunctionList&& functions, const bool isNative, C8DynString&& name) noexcept
        : m_Id(GenerateId())
        , m_Functions(::std::move(functions))
        , m_IsNative(isNative)
        , m_Name(::std::move(name))
    { }

    ~Module() noexcept;

    [[nodiscard]] uSys Id() const noexcept { return m_Id; }
    [[nodiscard]] const FunctionList& Functions() const noexcept { return m_Functions; }
    [[nodiscard]] bool IsNative() const noexcept { return m_IsNative; }
    [[nodiscard]] const C8DynString& Name() const noexcept { return m_Name; }
    [[nodiscard]]       C8DynString& Name()       noexcept { return m_Name; }

    void AttachModuleReference(const ModuleRef& module) noexcept;
private:
    static uSys GenerateId() noexcept;
private:
    uSys m_Id;
    FunctionList m_Functions;
    bool m_IsNative;
    C8DynString m_Name;
};

class ModuleBuilder final
{
public:
    ModuleBuilder() noexcept
        : m_FunctionsRaw { }
        , m_Functions(nullptr)
        , m_IsNative(false)
        , m_Name()
    { }

    ModuleBuilder& Functions(FunctionList&& functions) noexcept
    {
        if(m_Functions)
        {
            m_Functions->~FunctionList();
        }

        m_Functions = ::new(m_FunctionsRaw) FunctionList(::std::move(functions));
        return *this;
    }

    ModuleBuilder& IsNative(const bool isNative) noexcept
    {
        m_IsNative = isNative;
        return *this;
    }

    ModuleBuilder& Native() noexcept
    {
        m_IsNative = true;
        return *this;
    }

    ModuleBuilder& Emulated() noexcept
    {
        m_IsNative = false;
        return *this;
    }

    ModuleBuilder& Name(const C8DynString& name) noexcept
    {
        m_Name = name;
        return *this;
    }

    ModuleBuilder& Name(C8DynString&& name) noexcept
    {
        m_Name = ::std::move(name);
        return *this;
    }

    ModuleBuilder& Name(const c8* const name) noexcept
    {
        m_Name = C8DynString(name);
        return *this;
    }

    template<uSys Len>
    ModuleBuilder& Name(const c8(&name)[Len]) noexcept
    {
        m_Name = C8DynString::FromStatic(name);
        return *this;
    }

    ModuleBuilder& Name(const char* const name) noexcept
    {
        m_Name = C8DynString(reinterpret_cast<const c8*>(name));
        return *this;
    }
    
    ModuleBuilder& Name() noexcept
    {
        m_Name = C8DynString();
        return *this;
    }

    ModuleRef Build() noexcept
    {
        if(!m_Functions)
        {
            return nullptr;
        }

        ModuleRef module(::std::move(*m_Functions), m_IsNative, ::std::move(m_Name));

        m_Functions->~DynArray();
        m_Functions = nullptr;

        m_Name.~C8DynString();

        module->AttachModuleReference(module);

        return module;
    }
private:
    u8 m_FunctionsRaw[sizeof(FunctionList)];

    FunctionList* m_Functions;

    bool m_IsNative;
    C8DynString m_Name;
};

using ModuleList = ::std::vector<ModuleRef>;

}
