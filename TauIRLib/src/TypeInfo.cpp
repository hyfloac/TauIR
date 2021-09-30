#include "TauIR/TypeInfo.hpp"
#include "allocator/FixedBlockAllocator.hpp"
#include <atomic>

namespace tau::ir {

static thread_local FixedBlockAllocator<> g_allocator(_alignTo(sizeof(TypeInfo), 8));

void* TypeInfo::operator new(const ::std::size_t sz) noexcept
{
    if(sz != sizeof(TypeInfo))
    {
        return nullptr;
    }

    return g_allocator.allocate(sz);
}

void TypeInfo::operator delete(void* const ptr) noexcept
{
    if(!ptr)
    {
        return;
    }

    g_allocator.deallocate(ptr);
}

uSys TypeInfo::GenerateId() noexcept
{
    static ::std::atomic<uSys> idAccumulator(0);
    return ++idAccumulator;
}

}
