/**
 * @file
 */
#pragma once

#include <NumTypes.hpp>
#include <DynArray.hpp>
#include <TUMaths.hpp>
#include <String.hpp>
#include <RunTimeType.hpp>

#include "Common.hpp"

#if defined(__STDCPP_STRICT_POINTER_SAFETY__) && __STDCPP_STRICT_POINTER_SAFETY__ == 1
#error "You have managed to find the one architecture that has strict pointer safety, unfortunately, we need relaxed pointer safety."
#endif

static_assert(sizeof(u8) == 1, "A byte is not a byte in size.");
static_assert(CHAR_BIT == 8, "A byte is not 8 bits in size.");

namespace tau::ir {

class Module;
class TypeInfo;

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

struct FunctionArgument
{
    DEFAULT_CONSTRUCT_PU(FunctionArgument);
    DEFAULT_DESTRUCT(FunctionArgument);
    DEFAULT_CM_PU(FunctionArgument);
public:
    bool IsRegister;
    uSys RegisterOrStackOffset;

    FunctionArgument(const bool isRegister, const uSys registerOrStackOffset) noexcept
        : IsRegister(isRegister)
        , RegisterOrStackOffset(registerOrStackOffset)
    { }
};

class FunctionAttachment
{
    DELETE_CM(FunctionAttachment);
    RTT_BASE_IMPL(FunctionAttachment);
    RTT_BASE_CHECK(FunctionAttachment);
    RTT_BASE_CAST(FunctionAttachment);
protected:
    FunctionAttachment() noexcept
        : m_Next(nullptr)
    { }
public:
    virtual ~FunctionAttachment() noexcept
    {
        delete m_Next;
    }

    void Attach(FunctionAttachment* attachment) noexcept;

    [[nodiscard]] const FunctionAttachment*  Next() const noexcept { return m_Next; }
    [[nodiscard]]       FunctionAttachment*& Next()       noexcept { return m_Next; }
private:
    FunctionAttachment* m_Next;
};

class OptoFunctionCodeAttachment final : public FunctionAttachment
{
    DEFAULT_DESTRUCT(OptoFunctionCodeAttachment);
    DELETE_CM(OptoFunctionCodeAttachment);
    RTT_IMPL(OptoFunctionCodeAttachment, FunctionAttachment);
public:
    OptoFunctionCodeAttachment(
        const u8* const address,
        const uSys codeSize,
        const uSys localSize,
        DynArray<const TypeInfo*>&& localTypes,
        DynArray<uSys>&& localOffsets
    ) noexcept
        : m_Address(address)
        , m_CodeSize(codeSize)
        , m_LocalSize(localSize)
        , m_LocalTypes(::std::move(localTypes))
        , m_LocalOffsets(::std::move(localOffsets))
    { }

    [[nodiscard]] const u8* Address() const noexcept { return m_Address; }
    [[nodiscard]] uSys CodeSize() const noexcept { return m_CodeSize; }
    [[nodiscard]] uSys LocalSize() const noexcept { return m_LocalSize; }
    [[nodiscard]] const DynArray<const TypeInfo*>& LocalTypes() const noexcept { return m_LocalTypes; }
    [[nodiscard]] const DynArray<uSys>& LocalOffsets() const noexcept { return m_LocalOffsets; }
private:
    /**
     * The address of the IR code.
     */
    const u8* m_Address;
    /**
     * The number of bytes of IR code.
     */
    uSys m_CodeSize;
    /**
     * The number of bytes that locals will take up.
     */
    uSys m_LocalSize;
    /**
     * The types of all locals.
     */
    DynArray<const TypeInfo*> m_LocalTypes;
    /**
     * The offsets of all locals.
     *
     *   This is one less than the number of locals as we can ignore the
     * first offset as it is always zero.
     */
    DynArray<uSys> m_LocalOffsets;
};

class Function final
{
    DEFAULT_DESTRUCT(Function);
    DEFAULT_CM_PU(Function);
public:
    [[nodiscard]] const u8* Address() const noexcept { return m_Address; }
    [[nodiscard]] uSys CodeSize() const noexcept { return m_CodeSize; }
    [[nodiscard]] uSys LocalSize() const noexcept { return m_LocalSize; }
    [[nodiscard]] const DynArray<const TypeInfo*>& LocalTypes() const noexcept { return m_LocalTypes;   }
    [[nodiscard]] const DynArray<uSys>&          LocalOffsets() const noexcept { return m_LocalOffsets; }
    [[nodiscard]] const DynArray<FunctionArgument>& Arguments() const noexcept { return m_Arguments; }
    [[nodiscard]] FunctionFlags Flags() const noexcept { return m_Flags; }

