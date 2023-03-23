#include "TauIR/FunctionNameMangler.hpp"
#include "TauIR/Function.hpp"
#include <ToString.hpp>

namespace tau::ir {

C8DynString MangleFunctionName(const DynArray<FunctionArgument>& arguments) noexcept
{
    C8StringBuilder builder(arguments.Length() * 2 + 1 + 2);

    builder.append(u8"A0:");
    for(const FunctionArgument& argument : arguments)
    {
        if(argument.IsRegister)
        {
            builder.append(u8'A');
        }
        else
        {
            builder.append(u8'S');
        }

        builder.append(ToString<c8>(argument.RegisterOrStackOffset));
    }

    return builder.toString();
}

DynArray<FunctionArgument> DeMangleFunctionName(const C8DynString& mangledName) noexcept
{
    uSys argCount = 0;

    uSys argsLoc = -1;

    for(uSys i = 0; i < mangledName.Length(); ++i)
    {
        if(mangledName[i] == u8':')
        {
            argsLoc = i + 1;
            break;
        }
    }

    for(uSys i = argsLoc; i < mangledName.Length(); ++i)
    {
        if(mangledName[i] == u8'A' || mangledName[i] == u8'S')
        {
            ++argCount;
        }
    }

    DynArray<FunctionArgument> args(argCount);

    iSys argIndex = -1;
    
    for(uSys i = argsLoc; i < mangledName.Length(); ++i)
    {
        if(mangledName[i] == u8'A' || mangledName[i] == u8'S')
        {
            ++argIndex;
            args[argIndex].IsRegister = mangledName[i] == u8'A';
            args[argIndex].RegisterOrStackOffset = 0;
        }
        else
        {
            args[argIndex].RegisterOrStackOffset *= 10;
            args[argIndex].RegisterOrStackOffset += (mangledName[i] - u8'0');
        }
    }

    return args;
}

}
