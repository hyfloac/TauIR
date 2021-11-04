#include "TauIR/Function.hpp"
#include "TauIR/Emulator.hpp"
#include "TauIR/Module.hpp"
#include "TauIR/TypeInfo.hpp"

#include <cstdio>

int main(int argCount, char* args[])
{
    u8 code[] = {
        0x10,                   // Push.0
        0x13,                   // Push.3
        0x00,                   // Nop
        0x90, 0x10, 0x05, 0x00, // Push.N 5
        0x12,                   // Push.2
        0x00,                   // Nop
        0x40,                   // Pop.Arg.0
        0x1D                    // Ret
    };

    i32 subVal = 83;
    i32* subValPtr = &subVal;

    const u8* bytes = reinterpret_cast<const u8*>(&subValPtr);

    // Expect -631
    u8 codeAdd[] = {
        0x8B, 0x00, 0x2A, 0x00, 0x00, 0x00, // Const.N 42
        0x8B, 0x00, 0x11, 0x00, 0x00, 0x00, // Const.N 17
        0x38,                               // Mul.i32
        0x8B, 0x00, bytes[0], bytes[1], bytes[2], bytes[3], // Const.N high(&subVal)
        0x8B, 0x00, bytes[4], bytes[5], bytes[6], bytes[7], // Const.N  low(&subVal)
        0x20,                               // Pop.0
        0x90, 0x11, 0x00, 0x00,             // Push.Ptr 0
        0x36,                               // Sub.i32
        0x29,                               // Expand.SX.4.8
        0x40,                               // Pop.Arg.0
        0x1D                                // Ret
    };

    // Expect 0
    u8 codeDiv[] = {
        0x18,                               // Const.4
        0x8B, 0x00, 0x14, 0x00, 0x00, 0x00, // Const.N 17
        0x3A,                               // Div.i32
        // 0xA0, 0x26, 0x04, 0x00,             // Pop.Count 4
        0x29,                               // Expand.SX.4.8
        0x40,                               // Pop.Arg.0
        0x1D,                               // Ret
    };

    u8 codeSquare[] = {
        0x30,   // Push.Arg.0
        0x30,   // Push.Arg.0
        0x39,   // Mul.i64
        0x40,   // Pop.Arg.0
        0x1D    // Ret
    };

    // Expect 259
    u8 codeMain[] = {
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
    using Module = tau::ir::Module;

    const TypeInfo* intType = new TypeInfo(4, "int");
    const TypeInfo* byteType = new TypeInfo(1, "byte");
    const TypeInfo* floatType = new TypeInfo(4, "float");
    const TypeInfo* doubleType = new TypeInfo(8, "double");
    const TypeInfo* pointerType = new TypeInfo(sizeof(void*), "pointer");

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
    functions[0] = new Function(reinterpret_cast<uPtr>(codeMain), DynArray<const TypeInfo*>());
    functions[1] = new Function(reinterpret_cast<uPtr>(codeSquare), DynArray<const TypeInfo*>());
    
    ::std::vector<Ref<Module>> modules(1);
    modules[0].reset(::std::move(functions));

    tau::ir::Emulator emulator(::std::move(modules));

    emulator.Execute();

    const u64 retVal = emulator.ReturnVal();

    printf("Return Val: %llu (%lld) [0x%llX]\n", retVal, retVal, retVal);

    delete intType;
    delete byteType;
    delete floatType;
    delete doubleType;
    delete pointerType;

    return static_cast<int>(retVal);
}
