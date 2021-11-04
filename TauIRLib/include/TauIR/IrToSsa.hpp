#pragma once

#include <Objects.hpp>
#include "ssa/SsaWriter.hpp"

namespace tau::ir {

class Function;

class IrToSsa
{
    DELETE_CM(IrToSsa);
public:
    using SsaWriter = ssa::SsaWriter;
    using SsaFrameTracker = ssa::SsaFrameTracker;
    using VarId = SsaWriter::VarId;
public:
    static SsaWriter TransformFunction(const Function* function) noexcept;

private:
    static void HandlePop(const Function* function, SsaWriter& writer, SsaFrameTracker& frameTracker, uSys localIndex);
};

}
