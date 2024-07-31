#include "TauIR/ssa/SsaEmulator.hpp"
#include "TauIR/Function.hpp"
#include "TauIR/ssa/SsaFunctionAttachment.hpp"

void tau::ir::ssa::SsaEmulator::Execute(const Function* function, const Module* module) noexcept
{
    const u8* codePtr;
    uSys codeSize;
    VarId maxVarId;

    if(!GetSsaCode(function, &codePtr, &codeSize, &maxVarId))
    {
        return;
    }


    for(uSys i = 0; i < codeSize;)
    {
        u16 opcodeRaw = codePtr[i++];

        // Read Second Byte
        if(opcodeRaw & 0x80)
        {
            opcodeRaw <<= 8;
            opcodeRaw |= codePtr[i++];
        }

        const SsaOpcode opcode = static_cast<SsaOpcode>(opcodeRaw);

        switch(opcode)
        {
            case SsaOpcode::Nop: 
                break;
            case SsaOpcode::Label:
                break;
            case SsaOpcode::AssignImmediate:
                break;
            case SsaOpcode::AssignVariable:
                break;
            case SsaOpcode::ExpandSX:
                break;
            case SsaOpcode::ExpandZX:
                break;
            case SsaOpcode::Trunc:
                break;
            case SsaOpcode::RCast:
                break;
            case SsaOpcode::BCast:
                break;
            case SsaOpcode::Load:
                break;
            case SsaOpcode::StoreV:
                break;
            case SsaOpcode::StoreI:
                break;
            case SsaOpcode::ComputePtr:
                break;
            case SsaOpcode::BinOpVtoV:
                break;
            case SsaOpcode::BinOpVtoI:
                break;
            case SsaOpcode::BinOpItoV:
                break;
            case SsaOpcode::Split:
                break;
            case SsaOpcode::Join:
                break;
            case SsaOpcode::CompVtoV:
                break;
            case SsaOpcode::CompVtoI:
                break;
            case SsaOpcode::CompItoV:
                break;
            case SsaOpcode::Branch:
                break;
            case SsaOpcode::BranchCond:
                break;
            case SsaOpcode::Call:
                break;
            case SsaOpcode::CallExt:
                break;
            case SsaOpcode::CallInd:
                break;
            case SsaOpcode::CallIndExt:
                break;
            case SsaOpcode::Ret:
                break;
            default: return;
        }
    }
}

bool tau::ir::ssa::SsaEmulator::GetSsaCode(const Function* function, const u8** const pCode, uSys* const pSize, VarId* const pMaxVarId) noexcept
{
    if(!pCode || !pSize || !pMaxVarId)
    {
        return false;
    }

    {
        const SsaWriterFunctionAttachment* ssaWriterAttachment = function->FindAttachment<SsaWriterFunctionAttachment>();

        if(ssaWriterAttachment)
        {
            *pCode = ssaWriterAttachment->Writer().Buffer();
            *pSize = ssaWriterAttachment->Writer().Size();
            *pMaxVarId = ssaWriterAttachment->Writer().IdIndex();
            return true;
        }
    }

    {
        const SsaFunctionAttachment* ssaAttachment = function->FindAttachment<SsaFunctionAttachment>();

        if(ssaAttachment)
        {
            *pCode = ssaAttachment->Buffer();
            *pSize = ssaAttachment->Buffer().Size();
            *pMaxVarId = ssaAttachment->MaxVarId();
            return true;
        }
    }

    return false;
}
