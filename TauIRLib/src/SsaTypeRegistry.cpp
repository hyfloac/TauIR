#include "TauIR/ssa/SsaTypes.hpp"
#include <allocator/FixedBlockAllocator.hpp>
#include "TauIR/CompileControls.hpp"

#if TAU_IR_DEBUG_TYPES

namespace tau::ir::ssa {
static thread_local FixedBlockArenaAllocator<AllocationTracking::None> g_DebugTypeAllocator(AlignTo<uSys, 8>(sizeof(SsaCustomTypeDebugNode)));

void* SsaCustomTypeDebugNode::operator new(const std::size_t sz) noexcept
{
    if(sz != sizeof(g_DebugTypeAllocator))
    {
        return nullptr;
    }

    return g_DebugTypeAllocator.Allocate(sz);
}

void SsaCustomTypeDebugNode::operator delete(void* const ptr) noexcept
{
    if(!ptr)
    {
        return;
    }

    g_DebugTypeAllocator.Deallocate(ptr);
}

}

#endif