    [[nodiscard]] const C8DynString& Name() const noexcept { return m_Name; }
    [[nodiscard]]       C8DynString& Name()       noexcept { return m_Name; }

    [[nodiscard]] const WeakRef<tau::ir::Module>& Module() const noexcept { return m_Module; }
    [[nodiscard]]       WeakRef<tau::ir::Module>& Module()       noexcept { return m_Module; }

    [[nodiscard]] const FunctionAttachment*  Attachment() const noexcept { return m_Attachment; }
    [[nodiscard]]       FunctionAttachment*& Attachment()       noexcept { return m_Attachment; }

    template<typename T>
    T* FindAttachment() noexcept
    {
        if(!m_Attachment)
        {
            return nullptr;
        }

        FunctionAttachment* curr = m_Attachment;

        do
        {
            if(rtt_check<T>(curr))
            {
                return rtt_cast<T>(curr);
            }
            curr = curr->Next();
        } while(curr);

        return nullptr;
    }

    template<typename T>
    const T* FindAttachment() const noexcept
    {
        return const_cast<Function*>(this)->FindAttachment<T>();
    }

    template<typename T, typename... Args>
    void Attach(Args&&... args) noexcept
    {
        if(m_Attachment)
        {
            m_Attachment->Attach(new(::std::nothrow) T(TauAllocatorUtils::Forward<Args>(args)...));
        }
        else
        {
            m_Attachment = new(::std::nothrow) T(TauAllocatorUtils::Forward<Args>(args)...);
        }
    }

    template<typename T>
    void RemoveAttachment() noexcept
    {
        if(!m_Attachment)
        {
            return;
        }

        FunctionAttachment** prev = &m_Attachment;
        FunctionAttachment* curr = m_Attachment;

        do
        {
            if(rtt_check<T>(curr))
            {
                *prev = curr->Next();
                delete curr;
                return;
            }

            prev = &(curr->Next());
            curr = curr->Next();
        } while(curr);
    }
public:
    /**
     * Allocate with a fixed block allocator for performance.
     *
     *   We'll be allocating lots of functions for a program, and the
     * actual size this struct is constant, thus we can use a fixed block
     * allocator to drastically improve performance.
     */
    [[nodiscard]] void* operator new(::std::size_t sz) noexcept;
    void operator delete(void* ptr) noexcept;
private:
    Function(
        const u8* const address, 
        const uSys codeSize, 
        const uSys localSize, 
        DynArray<const TypeInfo*>&& localTypes, 
        DynArray<uSys>&& localOffsets, 
        DynArray<FunctionArgument>&& arguments,
        const FunctionFlags flags,
        C8DynString&& name,
        FunctionAttachment* attachment
    ) noexcept
        : m_Address(address)
        , m_CodeSize(codeSize)
        , m_LocalSize(localSize)
        , m_LocalTypes(::std::move(localTypes))
        , m_LocalOffsets(::std::move(localOffsets))
        , m_Arguments(::std::move(arguments))
        , m_Flags(flags)
        , m_Name(::std::move(name))
        , m_Attachment(attachment)
    { }
private:
    /**
     * The address of the IR code.
     */
    const u8* m_Address;
    /**
     * The number of bytes of IR code.
     */
    uSys m_CodeSize;
    /**
     * The number of bytes that locals will take up.
     */
    uSys m_LocalSize;
    /**
     * The types of all locals.
     */
    DynArray<const TypeInfo*> m_LocalTypes;
    /**
     * The offsets of all locals.
     *
     *   This is one less than the number of locals as we can ignore the
     * first offset as it is always zero.
     */
    DynArray<uSys> m_LocalOffsets;
    DynArray<FunctionArgument> m_Arguments;
    FunctionFlags m_Flags;
    C8DynString m_Name;
    WeakRef<tau::ir::Module> m_Module;
    FunctionAttachment* m_Attachment;

