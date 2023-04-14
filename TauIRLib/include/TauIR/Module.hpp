#pragma once

#include <NumTypes.hpp>
#include <DynArray.hpp>
#include <String.hpp>

#include "Common.hpp"

namespace tau::ir {

class Function;
class Module;

using FunctionList = DynArray<Function*>;

class ImportModule final
{
    DEFAULT_CONSTRUCT_PU(ImportModule);
    DEFAULT_DESTRUCT(ImportModule);
    DELETE_COPY(ImportModule);
    DEFAULT_MOVE_PU(ImportModule);
public:
    ImportModule(const ModuleRef& module, FunctionList&& functions) noexcept
        : m_Module(module)
        , m_Functions(::std::move(functions))
    { }

    ImportModule(const ModuleRef & module, const FunctionList& functions) noexcept
        : m_Module(module)
        , m_Functions(functions)
    { }

    [[nodiscard]] const ModuleRef& Module() const noexcept { return m_Module; }
    [[nodiscard]] const FunctionList& Functions() const noexcept { return m_Functions; }
private:
    ModuleRef m_Module;
    FunctionList m_Functions;
};

using ImportModuleList = DynArray<ImportModule>;

class Module final
{
    DELETE_COPY(Module);
    DEFAULT_MOVE_PU(Module);
public:
    Module(FunctionList&& functions, FunctionList&& exports, ImportModuleList&& imports, const bool isNative, C8DynString&& name) noexcept
        : m_Id(GenerateId())
        , m_Functions(::std::move(functions))
        , m_Exports(::std::move(exports))
        , m_Imports(::std::move(imports))
        , m_IsNative(isNative)
        , m_Name(::std::move(name))
    { }

    ~Module() noexcept;

    [[nodiscard]] uSys Id() const noexcept { return m_Id; }
    [[nodiscard]] const FunctionList&   Functions() const noexcept { return m_Functions; }
    [[nodiscard]] const FunctionList&     Exports() const noexcept { return m_Exports;   }
    [[nodiscard]] const ImportModuleList& Imports() const noexcept { return m_Imports;   }
    [[nodiscard]] bool IsNative() const noexcept { return m_IsNative; }
    [[nodiscard]] const C8DynString& Name() const noexcept { return m_Name; }
    [[nodiscard]]       C8DynString& Name()       noexcept { return m_Name; }

    void AttachModuleReference(const ModuleRef& module) noexcept;
private:
    static uSys GenerateId() noexcept;
public:
    /**
     * Allocate with a fixed block allocator for performance.
     *
     *   We'll be allocating lots of functions for a program, and the
     * actual size this struct is constant, thus we can use a fixed block
     * allocator to drastically improve performance.
     */
    [[nodiscard]] void* operator new(::std::size_t sz) noexcept;
    void operator delete(void* ptr) noexcept;
private:
    uSys m_Id;
    FunctionList m_Functions;
    FunctionList m_Exports;
    ImportModuleList m_Imports;
    bool m_IsNative;
    C8DynString m_Name;
};

class ModuleBuilder final
{
public:
    ModuleBuilder() noexcept
        : m_FunctionsRaw { }
        , m_ExportsRaw { }
        , m_ImportsRaw { }
        , m_Functions(nullptr)
        , m_Exports(nullptr)
        , m_Imports(nullptr)
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

    ModuleBuilder& Exports(FunctionList&& functions) noexcept
    {
        if(m_Exports)
        {
            m_Exports->~FunctionList();
        }

        m_Exports = ::new(m_ExportsRaw) FunctionList(::std::move(functions));
        return *this;
    }

    ModuleBuilder& Exports() noexcept
    {
        if(m_Exports)
        {
            m_Exports->~FunctionList();
        }

        m_Exports = ::new(m_ExportsRaw) FunctionList();
        return *this;
    }

    ModuleBuilder& Imports(ImportModuleList&& imports) noexcept
    {
        if(m_Imports)
        {
            m_Imports->~ImportModuleList();
        }

        m_Imports = ::new(m_ImportsRaw) ImportModuleList(::std::move(imports));
        return *this;
    }

    ModuleBuilder& Imports() noexcept
    {
        if(m_Imports)
        {
            m_Imports->~ImportModuleList();
        }

        m_Imports = ::new(m_ImportsRaw) ImportModuleList();
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

        ModuleRef module(::std::move(*m_Functions), ::std::move(*m_Exports), ::std::move(*m_Imports), m_IsNative, ::std::move(m_Name));

        m_Functions->~DynArray();
        m_Functions = nullptr;

        m_Exports->~DynArray();
        m_Exports = nullptr;

        m_Imports->~DynArray();
        m_Imports = nullptr;

        m_Name.~C8DynString();

        module->AttachModuleReference(module);

        return module;
    }
private:
    u8 m_FunctionsRaw[sizeof(FunctionList)];
    u8 m_ExportsRaw[sizeof(FunctionList)];
    u8 m_ImportsRaw[sizeof(FunctionList)];

    FunctionList* m_Functions;
    FunctionList* m_Exports;
    ImportModuleList* m_Imports;

    bool m_IsNative;
    C8DynString m_Name;
};

}
