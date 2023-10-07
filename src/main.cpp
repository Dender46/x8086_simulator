#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cassert>
#include <unordered_map>

#include "Defines.h"
#include "CpuMemory.h"
#include "CpuNames.h"
#include "Helpers.h"
#include "CpuOperations.h"
#include "DecoderOperands.h"
#include "CpuExecution.h"
#include "CycleEstimation.h"

u16 CombineLoAndHiToWord(const std::vector<u8>& bytesArr, int* byteIndex)
{
    const u16 byteLow  = bytesArr[++(*byteIndex)];
    const u16 byteHigh = bytesArr[++(*byteIndex)];
    return (byteHigh << 8) | byteLow;
}

int main(int argc, char* argv[])
{
    bool executeInstructions = false;
    bool dumpMemory = false;
    bool cyclesEstimate = false;
    while (argc--)
    {
        if (!strcmp(argv[argc], "--exec"))
        {
            executeInstructions = true;
        }
        if (!strcmp(argv[argc], "--dump"))
        {
            dumpMemory = true;
        }
        if (!strcmp(argv[argc], "--cyclesEstimate"))
        {
            cyclesEstimate = true;
        }
    }

    //std::ifstream file("listings/listing_0038_many_register_mov", std::ios::binary);
    //std::ifstream file("listings/listing_0039_more_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0040_challenge_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0041_add_sub_cmp_jnz", std::ios::binary);
    //std::ifstream file("listings/listing_0043_immediate_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0044_register_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0046_add_sub_cmp", std::ios::binary);
    //std::ifstream file("listings/listing_0048_ip_register", std::ios::binary);
    //std::ifstream file("listings/listing_0049_conditional_jumps", std::ios::binary);
    //std::ifstream file("listings/listing_0051_memory_mov", std::ios::binary);
    //std::ifstream file("listings/listing_0054_draw_rectangle", std::ios::binary);
    //std::ifstream file("listings/draw_rect_better", std::ios::binary);
    //std::ifstream file("listings/listing_0056_estimating_cycles", std::ios::binary);
    std::ifstream file("listings/listing_0057_challenge_cycles", std::ios::binary);
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

    std::unordered_map<int, Operation> operations;

    int byteIndex = 0, opBeginByte = 0;
    while (byteIndex < bytes.size())
    {
        opBeginByte = byteIndex;
        Operation operation{};
        Operand left{}, right{};

        u8 instructionByte = bytes[byteIndex];
        u8 bitD = (instructionByte & 2) >> 1;
        u8 bitW = (instructionByte & 1);

        if (operation.opIndex = IsReg_Mem_Reg(instructionByte); operation.opIndex != OpIndex::UNDEFINED)
        {
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
            }
            else
            {
                left.type = Operand::Type::Register;
                left.reg = registersMap[reg][bitW];
                right.type = Operand::Type::Memory;
                right.mem.pointsToWord = bitW == 1;
                right.mem.explicitWide = bitW == 1 ? MemoryExpr::ExplicitWide::Word : MemoryExpr::ExplicitWide::Byte;
                right.mem.SetRegistersOfExpression(rm, mod);

                if (mod == 0)
                {
                    if (rm == 0b0110)
                    {
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
            u8 rm      = (adjByte & 0b111);
            operation.opIndex = OpIndex((adjByte & 0b111'000) >> 3);

            if ((instructionByte & 0b1111'1110) == 0b1100'0110) // specific MOV instruction
            {
                operation.opIndex = OpIndex::MOV;
            }

            bool dataIsWord = operation.opIndex == OpIndex::MOV  ?  bitW == 1  :  bitS == 0 && bitW == 1;

            if (mod == 3) // register mode
            {
                left.type = Operand::Type::Register;
                left.reg = registersMap[rm][bitW];
            }
            else
            {
                left.type = Operand::Type::Memory;
                left.mem.SetRegistersOfExpression(rm, mod);
                left.mem.pointsToWord = dataIsWord;
                left.mem.explicitWide = bitW == 1 ? MemoryExpr::ExplicitWide::Word : MemoryExpr::ExplicitWide::Byte;

                if (mod == 0) // memory mode, no displacement follows
                {
                    // SPECIAL CASE
                    if (rm == 0b0110)
                    {
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
            operation.opIndex = OpIndex::MOV;
            u8 bitW = (instructionByte & 0b1000) >> 3;
            u8 reg  = (instructionByte & 0b0111);

            left.type = Operand::Type::Register;
            left.reg = registersMap[reg][bitW];
            right.type = Operand::Type::Immediate;
            right.immVal.value = bitW == 1 ? CombineLoAndHiToWord(bytes, &byteIndex) : bytes[++byteIndex];
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
        else if (operation.opIndex = IsImm_Accumulator(instructionByte); operation.opIndex != OpIndex::UNDEFINED)
        {
            left.type = Operand::Type::Register;
            left.reg = bitW == 1 ? RegisterIndex::ax : RegisterIndex::al;

            right.type = Operand::Type::Immediate;
            right.immVal.value = bitW == 1 ? CombineLoAndHiToWord(bytes, &byteIndex) : bytes[++byteIndex];
        }
        else if (IsJump(instructionByte) || IsLoop(instructionByte))
        {
            if (IsJump(instructionByte))
            {
                operation.type = Operation::Type::Jump;
                operation.opJumpIndex = OpJump(instructionByte & 0b1111);
            }
            else
            {
                operation.type = Operation::Type::Loop;
                operation.opLoopIndex = OpLoop(instructionByte & 0b0011);
            }

            left.type = Operand::Type::JumpDisplacement;
            left.jump.value = (s8)bytes[++byteIndex] + 2;
        }

        operation.operands[0] = std::move(left);
        operation.operands[1] = std::move(right);
        operation.size = byteIndex - opBeginByte;
        operations[opBeginByte] = operation;

        byteIndex++;
    }

    s16 ipReg = 0;
    u16 totalEstimatedCycles = 0;
    auto operationIt = operations.find(ipReg);
    while (operationIt != operations.cend())
    {
        const auto& op = operationIt->second;
        op.PrintOp();
        op.operands[0].Print();
        if (op.operands[1].type != Operand::Type::None)
        {
            std::cout << ", ";
            op.operands[1].Print();
        }

        if (executeInstructions)
        {
            switch (op.type)
            {
            case Operation::Type::Operation:
                std::cout << ExecuteOp(op.opIndex, op.operands) << " ip:" << HexString(ipReg) << " -> ";
                ipReg += op.size + 1;
                std::cout << HexString(ipReg);
                break;
            case Operation::Type::Jump:
            case Operation::Type::Loop:
                std::cout << " ip:" << HexString(ipReg) << " -> ";
                if (flags[Flag::FLAG_ZERO] == 0)
                {
                    ipReg += op.operands[0].jump.value; // disp is negative (future me: or is it?) (futurer me: this is handled by default right?)
                }
                else
                {
                    ipReg += op.size + 1;
                }
                std::cout << HexString(ipReg);
                break;
            default:
                break;
            }
        }

        if (cyclesEstimate)
        {
            int cyclesCount = CycleEstimation(op);
            totalEstimatedCycles += cyclesCount;
            std::cout << " | Clocks: +" << cyclesCount << " = " << totalEstimatedCycles;
        }

        operationIt = operations.find(ipReg);
        std::cout << '\n';
    }

    if (executeInstructions)
    {
        const auto printRegisterValue = [&](RegisterIndex regIndex) {
            auto regValue = *GetRegisterMem(regIndex);
            if (regValue == 0)
            {
                return;
            }
            auto regName = registerNames[regIndex]; // TODO: only considering full 16 bit registers
            std::cout << "\n\t" << regName
                << ": " << HexString(regValue) 
                << " (" << regValue << ")";
        };

        // Order of registers in memory is mixed up just like in 8086
        // so we manualy reorder values for better order
        std::cout << "\nFinal registers:";
        for (auto i : {RegisterIndex::ax, RegisterIndex::bx, RegisterIndex::cx, RegisterIndex::dx, RegisterIndex::sp, RegisterIndex::bp, RegisterIndex::si, RegisterIndex::di, })
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

    if (dumpMemory)
    {
        std::ofstream memoryDumpFile{ "memoryDump.data", std::ios::binary };
        std::copy(
            std::begin(mainMemory),
            std::end(mainMemory),
            std::ostream_iterator<u8>(memoryDumpFile)
        );
    }

    return 0;
}