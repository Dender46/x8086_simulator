#pragma once

#include "CpuMemory.h"
#include "CpuOperations.h"

int RegularOperationEstimate(const Operation& op);
int JumpOperationEstimate(const Operation& op);

int OperationMOVEstimate(const Operation& op);
int OperationArithmeticEstimate(const Operation& op);
int OperationCMPEstimate(const Operation& op);


int CycleEstimation(const Operation& op)
{
    switch (op.type)
    {
    case Operation::Type::Operation:    return RegularOperationEstimate(op);
    case Operation::Type::Jump:
    case Operation::Type::Loop:         return JumpOperationEstimate(op);
    default:
        assert(false);
        break;
    }
}

int RegularOperationEstimate(const Operation& op)
{
    switch (op.opIndex)
    {
    case OpIndex::MOV: return OperationMOVEstimate(op);
    case OpIndex::SUB:
    case OpIndex::ADD: return OperationArithmeticEstimate(op);
    case OpIndex::CMP: return OperationCMPEstimate(op);
    default:
        assert(false);
        break;
    }
}

int JumpOperationEstimate(const Operation& op)
{
    return 0; // TODO: too lazy to do this
}

bool OperandsMatch(const Operation& op, Operand::Type type0, Operand::Type type1)
{
    return op.operands[0].type == type0 && op.operands[1].type == type1;
}

int EffectiveAddressEstimate(const MemoryExpr& ea)
{
    const auto regCmp = [&](RegisterIndex t0, RegisterIndex t1) -> bool {
        return ea.registers[0] == t0 && ea.registers[1] == t1;
    };

    using enum RegisterIndex;
    enum Components{
        Disp  = (1 << 0),
        Base  = (1 << 1),
        Index = (1 << 2),
        All   = Disp | Base | Index
    };
    u8 components{};

    const auto reg0 = ea.registers[0];
    const auto reg1 = ea.registers[1];

    // Base or Index only
    if (reg1 == None && (reg0 == bx || reg0 == bp) || (reg0 == si || reg0 == di))
        components = Base;
    // Base + Index
    else if (regCmp(bp, di) || regCmp(bx, si) || regCmp(bp, si) || regCmp(bx, di))
        components = Base | Index;

    if (ea.disp != 0)
        components |= Disp;


    if ((components & All) == All) // Displacement + Base + Index
    {
        if (regCmp(bp, di) || regCmp(bx, si))
            return 11;
        if (regCmp(bp, si) || regCmp(bx, di))
            return 12;
        assert(false);
    }
    if ((components & (Base | Index)) == (Base | Index)) // Base + Index
    {
        if (regCmp(bp, di) || regCmp(bx, si))
            return 7;
        if (regCmp(bp, si) || regCmp(bx, di))
            return 8;
        assert(false);
    }
    if ((components & (Base | Disp)) == (Base | Disp)) // Displacement + Base or Index
        return 9;
    if ((components & Base) == Base) // Base or Index only
        return 5;
    if ((components & Disp) == Disp) // Displacement only
        return 6;

    assert(false);
}

int OperationMOVEstimate(const Operation& op)
{
    using enum Operand::Type;
    if (OperandsMatch(op, Memory, Register) && op.operands[1].reg == RegisterIndex::ax ||
        OperandsMatch(op, Register, Memory) && op.operands[0].reg == RegisterIndex::ax)
    {
        return 10;
    }
    else if (OperandsMatch(op, Register, Register))
    {
        return 2;
    }
    else if (OperandsMatch(op, Register, Memory))
    {
        return 8 + EffectiveAddressEstimate(op.operands[1].mem);
    }
    else if (OperandsMatch(op, Memory, Register))
    {
        return 9 + EffectiveAddressEstimate(op.operands[0].mem);
    }
    else if (OperandsMatch(op, Register, Immediate))
    {
        return 4;
    }
    else if (OperandsMatch(op, Memory, Immediate))
    {
        return 10 + EffectiveAddressEstimate(op.operands[0].mem);
    }
    // TODO: There are also seg-reg, reg16 operands case but they are not implemented thus not estimated
    assert(false);
}

int OperationArithmeticEstimate(const Operation& op)
{
    using enum Operand::Type;
    if (OperandsMatch(op, Register, Register))
    {
        return 3;
    }
    else if (OperandsMatch(op, Register, Memory))
    {
        return 9 + EffectiveAddressEstimate(op.operands[1].mem);
    }
    else if (OperandsMatch(op, Memory, Register))
    {
        return 16 + EffectiveAddressEstimate(op.operands[0].mem);
    }
    else if (OperandsMatch(op, Register, Immediate))
    {
        return 4;
    }
    else if (OperandsMatch(op, Memory, Immediate))
    {
        return 17 + EffectiveAddressEstimate(op.operands[0].mem);
    }
    else if (OperandsMatch(op, Register, Immediate) && op.operands[0].reg == RegisterIndex::ax)
    {
        return 4;
    }
    assert(false);
}

int OperationCMPEstimate(const Operation& op)
{
    using enum Operand::Type;
    if (OperandsMatch(op, Register, Register))
    {
        return 3;
    }
    else if (OperandsMatch(op, Register, Memory))
    {
        return 9 + EffectiveAddressEstimate(op.operands[1].mem);
    }
    else if (OperandsMatch(op, Memory, Register))
    {
        return 9 + EffectiveAddressEstimate(op.operands[0].mem);
    }
    else if (OperandsMatch(op, Register, Immediate))
    {
        return 4;
    }
    else if (OperandsMatch(op, Memory, Immediate))
    {
        return 10 + EffectiveAddressEstimate(op.operands[0].mem);
    }
    else if (OperandsMatch(op, Register, Immediate) && op.operands[0].reg == RegisterIndex::ax)
    {
        return 4;
    }
    assert(false);
}