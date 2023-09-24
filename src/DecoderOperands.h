#pragma once
#include <iostream>
#include <string>

#include "Defines.h"
#include "CpuNames.h"
#include "CpuMemory.h"

struct ImmediateValue
{
    s16 value;
};

struct JumpDisplacement
{
    s16 value;
};

struct MemoryExpr
{
    enum class ExplicitWide { None, Byte, Word };

    RegisterIndex registers[2]{}; // second register might not be present
    s16 disp = 0; // might be optional, or required for direct access mode
    ExplicitWide explicitWide = ExplicitWide::None; // used for immediate operation

    const char* GetExplicitWide() const
    {
        switch (explicitWide)
        {
        case MemoryExpr::ExplicitWide::None:  return "";      break;
        case MemoryExpr::ExplicitWide::Byte:  return "byte "; break;
        case MemoryExpr::ExplicitWide::Word:  return "word "; break;
        default:
            return "";
            break;
        }
    }

    void SetRegistersOfExpression(u8 rm, u8 mod)
    {
        switch (rm)
        {
        case 0: registers[0] = RegisterIndex::bx; registers[1] = RegisterIndex::si; break;
        case 1: registers[0] = RegisterIndex::bx; registers[1] = RegisterIndex::di; break;
        case 2: registers[0] = RegisterIndex::bp; registers[1] = RegisterIndex::si; break;
        case 3: registers[0] = RegisterIndex::bp; registers[1] = RegisterIndex::di; break;
        case 4: registers[0] = RegisterIndex::si; registers[1] = RegisterIndex::None; break;
        case 5: registers[0] = RegisterIndex::di; registers[1] = RegisterIndex::None; break;
        case 7: registers[0] = RegisterIndex::bx; registers[1] = RegisterIndex::None; break;
        case 6: 
            if (mod == 0) { // Direct access
                registers[0] = RegisterIndex::None; registers[1] = RegisterIndex::None;
            } else {
                registers[0] = RegisterIndex::bp; registers[1] = RegisterIndex::None;
            }
            break;
        }
    }
};

struct Operand
{
    enum class Type
    {
        None,
        Register,
        Immediate,
        Memory,
        DirectAddress,
        JumpDisplacement,
    };

    Type type = Type::None;
    union {
        RegisterIndex reg;
        MemoryExpr mem;
        ImmediateValue immVal;
        JumpDisplacement jump;
    };

    void Print() const
    {
        switch (type)
        {
        case Type::None:
            break;
        case Type::Register:
            std::cout << registerNames[reg];
            break;
        case Type::Immediate:
            std::cout << std::to_string(immVal.value);
            break;
        case Type::Memory:
            std::cout << mem.GetExplicitWide();
            std::cout << '[';
            if (mem.registers[0] != RegisterIndex::None) std::cout << registerNames[mem.registers[0]];
            if (mem.registers[1] != RegisterIndex::None) std::cout << " + " << registerNames[mem.registers[1]];
            if (mem.disp != 0)                           std::cout << " + " << std::to_string(mem.disp);
            std::cout << ']';
            break;
        case Type::DirectAddress:
            std::cout << mem.GetExplicitWide();
            std::cout << '[';
            std::cout << std::to_string(mem.disp);
            std::cout << ']';
            break;
        case Type::JumpDisplacement:
            std::cout << '$';
            if (jump.value > 0)
                std::cout << '+' << std::to_string(jump.value);
            else if (jump.value < 0)
                std::cout << std::to_string(jump.value);
            std::cout << "+0";
            break;
        default:
            break;
        }
    }
};