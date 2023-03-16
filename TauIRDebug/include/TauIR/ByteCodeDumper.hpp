#pragma once

#include <NumTypes.hpp>
#include <vector>

#include "TauIR/Common.hpp"

namespace tau::ir {
class Function;
class Module;

void DumpFunction(const tau::ir::Function* function, uSys functionIndex, const std::vector<Ref<Module>>& modules) noexcept;

namespace ssa {

class SsaCustomTypeRegistry;

void DumpSsa(const u8* codePtr, uSys length, uSys functionIndex, const ssa::SsaCustomTypeRegistry& registry) noexcept;

}

}


