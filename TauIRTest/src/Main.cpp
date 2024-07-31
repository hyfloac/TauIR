#include "TauIR/Function.hpp"
#include "TauIR/Emulator.hpp"
#include "TauIR/Module.hpp"
#include "TauIR/TypeInfo.hpp"
#include "TauIR/ByteCodeDumper.hpp"
#include "TauIR/IrToSsa.hpp"
#include "TauIR/FunctionNameMangler.hpp"
#include "TauIR/file/BinaryObject.hpp"

#include <ConPrinter.hpp>

#include "TauIR/ssa/SsaTypes.hpp"
#include "TauIR/ssa/opto/ConstantProp.hpp"
#include "TauIR/ssa/opto/DeadCodeElimination.hpp"
#include "TauIR/ssa/opto/Inliner.hpp"

static void TestSsa() noexcept;
static void TestIrToSsa() noexcept;
static void TestIrToSsaCallInd() noexcept;
static void TestCall() noexcept;
static void TestCallInd() noexcept;
static void TestPrint() noexcept;
static void TestCond() noexcept;
static void TestWriteFile() noexcept;

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
    TestWriteFile();

    return 0;
}

static void TestSsa() noexcept
{
    using namespace tau::ir::ssa;

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
        0x46, 0x09, 0x08, 0x00, 0x00, 0x00                                      // Ret u64 %8
    };

    const SsaCustomTypeRegistry registry;

    DumpSsa(code0, sizeof(code0), 0, registry);

    ConPrinter::PrintLn();

    ::std::vector<SsaCustomType> customTypes;
    customTypes.emplace_back(SsaType::I32);
    customTypes.emplace_back(SsaType::I32);
    customTypes.emplace_back(SsaType::I32);
    customTypes.emplace_back(SetPointer(SsaType::Void, true));
    customTypes.emplace_back(SetPointer(SsaType::I32, true));
    customTypes.emplace_back(SsaType::U32);
    customTypes.emplace_back(SsaType::U64);
    customTypes.emplace_back(SsaType::U64);

    tau::ir::FunctionBuilder funcBuilder;
    funcBuilder.Attachment<SsaFunctionAttachment>(code0, ::std::size(code0), 8, customTypes);

    tau::ir::Function* func = funcBuilder.Build();

        // Constant Prop
    {
        tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
        visitor.Traverse(func);
        visitor.UpdateAttachment(func);

        tau::ir::ssa::DumpSsa(func, 0, registry);
    }

    ConPrinter::PrintLn();

    // Usage Analysis
    {
        tau::ir::ssa::opto::UsageAnalyzerVisitor visitor(registry);
        visitor.Traverse(func);
        visitor.UpdateAttachment(func);

        tau::ir::ssa::DumpSsa(func, 0, registry);
    }

    ConPrinter::PrintLn();

    // Dead Code Elimination
    {
        tau::ir::ssa::opto::DeadCodeEliminationVisitor visitor(registry, func);
        visitor.Traverse(func);
        visitor.UpdateAttachment(func);

        tau::ir::ssa::DumpSsa(func, 0, registry);
    }

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
        .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default, false)
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

    ModuleRef mainModule = ModuleBuilder()
        .Functions(::std::move(functions))
        .Exports()
        .Imports()
        .Emulated()
        .Name(u8"Main")
        .Build();

    ::tau::ir::DumpFunction(mainFunc, 0, mainModule, 0);
    ConPrinter::PrintLn();
    ::tau::ir::DumpFunction(squareFunc, 1, mainModule, 0);
    ConPrinter::PrintLn();

    const tau::ir::ssa::SsaCustomTypeRegistry registry;
    tau::ir::IrToSsa::TransformFunction(mainFunc, mainModule, 0);
    tau::ir::IrToSsa::TransformFunction(squareFunc, mainModule, 0);

    tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
    tau::ir::ssa::DumpSsa(squareFunc, 1, registry);
    ConPrinter::PrintLn();

    const auto optoPass = [&]() {
        // Constant Prop
        {
            tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
            visitor.Traverse(mainFunc);
            visitor.UpdateAttachment(mainFunc);

            tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
        }

        {
            tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
            visitor.Traverse(squareFunc);
            visitor.UpdateAttachment(squareFunc);

            tau::ir::ssa::DumpSsa(squareFunc, 1, registry);
        }

        ConPrinter::PrintLn();

        // Usage Analysis
        {
            tau::ir::ssa::opto::UsageAnalyzerVisitor visitor(registry);
            visitor.Traverse(mainFunc);
            visitor.UpdateAttachment(mainFunc);
        }

        {
            tau::ir::ssa::opto::UsageAnalyzerVisitor visitor(registry);
            visitor.Traverse(squareFunc);
            visitor.UpdateAttachment(squareFunc);
        }

        ConPrinter::PrintLn();

        // Dead Code Elimination
        {
            tau::ir::ssa::opto::DeadCodeEliminationVisitor visitor(registry, mainFunc);
            visitor.Traverse(mainFunc);
            visitor.UpdateAttachment(mainFunc);

            tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
        }

        {
            tau::ir::ssa::opto::DeadCodeEliminationVisitor visitor(registry, squareFunc);
            visitor.Traverse(squareFunc);
            visitor.UpdateAttachment(squareFunc);

            tau::ir::ssa::DumpSsa(squareFunc, 1, registry);
        }

        ConPrinter::PrintLn();
    };

    optoPass();

    // Inline
    {
        tau::ir::ssa::opto::InlinerVisitor visitor(registry, mainModule);
        visitor.Traverse(mainFunc);
        visitor.UpdateAttachment(mainFunc);

        tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
    }

    {
        tau::ir::ssa::opto::InlinerVisitor visitor(registry, mainModule);
        visitor.Traverse(squareFunc);
        visitor.UpdateAttachment(squareFunc);

        tau::ir::ssa::DumpSsa(squareFunc, 1, registry);
    }

    ConPrinter::PrintLn();

    optoPass();
}

