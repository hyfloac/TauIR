#pragma once

#include <NumTypes.hpp>
#include <DynArray.hpp>
#include <String.hpp>

namespace tau::ir {

class Function;

class Module final
{
    DELETE_COPY(Module);
    DEFAULT_MOVE_PU(Module);
public:
    Module(DynArray<const Function*>&& functions, const bool isNative = false) noexcept
        : m_Id(GenerateId())
        , m_Functions(::std::move(functions))
        , m_IsNative(isNative)
        , m_Name{}
    { }

    ~Module() noexcept;

    [[nodiscard]] uSys Id() const noexcept { return m_Id; }
    [[nodiscard]] const DynArray<const Function*>& Functions() const noexcept { return m_Functions; }
    [[nodiscard]] bool IsNative() const noexcept { return m_IsNative; }
    [[nodiscard]] const C8DynString& Name() const noexcept { return m_Name; }
    [[nodiscard]]       C8DynString& Name()       noexcept { return m_Name; }
private:
    static uSys GenerateId() noexcept;
private:
    uSys m_Id;
    DynArray<const Function*> m_Functions;
    bool m_IsNative;
    C8DynString m_Name;
};

}
