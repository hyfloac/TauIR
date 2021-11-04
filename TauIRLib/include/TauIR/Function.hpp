/**
 * @file
 */
#pragma once

#include <NumTypes.hpp>
#include <DynArray.hpp>
#include <TUMaths.hpp>

#if defined(__STDCPP_STRICT_POINTER_SAFETY__) && __STDCPP_STRICT_POINTER_SAFETY__ == 1
#error "You have managed to find the one architecture that has strict pointer safety, unfortunately, we need relaxed pointer safety."
#endif

namespace tau::ir {

enum class InlineControl : u32
{
    /**
     *   Default inlining will follow the standard rules of inlining if the
     * function is small enough.
     */
    Default     = 0,
    /**
     *   NoInline inlining will force the optimizer to not inline this
     * function. Functions that this function calls may still be inlined.
     */
    NoInline    = 1,
    /**
     *   ForceInline inlining will force the optimizer to inline this
     * function. Function that this function calls are not necessarily
     * inlined.
     */
    ForceInline = 2,
    /**
     *   InlineHint will hint to the compiler that this function should be
     * inlined. This will increase the max acceptable inline function size.
     * If this is running in debug mode and this function meets the size
     * requirements this function may be inlined.
     */
    InlineHint  = 3
};

enum class CallingConvention : u32
{
    /**
     *   The default calling convention generates a very efficient but
     * non-portable calling convention for each function. This allows
     * better usage of registers for passing parameters and allows telling
     * other function which registers will be clobbered.
     */
    Default    = 0,
    /**
     * This will use the <a href="https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-160">Microsoft x64 Calling Convention</a>.
     *
     *   This will use registers @code RCX @endcode, @code RDX @endcode,
     * @code R8 @endcode, and @code R9 @endcode for integer/pointer
     * arguments from left-to-right. Remaining values are pushed onto the
     * stack right-to-left. Floating point arguments are passed in
     * @code XMM0 @endcode, @code XMM1 @endcode, @code XMM2 @endcode,
     * @code XMM3 @endcode left-to-right, with the remainder pushed onto
     * the stack right-to-left. This calling convention suffers from the
     * fact that register usage is restricted to the arguments index, for
     * example in @code void foo(int x, float y) @endcode, @code x @endcode
     * would be passed in @code RCX @endcode and @code y @endcode would be
     * passed in @code XMM1 @endcode.
     *
     *   Integer values are returned in @code RAX @endcode, and floating
     * point values are returned in @code XMM0 @endcode. Non-trivial custom
     * types that are not 1, 2, 4, 8, 16, 32, or 64 bits have memory
     * allocated by the caller with a pointer passed in the first
     * parameter, this pointer must also be returned in RAX.
     *
     *   Registers @code RAX @endcode, @code RCX @endcode,
     * @code RDX @endcode, @code R8 @endcode, @code R9 @endcode,
     * @code R10 @endcode, @code R11 @endcode, and @code XMM0-XMM5 @endcode
     * are considered volatile and do not need to be saved before using
     * them. All other registers are considered non-volatile and must be
     * saved and restored if they are used by the current function.
     * Additionally the upper values of @code YMM0-YMM15 @endcode and
     * @code ZMM0-ZMM15 @endcode are volatile. On processors with
     * @code AVX512VL @endcode the entirety of registers
     * @code ZMM16-ZMM31 @endcode are considered volatile.
     */
    MS64       = 1,
    Itanium64  = 2,
    Cdecl      = 3,
    StdCall    = 4,
    FastCall   = 5,
    VectorCall = 6,
};

enum class OptimizationControl : u32
{
    /**
     *   Default optimization control will optimize in release builds, but
     * not in debug builds.
     */
    Default       = 0,
    /**
     * NoOptimize will completely disable optimizations for this function.
     */
    NoOptimize    = 1,
    /**
     * ForceOptimize will cause the function to always be optimized.
     */
    ForceOptimize = 2,
    /**
     *   OptimizeHint will allow optimization in some debug builds where a
     * small amount of optimization is allowed.
     */
    OptimizeHint  = 3
};

union FunctionFlags final
{
    DEFAULT_DESTRUCT(FunctionFlags);
    DEFAULT_CM_PU(FunctionFlags);

    u32 Packed;
    struct
    {
        InlineControl InlineControl : 2;
        CallingConvention CallingConvention : 3;
        OptimizationControl OptimizationControl : 2;
        u32 Unused : 25;
    };

    FunctionFlags(const enum InlineControl inlineControl, const enum CallingConvention callingConvention, const enum OptimizationControl optimizationControl) noexcept
        : InlineControl(inlineControl)
        , CallingConvention(callingConvention)
        , OptimizationControl(optimizationControl)
    { }
};

class TypeInfo;

class Function final
{
    DEFAULT_DESTRUCT(Function);
    DEFAULT_CM_PU(Function);
public:
    Function(const uPtr address, const DynArray<const TypeInfo*>& localTypes, const FunctionFlags flags) noexcept
        : m_Address(address)
        , m_LocalSize(0)
        , m_LocalTypes(localTypes)
        , m_LocalOffsets(maxT(m_LocalTypes.count() - 1, 0))
        , m_Flags(flags)
    { LoadLocalOffsets(); }
    
    Function(const uPtr address, DynArray<const TypeInfo*>&& localTypes, const FunctionFlags flags) noexcept
        : m_Address(address)
        , m_LocalSize(0)
        , m_LocalTypes(::std::move(localTypes))
        , m_LocalOffsets(maxT(m_LocalTypes.count() - 1, 0))
        , m_Flags(flags)
    { LoadLocalOffsets(); }

    [[nodiscard]] uSys   Address() const noexcept { return m_Address;   }
    [[nodiscard]] uSys LocalSize() const noexcept { return m_LocalSize; }
    [[nodiscard]] const DynArray<const TypeInfo*>& LocalTypes() const noexcept { return m_LocalTypes;   }
    [[nodiscard]] const DynArray<uSys>&          LocalOffsets() const noexcept { return m_LocalOffsets; }
    [[nodiscard]] FunctionFlags Flags() const noexcept { return m_Flags; }
public: 
    [[nodiscard]] void* operator new(::std::size_t sz) noexcept;
    void operator delete(void* ptr) noexcept;
private:
    void LoadLocalOffsets() noexcept;
private:
    uPtr m_Address;
    uSys m_LocalSize;
    DynArray<const TypeInfo*> m_LocalTypes;
    DynArray<uSys> m_LocalOffsets;
    FunctionFlags m_Flags;
};

}
