#pragma once

#include <Objects.hpp>
#include "ssa/SsaWriter.hpp"

namespace tau::ir {

class Function;

class TypeInfo;

class IrToSsa
{
    DELETE_CM(IrToSsa);
public:
    using SsaWriter = ssa::SsaWriter;
    using SsaFrameTracker = ssa::SsaFrameTracker;
    using VarId = ssa::VarId;
public:
    static SsaWriter TransformFunction(const Function* function) noexcept;

private:
    static VarId PopRaw(SsaWriter& writer, SsaFrameTracker& frameTracker, uSys size, ssa::SsaType ssaType);
    static VarId PopLocal(const Function* function, SsaWriter& writer, SsaFrameTracker& frameTracker, VarId localIndex);
    static VarId PopArgument(const Function* function, SsaWriter& writer, SsaFrameTracker& frameTracker, VarId argumentIndex);
};

}
