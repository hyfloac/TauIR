#include "TauIR/TypeInfo.hpp"
#include "allocator/FixedBlockAllocator.hpp"
#include <atomic>

namespace tau::ir {

#ifdef _DEBUG
const TypeInfo TypeInfo::Void(0, "void");
const TypeInfo TypeInfo::Bool(1, "bool");
const TypeInfo TypeInfo::I8(1, "i8");
const TypeInfo TypeInfo::I16(2, "i16");
const TypeInfo TypeInfo::I32(4, "i32");
const TypeInfo TypeInfo::I64(8, "i64");
const TypeInfo TypeInfo::U8(1, "u8");
const TypeInfo TypeInfo::U16(2, "u16");
const TypeInfo TypeInfo::U32(4, "u32");
const TypeInfo TypeInfo::U64(8, "u64");
const TypeInfo TypeInfo::F32(4, "f32");
const TypeInfo TypeInfo::F64(8, "f32");
const TypeInfo TypeInfo::Char(1, "char");
#else
const TypeInfo TypeInfo::Void(0, "");
const TypeInfo TypeInfo::Bool(1, "");
const TypeInfo TypeInfo::I8(1, "");
const TypeInfo TypeInfo::I16(2, "");
const TypeInfo TypeInfo::I32(4, "");
const TypeInfo TypeInfo::I64(8, "");
const TypeInfo TypeInfo::U8(1, "");
const TypeInfo TypeInfo::U16(2, "");
const TypeInfo TypeInfo::U32(4, "");
const TypeInfo TypeInfo::U64(8, "");
const TypeInfo TypeInfo::F32(4, "");
const TypeInfo TypeInfo::F64(8, "");
const TypeInfo TypeInfo::Char(1, "");
#endif

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

bool TypeInfo::IsPointer(const TypeInfo* typeInfo) noexcept
{
    const uPtr address = reinterpret_cast<uPtr>(typeInfo);
    return address & 1;
}

TypeInfo* TypeInfo::StripPointer(TypeInfo* typeInfo) noexcept
{
    const uPtr address = reinterpret_cast<uPtr>(typeInfo) & ~7;
    return reinterpret_cast<TypeInfo*>(address);
}

const TypeInfo* TypeInfo::StripPointer(const TypeInfo* typeInfo) noexcept
{
    const uPtr address = reinterpret_cast<uPtr>(typeInfo) & ~7;
    return reinterpret_cast<const TypeInfo*>(address);
}

TypeInfo* TypeInfo::AddPointer(TypeInfo* typeInfo) noexcept
{
    const uPtr address = reinterpret_cast<uPtr>(typeInfo) | 1;
    return reinterpret_cast<TypeInfo*>(address);
}

const TypeInfo* TypeInfo::AddPointer(const TypeInfo* typeInfo) noexcept
{
    const uPtr address = reinterpret_cast<uPtr>(typeInfo) | 1;
    return reinterpret_cast<const TypeInfo*>(address);
}

TypeInfo* TypeInfo::SetPointer(TypeInfo* typeInfo, const bool isPointer) noexcept
{
    return isPointer ? AddPointer(typeInfo) : StripPointer(typeInfo);
}

const TypeInfo* TypeInfo::SetPointer(const TypeInfo* typeInfo, const bool isPointer) noexcept
{
    return isPointer ? AddPointer(typeInfo) : StripPointer(typeInfo);
}

uSys TypeInfo::GenerateId() noexcept
{
    static ::std::atomic<uSys> idAccumulator(0);
    return ++idAccumulator;
}

}
