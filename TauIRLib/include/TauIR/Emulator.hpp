#pragma once

#include <DynArray.hpp>
#include <vector>
#include "Common.hpp"

namespace tau::ir {

class Function;
class Module;

class Emulator
{
public:
    using ModuleList = ::std::vector<Ref<Module>>;
public:
    explicit Emulator(ModuleList&& modules)
        : m_Modules(::std::move(modules))
        , m_Stack(16 * 1024 * 1024)
        , m_Arguments(64)
        , m_StackPointer(0)
    {
        ::std::memset(m_Stack.arr(), 0, m_Stack.size());
        ::std::memset(m_Arguments.arr(), 0, m_Arguments.size() * sizeof(u64));
    }

    void LoadModule(const Ref<Module>& module)
    {
        m_Modules.push_back(module);
    }

    void LoadModule(Ref<Module>&& module)
    {
        m_Modules.push_back(::std::move(module));
    }

    void Execute() noexcept;

    [[nodiscard]] u64 ReturnVal() const noexcept { return m_Arguments[0]; }
private:
    void ExecuteFunction(const Function* function) noexcept;
    void PushLocal(const Function* function, uSys localHead, u16 local) noexcept;
    void PopLocal(const Function* function, uSys localHead, u16 local) noexcept;
    void PushArgument(uSys argument) noexcept;
    void PopArgument(uSys argument) noexcept;
    void DuplicateVal(uSys byteCount) noexcept;

    template<typename T>
    void PushValue(const T value) noexcept
    {
        (void) ::std::memcpy(m_Stack.arr() + m_StackPointer, &value, sizeof(T));
        m_StackPointer += sizeof(T);
    }

    template<typename T>
    T PopValue() noexcept
    {
        T ret;
        m_StackPointer -= sizeof(T);
        (void) ::std::memcpy(&ret, m_Stack.arr() + m_StackPointer, sizeof(T));
        return ret;
    }

    template<typename TRead, typename TWrite>
    void ResizeVal() noexcept
    {
        PushValue<TWrite>(static_cast<TWrite>(PopValue<TRead>()));
    }
private:
    ModuleList m_Modules;
    DynArray<u8> m_Stack;
    DynArray<u64> m_Arguments;
    uSys m_StackPointer;
};

}
