// ReSharper disable CppClangTidyPerformanceNoIntToPtr
#pragma once

#include <ReferenceCountingPointer.hpp>

template<typename T>
using Ref = ReferenceCountingPointer<T>;

template<typename T>
using StrongRef = StrongReferenceCountingPointer<T>;

template<typename T>
using WeakRef = WeakReferenceCountingPointer<T>;

template<uPtr TagMask, typename T>
[[nodiscard]] inline bool CheckPointerTag(T* ptr) noexcept
{
	return (reinterpret_cast<uPtr>(ptr) & TagMask) != 0;
}

template<uPtr TagBits, typename T>
[[nodiscard]] inline T* TagPointer(T* ptr) noexcept
{
	const uPtr address = reinterpret_cast<uPtr>(ptr) | TagBits;
	return reinterpret_cast<T*>(address);
}

template<uPtr TagMask, typename T>
[[nodiscard]] inline T* UnTagPointer(T* ptr) noexcept
{
	const uPtr address = reinterpret_cast<uPtr>(ptr) & ~TagMask;
	return reinterpret_cast<T*>(address);
}

static inline constexpr uSys MaxArgumentRegisters = 64;

namespace tau::ir {
class Module;
using ModuleRef = StrongRef<Module>;
}
