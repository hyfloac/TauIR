#include "TauIR/Function.hpp"
#include "allocator/FixedBlockAllocator.hpp"
#include "TauIR/TypeInfo.hpp"

namespace tau::ir {

static thread_local FixedBlockAllocator<> g_allocator(sizeof(Function));

void* Function::operator new(const ::std::size_t sz) noexcept
{
    if(sz != sizeof(Function))
    {
        return nullptr;
    }

    return g_allocator.allocate(sz);
}

void Function::operator delete(void* const ptr) noexcept
{
    if(!ptr)
    {
        return;
    }

    g_allocator.deallocate(ptr);
}

    
void Function::LoadLocalOffsets() noexcept
{
    if(m_LocalTypes.count() == 0)
    {
        return;
    }

    uSys currentOffset = m_LocalTypes[0]->Size();
    for(uSys i = 1; i < m_LocalTypes.count(); ++i)
    {
        m_LocalOffsets[i - 1] = currentOffset;
        currentOffset += m_LocalTypes[i]->Size();
    }

    m_LocalSize = currentOffset;
}

}
