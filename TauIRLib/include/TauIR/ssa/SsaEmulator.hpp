#pragma once

#include <Objects.hpp>
#include <NumTypes.hpp>

#include "SsaTypes.hpp"

namespace tau::ir {

class Function;
class Module;

}

namespace tau::ir::ssa {

class SsaEmulator
{
    DEFAULT_DESTRUCT(SsaEmulator);
    DELETE_CM(SsaEmulator);
public:
    static inline constexpr uSys PointerSize = sizeof(void*);
public:
    void Execute(const Function* function, const Module* module) noexcept;
private:
    static bool GetSsaCode(const Function* function, const u8** pCode, uSys* pSize, VarId* pMaxVarId) noexcept;
};

}
