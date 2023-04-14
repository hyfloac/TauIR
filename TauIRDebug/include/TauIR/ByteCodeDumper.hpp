#pragma once

#include <NumTypes.hpp>
#include <vector>

#include "TauIR/Common.hpp"

namespace tau::ir {

class Function;

void DumpFunction(const tau::ir::Function* function, uSys functionIndex, const ModuleRef& module, u16 moduleIndex) noexcept;

namespace ssa {

class SsaCustomTypeRegistry;

void DumpSsa(const u8* codePtr, uSys length, uSys functionIndex, const ssa::SsaCustomTypeRegistry& registry) noexcept;
void DumpSsa(const Function* function, uSys functionIndex, const SsaCustomTypeRegistry& registry) noexcept;

}

}


