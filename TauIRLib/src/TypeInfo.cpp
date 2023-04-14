#include "TauIR/TypeInfo.hpp"
#include "allocator/FixedBlockAllocator.hpp"
#include <atomic>
#include <mutex>

#include "TauIR/CompileControls.hpp"

namespace tau::ir {

#if defined(TAU_IR_DEBUG_TYPES)
const TypeInfo TypeInfo::Void(0, TypeInfoFlags::Void(), u8"void");
const TypeInfo TypeInfo::Bool(1, TypeInfoFlags::UnsignedInteger(), u8"bool");
const TypeInfo TypeInfo::I8(1, TypeInfoFlags::SignedInteger(), u8"i8");
const TypeInfo TypeInfo::I16(2, TypeInfoFlags::SignedInteger(), u8"i16");
const TypeInfo TypeInfo::I32(4, TypeInfoFlags::SignedInteger(), u8"i32");
const TypeInfo TypeInfo::I64(8, TypeInfoFlags::SignedInteger(), u8"i64");
const TypeInfo TypeInfo::U8(1, TypeInfoFlags::UnsignedInteger(), u8"u8");
const TypeInfo TypeInfo::U16(2, TypeInfoFlags::UnsignedInteger(), u8"u16");
const TypeInfo TypeInfo::U32(4, TypeInfoFlags::UnsignedInteger(), u8"u32");
const TypeInfo TypeInfo::U64(8, TypeInfoFlags::UnsignedInteger(), u8"u64");
const TypeInfo TypeInfo::F32(4, TypeInfoFlags::Float(), u8"f32");
const TypeInfo TypeInfo::F64(8, TypeInfoFlags::Float(), u8"f64");
const TypeInfo TypeInfo::Char(1, TypeInfoFlags::Char(), u8"char");
#else
const TypeInfo TypeInfo::Void(0, TypeInfoFlags::Void());
const TypeInfo TypeInfo::Bool(1, TypeInfoFlags::UnsignedInteger());
const TypeInfo TypeInfo::I8(1, TypeInfoFlags::SignedInteger());
const TypeInfo TypeInfo::I16(2, TypeInfoFlags::SignedInteger());
const TypeInfo TypeInfo::I32(4, TypeInfoFlags::SignedInteger());
const TypeInfo TypeInfo::I64(8, TypeInfoFlags::SignedInteger());
const TypeInfo TypeInfo::U8(1, TypeInfoFlags::UnsignedInteger());
const TypeInfo TypeInfo::U16(2, TypeInfoFlags::UnsignedInteger());
const TypeInfo TypeInfo::U32(4, TypeInfoFlags::UnsignedInteger());
const TypeInfo TypeInfo::U64(8, TypeInfoFlags::UnsignedInteger());
const TypeInfo TypeInfo::F32(4, TypeInfoFlags::Float());
const TypeInfo TypeInfo::F64(8, TypeInfoFlags::Float());
const TypeInfo TypeInfo::Char(1, TypeInfoFlags::Char());
#endif

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
static FixedBlockAllocator<TAU_IR_ALLOCATION_TRACKING>* g_allocator = ::new(::std::nothrow_t{}) FixedBlockAllocator<TAU_IR_ALLOCATION_TRACKING>(AlignTo<uSys, 8>(sizeof(TypeInfo)));
/**
 * \brief The mutex for locking the global allocator for TypeInfo objects.
 *
 *   We don't bother with the same heap allocation trick here, because
 * the destructor of the mutex might do something important, like
 * release a deadlock.
 */
static ::std::mutex g_allocatorMutex;

void* TypeInfo::operator new(const ::std::size_t sz) noexcept
{
    if(sz != sizeof(TypeInfo))
    {
        return nullptr;
    }

    ::std::lock_guard lock(g_allocatorMutex);

    return g_allocator->Allocate(sz);
}

void TypeInfo::operator delete(void* const ptr) noexcept
{
    if(!ptr)
    {
        return;
    }

    ::std::lock_guard lock(g_allocatorMutex);

    g_allocator->Deallocate(ptr);
}


uSys TypeInfo::GenerateId() noexcept
{
    static ::std::atomic<uSys> idAccumulator(0);
    return ++idAccumulator;
}

}