static void TestIrToSsaCallInd() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("Test IR to SSA Call Ind:");

    using namespace tau::ir;
    using namespace tau::ir::ssa;

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
    mainLocalTypes[0] = TypeInfo::Builder().Size(4).Flags(TypeInfoFlags::Function()).Name(squareMangledName).Build();

    Function* const mainFunc = FunctionBuilder()
        .Code(codeMain)
        .LocalTypes(mainLocalTypes)
        .Arguments()
        .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default, false)
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

    const ModuleRef mainModule = ModuleBuilder()
        .Functions(::std::move(functions))
        .Exports()
        .Imports()
        .Emulated()
        .Name(u8"Main")
        .Build();

    DumpFunction(mainFunc, 0, mainModule, 0);
    ConPrinter::PrintLn();
    DumpFunction(squareFunc, 1, mainModule, 0);
    ConPrinter::PrintLn();

    const SsaCustomTypeRegistry registry;
    IrToSsa::TransformFunction(mainFunc, mainModule, 0);
    IrToSsa::TransformFunction(squareFunc, mainModule, 0);

    DumpSsa(mainFunc, 0, registry);
    DumpSsa(squareFunc, 1, registry);
    ConPrinter::PrintLn();
    const auto optoPass = [&]() {
        // Constant Prop
        {
            tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
            visitor.Traverse(mainFunc);
            visitor.UpdateAttachment(mainFunc);

            tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
        }

        {
            tau::ir::ssa::opto::ConstantPropVisitor visitor(registry);
            visitor.Traverse(squareFunc);
            visitor.UpdateAttachment(squareFunc);

            tau::ir::ssa::DumpSsa(squareFunc, 1, registry);
        }

        ConPrinter::PrintLn();

        // Usage Analysis
        {
            tau::ir::ssa::opto::UsageAnalyzerVisitor visitor(registry);
            visitor.Traverse(mainFunc);
            visitor.UpdateAttachment(mainFunc);
        }

        {
            tau::ir::ssa::opto::UsageAnalyzerVisitor visitor(registry);
            visitor.Traverse(squareFunc);
            visitor.UpdateAttachment(squareFunc);
        }

        ConPrinter::PrintLn();

        // Dead Code Elimination
        {
            tau::ir::ssa::opto::DeadCodeEliminationVisitor visitor(registry, mainFunc);
            visitor.Traverse(mainFunc);
            visitor.UpdateAttachment(mainFunc);

            tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
        }

        {
            tau::ir::ssa::opto::DeadCodeEliminationVisitor visitor(registry, squareFunc);
            visitor.Traverse(squareFunc);
            visitor.UpdateAttachment(squareFunc);

            tau::ir::ssa::DumpSsa(squareFunc, 1, registry);
        }

        ConPrinter::PrintLn();
    };

    optoPass();

    // Inline
    {
        tau::ir::ssa::opto::InlinerVisitor visitor(registry, mainModule);
        visitor.Traverse(mainFunc);
        visitor.UpdateAttachment(mainFunc);

        tau::ir::ssa::DumpSsa(mainFunc, 0, registry);
    }

    {
        tau::ir::ssa::opto::InlinerVisitor visitor(registry, mainModule);
        visitor.Traverse(squareFunc);
        visitor.UpdateAttachment(squareFunc);

        tau::ir::ssa::DumpSsa(squareFunc, 1, registry);
    }

    ConPrinter::PrintLn();

    optoPass();
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
            .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default, false)
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

    ModuleRef mainModule = ModuleBuilder()
        .Functions(::std::move(functions))
        .Exports()
        .Imports()
        .Emulated()
        .Name(u8"Main")
        .Build();

    ::tau::ir::DumpFunction(mainModule->Functions()[0], 0, mainModule, 0);
    ConPrinter::PrintLn();
    ::tau::ir::DumpFunction(mainModule->Functions()[1], 1, mainModule, 0);
    ConPrinter::PrintLn();

    tau::ir::Emulator emulator(mainModule);
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
        mainLocalTypes[0] = TypeInfo::Builder().Size(4).Flags(TypeInfoFlags::Function()).Name(squareMangledName).Build();

        functions[0] = FunctionBuilder()
            .Code(codeMain)
            .LocalTypes(mainLocalTypes)
            .Arguments()
            .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default, false)
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

    ModuleRef mainModule = ModuleBuilder()
        .Functions(::std::move(functions))
        .Exports()
        .Imports()
        .Emulated()
        .Name(u8"Main")
        .Build();

    ::tau::ir::DumpFunction(mainModule->Functions()[0], 0, mainModule, 0);
    ConPrinter::PrintLn();
    ::tau::ir::DumpFunction(mainModule->Functions()[1], 1, mainModule, 0);
    ConPrinter::PrintLn();

    tau::ir::Emulator emulator(mainModule);
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
        0x80, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Call.Ext <0:NativePrintHelloWorld>
        0x1D                                            // Ret
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
        .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default, false)
        .Name(u8"Main")
        .Build();
    
    ModuleRef nativeModule = ModuleBuilder()
        .Functions(::std::move(nativeFunctions))
        .Exports()
        .Imports()
        .Native()
        .Name(u8"Native")
        .Build();

    ImportModuleList mainImports(1);
    {
        mainImports[0] = ImportModule(nativeModule, nativeModule->Functions());
    }

    ModuleRef mainModule = ModuleBuilder()
        .Functions(::std::move(functions))
        .Exports()
        .Imports(::std::move(mainImports))
        .Emulated()
        .Name(u8"Main")
        .Build();

    ::tau::ir::DumpFunction(mainModule->Functions()[0], 0, mainModule, 0);
    ConPrinter::PrintLn();

    tau::ir::Emulator emulator(mainModule);
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
        0x80, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Call.Ext <0:NativePrintSuccess>
        0x1E, 0x08, 0x00, 0x00, 0x00,                   // Jump .ret
                                                        // .isGreater:
        0x80, 0x1C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, //   Call.Ext <1:NativePrintFail>
                                                        // .ret
        0x1D                                            //   Ret
    };

    FunctionList nativeFunctions(2);
    {
        nativeFunctions[0] = FunctionBuilder()
            .Func(NativePrintSuccess)
            .Arguments()
            .Name(u8"NativePrintSuccess")
            .Build();
        nativeFunctions[1] = FunctionBuilder()
            .Func(NativePrintFail)
            .Arguments()
            .Name(u8"NativePrintFail")
            .Build();
    }

    FunctionList functions(1);
    {
        functions[0] = FunctionBuilder()
            .Code(codeMain)
            .LocalTypes()
            .Arguments()
            .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default, false)
            .Name(u8"Main")
            .Build();
    }

    ModuleRef nativeModule = ModuleBuilder()
        .Functions(::std::move(nativeFunctions))
        .Exports()
        .Imports()
        .Native()
        .Name(u8"Native")
        .Build();

    ImportModuleList mainImports(1);
    {
        mainImports[0] = ImportModule(nativeModule, nativeModule->Functions());
    }

    ModuleRef mainModule = ModuleBuilder()
        .Functions(::std::move(functions))
        .Exports()
        .Imports(::std::move(mainImports))
        .Emulated()
        .Name(u8"Main")
        .Build();

    ::tau::ir::DumpFunction(mainModule->Functions()[0], 0, mainModule, 0);
    ConPrinter::PrintLn();

    tau::ir::Emulator emulator(mainModule);
    emulator.Execute();

    ConPrinter::PrintLn();
}

