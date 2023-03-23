#include "TauIR/Function.hpp"
#include "allocator/FixedBlockAllocator.hpp"
#include "TauIR/TypeInfo.hpp"
#include "TauIR/CompileControls.hpp"

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
