#include "TauIR/Function.hpp"
#include "TauIR/Emulator.hpp"
#include "TauIR/Module.hpp"
#include "TauIR/TypeInfo.hpp"
#include "TauIR/ByteCodeDumper.hpp"

#include <ConPrinter.hpp>

#include "TauIR/ssa/SsaTypes.hpp"
#include "TauIR/ssa/opto/ConstantProp.hpp"

static void TestSsa() noexcept;
static void TestPrint() noexcept;
static void TestCond() noexcept;

int main(int argCount, char* args[])
{
    Console::Init();

    TestSsa();
    // return 0;

    // const u8 code[] = {
    //     0x10,                   // Push.0
    //     0x13,                   // Push.3
    //     0x00,                   // Nop
    //     0x90, 0x10, 0x05, 0x00, // Push.N 5
    //     0x12,                   // Push.2
    //     0x00,                   // Nop
    //     0x40,                   // Pop.Arg.0
    //     0x1D                    // Ret
    // };

    // i32 subVal = 83;
    // i32* subValPtr = &subVal;

    // const u8* bytes = reinterpret_cast<const u8*>(&subValPtr);

    // // Expect -631
    // const u8 codeAdd[] = {
    //     0x8B, 0x00, 0x2A, 0x00, 0x00, 0x00, // Const.N 42
    //     0x8B, 0x00, 0x11, 0x00, 0x00, 0x00, // Const.N 17
    //     0x38,                               // Mul.i32
    //     0x8B, 0x00, bytes[0], bytes[1], bytes[2], bytes[3], // Const.N high(&subVal)
    //     0x8B, 0x00, bytes[4], bytes[5], bytes[6], bytes[7], // Const.N  low(&subVal)
    //     0x20,                               // Pop.0
    //     0x90, 0x11, 0x00, 0x00,             // Push.Ptr 0
    //     0x36,                               // Sub.i32
    //     0x29,                               // Expand.SX.4.8
    //     0x40,                               // Pop.Arg.0
    //     0x1D                                // Ret
    // };
    //
    // // Expect 0
    // const u8 codeDiv[] = {
    //     0x18,                               // Const.4
    //     0x8B, 0x00, 0x14, 0x00, 0x00, 0x00, // Const.N 17
    //     0x3A,                               // Div.i32
    //     // 0xA0, 0x26, 0x04, 0x00,             // Pop.Count 4
    //     0x29,                               // Expand.SX.4.8
    //     0x40,                               // Pop.Arg.0
    //     0x1D,                               // Ret
    // };

    const u8 codeSquare[] = {
        0x30,   // Push.Arg.0
        0x30,   // Push.Arg.0
        0x39,   // Mul.i64
        0x40,   // Pop.Arg.0
        0x1D    // Ret
    };

    // Expect 259
    const u8 codeMain[] = {
        0x8B, 0x00, 0x10, 0x00, 0x00, 0x00, // Const.N 16
        0x29,                               // Expand.SX.4.8
        0x40,                               // Pop.Arg.0
        0x1C, 0x01, 0x00, 0x00, 0x00,       // Call <codeSquare>
        0x17,                               // Const.3
        0x30,                               // Push.Arg.0
        0x2A,                               // Trunc.8.4
        0x34,                               // Add.i32
        0x29,                               // Expand.SX.4.8
        0x40,                               // Pop.Arg.0
        0x1D                                // Ret
    };

    using TypeInfo = tau::ir::TypeInfo;
    using Function = tau::ir::Function;
    using FunctionFlags = tau::ir::FunctionFlags;
    using Module = tau::ir::Module;

    const TypeInfo* intType = new TypeInfo(4, u8"int");
    const TypeInfo* byteType = new TypeInfo(1, u8"byte");
    const TypeInfo* floatType = new TypeInfo(4, u8"float");
    const TypeInfo* doubleType = new TypeInfo(8, u8"double");
    const TypeInfo* pointerType = new TypeInfo(sizeof(void*), u8"pointer");

    DynArray<const TypeInfo*> types(6);
    types[0] = intType;
    types[1] = byteType;
    types[2] = doubleType;
    types[3] = byteType;
    types[4] = floatType;
    types[5] = intType;

    DynArray<const TypeInfo*> entryTypes(1);
    entryTypes[0] = TypeInfo::AddPointer(intType);

    DynArray<const Function*> functions(2);
    functions[0] = new Function(codeMain, sizeof(codeMain), DynArray<const TypeInfo*>(), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default));
    functions[1] = new Function(codeSquare, sizeof(codeSquare), DynArray<const TypeInfo*>(), FunctionFlags(tau::ir::InlineControl::Default, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default));

    ::std::vector<Ref<Module>> modules(1);
    modules[0].Reset(::std::move(functions));

    modules[0]->Name() = u8"Main";

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules);
    ConPrinter::Print("\n");
    ::tau::ir::DumpFunction(modules[0]->Functions()[1], 1, modules);
    ConPrinter::Print("\n");

    tau::ir::Emulator emulator(::std::move(modules));

    emulator.Execute();

    const u64 retVal = emulator.ReturnVal();

    ConPrinter::Print("Return Val: {} ({}) [0x{X}]\n", retVal, static_cast<i64>(retVal), retVal);

    delete intType;
    delete byteType;
    delete floatType;
    delete doubleType;
    delete pointerType;

    TestPrint();

    TestCond();

    return static_cast<int>(retVal);
}

