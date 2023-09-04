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

u16 CombineLoAndHiToWord(const std::vector<u8>& bytesArr, int* byteIndex)
{
    const u16 byteLow  = bytesArr[++(*byteIndex)];
    const u16 byteHigh = bytesArr[++(*byteIndex)];
    return (byteHigh << 8) | byteLow;
}

std::string CombineLoAndHiToString(const std::vector<u8>& bytesArr, int* byteIndex)
{
    return std::to_string(CombineLoAndHiToWord(bytesArr, byteIndex));
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
        executeInstructions = true;
    }

    //std::ifstream file("listings/listing_0038_many_register_mov", std::ios::binary);
    //std::ifstream file("listings/listing_0039_more_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0040_challenge_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0041_add_sub_cmp_jnz", std::ios::binary);
    //std::ifstream file("listings/listing_0043_immediate_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0044_register_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0046_add_sub_cmp", std::ios::binary);
    std::ifstream file("listings/listing_0048_ip_register", std::ios::binary);
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

    int byteIndex = 0;
    while (byteIndex < bytes.size())
    {
        std::string leftOperand, rightOperand;

        u8 byte = bytes[byteIndex];
        u8 bitD = (byte & 2) >> 1;
        u8 bitW = (byte & 1);

        if (auto opIndex = IsReg_Mem_Reg(byte); opIndex != OpIndex::UNDEFINED)
        {
            std::cout << operations[opIndex];

            byte = bytes[++byteIndex];

            u8 mod = (byte >> 6);
            u8 reg = (byte & 0b00'111'000) >> 3;
            u8 rm =  (byte & 0b00'000'111);
            if (mod == 3) // register mode
            {
                leftOperand  = registers[rm][bitW];
                rightOperand = registers[reg][bitW];

                if (executeInstructions)
                {
                    rightOperand += "\t; " + leftOperand + ": " +
                        ExecuteOp(opIndex, RegisterMemoryIndex(rm, bitW), registersMem[RegisterMemoryIndex(reg, bitW)]);
                }
            }
            else
            {
                leftOperand  = registers[reg][bitW];
                rightOperand = effectiveAddresses[rm];

                if (mod == 0) // memory mode, no displacement follows
                {
                    // BUT if a special occassion for a DIRECT ADDRESS case
                    if (rm == 0b0110)
                    {
                        u16 disp = bitW == 0 ? bytes[++byteIndex] : CombineLoAndHiToWord(bytes, &byteIndex);
                        rightOperand = '[' + disp;
                    }
                }
                else if (mod == 1) // memory mode, 8-bit displacement follows
                {
                    s8 disp = bytes[++byteIndex];
                    if (bitW == 1)
                        rightOperand += GetSign(disp) + std::to_string(abs(disp));
                    else
                        rightOperand += " + " + std::to_string(disp);
                }
                else if (mod == 2) // memory mode, 16-bit displacement follows
                {
                    u16 disp = CombineLoAndHiToWord(bytes, &byteIndex);
                    if (bitW == 1)
                        rightOperand += GetSign(disp) + std::to_string(disp);
                    else
                        rightOperand += " + " + std::to_string(disp);
                }

                rightOperand += ']';
                if (bitD == 0)
                {
                    std::swap(leftOperand, rightOperand);
                }
            }
        }
        else if (IsImm_Reg_Mem(byte))
        {
            u8 bitS = bitD;

            byte = bytes[++byteIndex];
            u8 mod     = (byte >> 6);
            u8 opIndex = (byte & 0b00'111'000) >> 3;
            u8 rm      = (byte & 0b00'000'111);

            std::cout << operations[opIndex];

            if (mod == 3) // register mode
            {
                u16 data = bitS == 0 && bitW == 1 ? CombineLoAndHiToWord(bytes, &byteIndex) : bytes[++byteIndex];
                leftOperand = registers[rm][bitW];
                rightOperand = std::to_string(data);

                if (executeInstructions)
                {
                    rightOperand += "\t; " + leftOperand + ": " +
                        ExecuteOp((OpIndex)opIndex, RegisterMemoryIndex(rm, bitW), data);
                }
            }
            else if (mod == 0) // memory mode, no displacement follows
            {
                if (opIndex == OpIndex::MOV)
                {
                    leftOperand = effectiveAddresses[rm] + ']';
                    if (bitW == 0)
                        rightOperand = "byte " + bytes[++byteIndex];
                    else
                        rightOperand = "word " + CombineLoAndHiToString(bytes, &byteIndex);
                }
                else
                {
                    if (bitW == 1)
                    {
                         // SPECIAL CASE
                        if (rm == 0b0110)
                            leftOperand = "word [" + CombineLoAndHiToString(bytes, &byteIndex) + ']';
                        else
                            leftOperand = "word " + std::string(effectiveAddresses[rm]) + ']';

                        if (bitS == 0)
                            rightOperand = CombineLoAndHiToString(bytes, &byteIndex);
                        else
                            rightOperand = std::to_string(bytes[++byteIndex]);
                    }
                    else
                    {
                        leftOperand = "byte " + std::string(effectiveAddresses[rm]) + ']';
                        rightOperand = std::to_string(bytes[++byteIndex]);
                    }
                }
            }
            else if (mod == 1) // memory mode, 8-bit displacement follows
            {
                leftOperand = effectiveAddresses[rm];
                u16 disp = bytes[++byteIndex];
                if (bitW == 1)
                    leftOperand += GetSign(disp) + std::to_string(abs((s16)disp)) + ']';
                else
                    leftOperand += " + " + disp + ']';
            }
            else if (mod == 2) // memory mode, 16-bit displacement follows
            {
                const auto disp = CombineLoAndHiToString(bytes, &byteIndex);
                //if (bitW == 1)
                //    std::cout << effectiveAddresses[rm] << " - " << std::numeric_limits<u16>::max() - disp + 1 << ']';
                //else
                if (opIndex == OpIndex::MOV)
                {
                    leftOperand = std::string(effectiveAddresses[rm]) + " + " + disp + ']';
                    if (bitW == 0)
                        rightOperand = "byte " + std::to_string(bytes[++byteIndex]);
                    else
                        rightOperand = "word " + CombineLoAndHiToString(bytes, &byteIndex);
                }
                else
                {
                    if (bitW == 1)
                    {
                        leftOperand = "word " + std::string(effectiveAddresses[rm]) + " + " + disp + ']';
                        if (bitS == 0)
                            rightOperand = CombineLoAndHiToString(bytes, &byteIndex);
                        else
                            rightOperand = std::to_string(bytes[++byteIndex]);
                    }
                    else
                    {
                        leftOperand = "byte " + std::string(effectiveAddresses[rm]) + " + " + disp + ']';
                        rightOperand = std::to_string(bytes[++byteIndex]);
                    }
                }
            }
        }
        else if ((byte & 0b1111'0000) == 0b1011'0000) // MOV immediate to register
        {
            std::cout << "mov ";
            u8 bitW = (byte & 0b0000'1000) >> 3;
            u8 reg  = (byte & 0b0000'0111);

            u16 data = bitW == 0 ? bytes[++byteIndex] : CombineLoAndHiToWord(bytes, &byteIndex);
            leftOperand  = registers[reg][bitW];
            rightOperand = std::to_string(data);

            if (executeInstructions)
            {
                rightOperand += "\t; " + leftOperand + ": " +
                    ExecuteOp(OpIndex::MOV, RegisterMemoryIndex(reg, bitW), data);
            }
        }
        else if ((byte & 0b1111'1100) == 0b1010'0000) // MOV accumulator
        {
            std::cout << "mov ";

            const auto data = CombineLoAndHiToString(bytes, &byteIndex);
            if (bitD == 0)
            {
                leftOperand = "ax";
                rightOperand = '[' + data + ']';
            }
            else
            {
                leftOperand = '[' + data + ']';
                rightOperand = "ax";
            }
        }
        else if (auto opIndex = IsImm_Accumulator(byte); opIndex != OpIndex::UNDEFINED)
        {
            u8 mask = 0b0011'1100;
            std::cout << operations[opIndex];
            if (bitW == 0)
            {
                leftOperand = "al";
                rightOperand = std::to_string(bytes[++byteIndex]);
            }
            else
            {
                leftOperand = "ax";
                rightOperand = CombineLoAndHiToString(bytes, &byteIndex);
            }
        }
        else if (IsJump(byte) || IsLoop(byte))
        {
            if (IsJump(byte))
            {
                u8 opIndex = byte & 0b0000'1111;
                std::cout << jumps[opIndex];
            }
            else
            {
                u8 opIndex = byte & 0b0000'0011;
                std::cout << loops[opIndex];
            }

            auto disp = (s8)bytes[++byteIndex] + 2;
            std::string dispStr = std::to_string(disp);
            std::cout << '$';
            if (disp > 0)
                std::cout << '+' << dispStr;
            else if (disp < 0)
                std::cout << dispStr;
            std::cout << "+0";
        }

        if (leftOperand != "" && rightOperand != "")
            std::cout << leftOperand << ", " << rightOperand;
        else
            std::cout << leftOperand;
        std::cout << '\n';
        byteIndex++;
    }

    if (executeInstructions)
    {
        const auto printRegisterValue = [&](u8 regIndex) {
            if (registersMem[regIndex] == 0)
            {
                return;
            }
            auto regName = registers[regIndex][1]; // TODO: only considering full 16 bit registers
            std::cout << "\n\t" << regName
                << ": " << HexString(registersMem[regIndex]) 
                << " (" << registersMem[regIndex] << ")";
        };

        // Order of registers in memory is mixed up just like in 8086
        // so we manualy reorder values for better order
        std::cout << "\nFinal registers:";
        for (auto i : {0, 3, 1, 2, 4, 5, 6, 7})
        {
            printRegisterValue(i);
        }

        std::cout << "\nFinal flags:\n\t";
        for (int i = 0; i < Flag::FLAG_COUNT; i++)
        {
            if (flags[i])
                std::cout << FlagStr((Flag)i);
        }
    }

    return 0;
}