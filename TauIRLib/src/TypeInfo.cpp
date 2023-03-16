#include "TauIR/TypeInfo.hpp"
#include "allocator/FixedBlockAllocator.hpp"
#include <atomic>
#include "TauIR/CompileControls.hpp"

namespace tau::ir {

#ifdef TAU_IR_DEBUG_TYPES
const TypeInfo TypeInfo::Void(0, u8"void");
const TypeInfo TypeInfo::Bool(1, u8"bool");
const TypeInfo TypeInfo::I8(1, u8"i8");
const TypeInfo TypeInfo::I16(2, u8"i16");
const TypeInfo TypeInfo::I32(4, u8"i32");
const TypeInfo TypeInfo::I64(8, u8"i64");
const TypeInfo TypeInfo::U8(1, u8"u8");
const TypeInfo TypeInfo::U16(2, u8"u16");
const TypeInfo TypeInfo::U32(4, u8"u32");
const TypeInfo TypeInfo::U64(8, u8"u64");
const TypeInfo TypeInfo::F32(4, u8"f32");
const TypeInfo TypeInfo::F64(8, u8"f64");
const TypeInfo TypeInfo::Char(1, u8"char");
#else
const TypeInfo TypeInfo::Void(0);
const TypeInfo TypeInfo::Bool(1);
const TypeInfo TypeInfo::I8(1);
const TypeInfo TypeInfo::I16(2);
const TypeInfo TypeInfo::I32(4);
const TypeInfo TypeInfo::I64(8);
const TypeInfo TypeInfo::U8(1);
const TypeInfo TypeInfo::U16(2);
const TypeInfo TypeInfo::U32(4);
const TypeInfo TypeInfo::U64(8);
const TypeInfo TypeInfo::F32(4);
const TypeInfo TypeInfo::F64(8);
const TypeInfo TypeInfo::Char(1);
#endif

static thread_local FixedBlockAllocator<TAU_IR_ALLOCATION_TRACKING> g_allocator(AlignTo<uSys, 8>(sizeof(TypeInfo)));

void* TypeInfo::operator new(const ::std::size_t sz) noexcept
{
    if(sz != sizeof(TypeInfo))
    {
        return nullptr;
    }

    return g_allocator.Allocate(sz);
}

void TypeInfo::operator delete(void* const ptr) noexcept
{
    if(!ptr)
    {
        return;
    }

    g_allocator.Deallocate(ptr);
}


uSys TypeInfo::GenerateId() noexcept
{
    static ::std::atomic<uSys> idAccumulator(0);
    return ++idAccumulator;
}

}
