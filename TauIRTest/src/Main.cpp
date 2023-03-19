#include "TauIR/Function.hpp"
#include "TauIR/Emulator.hpp"
#include "TauIR/Module.hpp"
#include "TauIR/TypeInfo.hpp"
#include "TauIR/ByteCodeDumper.hpp"
#include "TauIR/IrToSsa.hpp"

#include <ConPrinter.hpp>

#include "TauIR/ssa/SsaTypes.hpp"
#include "TauIR/ssa/opto/ConstantProp.hpp"

static void TestSsa() noexcept;
static void TestIrToSsa() noexcept;
static void TestCall() noexcept;
static void TestCallInd() noexcept;
static void TestPrint() noexcept;
static void TestCond() noexcept;

int main(int argCount, char* args[])
{
    Console::Init();

    TestSsa();
    TestIrToSsa();
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
    
    using TypeInfo = tau::ir::TypeInfo;

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
    
    delete intType;
    delete byteType;
    delete floatType;
    delete doubleType;
    delete pointerType;

    TestCall();
    TestCallInd();
    TestPrint();
    TestCond();

    return 0;
}

static void TestSsa() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("Test SSA:");

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

    ConPrinter::PrintLn();
}

static void TestIrToSsa() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("Test IR to SSA:");

    using TypeInfo = tau::ir::TypeInfo;
    using Function = tau::ir::Function;
    using FunctionArgument = tau::ir::FunctionArgument;
    using FunctionFlags = tau::ir::FunctionFlags;
    using Module = tau::ir::Module;

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

    DynArray<FunctionArgument> squareArgs(1);
    squareArgs[0] = FunctionArgument(true, 0);

    Function* const mainFunc = new Function(
        codeMain,
        sizeof(codeMain),
        DynArray<const TypeInfo*>(),
        DynArray<FunctionArgument>(0),
        FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default)
    );
    Function* const squareFunc = new Function(
        codeSquare,
        sizeof(codeSquare),
        DynArray<const TypeInfo*>(),
        squareArgs,
        FunctionFlags(tau::ir::InlineControl::Default, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default)
    );
    mainFunc->Name() = u8"Main";
    squareFunc->Name() = u8"Square";

    DynArray<const Function*> functions(2);
    functions[0] = mainFunc;
    functions[1] = squareFunc;

    ::std::vector<Ref<Module>> modules(1);
    {
        modules[0].Reset(::std::move(functions));
        modules[0]->Name() = u8"Main";
    }

    ::tau::ir::DumpFunction(mainFunc, 0, modules, 0);
    ConPrinter::PrintLn();
    ::tau::ir::DumpFunction(squareFunc, 1, modules, 0);
    ConPrinter::PrintLn();

    const tau::ir::ssa::SsaCustomTypeRegistry registry;
    const tau::ir::ssa::SsaWriter mainWriter = tau::ir::IrToSsa::TransformFunction(mainFunc, modules, 0);
    const tau::ir::ssa::SsaWriter squareWriter = tau::ir::IrToSsa::TransformFunction(squareFunc, modules, 0);

    tau::ir::ssa::DumpSsa(mainWriter.Buffer(), mainWriter.Size(), 0, registry);
    tau::ir::ssa::DumpSsa(squareWriter.Buffer(), squareWriter.Size(), 1, registry);

    {
        tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
        visitor.Traverse(mainWriter.Buffer(), mainWriter.Size(), mainWriter.IdIndex());

        tau::ir::ssa::DumpSsa(visitor.Writer().Buffer(), visitor.Writer().Size(), 0, registry);
    }

    {
        tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
        visitor.Traverse(squareWriter.Buffer(), squareWriter.Size(), squareWriter.IdIndex());

        tau::ir::ssa::DumpSsa(visitor.Writer().Buffer(), visitor.Writer().Size(), 1, registry);
    }

    ConPrinter::PrintLn();
}

