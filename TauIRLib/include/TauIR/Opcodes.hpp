#pragma once

#include <NumTypes.hpp>
#include <EnumBitFields.hpp>

namespace tau::ir {

enum class CompareCondition : u8
{
    Above          = 0x0000,
    AboveOrEqual   = 0x0001,
    Below          = 0x0002,
    BelowOrEqual   = 0x0003,
    Equal          = 0x0004,
    Greater        = 0x0005,
    GreaterOrEqual = 0x0006,
    Less           = 0x0007,
    LessOrEqual    = 0x0008,
    NotEqual       = 0x0009,
};

ENUM_FLAGS(CompareCondition);

enum class Opcode : u16
{
    Nop                   = 0x0000,
    Push0                 = 0x0010,
    Push1                 = 0x0011,
    Push2                 = 0x0012,
    Push3                 = 0x0013,
    PushN                 = 0x9010,
    PushArg0              = 0x0030,
    PushArg1              = 0x0031,
    PushArg2              = 0x0032,
    PushArg3              = 0x0033,
    PushArgN              = 0x9030,
    PushPtr               = 0x9011,
    PushGlobal            = 0x9012,
    PushGlobalExt         = 0x9013,
    PushGlobalPtr         = 0x9014,
    PushGlobalExtPtr      = 0x9015,
    Pop0                  = 0x0020,
    Pop1                  = 0x0021,
    Pop2                  = 0x0022,
    Pop3                  = 0x0023,
    PopN                  = 0xA020,
    PopArg0               = 0x0040,
    PopArg1               = 0x0041,
    PopArg2               = 0x0042,
    PopArg3               = 0x0043,
    PopArgN               = 0xA040,
    PopPtr                = 0xA021,
    PopGlobal             = 0xA022,
    PopGlobalExt          = 0xA023,
    PopGlobalPtr          = 0xA024,
    PopGlobalExtPtr       = 0xA025,
    PopCount              = 0xA026,
    Dup1                  = 0x000C,
    Dup2                  = 0x000D,
    Dup4                  = 0x000E,
    Dup8                  = 0x000F,
    ExpandSX12            = 0x0024,
    ExpandSX14            = 0x0025,
    ExpandSX18            = 0x0026,
    ExpandSX24            = 0x0027,
    ExpandSX28            = 0x0028,
    ExpandSX48            = 0x0029,
    ExpandZX12            = 0x0044,
    ExpandZX14            = 0x0045,
    ExpandZX18            = 0x0046,
    ExpandZX24            = 0x0047,
    ExpandZX28            = 0x0048,
    ExpandZX48            = 0x0049,
    Trunc84               = 0x002A,
    Trunc82               = 0x002B,
    Trunc81               = 0x002C,
    Trunc42               = 0x002D,
    Trunc41               = 0x002E,
    Trunc21               = 0x002F,
    Load                  = 0x001B,
    LoadGlobal            = 0x901A,
    LoadGlobalExt         = 0x901B,
    Store                 = 0x004A,
    StoreGlobal           = 0xA04A,
    StoreGlobalExt        = 0xA04B,
    Const0                = 0x0014,
    Const1                = 0x0015,
    Const2                = 0x0016,
    Const3                = 0x0017,
    Const4                = 0x0018,
    ConstFF               = 0x0019,
    Const7F               = 0x001A,
    ConstN                = 0x8B00,
    AddI32                = 0x0034,
    AddI64                = 0x0035,
    SubI32                = 0x0036,
    SubI64                = 0x0037,
    MulI32                = 0x0038,
    MulI64                = 0x0039,
    DivI32                = 0x003A,
    DivI64                = 0x003B,
    CompI32Above          = 0x8070,
    CompI32AboveOrEqual   = 0x8071,
    CompI32Below          = 0x8072,
    CompI32BelowOrEqual   = 0x8073,
    CompI32Equal          = 0x8074,
    CompI32Greater        = 0x8075,
    CompI32GreaterOrEqual = 0x8076,
    CompI32Less           = 0x8077,
    CompI32LessOrEqual    = 0x8078,
    CompI32NotEqual       = 0x8079,
    CompI64Above          = 0x8080,
    CompI64AboveOrEqual   = 0x8081,
    CompI64Below          = 0x8082,
    CompI64BelowOrEqual   = 0x8083,
    CompI64Equal          = 0x8084,
    CompI64Greater        = 0x8085,
    CompI64GreaterOrEqual = 0x8086,
    CompI64Less           = 0x8087,
    CompI64LessOrEqual    = 0x8088,
    CompI64NotEqual       = 0x8089,
    Call                  = 0x001C,
    CallExt               = 0x801C,
    CallInd               = 0x801D,
    CallIndExt            = 0x801E,
    Ret                   = 0x001D,
    Jump                  = 0x001E,
    JumpTrue              = 0x0070,
    JumpFalse             = 0x0071
};

}
