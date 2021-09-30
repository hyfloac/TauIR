#pragma once

#include <NumTypes.hpp>
#include <DynArray.hpp>

namespace tau::ir {

class Function;

class Module final
{
    DELETE_COPY(Module);
    DEFAULT_MOVE_PU(Module);
public:
    Module(DynArray<const Function*>&& functions) noexcept
        : m_Id(GenerateId())
        , m_Functions(::std::move(functions))
    { }

    ~Module() noexcept;

    [[nodiscard]] uSys Id() const noexcept { return m_Id; }
    [[nodiscard]] const DynArray<const Function*>& Functions() const noexcept { return m_Functions; }
private:
    static uSys GenerateId() noexcept;
private:
    uSys m_Id;
    DynArray<const Function*> m_Functions;
};

}