static void TestCall() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("Test Call (Expect 259):");

    using TypeInfo = tau::ir::TypeInfo;
    using Function = tau::ir::Function;
    using FunctionArgument = tau::ir::FunctionArgument;
    using FunctionFlags = tau::ir::FunctionFlags;
    using Module = tau::ir::Module;

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

    DynArray<const Function*> functions(2);
    {
        DynArray<FunctionArgument> squareArgs(1);
        squareArgs[0] = FunctionArgument(true, 0);

        Function* const mainFunc = new Function(
            codeMain,
            sizeof(codeMain),
            DynArray<const TypeInfo*>(),
            DynArray<FunctionArgument>(0),
            FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default)
        );
        Function* const squareFunc = new Function(
            codeSquare,
            sizeof(codeSquare),
            DynArray<const TypeInfo*>(),
            squareArgs,
            FunctionFlags(tau::ir::InlineControl::Default, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default)
        );
        mainFunc->Name() = u8"Main";
        squareFunc->Name() = u8"Square";

        functions[0] = mainFunc;
        functions[1] = squareFunc;
    }

    ::std::vector<Ref<Module>> modules(1);
    {
        modules[0].Reset(::std::move(functions));
        modules[0]->Name() = u8"Main";
    }

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules, 0);
    ConPrinter::PrintLn();
    ::tau::ir::DumpFunction(modules[0]->Functions()[1], 1, modules, 0);
    ConPrinter::PrintLn();

    tau::ir::Emulator emulator(::std::move(modules));
    emulator.Execute();

    const u64 retVal = emulator.ReturnVal();
    ConPrinter::PrintLn("Return Val: {} ({}) [0x{X}]", retVal, static_cast<i64>(retVal), retVal);

    ConPrinter::PrintLn();
}

static void TestCallInd() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("Test Call Indirect (Expect 259):");

    using TypeInfo = tau::ir::TypeInfo;
    using Function = tau::ir::Function;
    using FunctionArgument = tau::ir::FunctionArgument;
    using FunctionFlags = tau::ir::FunctionFlags;
    using Module = tau::ir::Module;

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
        0x15,                               // Const.1 <Square>
        0x80, 0x1D,                         // Call.Ind
        0x17,                               // Const.3
        0x30,                               // Push.Arg.0
        0x2A,                               // Trunc.8.4
        0x34,                               // Add.i32
        0x29,                               // Expand.SX.4.8
        0x40,                               // Pop.Arg.0
        0x1D                                // Ret
    };

    DynArray<const Function*> functions(2);
    {
        DynArray<FunctionArgument> squareArgs(1);
        squareArgs[0] = FunctionArgument(true, 0);

        Function* const mainFunc = new Function(
            codeMain, 
            sizeof(codeMain), 
            DynArray<const TypeInfo*>(), 
            DynArray<FunctionArgument>(0), 
            FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default)
        );
        Function* const squareFunc = new Function(
            codeSquare, 
            sizeof(codeSquare), 
            DynArray<const TypeInfo*>(), 
            squareArgs, 
            FunctionFlags(tau::ir::InlineControl::Default, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default)
        );

        mainFunc->Name() = u8"Main";
        squareFunc->Name() = u8"Square";

        functions[0] = mainFunc;
        functions[1] = squareFunc;
    }

    ::std::vector<Ref<Module>> modules(1);
    {
        modules[0].Reset(::std::move(functions));
        modules[0]->Name() = u8"Main";
    }

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules, 0);
    ConPrinter::PrintLn();
    ::tau::ir::DumpFunction(modules[0]->Functions()[1], 1, modules, 0);
    ConPrinter::PrintLn();

    tau::ir::Emulator emulator(::std::move(modules));
    emulator.Execute();

    const u64 retVal = emulator.ReturnVal();
    ConPrinter::PrintLn("Return Val: {} ({}) [0x{X}]", retVal, static_cast<i64>(retVal), retVal);

    ConPrinter::PrintLn();
}

static void NativePrintHelloWorld() noexcept
{
    ConPrinter::PrintLn("Hello World!");
}

