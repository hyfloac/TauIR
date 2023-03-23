#pragma once

#include <NumTypes.hpp>
#include <vector>

#include "TauIR/Common.hpp"

namespace tau::ir {

class Function;
using ModuleList = ::std::vector<ModuleRef>;

void DumpFunction(const tau::ir::Function* function, uSys functionIndex, const ModuleList& modules, u16 moduleIndex) noexcept;

namespace ssa {

class SsaCustomTypeRegistry;

void DumpSsa(const u8* codePtr, uSys length, uSys functionIndex, const ssa::SsaCustomTypeRegistry& registry) noexcept;
void DumpSsa(const Function* function, uSys functionIndex, const SsaCustomTypeRegistry& registry) noexcept;

}

}


