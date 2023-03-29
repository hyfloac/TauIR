#pragma once

#include <Objects.hpp>

#include "Common.hpp"
#include "TauIR/IrWriter.hpp"

#include <vector>

namespace tau::ir {

class Function;
class TypeInfo;
class Module;
using ModuleList = ::std::vector<ModuleRef>;

class IrToSsa
{
    DELETE_CM(IrToSsa);
public:
    static void TransformFunction(Function* function, const ModuleList& modules, u16 currentModule) noexcept;
};

}
