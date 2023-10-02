#pragma once
#include <iostream>
#include <string>

#include "Defines.h"
#include "CpuMemory.h"
#include "CpuNames.h"
#include "Helpers.h"
#include "CpuOperations.h"
#include "DecoderOperands.h"

std::string OutputChangeInFlags(const bool* prevFlags)
{
    std::string prevFlagsStr, currFlagsStr;
    for (int i = 0; i < Flag::FLAG_COUNT; i++)
    {
        if (prevFlags[i])   prevFlagsStr += FlagStr((Flag)i);
        if (flags[i])       currFlagsStr += FlagStr((Flag)i);
    }

    if (prevFlagsStr == "" && currFlagsStr == "" || prevFlagsStr == currFlagsStr)
        return "";

    return "flags: " + prevFlagsStr + "->" + currFlagsStr;
}

std::string ExecuteOp(OpIndex opIndex, const Operand operands[2])
{
    bool prevFlags[Flag::FLAG_COUNT] = {};
    for (int i : flags)
        prevFlags[i] = flags[i];

    u16 prevDestData;
    std::string regName;
    MemoryAccess destination = [&](){
        MemoryAccess dest{};
        switch (operands[0].type)
        {
        case Operand::Type::Register:
            dest.type = MemoryAccess::Type::Full;
            dest.full = GetRegisterMem(operands[0].reg);
            prevDestData = *dest;
            regName = std::string(" ; ") + registerNames[operands[0].reg] + ":";
            break;
        case Operand::Type::DirectAddress:
            dest.type = operands[0].mem.pointsToWord ? MemoryAccess::Type::Word : MemoryAccess::Type::Byte;
            dest.SetAddress(&mainMemory[operands[0].mem.disp]);
            prevDestData = *dest;
            regName = std::string(" ; ") + registerNames[operands[0].reg] + ":";
            break;
        case Operand::Type::Memory:
            dest.type = operands[0].mem.pointsToWord ? MemoryAccess::Type::Word : MemoryAccess::Type::Byte;
            dest.SetAddress(&mainMemory[operands[0].mem.Evaluate()]);
            prevDestData = *dest;
            regName = std::string(" ; ") + registerNames[operands[0].reg] + ":";
            break;
            // TODO: add more destinations
        }
        assert(dest.type != MemoryAccess::Type::None);
        return dest;
    }();

    u16 data;
    switch (operands[1].type)
    {
    case Operand::Type::Register:
        data = *GetRegisterMem(operands[1].reg);
        break;
    case Operand::Type::Immediate:
        data = operands[1].immVal.value;
        break;
    case Operand::Type::DirectAddress:
        {
            MemoryAccess memAccess{};
            memAccess.type = operands[1].mem.pointsToWord ? MemoryAccess::Type::Word : MemoryAccess::Type::Byte;
            memAccess.SetAddress(&mainMemory[operands[1].mem.disp]);
            data = *memAccess;
        }
        break;
    case Operand::Type::Memory:
        {
            MemoryAccess memAccess{};
            memAccess.type = operands[1].mem.pointsToWord ? MemoryAccess::Type::Word : MemoryAccess::Type::Byte;
            memAccess.SetAddress(&mainMemory[operands[1].mem.Evaluate()]);
            data = *memAccess;
        }
        break;
        // TODO: add more data retrieval
    }

    // Execute operation
    u16 newValue = 0;
    switch (opIndex)
    {
    case OpIndex::MOV:
        newValue = data;
        destination.SetData(data);
        break;
    case OpIndex::ADD:
        newValue = destination + data;
        destination.SetData(newValue);
        break;
    case OpIndex::SUB:
        newValue = destination - data;
        destination.SetData(newValue);
        break;
    case OpIndex::CMP:
        newValue = destination - data;
        break;
    }

    // Check flags and format output
    switch (opIndex)
    {
    case OpIndex::MOV:
        return regName + HexString(prevDestData) + " -> " + HexString(newValue);
        break;
    case OpIndex::ADD:
    case OpIndex::SUB:
        flags[Flag::FLAG_ZERO] = newValue == 0;
        flags[Flag::FLAG_SIGNED] = destination.IsWide()
            ? (newValue) & 0x8000
            : (newValue) & 0x80;
        return regName + HexString(prevDestData) + " -> " + HexString(newValue) + "\t" + OutputChangeInFlags(prevFlags);
    case OpIndex::CMP:
        flags[Flag::FLAG_ZERO] = newValue == 0;
        flags[Flag::FLAG_SIGNED] = destination.IsWide()
            ? (newValue) & 0x8000
            : (newValue) & 0x80;
        return OutputChangeInFlags(prevFlags);
    }

    assert(false);
    return "";
}