    friend class FunctionBuilder;
};

class FunctionBuilder final
{
    DELETE_CM(FunctionBuilder);
public:
    FunctionBuilder() noexcept
        : m_Address(nullptr)
        , m_CodeSize(0)
        , m_LocalSize(0)
        , m_LocalTypesRaw{ }
        , m_LocalOffsetsRaw{ }
        , m_ArgumentsRaw{ }
        , m_LocalTypes(nullptr)
        , m_LocalOffsets(nullptr)
        , m_Arguments(nullptr)
        , m_Flags(InlineControl::Default, CallingConvention::Default, OptimizationControl::Default)
        , m_Name()
        , m_Attachment(nullptr)
    { }

    ~FunctionBuilder() noexcept
    {
        if(m_LocalTypes)
        {
            m_LocalTypes->~DynArray();
        }

        if(m_LocalOffsets)
        {
            m_LocalOffsets->~DynArray();
        }

        if(m_Arguments)
        {
            m_Arguments->~DynArray();
        }
    }

    FunctionBuilder& Address(const u8* const address) noexcept
    {
        m_Address = address;
        return *this;
    }

    FunctionBuilder& Address(const void* const address) noexcept
    {
        m_Address = reinterpret_cast<const u8*>(address);
        return *this;
    }

    FunctionBuilder& Address(const uPtr address) noexcept
    {
        m_Address = reinterpret_cast<const u8*>(address);
        return *this;
    }

    FunctionBuilder& CodeSize(const uSys codeSize) noexcept
    {
        m_CodeSize = codeSize;
        return *this;
    }

    FunctionBuilder& CodeSize() noexcept
    {
        m_CodeSize = 0;
        return *this;
    }

    template<uSys Size>
    FunctionBuilder& Code(const u8(&code)[Size]) noexcept
    {
        m_Address = static_cast<const u8*>(code);
        m_CodeSize = Size;
        return *this;
    }

    template<typename F>
    FunctionBuilder& Func(const F func) noexcept
    {
        m_Address = reinterpret_cast<const u8*>(func);
        m_CodeSize = 0;
        m_Flags = FunctionFlags(InlineControl::NoInline, CallingConvention::Cdecl, OptimizationControl::NoOptimize);
        LocalTypes();
        return *this;
    }

    FunctionBuilder& LocalTypes(const DynArray<const TypeInfo*>& localTypes) noexcept
    {
        if(m_LocalTypes)
        {
            m_LocalTypes->~DynArray();
        }

        if(m_LocalOffsets)
        {
            m_LocalOffsets->~DynArray();
        }

        m_LocalTypes = ::new(m_LocalTypesRaw) DynArray<const TypeInfo*>(localTypes);
        m_LocalSize = maxT(static_cast<iSys>(m_LocalTypes->Count()) - 1, 0);
        m_LocalOffsets = ::new(m_LocalOffsetsRaw) DynArray<uSys>(m_LocalSize);
        LoadLocalOffsets();
        return *this;
    }

    FunctionBuilder& LocalTypes(DynArray<const TypeInfo*>&& localTypes) noexcept
    {
        if(m_LocalTypes)
        {
            m_LocalTypes->~DynArray();
        }

        if(m_LocalOffsets)
        {
            m_LocalOffsets->~DynArray();
        }

        m_LocalTypes = ::new(m_LocalTypesRaw) DynArray<const TypeInfo*>(::std::move(localTypes));
        m_LocalSize = maxT(static_cast<iSys>(m_LocalTypes->Count()) - 1, 0);
        m_LocalOffsets = ::new(m_LocalOffsetsRaw) DynArray<uSys>(m_LocalSize);
        LoadLocalOffsets();
        return *this;
    }

    FunctionBuilder& LocalTypes() noexcept
    {
        if(m_LocalTypes)
        {
            m_LocalTypes->~DynArray();
        }

        if(m_LocalOffsets)
        {
            m_LocalOffsets->~DynArray();
        }

        m_LocalTypes = ::new(m_LocalTypesRaw) DynArray<const TypeInfo*>();
        m_LocalOffsets = ::new(m_LocalOffsetsRaw) DynArray<uSys>();
        return *this;
    }

    FunctionBuilder& Arguments(const DynArray<FunctionArgument>& arguments) noexcept
    {
        if(m_Arguments)
        {
            m_Arguments->~DynArray();
        }
        
        m_Arguments = ::new(m_ArgumentsRaw) DynArray<FunctionArgument>(arguments);
        return *this;
    }

    FunctionBuilder& Arguments(DynArray<FunctionArgument>&& arguments) noexcept
    {
        if(m_Arguments)
        {
            m_Arguments->~DynArray();
        }
        
        m_Arguments = ::new(m_ArgumentsRaw) DynArray<FunctionArgument>(::std::move(arguments));
        return *this;
    }

