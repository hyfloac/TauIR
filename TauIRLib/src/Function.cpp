#include "TauIR/Function.hpp"
#include "allocator/FixedBlockAllocator.hpp"
#include "TauIR/TypeInfo.hpp"
#include "TauIR/SafetyControls.hpp"

namespace tau::ir {

static thread_local FixedBlockAllocator<TAU_IR_ALLOCATION_TRACKING> g_allocator(sizeof(Function));

void* Function::operator new(const ::std::size_t sz) noexcept
{
    if(sz != sizeof(Function))
    {
        return nullptr;
    }

    return g_allocator.Allocate(sz);
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

    uSys currentOffset;

    if(TypeInfo::IsPointer(m_LocalTypes[0]))
    {
        currentOffset = sizeof(void*);
    }
    else
    {
        currentOffset = TypeInfo::StripPointer(m_LocalTypes[0])->Size();
    }

    for(uSys i = 1; i < m_LocalTypes.count(); ++i)
    {
        m_LocalOffsets[i - 1] = currentOffset;
        if(TypeInfo::IsPointer(m_LocalTypes[i]))
        {
            currentOffset += TypeInfo::StripPointer(m_LocalTypes[i])->Size();
        }
        else
        {
            currentOffset += sizeof(void*);
        }
    }

    m_LocalSize = currentOffset;
}

}