static void TestPrint() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("\nCalling Native Test Print:");

    using TypeInfo = tau::ir::TypeInfo;
    using Function = tau::ir::Function;
    using FunctionArgument = tau::ir::FunctionArgument;
    using FunctionFlags = tau::ir::FunctionFlags;
    using Module = tau::ir::Module;
    
    constexpr u8 codeMain[] = {
        0x80, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, // Call.Ext <0:NativePrintHelloWorld>
        0x1D                                // Ret
    };
    
    DynArray<const Function*> nativeFunctions(1);
    {
        Function* const nativePrintHelloWorld = new Function(reinterpret_cast<const void*>(NativePrintHelloWorld), 0, DynArray<const TypeInfo*>(), DynArray<FunctionArgument>(0), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Cdecl, tau::ir::OptimizationControl::NoOptimize));
        nativePrintHelloWorld->Name() = u8"NativePrintHelloWorld";
        nativeFunctions[0] = nativePrintHelloWorld;
    }

    DynArray<const Function*> functions(1);
    functions[0] = new Function(codeMain, sizeof(codeMain), DynArray<const TypeInfo*>(), DynArray<FunctionArgument>(0), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default));

    ::std::vector<Ref<Module>> modules(2);
    {
        modules[0].Reset(::std::move(functions), false);
        modules[1].Reset(::std::move(nativeFunctions), true);

        modules[0]->Name() = u8"Main";
        modules[1]->Name() = u8"Native";
    }

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules, 0);
    ConPrinter::Print("\n");

    tau::ir::Emulator emulator(::std::move(modules));
    emulator.Execute();

    ConPrinter::PrintLn();
}

static void NativePrintSuccess(const DynArray<u64>& arguments, DynArray<u8>& stack, uSys& stackPointer) noexcept
{
    ConPrinter::PrintLn("Success, {} is not greater than {}.", arguments[0], arguments[1]);
}

static void NativePrintFail(const DynArray<u64>& arguments, DynArray<u8>& stack, uSys& stackPointer) noexcept
{
    ConPrinter::PrintLn("Fail, {} was greater than {}...", arguments[0], arguments[1]);
}

static void TestCond() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("Conditional Branching Test:");

    using TypeInfo = tau::ir::TypeInfo;
    using Function = tau::ir::Function;
    using FunctionArgument = tau::ir::FunctionArgument;
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
    {
        Function* const nativePrintSuccess = new Function(reinterpret_cast<const void*>(NativePrintSuccess), 0, DynArray<const TypeInfo*>(), DynArray<FunctionArgument>(0), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Cdecl, tau::ir::OptimizationControl::NoOptimize));
        Function* const nativePrintFail = new Function(reinterpret_cast<const void*>(NativePrintFail), 0, DynArray<const TypeInfo*>(), DynArray<FunctionArgument>(0), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Cdecl, tau::ir::OptimizationControl::NoOptimize));

        nativePrintSuccess->Name() = u8"NativePrintSuccess";
        nativePrintFail->Name() = u8"NativePrintFail";

        nativeFunctions[0] = nativePrintSuccess;
        nativeFunctions[1] = nativePrintFail;
    }

    DynArray<const Function*> functions(1);
    {
        Function* const mainFunc = new Function(codeMain, sizeof(codeMain), DynArray<const TypeInfo*>(), DynArray<FunctionArgument>(0), FunctionFlags(tau::ir::InlineControl::NoInline, tau::ir::CallingConvention::Default, tau::ir::OptimizationControl::Default));

        mainFunc->Name() = u8"Main";

        functions[0] = mainFunc;
    }

    ::std::vector<Ref<Module>> modules(2);
    {
        modules[0].Reset(::std::move(functions), false);
        modules[1].Reset(::std::move(nativeFunctions), true);

        modules[0]->Name() = u8"Main";
        modules[1]->Name() = u8"Native";
    }

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules, 0);
    ConPrinter::Print("\n");

    tau::ir::Emulator emulator(::std::move(modules));
    emulator.Execute();

    ConPrinter::PrintLn();
}