static void TestWriteFile() noexcept
{
    ConPrinter::PrintLn();
    ConPrinter::PrintLn();
    ConPrinter::PrintLn("Test file generation:");

    using namespace tau::ir;
    using namespace tau::ir::file;
    using namespace tau::ir::file::v0_0;

    FILE* const file = fopen("test_exe.tire", "w+b");

    constexpr u8 exeHeader[] = { 'M', 'S' };

    (void) fwrite(exeHeader, sizeof(exeHeader), 1, file);

    const i64 zeroPointer = WriteFileHeader(file);

    const C8DynString strings[] = {
        StringSectionName,
        ModuleInfoSectionName,
        TypesSectionName,
        GlobalsSectionName,
        FunctionsSectionName,
        CodeSectionName,
        u8"Test Module",
        u8"A module for testing the TIRE writer functions.",
        u8"hyfloac",
        u8"https://github.com/hyfloac/TauIr",
        u8"Square",
        u8"Main"
    };

    u64 stringPointers[::std::size(strings)];

    const i64 stringSectionPointer = WriteStringSection(file, zeroPointer, strings, static_cast<u32>(::std::size(strings)), stringPointers);
    
    u64 sectionNames[5];
    sectionNames[0] = stringPointers[1];
    sectionNames[1] = stringPointers[2];
    sectionNames[2] = stringPointers[3];
    sectionNames[3] = stringPointers[4];
    sectionNames[4] = stringPointers[5];
    
    u64 sectionPointers[5];
    
    (void) WriteSectionHeader(file, zeroPointer, stringSectionPointer, stringPointers[0], sectionNames, static_cast<u16>(::std::size(sectionNames)), sectionPointers);
    
    {
        ModuleInfoSection moduleInfo;
        moduleInfo.ModuleVersion = MakeFileVersion(1, 0, 0);
        moduleInfo.TauIRVersion = TauIRVersion0;
        moduleInfo.NamePointer = stringPointers[6];
        moduleInfo.DescriptionPointer = stringPointers[7];
        moduleInfo.AuthorPointer = stringPointers[8];
        moduleInfo.WebsitePointer = stringPointers[9];
        (void) ::std::memset(moduleInfo.Reserved, 0, sizeof(moduleInfo.Reserved));
    
        (void) WriteModuleInfoSection(file, zeroPointer, sectionPointers[0], moduleInfo);
    }

    {
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
            DynArray<::tau::ir::FunctionArgument> squareArgs(1);
            squareArgs[0] = ::tau::ir::FunctionArgument(true, 0);

            functions[0] = FunctionBuilder()
                .Code(codeMain)
                .LocalTypes()
                .Arguments()
                .Flags(InlineControl::NoInline, CallingConvention::Default, OptimizationControl::Default, false)
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

        ModuleRef mainModule = ModuleBuilder()
            .Functions(::std::move(functions))
            .Exports()
            .Imports()
            .Emulated()
            .Name(u8"Main")
            .Build();

        const u64 namePointers[] = { stringPointers[10], stringPointers[11] };
        u64 codePointers[2];

        WriteFunctionSection(file, zeroPointer, sectionPointers[3], mainModule.Get(), namePointers, codePointers);
    }

    WriteFinal(file, zeroPointer);

    do
    {
        (void) fflush(file);
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        FileHeader* const fileHeader = ReadFileHeader(file);

        if(!fileHeader)
        {
            ConPrinter::PrintLn(u8"Failed to read file header.");
            break;
        }

        ConPrinter::PrintLn(fileHeader);
        
        SectionHeader* const sectionHeader = ReadSectionHeader(file, fileHeader);


        FreeFile(sectionHeader);
        FreeFile(fileHeader);
    } while(false);

    (void) fclose(file);
}
