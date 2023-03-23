#pragma once

#include <DynArray.hpp>
#include <String.hpp>

namespace tau::ir {
struct FunctionArgument;

C8DynString MangleFunctionName(const DynArray<FunctionArgument>& arguments) noexcept;
DynArray<FunctionArgument> DeMangleFunctionName(const C8DynString& mangledName) noexcept;

}
