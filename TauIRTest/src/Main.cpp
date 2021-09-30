#include <allocator/PageAllocator.hpp>

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
        0x99, 0x99,             // Ret
        0x00,                   // Nop
        0x00,                   // Nop
        0x00,                   // Nop
        0x00                    // Nop
    };

    using TypeInfo = tau::ir::TypeInfo;
    using Function = tau::ir::Function;
    using Module = tau::ir::Module;

    const TypeInfo* intType = new TypeInfo(4, "int");
    const TypeInfo* byteType = new TypeInfo(1, "byte");
    const TypeInfo* floatType = new TypeInfo(4, "float");
    const TypeInfo* doubleType = new TypeInfo(8, "double");

    DynArray<const TypeInfo*> types(6);
    types[0] = intType;
    types[1] = byteType;
    types[2] = doubleType;
    types[3] = byteType;
    types[4] = floatType;
    types[5] = intType;

    DynArray<const Function*> functions(1);
    functions[0] = new Function(reinterpret_cast<uPtr>(code), types);
    
    ::std::vector<Ref<Module>> modules(1);
    modules[0] = Ref<Module>(::std::move(functions));

    tau::ir::Emulator emulator(::std::move(modules));

    emulator.Execute();

    printf("Return Val: %llu [0x%llX]\n", emulator.ReturnVal(), emulator.ReturnVal());

    return static_cast<int>(emulator.ReturnVal());
}
