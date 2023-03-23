#include "TauIR/Function.hpp"
#include "TauIR/Emulator.hpp"
#include "TauIR/Module.hpp"
#include "TauIR/TypeInfo.hpp"
#include "TauIR/ByteCodeDumper.hpp"
#include "TauIR/IrToSsa.hpp"
#include "TauIR/FunctionNameMangler.hpp"

#include <ConPrinter.hpp>

#include "TauIR/ssa/SsaTypes.hpp"
#include "TauIR/ssa/opto/ConstantProp.hpp"

static void TestSsa() noexcept;
static void TestIrToSsa() noexcept;
static void TestIrToSsaCallInd() noexcept;
static void TestCall() noexcept;
static void TestCallInd() noexcept;
static void TestPrint() noexcept;
static void TestCond() noexcept;

int main(int argCount, char* args[])
{
    Console::Init();

    TestSsa();
    TestIrToSsa();
    TestIrToSsaCallInd();
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

    using namespace tau::ir;

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

    Function* const mainFunc = FunctionBuilder()
        .Code(codeMain)
        .LocalTypes()
        .Arguments()
        .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default)
        .Name(u8"Main")
        .Build();
    Function* const squareFunc = FunctionBuilder()
        .Code(codeSquare)
        .LocalTypes()
        .Arguments(squareArgs)
        .Flags()
        .Name(u8"Square")
        .Build();

    FunctionList functions(2);
    functions[0] = mainFunc;
    functions[1] = squareFunc;

    ModuleList modules;
    {
        modules.push_back(ModuleBuilder()
            .Functions(::std::move(functions))
            .Emulated()
            .Name(u8"Main")
            .Build());
    }

    ::tau::ir::DumpFunction(mainFunc, 0, modules, 0);
    ConPrinter::PrintLn();
    ::tau::ir::DumpFunction(squareFunc, 1, modules, 0);
    ConPrinter::PrintLn();

    const tau::ir::ssa::SsaCustomTypeRegistry registry;
    tau::ir::IrToSsa::TransformFunction(mainFunc, modules, 0);
    tau::ir::IrToSsa::TransformFunction(squareFunc, modules, 0);

    tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
    tau::ir::ssa::DumpSsa(squareFunc, 1, registry);

    {
        tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
        visitor.Traverse(mainFunc);
        visitor.UpdateAttachment(mainFunc);

        tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
    }

    {
        tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
        visitor.Traverse(squareFunc);
        visitor.UpdateAttachment(mainFunc);

        tau::ir::ssa::DumpSsa(squareFunc, 1, registry);
    }

    ConPrinter::PrintLn();
}

static void TestIrToSsaCallInd() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("Test IR to SSA Call Ind:");

    using namespace tau::ir;

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
        0x20,                               // Pop.0
        0x80, 0x1D, 0x00, 0x00,             // Call.Ind 0
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

    const C8DynString squareMangledName = MangleFunctionName(squareArgs);

    ConPrinter::PrintLn("Square Mangled Name: {}", squareMangledName);

    DynArray<const TypeInfo*> mainLocalTypes(1);
    mainLocalTypes[0] = new TypeInfo(4, squareMangledName);

    Function* const mainFunc = FunctionBuilder()
        .Code(codeMain)
        .LocalTypes(mainLocalTypes)
        .Arguments()
        .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default)
        .Name(u8"Main")
        .Build();
    Function* const squareFunc = FunctionBuilder()
        .Code(codeSquare)
        .LocalTypes()
        .Arguments(squareArgs)
        .Flags()
        .Name(u8"Square")
        .Build();

    FunctionList functions(2);
    functions[0] = mainFunc;
    functions[1] = squareFunc;

    ModuleList modules;
    {
        modules.push_back(ModuleBuilder()
            .Functions(::std::move(functions))
            .Emulated()
            .Name(u8"Main")
            .Build());
    }

    ::tau::ir::DumpFunction(mainFunc, 0, modules, 0);
    ConPrinter::PrintLn();
    ::tau::ir::DumpFunction(squareFunc, 1, modules, 0);
    ConPrinter::PrintLn();

    const tau::ir::ssa::SsaCustomTypeRegistry registry;
    tau::ir::IrToSsa::TransformFunction(mainFunc, modules, 0);
    tau::ir::IrToSsa::TransformFunction(squareFunc, modules, 0);

    tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
    tau::ir::ssa::DumpSsa(squareFunc, 1, registry);

    {
        tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
        visitor.Traverse(mainFunc);
        visitor.UpdateAttachment(mainFunc);

        tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
    }

    {
        tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
        visitor.Traverse(squareFunc);
        visitor.UpdateAttachment(mainFunc);

        tau::ir::ssa::DumpSsa(squareFunc, 1, registry);
    }

    ConPrinter::PrintLn();
}

