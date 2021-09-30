#pragma once

#include <NumTypes.hpp>

namespace tau::ir {

enum class Opcode : u16
{
    Nop              = 0x0000,
    Push0            = 0x0010,
    Push1            = 0x0011,
    Push2            = 0x0012,
    Push3            = 0x0013,
    PushN            = 0x9010,
    PushArg0         = 0x0030,
    PushArg1         = 0x0031,
    PushArg2         = 0x0032,
    PushArg3         = 0x0033,
    PushArgN         = 0x9030,
    PushPtr          = 0x9011,
    PushGlobal       = 0x9012,
    PushGlobalExt    = 0x9013,
    PushGlobalPtr    = 0x9014,
    PushGlobalExtPtr = 0x9015,
    Pop0             = 0x0020,
    Pop1             = 0x0021,
    Pop2             = 0x0022,
    Pop3             = 0x0023,
    PopN             = 0xA020,
    PopArg0          = 0x0040,
    PopArg1          = 0x0041,
    PopArg2          = 0x0042,
    PopArg3          = 0x0043,
    PopArgN          = 0xA040,
    PopGlobal        = 0xA022,
    PopGlobalExt     = 0xA023,
    Ret = 0x9999 //tmp
};

}
