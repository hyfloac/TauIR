#pragma once

#include <NumTypes.hpp>

namespace tau::ir::ssa {

enum class SsaBinaryOperation : u8
{
    Add              = 0,
    Sub              = 1,
    Mul              = 2,
    Div              = 3,
    Rem              = 4,
    BitShiftLeft     = 5,
    BitShiftRight    = 6,
    BarrelShiftLeft  = 7,
    BarrelShiftRight = 8
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
    Store           = 0x0039,
    ComputePtr      = 0x003A,
    BinOpVtoV       = 0x0050,
    BinOpVtoI       = 0x0051,
    BinOpItoV       = 0x0052,
    Split           = 0x0020,
    Join            = 0x0021
};

}
