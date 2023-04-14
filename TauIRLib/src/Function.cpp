#include "TauIR/Function.hpp"
#include "allocator/FixedBlockAllocator.hpp"
#include "TauIR/TypeInfo.hpp"
#include "TauIR/CompileControls.hpp"
#include <mutex>

namespace tau::ir {

RTT_BASE_IMPL_TU(FunctionAttachment);

void FunctionAttachment::Attach(FunctionAttachment* attachment) noexcept
{
    if(!m_Next)
    {
        m_Next = attachment;
        return;
    }

    FunctionAttachment* next = m_Next;
    
    while(next->m_Next)
    {
        next = next->m_Next;
    }

    next->m_Next = attachment;
}

/**
 * \brief The global allocator for TypeInfo objects.
 *
 *   This is a heap allocated object to prevent the destructor being
 * called at program exit. The destructor frees huge swathes of memory
 * pages, which can potentially be rather slow (in my experience most
 * memory allocators tend to end up being release heavy rather than
 * alloc heavy). Ultimately, after exiting the OS is just going to drop
 * the page table anyways, we don't need to worry about the
 * complexities of putting the reserved pages back into bins for reuse.
 * There will still be a cost with freeing physical memory, but that
 * is going to happen regardless.
 *
 *   We have to create our own instance of ::std::nothrow because the
 * primary one may not have been created yet. In reality operator new
 * doesn't actually care about which instance of nothrow_t it points
 * to, as it only uses the argument to change the function overload.
 */
static FixedBlockAllocator<TAU_IR_ALLOCATION_TRACKING>* g_allocator = ::new(::std::nothrow_t {}) FixedBlockAllocator<TAU_IR_ALLOCATION_TRACKING>(AlignTo<uSys, 8>(sizeof(Function)));
/**
 * \brief The mutex for locking the global allocator for TypeInfo objects.
 *
 *   We don't bother with the same heap allocation trick here, because
 * the destructor of the mutex might do something important, like
 * release a deadlock.
 */
static ::std::mutex g_allocatorMutex;

void* Function::operator new(const ::std::size_t sz) noexcept
{
    if(sz != sizeof(Function))
    {
        return nullptr;
    }

    ::std::lock_guard lock(g_allocatorMutex);

    return g_allocator->Allocate(sz);
}

void Function::operator delete(void* const ptr) noexcept
{
    if(!ptr)
    {
        return;
    }

    ::std::lock_guard lock(g_allocatorMutex);

    g_allocator->deallocate(ptr);
}

    
void FunctionBuilder::LoadLocalOffsets() noexcept
{
    if(GetLocalTypes().count() == 0)
    {
        return;
    }

    uSys currentOffset;

    if(TypeInfo::IsPointer(GetLocalTypes()[0]))
    {
        currentOffset = sizeof(void*);
    }
    else
    {
        currentOffset = TypeInfo::StripPointer(GetLocalTypes()[0])->Size();
    }

    for(uSys i = 1; i < GetLocalTypes().count(); ++i)
    {
        GetLocalOffsets()[i - 1] = currentOffset;
        if(TypeInfo::IsPointer(GetLocalTypes()[i]))
        {
            currentOffset += TypeInfo::StripPointer(GetLocalTypes()[i])->Size();
        }
        else
        {
            currentOffset += sizeof(void*);
        }
    }

    m_LocalSize = currentOffset;
}

}
