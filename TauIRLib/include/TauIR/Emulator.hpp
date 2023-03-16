#pragma once

#include <DynArray.hpp>
#include <ConPrinter.hpp>
#include <vector>
#include "Common.hpp"

namespace tau::ir {

class Function;
class Module;

class Emulator
{
public:
    using ModuleList = ::std::vector<Ref<Module>>;
    using ArgumentRegisterType = u64;

    static inline constexpr uSys MaxArgumentRegisters = 64;
    static inline constexpr uSys PointerSize = sizeof(void*);

    template<typename T>
    static inline constexpr T ExecutionStackSize = T{ 16 } * T{ 1024 } * T{ 1024 };

    template<typename T>
    static inline constexpr T LocalsStackSize = ExecutionStackSize<T>;

    struct SizeAndOffset
    {
        uSys Size;
        uSys Offset;
    };
public:
    explicit Emulator(ModuleList&& modules)
        : m_Modules(::std::move(modules))
        , m_ExecutionStack(ExecutionStackSize<uSys>)
        , m_LocalsStack(LocalsStackSize<uSys>)
        , m_Arguments(MaxArgumentRegisters)
        , m_StackPointer(0)
    {
        (void) ::std::memset(m_ExecutionStack.arr(), 0, m_ExecutionStack.size());
        (void) ::std::memset(m_LocalsStack.arr(), 0, m_LocalsStack.size());
        (void) ::std::memset(m_Arguments.arr(), 0, m_Arguments.size() * sizeof(u64));
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
    void ExecuteFunction(const Function* function, const Module* module) noexcept;
    void PushLocal(const Function* function, uSys localsHead, u16 local) noexcept;
    void PopLocal(const Function* function, uSys localsHead, u16 local) noexcept;
    void PushArgument(uSys argument) noexcept;
    void PopArgument(uSys argument) noexcept;
    void DuplicateVal(uSys byteCount) noexcept;
    void SetLocal(const Function* function, uSys localsHead, u16 local, const void* data) noexcept;

    template<typename T>
    T GetLocal(const Function* const function, const uSys localsHead, const u16 local) const noexcept
    {
        // Get the size and offset of the local.
        const auto [size, offset] = GetLocalSizeAndOffset(function, local);

        if(sizeof(T) != size)
        {
            ConPrinter::PrintLn("Type with size {} does not match local #{} with size {}.", sizeof(T), local, size);
            return T{};
        }

        T ret;

        // Copy the value.
        (void) ::std::memcpy(&ret, m_LocalsStack.arr() + localsHead + offset, size);

        return ret;
    }

    template<typename T>
    void PushValue(const T value) noexcept
    {
        (void) ::std::memcpy(m_ExecutionStack.arr() + m_StackPointer, &value, sizeof(T));
        m_StackPointer += sizeof(T);
    }

    template<typename T>
    T PopValue() noexcept
    {
        T ret;
        m_StackPointer -= sizeof(T);
        (void) ::std::memcpy(&ret, m_ExecutionStack.arr() + m_StackPointer, sizeof(T));
        return ret;
    }

    template<typename TRead, typename TWrite>
    void ResizeVal() noexcept
    {
        PushValue<TWrite>(static_cast<TWrite>(PopValue<TRead>()));
    }
private:
    static SizeAndOffset GetLocalSizeAndOffset(const Function* function, u16 local) noexcept;
    static SizeAndOffset GetLocalUnderlyingSizeAndOffset(const Function* function, u16 local) noexcept;
private:
    ModuleList m_Modules;
    DynArray<u8> m_ExecutionStack;
    DynArray<u8> m_LocalsStack;
    DynArray<ArgumentRegisterType> m_Arguments;
    uSys m_StackPointer;
};

}
