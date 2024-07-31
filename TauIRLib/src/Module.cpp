#include "TauIR/Module.hpp"
#include "TauIR/Function.hpp"
#include <ConPrinter.hpp>
#include <atomic>
#include <allocator/FixedBlockAllocator.hpp>
#include <mutex>

#include "TauIR/CompileControls.hpp"

namespace tau::ir {

uSys Module::GenerateId() noexcept
{
    static ::std::atomic<uSys> idAccumulator(0);
    return ++idAccumulator;
}

Module::~Module() noexcept
{
#if _DEBUG
    ConPrinter::PrintLn("Destroying Module ''{}''", m_Name);
#endif

    for(uSys i = 0; i < m_Functions.count(); ++i)
    {
        delete m_Functions[i];
    }
}

void Module::AttachModuleReference(const ModuleRef& module) noexcept
{
    for(Function* function : m_Functions)
    {
        function->Module() = module;
    }
}

static FixedBlockAllocator<TAU_IR_ALLOCATION_TRACKING> g_allocator(sizeof(Module), PageCountVal{ 128 });
static ::std::mutex g_allocatorMutex;

void* Module::operator new(const ::std::size_t sz) noexcept
{
    if(sz != sizeof(Module))
    {
        return nullptr;
    }

    ::std::lock_guard lock(g_allocatorMutex);

    return g_allocator.Allocate(sz);
}

void Module::operator delete(void* const ptr) noexcept
{
    if(!ptr)
    {
        return;
    }

    ::std::lock_guard lock(g_allocatorMutex);

    g_allocator.deallocate(ptr);
}


}
