#include <iostream>
#include <fstream>
#include <filesystem>
#include <bitset>
#include <string>

#include "Defines.h"
#include "CpuMemory.h"
#include "CpuNames.h"
#include "Helpers.h"
#include "OperationTypes.h"
#include "DecoderOperands.h"

u16 CombineLoAndHiToWord(const std::vector<u8>& bytesArr, int* byteIndex)
{
    const u16 byteLow  = bytesArr[++(*byteIndex)];
    const u16 byteHigh = bytesArr[++(*byteIndex)];
    return (byteHigh << 8) | byteLow;
}

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

std::string ExecuteOp(OpIndex opIndex, u8 outputRegMemIndex, u16 data)
{
    auto prevData = registersMem[outputRegMemIndex];
    bool prevFlags[Flag::FLAG_COUNT] = {};
    for (int i : flags)
        prevFlags[i] = flags[i];

    // Execute operation
    switch (opIndex)
    {
    case OpIndex::MOV:
        registersMem[outputRegMemIndex] = data;
        break;
    case OpIndex::ADD:
        registersMem[outputRegMemIndex] += data;
        break;
    case OpIndex::SUB:
        registersMem[outputRegMemIndex] -= data;
        break;
    case OpIndex::CMP:
        break;
    }

    // Check flags and format output
    switch (opIndex)
    {
    case MOV:
        return HexString(prevData) + " -> " + HexString(registersMem[outputRegMemIndex]);
        break;
    case ADD:
    case SUB:
        flags[Flag::FLAG_ZERO] = registersMem[outputRegMemIndex] == 0;
        flags[Flag::FLAG_SIGNED] = (registersMem[outputRegMemIndex] & 0x8000) >> 15 == 1;
        return HexString(prevData) + " -> " + HexString(registersMem[outputRegMemIndex]) + "\t" + OutputChangeInFlags(prevFlags);
    case CMP:
        flags[Flag::FLAG_SIGNED] = (registersMem[outputRegMemIndex] & 0x8000) >> 15 == 1;
        return OutputChangeInFlags(prevFlags);
    }

    return "";
}

u8 RegisterMemoryIndex(u8 reg, u8 bitW)
{
    if (reg < 4 && bitW == 1) // whole register
        return reg;
    if (reg < 4 && bitW == 0) // TODO: lower bits of register
        return reg;
    if (reg >= 4 && bitW == 0) // TODO: higher bits of register
        return reg - 4;
    if (reg >= 4 && bitW == 1) // other registers
        return reg;
    
    assert(false);
    return u8_max;
}