static void TestSsa() noexcept
{
    const u8 code0[] = {
        0x30, 0x04, 0x04, 0x00, 0x00, 0x00,                                     // i32 %1 = 4
        0x31, 0x04, 0x01, 0x00, 0x00, 0x00,                                     // i32 %2 = %1
        0x52, 0x02, 0x04, 0x02, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,       // i32 %3 = %2 * 7
        0x3A, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x04, 0x00, 0x17, // void* %4 = [%a0 + %a1 * 4 + 23]
        0x36, 0x84, 0x80, 0x04, 0x00, 0x00, 0x00,                               // i32* %5 = %4
        0x39, 0x04, 0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,             // Store i32 %5, %3
        0x37, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00,                               // u32 %6 = RCast u32 %3
        0x33, 0x09, 0x08, 0x06, 0x00, 0x00, 0x00,                               // u64 %7 = Expand.ZX u32 %6
        0x50, 0x05, 0x09, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x80,       // u64 %8 = %7 << %a2
    };

    const tau::ir::ssa::SsaCustomTypeRegistry registry;

    tau::ir::ssa::DumpSsa(code0, sizeof(code0), 0, registry);

    tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);

    visitor.Traverse(code0, sizeof(code0), 8);

    tau::ir::ssa::DumpSsa(visitor.Writer().Buffer(), visitor.Writer().Size(), 0, registry);
}

static void NativePrint(const c8* str) noexcept
{
    ConPrinter::Print(str);
}

static void NativePrintLn(const c8* str) noexcept
{
    ConPrinter::PrintLn(str);
}

static void NativePrintHelloWorld() noexcept
{
    ConPrinter::PrintLn("Hello World!");
}

