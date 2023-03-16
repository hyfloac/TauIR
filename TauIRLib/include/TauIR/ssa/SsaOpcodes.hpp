#pragma once

#include <NumTypes.hpp>

namespace tau::ir::ssa {

enum class SsaBinaryOperation : u8
{
    Add              = 0x00,
    Sub              = 0x01,
    Mul              = 0x02,
    Div              = 0x03,
    Rem              = 0x04,
    BitShiftLeft     = 0x05,
    BitShiftRight    = 0x06,
    BarrelShiftLeft  = 0x07,
    BarrelShiftRight = 0x08,
    Comp             = 0x70
};

enum class SsaOpcode : u16
{
    Nop             = 0x0000,
    Label           = 0x0001,
    AssignImmediate = 0x0030,
    AssignVariable  = 0x0031,
    ExpandSX        = 0x0032,
    ExpandZX        = 0x0033,
    Trunc           = 0x0034,
    RCast           = 0x0036,
    BCast           = 0x0037,
    Load            = 0x0038,
    StoreV          = 0x0039,
    StoreI          = 0x003B,
    ComputePtr      = 0x003A,
    BinOpVtoV       = 0x0050,
    BinOpVtoI       = 0x0051,
    BinOpItoV       = 0x0052,
    Split           = 0x0020,
    Join            = 0x0021,
    Branch          = 0x0040,
    BranchCond      = 0x0041,
    Call            = 0x0042,
    CallExt         = 0x0043,
    CallInd         = 0x0044,
    Ret             = 0x0045,
};

}