int main(int argc, char* argv[])
{
    bool executeInstructions = false;
    if (argc == 2 && strcmp(argv[1], "-exec") == 0)
    {
        executeInstructions = false;
    }

    //std::ifstream file("listings/listing_0038_many_register_mov", std::ios::binary);
    //std::ifstream file("listings/listing_0039_more_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0040_challenge_movs", std::ios::binary);
    std::ifstream file("listings/listing_0041_add_sub_cmp_jnz", std::ios::binary);
    //std::ifstream file("listings/listing_0043_immediate_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0044_register_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0046_add_sub_cmp", std::ios::binary);
    //std::ifstream file("listings/listing_0048_ip_register", std::ios::binary);
    //std::ifstream file("listings/listing_0049_conditional_jumps", std::ios::binary);
    //std::ifstream file("listings/listing_0051_memory_mov", std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "!!! Can't open file !!!\n";
        return 0;
    }
    std::vector<u8> bytes(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );

    std::cout << "bits 16\n";

    struct Operation
    {
        std::string instructionStr;
        Operand operands[2];
    };

    std::vector<Operation> operations;

    int prevByteIndex = 0;
    int byteIndex = 0;
    while (byteIndex < bytes.size())
    {
        Operand left{}, right{};
        std::string instructionStr;

        u8 instructionByte = bytes[byteIndex];
        u8 bitD = (instructionByte & 2) >> 1;
        u8 bitW = (instructionByte & 1);

        if (auto opIndex = IsReg_Mem_Reg(instructionByte); opIndex != OpIndex::UNDEFINED)
        {
            instructionStr = operationNames[opIndex];

            u8 adjByte = bytes[++byteIndex];

            u8 mod = (adjByte >> 6);
            u8 reg = (adjByte & 0b00'111'000) >> 3;
            u8 rm =  (adjByte & 0b00'000'111);
            if (mod == 3) // register mode
            {
                left.type = Operand::Type::Register;
                left.reg = registersMap[rm][bitW];
                right.type = Operand::Type::Register;
                right.reg = registersMap[reg][bitW];

                if (executeInstructions)
                {
                    //rightOperand += "\t; " + leftOperand + ": " +
                    //    ExecuteOp(opIndex, RegisterMemoryIndex(rm, bitW), registersMem[RegisterMemoryIndex(reg, bitW)]);
                }
            }
            else
            {
                left.type = Operand::Type::Register;
                left.reg = registersMap[reg][bitW];
                right.type = Operand::Type::Memory;
                right.mem.SetRegistersOfExpression(rm, mod);

                if (mod == 0)
                {
                    if (rm == 0b0110)
                    {
                        right.type = Operand::Type::DirectAddress;
                        right.mem.disp = bitW == 1 ? CombineLoAndHiToWord(bytes, &byteIndex) : bytes[++byteIndex];
                    }
                }
                else if (mod == 1) // memory mode, 8-bit displacement follows
                {
                    right.mem.disp = bytes[++byteIndex];
                }
                else if (mod == 2) // memory mode, 16-bit displacement follows
                {
                    right.mem.disp = CombineLoAndHiToWord(bytes, &byteIndex);
                }

                if (bitD == 0)
                {
                    std::swap(left, right);
                }
            }
        }
        else if (IsImm_Reg_Mem(instructionByte))
        {
            u8 bitS = bitD;

            u8 adjByte = bytes[++byteIndex];
            u8 mod     = (adjByte >> 6);
            u8 opIndex = (adjByte & 0b111'000) >> 3;
            u8 rm      = (adjByte & 0b111);

            if ((instructionByte & 0b1111'1110) == 0b1100'0110) // specific MOV instruction
            {
                opIndex = OpIndex::MOV;
                instructionStr = operationNames[opIndex];
            }
            else
            {
                instructionStr = operationNames[opIndex];
            }

            bool dataIsWord = opIndex == OpIndex::MOV  ?  bitW == 1  :  bitS == 0 && bitW == 1;

            if (mod == 3) // register mode
            {
                left.type = Operand::Type::Register;
                left.reg = registersMap[rm][bitW];

                if (executeInstructions)
                {
                    //rightOperand += "\t; " + leftOperand + ": " +
                    //    ExecuteOp((OpIndex)opIndex, RegisterMemoryIndex(rm, bitW), right.immVal.value);
                }
            }
            else
            {
                left.type = Operand::Type::Memory;
                left.mem.SetRegistersOfExpression(rm, mod);
                left.mem.explicitWide = bitW == 1 ? MemoryExpr::ExplicitWide::Word : MemoryExpr::ExplicitWide::Byte;

                if (mod == 0) // memory mode, no displacement follows
                {
                    // SPECIAL CASE
                    if (rm == 0b0110)
                    {
                        left.type = Operand::Type::DirectAddress;
                        left.mem.disp = CombineLoAndHiToWord(bytes, &byteIndex);
                    }
                }
                else if (mod == 1) // memory mode, 8-bit displacement follows
                {
                    left.mem.disp = bytes[++byteIndex];
                }
                else if (mod == 2) // memory mode, 16-bit displacement follows
                {
                    left.mem.disp = CombineLoAndHiToWord(bytes, &byteIndex);
                }
            }

            right.type = Operand::Type::Immediate;
            right.immVal.value = dataIsWord ? CombineLoAndHiToWord(bytes, &byteIndex) : bytes[++byteIndex];
        }
        else if ((instructionByte & 0b1111'0000) == 0b1011'0000) // MOV immediate to register
        {
            instructionStr = operationNames[OpIndex::MOV];
            u8 bitW = (instructionByte & 0b1000) >> 3;
            u8 reg  = (instructionByte & 0b0111);

            left.type = Operand::Type::Register;
            left.reg = registersMap[reg][bitW];
            right.type = Operand::Type::Immediate;
            right.immVal.value = bitW == 1 ? CombineLoAndHiToWord(bytes, &byteIndex) : bytes[++byteIndex];

            //if (executeInstructions)
            //{
            //    rightOperand += "\t; " + leftOperand + ": " +
            //        ExecuteOp(OpIndex::MOV, RegisterMemoryIndex(reg, bitW), data);
            //}
        }
        else if ((instructionByte & 0b1111'1100) == 0b1010'0000) // MOV accumulator
        {
            assert(false);
            //instructionStr = operationNames[OpIndex::MOV];
            //const auto data = CombineLoAndHiToString(bytes, &byteIndex);
            //if (bitD == 0)
            //{
            //    leftOperand = "ax";
            //    rightOperand = '[' + data + ']';
            //}
            //else
            //{
            //    leftOperand = '[' + data + ']';
            //    rightOperand = "ax";
            //}
        }
        else if (auto opIndex = IsImm_Accumulator(instructionByte); opIndex != OpIndex::UNDEFINED)
        {
            instructionStr = operationNames[opIndex];

            left.type = Operand::Type::Register;
            left.reg = bitW == 1 ? RegisterIndex::ax : RegisterIndex::al;

            right.type = Operand::Type::Immediate;
            right.immVal.value = bitW == 1 ? CombineLoAndHiToWord(bytes, &byteIndex) : bytes[++byteIndex];
        }
        else if (IsJump(instructionByte) || IsLoop(instructionByte))
        {
            if (IsJump(instructionByte))
            {
                const u8 opIndex = instructionByte & 0b1111;
                instructionStr = jumpNames[opIndex];
            }
            else
            {
                const u8 opIndex = instructionByte & 0b0011;
                instructionStr = loopNames[opIndex];
            }

            left.type = Operand::Type::JumpDisplacement;
            left.jump.value = (s8)bytes[++byteIndex] + 2;
        }

        operations.push_back({instructionStr, {left, right}});

        prevByteIndex = byteIndex++;
        if (executeInstructions)
        {
            //std::cout << " ip: " << HexString(prevByteIndex) << " -> " << HexString(byteIndex) << "\n";
        }
    }

    for (const Operation& op : operations)
    {
        std::cout << op.instructionStr << " ";
        op.operands[0].Print();
        if (op.operands[1].type != Operand::Type::None)
        {
            std::cout << ", ";
            op.operands[1].Print();
        }
        std::cout << '\n';
    }

    if (executeInstructions)
    {
        const auto printRegisterValue = [&](u8 regIndex) {
            //if (registersMem[regIndex] == 0)
            //{
            //    return;
            //}
            //auto regName = registerNames[regIndex][1]; // TODO: only considering full 16 bit registers
            //std::cout << "\n\t" << regName
            //    << ": " << HexString(registersMem[regIndex]) 
            //    << " (" << registersMem[regIndex] << ")";
        };

        // Order of registers in memory is mixed up just like in 8086
        // so we manualy reorder values for better order
        std::cout << "\nFinal registers:";
        for (auto i : {0, 3, 1, 2, 4, 5, 6, 7})
        {
            printRegisterValue(i);
        }
        // ip register can be in the registersMem, but it it's considered as a separate "hidden" regitser
        // so we print it out separately
        std::cout << "\n\tip: " << HexString(byteIndex) << " (" << byteIndex << ")";

        std::cout << "\nFinal flags:\n\t";
        for (int i = 0; i < Flag::FLAG_COUNT; i++)
        {
            if (flags[i])
                std::cout << FlagStr((Flag)i);
        }
    }

    return 0;
}