#pragma once

#include <NumTypes.hpp>

namespace tau::ir {
class Function;

void DumpFunction(const tau::ir::Function* function, uSys functionIndex) noexcept;

namespace ssa {

class SsaCustomTypeRegistry;

void DumpSsa(const u8* codePtr, uSys length, uSys functionIndex, const ssa::SsaCustomTypeRegistry& registry) noexcept;

}

}


