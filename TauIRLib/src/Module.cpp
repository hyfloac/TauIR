#include "TauIR/Module.hpp"
#include "TauIR/Function.hpp"
#include <ConPrinter.hpp>
#include <atomic>

namespace tau::ir {

uSys Module::GenerateId() noexcept
{
    static ::std::atomic<uSys> idAccumulator(0);
    return ++idAccumulator;
}

Module::~Module() noexcept
{
#if _DEBUG
    ConPrinter::PrintLn("Destroying Module '{}'", m_Name);
#endif

    for(uSys i = 0; i < m_Functions.count(); ++i)
    {
        delete m_Functions[i];
    }
}

}