    FunctionBuilder& Arguments() noexcept
    {
        if(m_Arguments)
        {
            m_Arguments->~DynArray();
        }

        m_Arguments = ::new(m_ArgumentsRaw) DynArray<FunctionArgument>();
        return *this;
    }

    FunctionBuilder& Flags(const FunctionFlags flags) noexcept
    {
        m_Flags = flags;
        return *this;
    }

    FunctionBuilder& Flags(const InlineControl inlineControl, const CallingConvention callingConvention, const OptimizationControl optimizationControl) noexcept
    {
        m_Flags = FunctionFlags(inlineControl, callingConvention, optimizationControl);
        return *this;
    }

    FunctionBuilder& Flags() noexcept
    {
        m_Flags = FunctionFlags(InlineControl::Default, CallingConvention::Default, OptimizationControl::Default);
        return *this;
    }

    FunctionBuilder& Name(const C8DynString& name) noexcept
    {
        m_Name = name;
        return *this;
    }

    FunctionBuilder& Name(C8DynString&& name) noexcept
    {
        m_Name = ::std::move(name);
        return *this;
    }

    FunctionBuilder& Name(const c8* const name) noexcept
    {
        m_Name = C8DynString(name);
        return *this;
    }

    template<uSys Len>
    FunctionBuilder& Name(const c8(&name)[Len]) noexcept
    {
        m_Name = C8DynString::FromStatic(name);
        return *this;
    }

    FunctionBuilder& Name(const char* const name) noexcept
    {
        m_Name = C8DynString(reinterpret_cast<const c8*>(name));
        return *this;
    }

    FunctionBuilder& Name() noexcept
    {
        m_Name = C8DynString();
        return *this;
    }

    FunctionBuilder& Attachment(FunctionAttachment* attachment) noexcept
    {
        m_Attachment = attachment;
        return *this;
    }

    template<typename T, typename... Args>
    FunctionBuilder& Attachment(Args&&... args) noexcept
    {
        m_Attachment = new(::std::nothrow) T(TauAllocatorUtils::Forward<Args>(args)...);
        return *this;
    }

    FunctionBuilder& Attachment() noexcept
    {
        m_Attachment = nullptr;
        return *this;
    }

    Function* Build() noexcept
    {
        Function* ret = new Function(
            m_Address,
            m_CodeSize,
            m_LocalSize,
            ::std::move(*m_LocalTypes),
            ::std::move(*m_LocalOffsets),
            ::std::move(*m_Arguments),
            m_Flags,
            ::std::move(m_Name),
            m_Attachment
        );

        m_LocalTypes->~DynArray();
        m_LocalOffsets->~DynArray();
        m_Arguments->~DynArray();
        m_Name.~C8DynString();
        m_Attachment = nullptr;

        return ret;
    }
private:
    void LoadLocalOffsets() noexcept;

    DynArray<const TypeInfo*>& GetLocalTypes() noexcept { return *m_LocalTypes; }
    DynArray<uSys>& GetLocalOffsets() noexcept { return *m_LocalOffsets; }
private:
    const u8* m_Address;
    uSys m_CodeSize;
    uSys m_LocalSize;

    u8 m_LocalTypesRaw[sizeof(DynArray<const TypeInfo*>)];
    u8 m_LocalOffsetsRaw[sizeof(DynArray<uSys>)];
    u8 m_ArgumentsRaw[sizeof(DynArray<FunctionArgument>)];

    DynArray<const TypeInfo*>* m_LocalTypes;
    DynArray<uSys>* m_LocalOffsets;
    DynArray<FunctionArgument>* m_Arguments;

    FunctionFlags m_Flags;
    C8DynString m_Name;
    FunctionAttachment* m_Attachment;
};

inline void* PrepareNativeFunctionPointer(const Function* const function) noexcept
{
    return const_cast<void*>(reinterpret_cast<const void*>(function->Address()));
}

inline void CallNativeFunctionPointer(const Function* const function, DynArray<u64>& arguments, DynArray<u8>& stack, uSys& stackPointer) noexcept
{
    reinterpret_cast<void(*)(DynArray<u64>&, DynArray<u8>&, uSys&)>(PrepareNativeFunctionPointer(function))(arguments, stack, stackPointer);
}

}
