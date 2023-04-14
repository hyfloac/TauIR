#pragma once

#include <Objects.hpp>

#include "Common.hpp"
#include "ssa/SsaWriter.hpp"

namespace tau::ir {

class Function;
class TypeInfo;
class Module;
using ModuleList = ::std::vector<ModuleRef>;

class IrToSsa
{
    DELETE_CM(IrToSsa);
public:
    using SsaWriter = ssa::SsaWriter;
    using SsaFrameTracker = ssa::SsaFrameTracker;
    using VarId = ssa::VarId;
public:
    static void TransformFunction(Function* function, const ModuleRef& module, u16 currentModule) noexcept;
public:
    static VarId PopRaw(SsaWriter& writer, SsaFrameTracker& frameTracker, uSys size, ssa::SsaType ssaType);
    static VarId PopLocal(const Function* function, SsaWriter& writer, SsaFrameTracker& frameTracker, VarId localIndex);
    static VarId PopArgument(const Function* function, SsaWriter& writer, SsaFrameTracker& frameTracker, VarId argIndex);
};

}