static void TestCall() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("Test Call (Expect 259):");

    using namespace tau::ir;

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

    FunctionList functions(2);
    {
        DynArray<FunctionArgument> squareArgs(1);
        squareArgs[0] = FunctionArgument(true, 0);

        functions[0] = FunctionBuilder()
            .Code(codeMain)
            .LocalTypes()
            .Arguments()
            .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default)
            .Name(u8"Main")
            .Build();
        functions[1] = FunctionBuilder()
            .Code(codeSquare)
            .LocalTypes()
            .Arguments(squareArgs)
            .Flags()
            .Name(u8"Square")
            .Build();
    }

    ModuleList modules;
    {
        modules.push_back(ModuleBuilder()
            .Functions(::std::move(functions))
            .Emulated()
            .Name(u8"Main")
            .Build());
    }

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules, 0);
    ConPrinter::PrintLn();
    ::tau::ir::DumpFunction(modules[0]->Functions()[1], 1, modules, 0);
    ConPrinter::PrintLn();

    tau::ir::Emulator emulator(modules);
    emulator.Execute();

    const u64 retVal = emulator.ReturnVal();
    ConPrinter::PrintLn("Return Val: {} ({}) [0x{X}]", retVal, static_cast<i64>(retVal), retVal);

    ConPrinter::PrintLn();
}

static void TestCallInd() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("Test Call Indirect (Expect 259):");

    using namespace tau::ir;

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
        0x20,                               // Pop.0
        0x80, 0x1D, 0x00, 0x00,             // Call.Ind 0
        0x17,                               // Const.3
        0x30,                               // Push.Arg.0
        0x2A,                               // Trunc.8.4
        0x34,                               // Add.i32
        0x29,                               // Expand.SX.4.8
        0x40,                               // Pop.Arg.0
        0x1D                                // Ret
    };

    FunctionList functions(2);
    {
        DynArray<FunctionArgument> squareArgs(1);
        squareArgs[0] = FunctionArgument(true, 0);

        const C8DynString squareMangledName = MangleFunctionName(squareArgs);

        ConPrinter::PrintLn("Square Mangled Name: {}", squareMangledName);
        
        DynArray<const TypeInfo*> mainLocalTypes(1);
        mainLocalTypes[0] = new TypeInfo(4, squareMangledName);

        functions[0] = FunctionBuilder()
            .Code(codeMain)
            .LocalTypes(mainLocalTypes)
            .Arguments()
            .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default)
            .Name(u8"Main")
            .Build();;
        functions[1] = FunctionBuilder()
            .Code(codeSquare)
            .LocalTypes()
            .Arguments(squareArgs)
            .Flags()
            .Name(u8"Square")
            .Build();
    }

    ModuleList modules;
    {
        modules.push_back(ModuleBuilder()
            .Functions(::std::move(functions))
            .Emulated()
            .Name(u8"Main")
            .Build());
    }

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules, 0);
    ConPrinter::PrintLn();
    ::tau::ir::DumpFunction(modules[0]->Functions()[1], 1, modules, 0);
    ConPrinter::PrintLn();

    tau::ir::Emulator emulator(modules);
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

    using namespace tau::ir;

    constexpr u8 codeMain[] = {
        0x80, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, // Call.Ext <0:NativePrintHelloWorld>
        0x1D                                // Ret
    };
    
    FunctionList nativeFunctions(1);
    {
        nativeFunctions[0] = FunctionBuilder()
            .Func(NativePrintHelloWorld)
            .Arguments()
            .Name(u8"NativePrintHelloWorld")
            .Build();
    }

    FunctionList functions(1);
    functions[0] = FunctionBuilder()
        .Code(codeMain)
        .LocalTypes()
        .Arguments()
        .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default)
        .Name(u8"Main")
        .Build();

    ModuleList modules;
    {
        modules.push_back(ModuleBuilder()
            .Functions(::std::move(functions))
            .Emulated()
            .Name(u8"Main")
            .Build());
        modules.push_back(ModuleBuilder()
            .Functions(::std::move(nativeFunctions))
            .Native()
            .Name(u8"Native")
            .Build());
    }

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules, 0);
    ConPrinter::Print("\n");

    tau::ir::Emulator emulator(modules);
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

    using namespace tau::ir;

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

    FunctionList nativeFunctions(2);
    {
        nativeFunctions[0] = FunctionBuilder()
            .Func(NativePrintSuccess)
            .Arguments()
            .Name(u8"NativePrintSuccess")
            .Build();;
        nativeFunctions[1] = FunctionBuilder()
            .Func(NativePrintFail)
            .Arguments()
            .Name(u8"NativePrintFail")
            .Build();;
    }

    FunctionList functions(1);
    {
        functions[0] = FunctionBuilder()
            .Code(codeMain)
            .LocalTypes()
            .Arguments()
            .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default)
            .Name(u8"Main")
            .Build();;
    }

    ModuleList modules;
    {
        modules.push_back(ModuleBuilder()
            .Functions(::std::move(functions))
            .Emulated()
            .Name(u8"Main")
            .Build());
        modules.push_back(ModuleBuilder()
            .Functions(::std::move(nativeFunctions))
            .Native()
            .Name(u8"Native")
            .Build());
    }

    ::tau::ir::DumpFunction(modules[0]->Functions()[0], 0, modules, 0);
    ConPrinter::Print("\n");

    tau::ir::Emulator emulator(modules);
    emulator.Execute();

    ConPrinter::PrintLn();
}