static void TestPrint() noexcept
{
    ConPrinter::PrintLn("\n\nCalling Native Test Print:");

    using TypeInfo = tau::ir::TypeInfo;
    using Function = tau::ir::Function;
    using FunctionFlags = tau::ir::FunctionFlags;
    using Module = tau::ir::Module;
    
    constexpr u8 codeMain[] = {
        0x80, 0x1C, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, // Call.Ext <0:NativePrintHelloWorld>
        0x1D                                // Ret
    };
    
    DynArray<const Function*> nativeFunctions(3);
    nativeFunctions[0] = new Function(reinterpret_cast<const void*>(NativePrint), 0, DynArray<const TypeInfo*>(), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Cdecl, tau::ir::OptimizationControl::NoOptimize));
    nativeFunctions[1] = new Function(reinterpret_cast<const void*>(NativePrintLn), 0, DynArray<const TypeInfo*>(), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Cdecl, tau::ir::OptimizationControl::NoOptimize));
    nativeFunctions[2] = new Function(reinterpret_cast<const void*>(NativePrintHelloWorld), 0, DynArray<const TypeInfo*>(), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Cdecl, tau::ir::OptimizationControl::NoOptimize));

    DynArray<const Function*> functions(1);
    functions[0] = new Function(codeMain, sizeof(codeMain), DynArray<const TypeInfo*>(), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default));

    ::std::vector<Ref<Module>> modules(2);
    modules[0].Reset(::std::move(functions), false);
    modules[1].Reset(::std::move(nativeFunctions), true);

    modules[0]->Name() = u8"Main";
    modules[1]->Name() = u8"Native";

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules);
    ConPrinter::Print("\n");


    tau::ir::Emulator emulator(::std::move(modules));

    emulator.Execute();

    const u64 retVal = emulator.ReturnVal();

    ConPrinter::Print("Return Val: {} ({}) [0x{X}]\n", retVal, static_cast<i64>(retVal), retVal);
}

static void NativePrintSuccess(const DynArray<u64>& arguments) noexcept
{
    ConPrinter::PrintLn("Success, {} is not greater than {}.", arguments[0], arguments[1]);
}

static void NativePrintFail(const DynArray<u64>& arguments) noexcept
{
    ConPrinter::PrintLn("Fail, {} was greater than {}...", arguments[0], arguments[1]);
}

static void TestCond() noexcept
{
    ConPrinter::PrintLn("\n\nConditional Branching Test:");

    using TypeInfo = tau::ir::TypeInfo;
    using Function = tau::ir::Function;
    using FunctionFlags = tau::ir::FunctionFlags;
    using Module = tau::ir::Module;
    
    constexpr u8 codeMain[] = {
        0x18,                                           // Const.4
        0x17,                                           // Const.3
        0x0F,                                           // Dup.8
        0x29,                                           // Expand.SX.4.8
        0x40,                                           // Pop.Arg.0
        0x29,                                           // Expand.SX.4.8
        0x41,                                           // Pop.Arg.1
        0x80, 0x75,                                     // Comp.i32.Greater
        0x70, 0x0D, 0x00, 0x00, 0x00,                   // Jump.True .isGreater
        0x80, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, // Call.Ext <0:NativePrintSuccess>
        0x1E, 0x08, 0x00, 0x00, 0x00,                   // Jump .ret
                                                        // .isGreater:
        0x80, 0x1C, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, //   Call.Ext <0:NativePrintFail>
                                                        // .ret
        0x1D                                            //   Ret
    };

    DynArray<const Function*> nativeFunctions(2);
    nativeFunctions[0] = new Function(reinterpret_cast<const void*>(NativePrintSuccess), 0, DynArray<const TypeInfo*>(), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Cdecl, tau::ir::OptimizationControl::NoOptimize));
    nativeFunctions[1] = new Function(reinterpret_cast<const void*>(NativePrintFail), 0, DynArray<const TypeInfo*>(), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Cdecl, tau::ir::OptimizationControl::NoOptimize));

    DynArray<const Function*> functions(1);
    functions[0] = new Function(codeMain, sizeof(codeMain), DynArray<const TypeInfo*>(), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default));

    ::std::vector<Ref<Module>> modules(2);
    modules[0].Reset(::std::move(functions), false);
    modules[1].Reset(::std::move(nativeFunctions), true);

    modules[0]->Name() = u8"Main";
    modules[1]->Name() = u8"Native";

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules);
    ConPrinter::Print("\n");

    tau::ir::Emulator emulator(::std::move(modules));

    emulator.Execute();

    const u64 retVal = emulator.ReturnVal();

    ConPrinter::Print("Return Val: {} ({}) [0x{X}]\n", retVal, static_cast<i64>(retVal), retVal);
}
